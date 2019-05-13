
/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "getvideothread.h"

#include <QTimer>
#include <QDateTime>
#include <QDebug>

void Yuv420Cut(int x,int y,int desW,int desH,int srcW,int srcH,uint8_t *srcBuffer,uint8_t *desBuffer)
{
    int tmpRange;
    int bufferIndex;

    int yIndex = 0;
    bufferIndex = 0 + x + y*srcW;
    tmpRange = srcW * desH;
    for (int i=0;i<tmpRange;) //逐行拷贝Y分量数据
    {
        memcpy(desBuffer+yIndex,srcBuffer+bufferIndex+i,desW);
        i += srcW;
        yIndex += desW;
    }

    int uIndex = desW * desH;
    int uIndexStep = srcW/2;
    int uWidthCopy = desW/2;
    bufferIndex = srcW * srcH+x/2 + y /2 *srcW / 2;
    tmpRange = srcW * desH / 4;
    for (int i=0;i<tmpRange;) //逐行拷贝U分量数据
    {
        memcpy(desBuffer+uIndex,srcBuffer+bufferIndex+i,uWidthCopy);
        i += uIndexStep;
        uIndex += uWidthCopy;
    }


    int vIndex = desW * desH +  desW * desH /4;
    int vIndexStep = srcW/2;
    int vWidthCopy = desW/2;
    bufferIndex = srcW*srcH + srcW*srcH/4 + x/2 + y /2 *srcW / 2;
    tmpRange = srcW * desH / 4;
    for (int i=0;i<tmpRange;) //逐行拷贝V分量数据
    {
        memcpy(desBuffer+vIndex,srcBuffer+bufferIndex+i,vWidthCopy);
        i += vIndexStep;
        vIndex += vWidthCopy;
    }
}

GetVideoThread::GetVideoThread()
{
    m_isRun = false;

    pFormatCtx = NULL;
    out_buffer = NULL;
    aFrame = NULL;
    pFrame = NULL;
    pFrameYUV = NULL;
    pCodecCtx = NULL;
    aCodecCtx = NULL;

    m_pause = false;

    m_saveVideoFileThread = NULL;

    audio_buf_size = 0;

    connect(this,SIGNAL(withChanged(int,int)),this,SLOT(slotWithChanged(int,int)),Qt::BlockingQueuedConnection);
    connect(this,SIGNAL(loading(bool)),this,SLOT(slotLoading(bool)),Qt::BlockingQueuedConnection);

}


GetVideoThread::~GetVideoThread()
{

}

ErroCode GetVideoThread::init(QString videoDevName, bool useVideo, QString audioDevName, bool useAudio)
{

    AVCodec			*pCodec = NULL;
    AVCodec			*aCodec = NULL;

    AVFormatContext *pFormatCtx2 = avformat_alloc_context();
    AVDictionary* options = NULL;
    av_dict_set(&options,"list_devices","true",0);
    AVInputFormat *iformat = av_find_input_format("dshow");
    fprintf(stderr,"Device Info=============\n");
    avformat_open_input(&pFormatCtx2,"video=dummy",iformat,&options);
    fprintf(stderr,"========================\n");

    pFormatCtx = avformat_alloc_context();

    AVInputFormat *ifmt = av_find_input_format("dshow");

    if (useAudio)
    {
        QString audioDevOption = QString("audio=%1").arg(audioDevName);

        if(avformat_open_input(&pFormatCtx,audioDevOption.toUtf8(),ifmt,NULL)!=0){
            fprintf(stderr,"Couldn't open input stream audio.（无法打开输入流）\n");
            return AudioOpenFailed;
        }

    }


    if (useVideo)
    {
        QString videoDevOption = QString("video=%1").arg(videoDevName);
        if(avformat_open_input(&pFormatCtx,videoDevOption.toUtf8(),ifmt,NULL)!=0){
            qDebug()<<"Couldn't open input stream video.（无法打开输入流）\n";
            return VideoOpenFailed;
        }
    }

    videoindex=-1;
    pCodecCtx = NULL;
    if (useVideo)
    {
        for(i=0; i<pFormatCtx->nb_streams; i++)
            if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
            {
                videoindex=i;
                break;
            }
        if(videoindex==-1)
        {
            printf("Didn't find a video stream.（没有找到视频流）\n");
            return VideoOpenFailed;
        }

        pCodecCtx = pFormatCtx->streams[videoindex]->codec;
        pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
        if(pCodec == NULL)
        {
            printf("video Codec not found.\n");
            return VideoDecoderOpenFailed;
        }

        if(avcodec_open2(pCodecCtx, pCodec,NULL)<0)
        {
            printf("Could not open video codec.\n");
            return VideoDecoderOpenFailed;
        }

        pFrame    = av_frame_alloc();
        pFrameYUV = av_frame_alloc();
        out_buffer=(uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
        avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);

        setPicRange(0,0,pCodecCtx->width, pCodecCtx->height);
    }

    audioindex = -1;
    aCodecCtx = NULL;
    if (useAudio)
    {

        for(i=0; i<pFormatCtx->nb_streams; i++)
            if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO)
            {
                audioindex=i;
                break;
            }
        if(audioindex==-1)
        {
            printf("Didn't find a video stream.（没有找到视频流）\n");
            return AudioOpenFailed;
        }

        aCodecCtx = pFormatCtx->streams[audioindex]->codec;

        aCodec = avcodec_find_decoder(aCodecCtx->codec_id);
        if(aCodec == NULL)
        {
            printf("audio Codec not found.\n");
            return AudioDecoderOpenFailed;
        }

        if(avcodec_open2(aCodecCtx, aCodec,NULL)<0)
        {
            printf("Could not open video codec.\n");
            return AudioDecoderOpenFailed;
        }

        qDebug()<<"audio info:";
        qDebug()<<"audio info:"<<aCodecCtx->bit_rate<<aCodecCtx->sample_fmt<<aCodecCtx->bit_rate<<aCodecCtx->sample_rate<<aCodecCtx->channels;

        aFrame = av_frame_alloc();

    }

    return SUCCEED;
}

void GetVideoThread::deInit()
{
    if (out_buffer)
    {
        av_free(out_buffer);
        out_buffer = NULL;
    }

    if (aFrame)
    {
        av_free(aFrame);
        aFrame = NULL;
    }

    if (pFrame)
    {
        av_free(pFrame);
        pFrame = NULL;
    }

    if (pFrameYUV)
    {
        av_free(pFrameYUV);
        pFrameYUV = NULL;
    }

    if (pCodecCtx)
        avcodec_close(pCodecCtx);
    if (aCodecCtx)
        avcodec_close(aCodecCtx);

//    avformat_flush(pFormatCtx);

    avformat_close_input(&pFormatCtx);

    avformat_free_context(pFormatCtx);

}

void GetVideoThread::startRecord()
{
    m_isRun = true;
    start();
}

void GetVideoThread::pauseRecord()
{
    m_pause = true;
}

void GetVideoThread::restoreRecord()
{
    m_pause = false;
}

void GetVideoThread::stopRecord()
{
    m_isRun = false;
}

void GetVideoThread::setPicRange(int x,int y,int w,int h)
{
    pic_x = x;
    pic_y = y;
    pic_w = w;
    pic_h = h;
}

void GetVideoThread::setSaveVideoFileThread(SaveVideoFileThread * p)
{
    m_saveVideoFileThread = p;
}

void GetVideoThread::run()
{

    struct SwsContext *img_convert_ctx = NULL;

    int y_size = 0;
    int yuvSize = 0;
    int size = 0;

    audio_buf_size = 0;

    if (pCodecCtx)
    {

        //-------------------------------------------------------------//

        int numBytes = avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width,pCodecCtx->height);
        out_buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
        avpicture_fill((AVPicture *) pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P,pCodecCtx->width, pCodecCtx->height);

        img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
        //------------------------------

        y_size = pCodecCtx->width * pCodecCtx->height;
        yuvSize = pic_w * pic_h;
        size = avpicture_get_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);

    }

    int ret, got_frame;

    AVPacket *packet=(AVPacket *)av_malloc(sizeof(AVPacket));
//    //Output Information-----------------------------
//    printf("File Information（文件信息）---------------------\n");
//    av_dump_format(pFormatCtx,0,NULL,0);
//    printf("-------------------------------------------------\n");

    qint64 firstTime = QDateTime::currentMSecsSinceEpoch();

    bool m_getFirst = false;
    qint64 timeIndex = 0;

    while(m_isRun )
    {

        if (av_read_frame(pFormatCtx, packet)<0)
        {
            qDebug()<<"read failed!";
            msleep(10);
            continue;
        }

        if (m_pause)
        {
            av_packet_unref(packet);
            msleep(10);
            continue;
        }

        if(packet->stream_index==videoindex)
        {
            long time = 0;
            if (m_saveVideoFileThread)
            {
                if (m_getFirst)
                {
                    qint64 secondTime = QDateTime::currentMSecsSinceEpoch();

                    time = secondTime - firstTime + timeIndex;
                }
                else
                {
                    firstTime = QDateTime::currentMSecsSinceEpoch();
                    timeIndex = m_saveVideoFileThread->getVideoPts()*1000;
                    m_getFirst = true;
                }

            }

            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_frame, packet);

            if(ret < 0)
            {
                printf("video Decode Error.（解码错误）\n");
                return;
            }

            if(got_frame && pCodecCtx)
            {
                sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);

                if (m_saveVideoFileThread)
                {
                    uint8_t * picture_buf = (uint8_t *)av_malloc(size);
                    memcpy(picture_buf,pFrameYUV->data[0],y_size);
                    memcpy(picture_buf+y_size,pFrameYUV->data[1],y_size/4);
                    memcpy(picture_buf+y_size+y_size/4,pFrameYUV->data[2],y_size/4);
                    uint8_t * yuv_buf = (uint8_t *)malloc(size);

                    ///将YUV图像裁剪成目标大小
                    Yuv420Cut(pic_x,pic_y,pic_w,pic_h,pCodecCtx->width,pCodecCtx->height,picture_buf,yuv_buf);
                    m_saveVideoFileThread->videoDataQuene_Input(yuv_buf,yuvSize*3/2,time);

                    av_free(picture_buf);
                }
            }

        }
        else if(packet->stream_index == audioindex)
        {
            ret = avcodec_decode_audio4(aCodecCtx, aFrame, &got_frame, packet);
            if(ret < 0)
            {
                fprintf(stderr,"video Audio Error.\n");
                return;
            }

            if (got_frame)
            {
                if (m_saveVideoFileThread)
                {
                    int size = av_samples_get_buffer_size(NULL,aCodecCtx->channels, aFrame->nb_samples,aCodecCtx->sample_fmt, 1);

                    memcpy(audio_buf + audio_buf_size, aFrame->data[0], size);
                    audio_buf_size += size;

                    int index = 0;
                    int ONEAudioSize = m_saveVideoFileThread->audio_input_frame_size;//4096

                    int totalSize = audio_buf_size;
                    int leftSize  = audio_buf_size;

                    while(1)
                    {
                        if (leftSize >= ONEAudioSize)
                        {
                            uint8_t * buffer = (uint8_t *)malloc(ONEAudioSize);
                            memcpy(buffer, audio_buf+index, ONEAudioSize);
                            m_saveVideoFileThread->audioDataQuene_Input((uint8_t*)buffer, ONEAudioSize);

                            index    += ONEAudioSize;
                            leftSize -= ONEAudioSize;
                        }
                        else
                        {
                            if (leftSize > 0)
                            {
                                memcpy(audio_buf, audio_buf+index, leftSize);
                            }
                            audio_buf_size = leftSize;
                            break;
                        }
                    }
                }
            }
        }

        av_packet_unref(packet);
    }

    sws_freeContext(img_convert_ctx);

    qDebug()<<"record stopping...";

    m_pause = false;

    deInit();

    qDebug()<<"record finished!";

}
