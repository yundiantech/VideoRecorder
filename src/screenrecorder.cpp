
/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "screenrecorder.h"

#include <QDateTime>
#include <QDebug>

extern int audio_input_frame_size;

ScreenRecorder::ScreenRecorder()
{
    m_isRun = false;
}

ScreenRecorder::~ScreenRecorder()
{

}

ErroCode ScreenRecorder::init(QString audioDevName)
{

    AVCodec			*pCodec = NULL;
    AVCodec			*aCodec = NULL;

    av_register_all();
    avformat_network_init();
    avdevice_register_all();  //Register Device

    pFormatCtx = avformat_alloc_context();

    AVInputFormat *ifmt = av_find_input_format("dshow");

    QString audioDevOption = QString("audio=%1").arg(audioDevName);
    if(avformat_open_input(&pFormatCtx,audioDevOption.toUtf8(),ifmt,NULL)!=0){
        fprintf(stderr,"Couldn't open input stream audio.（无法打开输入流）\n");
        return AudioOpenFailed;
    }


    if(avformat_open_input(&pFormatCtx,"video=screen-capture-recorder",ifmt,NULL)!=0){
        fprintf(stderr,"Couldn't open input stream video.（无法打开输入流）\n");
        return VideoOpenFailed;
    }

    videoindex=-1;
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

    audioindex = -1;
    aCodecCtx = NULL;

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


    aFrame=avcodec_alloc_frame();
    pFrame=avcodec_alloc_frame();
    pFrameYUV=avcodec_alloc_frame();
    out_buffer=(uint8_t *)av_malloc(avpicture_get_size(PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
    avpicture_fill((AVPicture *)pFrameYUV, out_buffer, PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);

    setWidth(pCodecCtx->width, pCodecCtx->height);

    return SUCCEED;
}

void ScreenRecorder::deInit()
{
    av_free(out_buffer);
    av_free(aFrame);
    av_free(pFrame);
    av_free(pFrameYUV);
    avcodec_close(pCodecCtx);
    if (aCodecCtx)
        avcodec_close(aCodecCtx);

///下面这2个释放这里会奔溃 这里先无视 后面再完善它
//    avformat_close_input(&pFormatCtx);
//    avformat_free_context(pFormatCtx);
}

void ScreenRecorder::startRecord()
{
    m_isRun = true;
//    m_writeFile->startWrite();
    startEncode();
    start();
}

void ScreenRecorder::stopRecord()
{
    m_isRun = false;

    stopEncode();
}

void ScreenRecorder::run()
{

    int ret, got_frame;

    AVPacket *packet=(AVPacket *)av_malloc(sizeof(AVPacket));
//    //Output Information-----------------------------
//    printf("File Information（文件信息）---------------------\n");
//    av_dump_format(pFormatCtx,0,NULL,0);
//    printf("-------------------------------------------------\n");

    struct SwsContext *img_convert_ctx;
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
    //------------------------------


    int y_size = pCodecCtx->width * pCodecCtx->height;

    int size = avpicture_get_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);


    qint64 firstTime = QDateTime::currentMSecsSinceEpoch();

    while(m_isRun )
    {
        if (av_read_frame(pFormatCtx, packet)<0)
        {
            msleep(10);
            continue;
        }

        if(packet->stream_index==videoindex)
        {
            qint64 secondTime = QDateTime::currentMSecsSinceEpoch();

            if ((secondTime - firstTime) >= 100)
            {
                firstTime = secondTime;
                ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_frame, packet);
                if(ret < 0)
                {
                    printf("video Decode Error.（解码错误）\n");
                    return;
                }

                if(got_frame)
                {
                    sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);

                    uint8_t * picture_buf = (uint8_t *)av_malloc(size);
                    memcpy(picture_buf,pFrameYUV->data[0],y_size);
                    memcpy(picture_buf+y_size,pFrameYUV->data[1],y_size/4);
                    memcpy(picture_buf+y_size+y_size/4,pFrameYUV->data[2],y_size/4);


                    videoDataQuene_Input(picture_buf,y_size*3/2);
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

                int size = av_samples_get_buffer_size(NULL,aCodecCtx->channels, aFrame->nb_samples,aCodecCtx->sample_fmt, 1);

                int index = 0;

                int ONEAudioSize = audio_input_frame_size * 4;//4096

                for (int i=0;i<(size/ONEAudioSize);i++)
                {

                    int framSize = ONEAudioSize;
                    if (i==size/ONEAudioSize)
                    {
                        framSize = size%ONEAudioSize;
                    }

                    if (framSize<=0){
                        break;
                    }

                    uint8_t * audio_buf = (uint8_t *)malloc(4096*2);
                    memcpy(audio_buf, aFrame->data[0]+index, framSize);

                    audioDataQuene_Input((uint8_t*)audio_buf,ONEAudioSize);

                    index += framSize;

                }
            }

        }

        av_free_packet(packet);
    }

    qDebug()<<"record stopping...";

//    m_writeFile->stopWrite();

    qDebug()<<"record finished!";

    sws_freeContext(img_convert_ctx);


    deInit();

}
