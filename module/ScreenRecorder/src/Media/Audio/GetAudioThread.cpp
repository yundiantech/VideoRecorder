/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "MoudleConfig.h"
#include "GetAudioThread.h"

#include <thread>
#include <QDebug>

#if defined __linux
#include <xcb/xcb.h>
#endif
//#include <QDebug>
//Show Dshow Device
static void show_dshow_device()
{
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    AVDictionary* options = nullptr;
    av_dict_set(&options,"list_devices","true",0);
    AVInputFormat *iformat = av_find_input_format("dshow");
    printf("========Device Info=============\n");
    avformat_open_input(&pFormatCtx,"video=dummy",iformat,&options);
    printf("================================\n");
}

GetAudioThread::GetAudioThread()
{
    pFormatCtx = nullptr;
    aCodecCtx = nullptr;
    aFrame = nullptr;

    mIsThreadRunning = false;
    mIsStop = true;
    m_pause = false;

    aFrame = nullptr;
    aFrame_ReSample = nullptr;

    mCallBackFunc = nullptr;
    mCallBackFuncParam = nullptr;

    audio_buf_size_L = 0;
    audio_buf_size_R = 0;

    mONEFrameSize = 0;

    mCond = new Cond();
//    mIsNeedReOpenWhenReadFailed = true;
}

GetAudioThread::~GetAudioThread()
{

}

bool GetAudioThread::openDevice(const std::string &deviceName)
{
    mCond->Lock();

    mDeviceName = deviceName;
    bool isSucceed = init(mDeviceName.c_str());

    if (!isSucceed)
    {
        deInit();
    }

    mCond->Unlock();

    return isSucceed;
}

bool GetAudioThread::init(const char * const deviceName)
{
//    show_dshow_device();

    if (pFormatCtx != nullptr)
    {
        return true;
    }

    AVCodec			*pCodec = nullptr;

    pFormatCtx = avformat_alloc_context();

    if (strcmp(deviceName, "virtual-audio-capturer") == 0)
    {
        pFormatCtx->flags |= AVFMT_FLAG_NONBLOCK;
    }

#if defined(WIN32)

    AVInputFormat *ifmt = av_find_input_format("dshow"); //使用dshow

    char deviceNameStr[512] = {0};
    sprintf(deviceNameStr, "audio=%s", deviceName);

//    if(avformat_open_input(&pFormatCtx, "audio=virtual-audio-capturer", ifmt, nullptr)!=0)
    if(avformat_open_input(&pFormatCtx, deviceNameStr, ifmt, nullptr)!=0)
    {
        fprintf(stderr, "Couldn't open input stream audio %s.（无法打开输入流）\n", deviceNameStr);
        return false;
    }

#elif defined __linux
//Linux
    //Linux
    AVInputFormat *ifmt = av_find_input_format("alsa");
    if(avformat_open_input(&pFormatCtx, deviceName, ifmt, NULL)!=0)
    {
        printf("Couldn't open input stream. default\n");
        return false;
    }

#else
    show_avfoundation_device();
    //Mac
    AVInputFormat *ifmt=av_find_input_format("avfoundation");
    //Avfoundation
    //[video]:[audio]
    if(avformat_open_input(&pFormatCtx,"0",ifmt,nullptr)!=0)
    {
        fprintf(stderr, "Couldn't open input stream.\n");
        return VideoOpenFailed;
    }
#endif

    audioStream = -1;
    aCodecCtx   = nullptr;

    for(unsigned int i=0; i < pFormatCtx->nb_streams; i++)
    {
        if(pFormatCtx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_AUDIO)
        {
            audioStream = static_cast<int>(i);
            break;
        }
    }

    if(audioStream == -1)
    {
        printf("Didn't find a audio stream.（没有找到音频流）\n");
        return false;
    }

    //find the decoder
    aCodecCtx = avcodec_alloc_context3(nullptr);
    avcodec_parameters_to_context(aCodecCtx, pFormatCtx->streams[audioStream]->codecpar);

    pCodec = avcodec_find_decoder(aCodecCtx->codec_id);

    if(pCodec == nullptr)
    {
        printf("audio Codec not found.\n");
        return false;
    }

    if(avcodec_open2(aCodecCtx, pCodec, nullptr)<0)
    {
        printf("Could not open audio codec.\n");
        return false;
    }

    ///解码音频相关
    aFrame = av_frame_alloc();

    initResample();

    return true;
}

void GetAudioThread::deInit()
{
qDebug()<<__FUNCTION__<<"000";
    if (swrCtx != nullptr)
    {
        swr_free(&swrCtx);

        swrCtx = nullptr;
    }

    if (aFrame)
    {
        av_free(aFrame);
        aFrame = nullptr;
    }

    if (aFrame_ReSample)
    {
        av_free(aFrame_ReSample);
        aFrame_ReSample = nullptr;
    }
qDebug()<<__FUNCTION__<<"333";
    if (aCodecCtx)
        avcodec_close(aCodecCtx);
qDebug()<<__FUNCTION__<<"444";
    if (pFormatCtx)
    {
//qDebug()<<__FUNCTION__<<pFormatCtx->iformat<<pFormatCtx->filename<<pFormatCtx->url;

//        if (strcmp(pFormatCtx->filename, "virtual-audio-capturer") == 0)
//        {
//qDebug()<<__FUNCTION__<<"000";
//        }
//        else
        {
qDebug()<<__FUNCTION__<<"555";
            avformat_close_input(&pFormatCtx);
            avformat_free_context(pFormatCtx);
        }

        pFormatCtx = nullptr;
    }
qDebug()<<__FUNCTION__<<"999";
}

void GetAudioThread::startRecord(int outOneFrameSize, std::function<void (PCMFramePtr pcmFrame, void *param)> func, void *param)
{
    mIsStop = false;

    mONEFrameSize = outOneFrameSize;

    mCallBackFunc = func;
    mCallBackFuncParam = param;

    //启动新的线程
    std::thread([&](GetAudioThread *pointer)
    {
        pointer->run();

    }, this).detach();

}

void GetAudioThread::pauseRecord()
{
    m_pause = true;
}

void GetAudioThread::restoreRecord()
{
    m_getFirst = false;
    m_pause = false;
}

void GetAudioThread::stopRecord(bool isBlock)
{
qDebug()<<__FUNCTION__<<"111"<<isBlock<<mIsThreadRunning;
    mIsStop = true;

    if (isBlock)
    {
        while(mIsThreadRunning)
        {
           MoudleConfig::mSleep(10);
        }
    }
qDebug()<<__FUNCTION__<<"222"<<isBlock<<mIsThreadRunning;
}

bool GetAudioThread::initResample()
{
    //重采样设置选项-----------------------------------------------------------start
    aFrame_ReSample = nullptr;

    //frame->16bit 44100 PCM 统一音频采样格式与采样率
    swrCtx = nullptr;

    //输入的声道布局
    int in_ch_layout;

    /// 由于ffmpeg编码aac需要输入FLTP格式的数据。
    /// 因此这里将音频重采样成44100 双声道  AV_SAMPLE_FMT_FLTP
    //重采样设置选项----------------
    //输入的采样格式
    in_sample_fmt = aCodecCtx->sample_fmt;
    //输出的采样格式 32bit PCM
    out_sample_fmt = AV_SAMPLE_FMT_FLTP;
    //输入的采样率
    in_sample_rate = aCodecCtx->sample_rate;
    //输入的声道布局
    in_ch_layout = aCodecCtx->channel_layout;

    //输出的采样率
    out_sample_rate = 44100;

    //输出的声道布局
    out_ch_layout = AV_CH_LAYOUT_STEREO;
    audio_tgt_channels = av_get_channel_layout_nb_channels(out_ch_layout);

//        //输出的声道布局
//        out_ch_layout = av_get_default_channel_layout(audio_tgt_channels); ///AV_CH_LAYOUT_STEREO
//        out_ch_layout &= ~AV_CH_LAYOUT_STEREO;

    if (in_ch_layout <= 0)
    {
        if (aCodecCtx->channels == 2)
        {
            in_ch_layout = AV_CH_LAYOUT_STEREO;
        }
        else
        {
            in_ch_layout = AV_CH_LAYOUT_MONO;
        }
    }

    swrCtx = swr_alloc_set_opts(nullptr, out_ch_layout, out_sample_fmt, out_sample_rate,
                                         in_ch_layout, in_sample_fmt, in_sample_rate, 0, nullptr);

    /** Open the resampler with the specified parameters. */
    int ret = swr_init(swrCtx);
    if (ret < 0)
    {
        char buff[128]={0};
        av_strerror(ret, buff, 128);

        fprintf(stderr, "Could not open resample context %s\n", buff);
        swr_free(&swrCtx);
        swrCtx = nullptr;

        return false;
    }

    return true;
}

void GetAudioThread::run()
{
    mIsThreadRunning = true;

    int64_t firstTime = MoudleConfig::getTimeStamp_MilliSecond();
    m_getFirst = false;
    int64_t timeIndex = 0;

    mAudioPts = 0.0;

while(!mIsStop)
{
    if (!openDevice(mDeviceName))
    {
        MoudleConfig::mSleep(1000);
        continue;
    }

    mLastReadFailedTime = MoudleConfig::getTimeStamp_MilliSecond();

    while(!mIsStop)
    {
        AVPacket packet;

        int ret = av_read_frame(pFormatCtx, &packet);

        if (ret<0)
        {
//            char buffer[1024] = {0};
//            av_strerror(ret, buffer, 1024);
//            qDebug("av_read_frame = %d %s\n", ret, buffer);

            if (pFormatCtx->flags & AVFMT_FLAG_NONBLOCK)
            {
                ///一秒内都没有读取到过数据，则认为真的失败了。
                if ((MoudleConfig::getTimeStamp_MilliSecond() - mLastReadFailedTime) > 3000)
                {
                    qDebug("audio read failed! %s\n", mDeviceName.c_str());
                    fprintf(stderr, "audio read failed! %s\n", mDeviceName.c_str());
                    break;
                }

                MoudleConfig::mSleep(1);
                continue;
            }
            else
            {
                fprintf(stderr, "audio read failed! %s\n", mDeviceName.c_str());

                break;
            }
        }

        mLastReadFailedTime = MoudleConfig::getTimeStamp_MilliSecond();

        if (m_pause)
        {
            av_packet_unref(&packet);
            MoudleConfig::mSleep(10);
            continue;
        }
//qDebug()<<packet.stream_index<<audioStream<<packet.size;
        if(packet.stream_index == audioStream)
        {
            int64_t time = 0;
//            if (m_saveVideoFileThread)
            {
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
            }
//fprintf(stderr, "read audio frame %s size = %d \n", mDeviceName.c_str(), packet.size);
            if (int ret = avcodec_send_packet(aCodecCtx, &packet) && ret != 0)
            {
               char buffer[1024] = {0};
               av_strerror(ret, buffer, 1024);
               fprintf(stderr, "input AVPacket to decoder failed! ret = %d %s\n", ret, buffer);
            }
            else
            {
                int iii=0;
            //    while (0 == avcodec_receive_frame(pCodecCtx, pFrame))
                while(1)
                {
                    int ret = avcodec_receive_frame(aCodecCtx, aFrame);
                    if (ret != 0)
                    {
            //            char buffer[1024] = {0};
            //            av_strerror(ret, buffer, 1024);
            //            fprintf(stderr, "avcodec_receive_frame = %d %s\n", ret, buffer);
                        break;
                    }

                    int dst_nb_samples = av_rescale_rnd(aFrame->nb_samples, out_sample_rate, in_sample_rate, AV_ROUND_UP);


                    ///解码一帧后才能获取到采样率等信息，因此将初始化放到这里
                    if (aFrame_ReSample == nullptr || aFrame_ReSample->nb_samples != dst_nb_samples)
                    {
                        if (aFrame_ReSample)
                        {
                            av_free(aFrame_ReSample);
                            aFrame_ReSample = nullptr;
                        }

                        aFrame_ReSample = av_frame_alloc();

//                        int nb_samples = av_rescale_rnd(swr_get_delay(swrCtx, out_sample_rate) + aFrame->nb_samples, out_sample_rate, in_sample_rate, AV_ROUND_UP);
//                        av_samples_fill_arrays(aFrame_ReSample->data, aFrame_ReSample->linesize, audio_buf_resample, audio_tgt_channels, aFrame_ReSample->nb_samples, out_sample_fmt, 0);

                        aFrame_ReSample->format = out_sample_fmt;
                        aFrame_ReSample->channel_layout = out_ch_layout;
                        aFrame_ReSample->sample_rate = out_sample_rate;
                        aFrame_ReSample->nb_samples = dst_nb_samples;
//qDebug()<<__FUNCTION__<<"samples 111:"<<aFrame->nb_samples<<aFrame_ReSample->nb_samples<<dst_nb_samples;

                        ret = av_frame_get_buffer(aFrame_ReSample, 0);
                        if (ret < 0)
                        {
                            fprintf(stderr, "Error allocating an audio buffer\n");
//                            exit(1);
                        }
                    }

                    ///执行重采样
                    int len2 = swr_convert(swrCtx, aFrame_ReSample->data, aFrame_ReSample->nb_samples, (const uint8_t**)aFrame->data, aFrame->nb_samples);

    ///下面这两种方法计算的大小是一样的
    #if 0
                    int resampled_data_size = len2 * audio_tgt_channels * av_get_bytes_per_sample(out_sample_fmt);
    #else
                    int resampled_data_size = av_samples_get_buffer_size(NULL, audio_tgt_channels, aFrame_ReSample->nb_samples, out_sample_fmt, 1);
    #endif

//qDebug()<<__FUNCTION__<<"samples:"<<len2<<aFrame_ReSample->nb_samples<<aFrame->nb_samples<<dst_nb_samples<<resampled_data_size;

                    int OneChannelDataSize = resampled_data_size / audio_tgt_channels;

//fprintf(stderr, "OneChannelDataSize=%d %d %d\n", OneChannelDataSize, mAudioEncoder->getONEFrameSize(), aFrame->nb_samples);
/// 由于平面模式的pcm存储方式为：LLLLLLLLLLLLLLLLLLLLLRRRRRRRRRRRRRRRRRRRRR，
/// 因此这里需要将左右声道数据分开存入文件才可以正常播放。
/// 使用播放器单独播放左右 声道数据测试即可(以单声道 44100 32bit打开播放)。
//static FILE *fp1 = fopen("out-L.pcm", "wb");
//fwrite(aFrame_ReSample->data[0], 1, OneChannelDataSize, fp1);
//if (audio_tgt_channels >= 2)
//{
//    static FILE *fp2 = fopen("out-R.pcm", "wb");
//    fwrite(aFrame_ReSample->data[1], 1, OneChannelDataSize, fp2);
//}
                    dealWithAudioFrame(OneChannelDataSize);
                }
            }
        }
        else
        {
            fprintf(stderr, "other %d \n", packet.stream_index);
        }

        av_packet_unref(&packet);

    }

    fprintf(stderr, "audio record stopping... \n");
    qDebug("audio record stopping...  %s\n", mDeviceName.c_str());

    m_pause = false;

    deInit();

    fprintf(stderr, "audio record finished! \n");
    qDebug("audio record finished! %s\n", mDeviceName.c_str());
}

    mIsThreadRunning = false;

}

void GetAudioThread::dealWithAudioFrame(const int &OneChannelDataSize)
{
    ///编码器一帧的采样为1024，而这里一次获取到的不是1024，因此需要放入队列，然后每次从队里取1024次采样交给编码器。
    ///PS：平面模式的pcm存储方式为：LLLLLLLLLLLLLLLLLLLLLRRRRRRRRRRRRRRRRRRRRR，需要了解这句话的含义。

    memcpy(audio_buf_L + audio_buf_size_L, aFrame_ReSample->data[0], OneChannelDataSize);
    audio_buf_size_L += OneChannelDataSize;

    if (audio_tgt_channels >= 2)
    {
        memcpy(audio_buf_R + audio_buf_size_R, aFrame_ReSample->data[0], OneChannelDataSize);
        audio_buf_size_R += OneChannelDataSize;
    }

//    if (audio_tgt_channels >= 2)
//    {
//        memcpy(audio_buf_R + audio_buf_size_R, aFrame_ReSample->data[1], OneChannelDataSize);
//        audio_buf_size_R += OneChannelDataSize;
//    }

    int index = 0;

    int leftSize  = audio_buf_size_L;

    int ONEChannelAudioSize = mONEFrameSize / audio_tgt_channels;

    ///由于采集到的数据很大，而编码器一次只需要很少的数据。
    ///因此将采集到的数据分成多次传给编码器。
    /// 由于平面模式的pcm存储方式为：LLLLLLLLLLLLLLLLLLLLLRRRRRRRRRRRRRRRRRRRRR，因此这里合并完传给编码器就行了
    while(1)
    {
        if (leftSize >= ONEChannelAudioSize)
        {
            uint8_t * buffer = (uint8_t *)malloc(ONEChannelAudioSize * audio_tgt_channels);
            memcpy(buffer, audio_buf_L+index, ONEChannelAudioSize);

            if (audio_tgt_channels >= 2)
            {
                memcpy(buffer+ONEChannelAudioSize, audio_buf_R+index, ONEChannelAudioSize);
            }

            ///一秒44100采样，一帧1024次采样，那么一帧的时间就是： 1000 / (44100 / 1024) = 23.2199580毫秒
            mAudioPts += 23.2199580;

            int64_t time = mAudioPts;

            PCMFramePtr framePtr = std::make_shared<PCMFrame>();
            framePtr->setFrameBuffer(buffer, ONEChannelAudioSize * audio_tgt_channels, time);

            free(buffer);

            if (mCallBackFunc != nullptr)
            {
                mCallBackFunc(framePtr, mCallBackFuncParam);
            }

            index    += ONEChannelAudioSize;
            leftSize -= ONEChannelAudioSize;
        }
        else
        {
            if (leftSize > 0)
            {
                memcpy(audio_buf_L, audio_buf_L+index, leftSize);
                memcpy(audio_buf_R, audio_buf_R+index, leftSize);
            }
            audio_buf_size_L = leftSize;
            audio_buf_size_R = leftSize;
            break;
        }
    }
}
