/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "MoudleConfig.h"
#include "GetVideoThread.h"

//'1' Use Dshow
//'0' Use VFW
#define USE_DSHOW 0

////Show Dshow Device
//void show_dshow_device()
//{
//    AVFormatContext *pFormatCtx = avformat_alloc_context();
//    AVDictionary* options = nullptr;
//    av_dict_set(&options,"list_devices","true",0);
//    AVInputFormat *iformat = av_find_input_format("dshow");
//    printf("========Device Info=============\n");
//    avformat_open_input(&pFormatCtx,"video=dummy",iformat,&options);
//    printf("================================\n");
//}

////Show Dshow Device Option
//void show_dshow_device_option()
//{
//    AVFormatContext *pFormatCtx = avformat_alloc_context();
//    AVDictionary* options = nullptr;
//    av_dict_set(&options,"list_options","true",0);
//    AVInputFormat *iformat = av_find_input_format("dshow");
//    printf("========Device Option Info======\n");
//    avformat_open_input(&pFormatCtx,"video=Integrated Camera",iformat,&options);
//    printf("================================\n");
//}

////Show VFW Device
//void show_vfw_device()
//{
//    AVFormatContext *pFormatCtx = avformat_alloc_context();
//    AVInputFormat *iformat = av_find_input_format("vfwcap");
//    printf("========VFW Device Info======\n");
//    avformat_open_input(&pFormatCtx,"list",iformat,nullptr);
//    printf("=============================\n");
//}

////Show AVFoundation Device
//void show_avfoundation_device()
//{
//    AVFormatContext *pFormatCtx = avformat_alloc_context();

//    AVDictionary* options = nullptr;
//    av_dict_set(&options,"list_devices","true",0);
//    AVInputFormat *iformat = av_find_input_format("avfoundation");
//    printf("==AVFoundation Device Info===\n");
//    avformat_open_input(&pFormatCtx, "",iformat, &options);
//    printf("=============================\n");
//}

GetVideoThread::GetVideoThread()
{
    mIsCloseCamera = true;
    mIsStop = false;
    mIsThreadRuning = false;
    mIsReadingVideo = false;

    pFormatCtx = nullptr;
    out_buffer = nullptr;
    out_buffer_rgb24 = nullptr;

    pFrame = nullptr;
    pFrameYUV = nullptr;
    pFrameRGB = nullptr;

    pCodecCtx = nullptr;

    mCallBackFunc = nullptr;
    mCallBackFuncParam = nullptr;

    m_pause = false;

    mCond = new Cond();
}

GetVideoThread::~GetVideoThread()
{

}

GetVideoThread::ErroCode GetVideoThread::openCamera(const std::string &deviceName)
{
    mCond->Lock();

    if (mDeviceName != deviceName)
    {
        closeCamera(true);
    }

    mDeviceName = deviceName;
//    mIsCloseCamera = false;
    ErroCode code = init(mDeviceName.c_str());

    mIsCloseCamera = false;

    mCond->Unlock();

    return code;
}

void GetVideoThread::closeCamera(bool isBlock)
{
    mIsCloseCamera = true;

    if (isBlock)
    {
        while(mIsReadingVideo)
        {
            MoudleConfig::mSleep(100);
        }
    }

}

GetVideoThread::ErroCode GetVideoThread::init(const std::string deviceName)
{
    if (pFormatCtx != nullptr)
    {
        return SUCCEED;
    }

    AVCodec			*pCodec = nullptr;

    pFormatCtx = avformat_alloc_context();

    ///设置成非阻塞模式。
    pFormatCtx->flags |= AVFMT_FLAG_NONBLOCK;

#if defined(WIN32)

//    //Show Dshow Device
//    show_dshow_device();
//    //Show Device Options
//    show_dshow_device_option();
//    //Show VFW Options
//    show_vfw_device();

    AVInputFormat *ifmt = av_find_input_format("dshow"); //使用dshow

    char deviceNameStr[512] = {0};
    sprintf(deviceNameStr, "video=%s", deviceName.c_str());

//    if(avformat_open_input(&pFormatCtx, "video=screen-capture-recorder", ifmt, nullptr)!=0)
//    if(avformat_open_input(&pFormatCtx, "video=Techshino TCF242", ifmt, nullptr)!=0)
    if(avformat_open_input(&pFormatCtx, deviceNameStr, ifmt, nullptr)!=0)
    {
//        fprintf(stderr, "Couldn't open input stream video.（无法打开输入流）\n");

        avformat_close_input(&pFormatCtx);
        avformat_free_context(pFormatCtx);

        pFormatCtx = nullptr;

        return VideoOpenFailed;
    }
#elif defined __linux
//Linux
//    AVInputFormat *ifmt=av_find_input_format("video4linux2");
//    if(avformat_open_input(&pFormatCtx, "/dev/video0", ifmt, NULL)!=0)
//    {
//        fprintf(stderr, "Couldn't open input stream.\n");
//        return -1;
//    }

    AVDictionary* options = NULL;
//    av_dict_set(&options,"list_devices","true", 0);
    /* set frame per second */
//    av_dict_set( &options,"framerate","30", 0);
    av_dict_set( &options,"show_region","1", 0);
//    av_dict_set( &options,"video_size","1240x480", 0);
//    av_dict_set( &options, "preset", "medium", 0 );

    /*
    X11 video input device.
    To enable this input device during configuration you need libxcb installed on your system. It will be automatically detected during configuration.
    This device allows one to capture a region of an X11 display.
    refer : https://www.ffmpeg.org/ffmpeg-devices.html#x11grab
    */
    AVInputFormat *ifmt = av_find_input_format("x11grab");
    if(avformat_open_input(&pFormatCtx, ":0.0+10,250", ifmt, &options) != 0)
//    if(avformat_open_input(&pFormatCtx, ":0.0", ifmt, &options) != 0)
    {
        fprintf(stderr, "\nerror in opening input device\n");
        return VideoOpenFailed;
    }
#else
    show_avfoundation_device();
    //Mac
    AVInputFormat *ifmt=av_find_input_format("avfoundation");
    //Avfoundation
    //[video]:[audio]
    if(avformat_open_input(&pFormatCtx,"0",ifmt,NULL)!=0)
    {
        fprintf(stderr, "Couldn't open input stream.\n");
        return VideoOpenFailed;
    }
#endif

    videoindex=-1;
    pCodecCtx = NULL;

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
        fprintf(stderr, "\n###\n video Codec not found. pCodecCtx->codec_id=%d %d \n", pCodecCtx->codec_id, AV_CODEC_ID_MJPEG);
        return VideoDecoderOpenFailed;
    }

    if(avcodec_open2(pCodecCtx, pCodec,NULL)<0)
    {
        printf("Could not open video codec.\n");
        return VideoDecoderOpenFailed;
    }

    if (pCodecCtx->pix_fmt == AV_PIX_FMT_NONE)
    if (pCodecCtx->codec_id == AV_CODEC_ID_MJPEG)
    {
        pCodecCtx->pix_fmt = AV_PIX_FMT_YUV422P;
    }

//    fprintf(stderr, "\n###\n video Codec not found. pCodecCtx->codec_id=%d %d \n", pCodecCtx->codec_id, AV_CODEC_ID_MJPEG);

    pFrame = av_frame_alloc();
    pFrameYUV = av_frame_alloc();
    pFrameRGB = av_frame_alloc();

    //***************
//    int Screen_W = GetSystemMetrics(SM_CXSCREEN); //获取屏幕宽高
//    int Screen_H = GetSystemMetrics(SM_CYSCREEN);

    return SUCCEED;
}

void GetVideoThread::deInit()
{
    if (out_buffer)
    {
        av_free(out_buffer);
        out_buffer = NULL;
    }

    if (out_buffer_rgb24)
    {
        av_free(out_buffer_rgb24);
        out_buffer_rgb24 = NULL;
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

    if (pFrameRGB)
    {
        av_free(pFrameRGB);
        pFrameRGB = NULL;
    }

    if (pCodecCtx)
        avcodec_close(pCodecCtx);

    if (pFormatCtx != nullptr)
    {
        avformat_close_input(&pFormatCtx);
        avformat_free_context(pFormatCtx);
    }
}

void GetVideoThread::startRecord(std::function<void (VideoRawFramePtr yuvFrame, VideoRawFramePtr rgbFrame, void *param)> func, void *param)
{
    mCallBackFunc = func;
    mCallBackFuncParam = param;

    mIsStop = false;
    mIsThreadRuning = false;

    //启动新的线程
    std::thread([&](GetVideoThread *pointer)
    {
        pointer->run();

    }, this).detach();

}

void GetVideoThread::pauseRecord()
{
    m_pause = true;
}

void GetVideoThread::restoreRecord()
{
    m_getFirst = false;
    m_pause = false;
}

void GetVideoThread::stopRecord(const bool &isBlock)
{
    mIsStop = true;

    if (isBlock)
    {
        while(mIsThreadRuning)
        {
            MoudleConfig::mSleep(5);
        }
    }
}

void GetVideoThread::run()
{
    mIsThreadRuning = true;

while(!mIsStop)
{
    if (mIsCloseCamera)
    {
        MoudleConfig::mSleep(1000);
        continue;
    }

    mIsReadingVideo = true;

    if (openCamera(mDeviceName) != SUCCEED)
    {
        MoudleConfig::mSleep(1000);
        continue;
    }

    struct SwsContext *img_convert_ctx = NULL;
    struct SwsContext *img_convert_ctx_rgb24 = NULL;

    int y_size = 0;
    int yuvSize = 0;

    if (pCodecCtx)
    {
        y_size = pCodecCtx->width * pCodecCtx->height;
        yuvSize = avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
        ///理论上 这里的 size = y_size * 3 / 2

        int numBytes = yuvSize;
        out_buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
        avpicture_fill((AVPicture *) pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P,pCodecCtx->width, pCodecCtx->height);

        img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                         pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P,
                                         SWS_BICUBIC, NULL, NULL, NULL);

//int mjpegSize = av_image_get_buffer_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, 1);  //按1字节进行内存对齐,得到的内存大小最接近实际大小
//fprintf(stderr, "%s mjpegSize=%d \n", __FUNCTION__, mjpegSize);

        int rgb24Size = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 1);  //按1字节进行内存对齐,得到的内存大小最接近实际大小
    //    int yuvSize = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 0);  //按0字节进行内存对齐，得到的内存大小是0
    //    int yuvSize = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 4);   //按4字节进行内存对齐，得到的内存大小稍微大一些

        out_buffer_rgb24 = static_cast<uint8_t *>(av_malloc(static_cast<unsigned int>(rgb24Size) * sizeof(uint8_t)));
        av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, out_buffer_rgb24, AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 1);

        img_convert_ctx_rgb24 = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                         pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB24,
                                         SWS_BICUBIC, NULL, NULL, NULL);

    }

    AVPacket *packet=(AVPacket *)av_malloc(sizeof(AVPacket));

    int64_t firstTime = MoudleConfig::getTimeStamp_MilliSecond();
    m_getFirst = false;
    int64_t timeIndex = 0;

    mLastReadFailedTime = MoudleConfig::getTimeStamp_MilliSecond();

    while(!mIsStop)
    {
        if (mIsCloseCamera)
        {
            break;
        }

        if (av_read_frame(pFormatCtx, packet)<0)
        {
            ///一秒内都没有读取到过数据，则认为真的失败了。
            if ((MoudleConfig::getTimeStamp_MilliSecond() - mLastReadFailedTime) > 1000)
            {
                fprintf(stderr, "camera read failed! %s\n", mDeviceName.c_str());
                break;
            }

            MoudleConfig::mSleep(10);
            continue;
        }

        mLastReadFailedTime = MoudleConfig::getTimeStamp_MilliSecond();

        if (m_pause)
        {
            av_packet_unref(packet);
            MoudleConfig::mSleep(10);
            continue;
        }

        if(packet->stream_index==videoindex)
        {
            int64_t time = 0;

            if (m_getFirst)
            {
                int64_t secondTime = MoudleConfig::getTimeStamp_MilliSecond();
                time = secondTime - firstTime + timeIndex;
            }
            else
            {
                firstTime = MoudleConfig::getTimeStamp_MilliSecond();
                timeIndex = 0;
                m_getFirst = true;
            }

            if (avcodec_send_packet(pCodecCtx, packet) != 0)
            {
               fprintf(stderr, "input AVPacket to decoder failed!\n");
               av_packet_unref(packet);
               continue;
            }

            while (0 == avcodec_receive_frame(pCodecCtx, pFrame))
            {
//int64_t t1 = MoudleConfig::getTimeStamp_MilliSecond();

                /// 转换成YUV420
                /// 由于解码后的数据不一定是yuv420p，比如硬件解码后会是yuv420sp，因此这里统一转成yuv420p
                sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);

//int64_t t2 = MoudleConfig::getTimeStamp_MilliSecond();

                /// 转换成RGB24
                sws_scale(img_convert_ctx_rgb24, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);

//int64_t t3 = MoudleConfig::getTimeStamp_MilliSecond();

//fprintf(stderr, "%s %d %d %I64d %I64d \n",__FUNCTION__, pCodecCtx->pix_fmt, AV_PIX_FMT_YUV420P, t2-t1, t3-t2);

                if (mCallBackFunc != nullptr)
                {
                    VideoRawFramePtr yuvFrame = std::make_shared<VideoRawFrame>();
                    VideoRawFramePtr rgbFrame = std::make_shared<VideoRawFrame>();

                    yuvFrame->initBuffer(pCodecCtx->width, pCodecCtx->height, VideoRawFrame::FRAME_TYPE_YUV420P);
                    yuvFrame->setFramebuf(out_buffer);

                    rgbFrame->initBuffer(pCodecCtx->width, pCodecCtx->height, VideoRawFrame::FRAME_TYPE_RGB24);
                    rgbFrame->setFramebuf(out_buffer_rgb24);
//FILE *fp = fopen("out.rgb24", "wb");
//fwrite(out_buffer_rgb24, 1, av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 1), fp);
//fclose(fp);
//fprintf(stderr, "%s %d %d %d \n", __FUNCTION__,
//        av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 1),
//        pCodecCtx->width,pCodecCtx->height);
                    mCallBackFunc(yuvFrame, rgbFrame, mCallBackFuncParam);
                }

            }
        }
        else
        {
            fprintf(stderr, "other %d \n", packet->stream_index);
        }
        av_packet_unref(packet);

    }

    sws_freeContext(img_convert_ctx);
    sws_freeContext(img_convert_ctx_rgb24);

//    fprintf(stderr, "record stopping... \n");

    m_pause = false;

    deInit();

    if (mIsCloseCamera)
    {
        mIsReadingVideo = false;
    }
}

    fprintf(stderr, "record finished! \n");
    mIsReadingVideo = false;
    mIsThreadRuning = false;

}

