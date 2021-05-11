/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "VideoFileWriter.h"
#include "AppConfig.h"

#include <QFileInfo>
#include <QDir>

#include <thread>

#include "MoudleConfig.h"

#include <QString>
#include "AppConfig.h"

//double videoPts = 0.0;
//static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt)
//{
//    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

//char buf[AV_TS_MAX_STRING_SIZE];
//av_ts_make_time_string(buf, pkt->dts, time_base);

//char buf1[AV_TS_MAX_STRING_SIZE];
//av_ts_make_time_string(buf1, pkt->pts, time_base);

//char buf2[AV_TS_MAX_STRING_SIZE];
//av_ts_make_time_string(buf2, pkt->duration, time_base);
//videoPts = pkt->pts;
//qDebug()<<pkt->pts<<pkt->dts<<buf<<buf1<<buf2;

////    printf("pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
////           av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
////           av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
////           av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
////           pkt->stream_index);
//}

//static int write_frame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt, bool isAudio)
//{
//    /* rescale output packet timestamp values from codec to stream timebase */
//    av_packet_rescale_ts(pkt, *time_base, st->time_base);
//    pkt->stream_index = st->index;

//    if (!isAudio)
//    {
//        log_packet(fmt_ctx, pkt);
//    }

//    /* Write the compressed frame to the media file. */

//    return av_interleaved_write_frame(fmt_ctx, pkt);
//}

/**
*  Add ADTS header at the beginning of each and every AAC packet.
*  This is needed as MediaCodec encoder generates a packet of raw
*  AAC data.
*
*  Note the packetLen must count in the ADTS header itself !!! .
*注意，这里的packetLen参数为raw aac Packet Len + 7; 7 bytes adts header
**/
static void addADTStoPacket(uint8_t* packet, int packetLen)
{
   int profile = 2;  //AAC LC，MediaCodecInfo.CodecProfileLevel.AACObjectLC;
   int freqIdx = 4;  //32K, 见后面注释avpriv_mpeg4audio_sample_rates中32000对应的数组下标，来自ffmpeg源码
   int chanCfg = 2;  //见后面注释channel_configuration，Stero双声道立体声

   /*int avpriv_mpeg4audio_sample_rates[] = {
       96000, 88200, 64000, 48000, 44100, 32000,
               24000, 22050, 16000, 12000, 11025, 8000, 7350
   };
   channel_configuration: 表示声道数chanCfg
   0: Defined in AOT Specifc Config
   1: 1 channel: front-center
   2: 2 channels: front-left, front-right
   3: 3 channels: front-center, front-left, front-right
   4: 4 channels: front-center, front-left, front-right, back-center
   5: 5 channels: front-center, front-left, front-right, back-left, back-right
   6: 6 channels: front-center, front-left, front-right, back-left, back-right, LFE-channel
   7: 8 channels: front-center, front-left, front-right, side-left, side-right, back-left, back-right, LFE-channel
   8-15: Reserved
   */

   // fill in ADTS data
   packet[0] = (uint8_t)0xFF;
   packet[1] = (uint8_t)0xF9;
   packet[2] = (uint8_t)(((profile-1)<<6) + (freqIdx<<2) +(chanCfg>>2));
   packet[3] = (uint8_t)(((chanCfg&3)<<6) + (packetLen>>11));
   packet[4] = (uint8_t)((packetLen&0x7FF) >> 3);
   packet[5] = (uint8_t)(((packetLen&7)<<5) + 0x1F);
   packet[6] = (uint8_t)0xFC;
}

VideoFileWriter::VideoFileWriter()
{
    mIsStop = false;
    mIsThreadRunning = false;

    m_containsVideo = true;
    m_containsAudio = true;

    mCondAudio = new Cond();
    mCondVideo = new Cond();

    mVideoFrameRate = 15;

    mLastVideoFrame = nullptr;

    WIDTH  = 0;
    HEIGHT = 0;

    mBitRate = 450000;
    mQuality = 10;

    audio_pts = 0;
    video_pts = 0;

    mLastFileVideoPts = 0;

    mIsUseMuteAudio   = false;
    mLastGetAudioTime = 0;

    mVideoEncoder = new VideoEncoder();
}

VideoFileWriter::~VideoFileWriter()
{

}

void VideoFileWriter::setContainsVideo(bool value)
{
    m_containsVideo = value;
}

void VideoFileWriter::setContainsAudio(bool value)
{
    m_containsAudio = value;
}

void VideoFileWriter::setVideoFrameRate(int value)
{
    mVideoFrameRate = value;
}

void VideoFileWriter::setFileName(const std::string &filePath)
{
    mFilePath = filePath;
//    mFilePath = QString::fromStdString(filePath);
}

std::list<VideoFileInfo> VideoFileWriter::getVideoFileList()
{
    std::list<VideoFileInfo> list = mVideoFileList;
    mVideoFileList.clear();
    return list;
}

void VideoFileWriter::setQuality(const int &value)
{
    mBitRate = 450000 + (value - 5) * 50000;
    mQuality = value;

    mVideoEncoder->setQuality(value);
}

void VideoFileWriter::inputYuvFrame(VideoRawFramePtr yuvFrame)
{
    mCondVideo->Lock();
    mVideoFrameList.push_back(yuvFrame);
    mCondVideo->Unlock();
//qDebug()<<__FUNCTION__<<mVideoFrameList.size()<<mPcmFrameList.size();
    mCondVideo->Signal();
}

VideoRawFramePtr VideoFileWriter::readYuvFrame(const int64_t &time)
{
    VideoRawFramePtr yuvFrame = nullptr;

    mCondVideo->Lock();

    if (!mVideoFrameList.empty())
    {
        VideoRawFramePtr tmpFrame = mVideoFrameList.front();

        if (time >= tmpFrame->getPts())
        {
            /// 期望的时间 比队列第一帧大.
            /// 则依次往后找直到找到最接近的一帧，并丢弃中途中的帧.

            while(1)
            {
                std::list<VideoRawFramePtr>::iterator iter_fist = mVideoFrameList.begin();
                if (iter_fist == mVideoFrameList.end()) break;

                VideoRawFramePtr framePtr = *iter_fist;

                std::list<VideoRawFramePtr>::iterator iter_second = ++iter_fist;

                ///队列中数据不足2个了，则先退出，等待有2个再来。
                if (iter_second == mVideoFrameList.end())
                {
                    yuvFrame = nullptr;
                    break;
                }

                VideoRawFramePtr framePtr2 = *iter_second;

                ///找到了，则退出
                if (time < framePtr2->getPts())
                {
                    yuvFrame = framePtr;
                    mVideoFrameList.pop_front();
                    break;
                }

                ///第一帧不符合要求，则删掉第一帧，继续判断第二帧。
//                mVideoFrameList.pop_front();
                mVideoFrameList.erase(iter_fist);
            }
        }
        else if (time == 0)
        {
            yuvFrame = tmpFrame; ///如果是0 则直接用队列开头第一帧
            mVideoFrameList.pop_front();
        }
        else
        {
//            qDebug()<<__FUNCTION__<<time<<"use last frame";
            ///期望的时间 比队列第一帧小， 则直接重复上一帧
            yuvFrame = mLastVideoFrame;
        }
    }

    mCondVideo->Unlock();

    return yuvFrame;
}

void VideoFileWriter::clearYuvFrame()
{
    mCondVideo->Lock();
    mVideoFrameList.clear();
    mCondVideo->Unlock();
}

void VideoFileWriter::inputPcmFrame(PCMFramePtr pcmFrame)
{
    mCondAudio->Lock();
    mPcmFrameList.push_back(pcmFrame);
    mCondAudio->Unlock();
    mCondAudio->Signal();
}

PCMFramePtr VideoFileWriter::readPcmFrame()
{
    PCMFramePtr frame = nullptr;

    mCondAudio->Lock();
    if (!mPcmFrameList.empty())
    {
        frame = mPcmFrameList.front();
        mPcmFrameList.pop_front();
    }
    mCondAudio->Unlock();

    return frame;
}

void VideoFileWriter::clearPcmFrame()
{
    mCondAudio->Lock();
    mPcmFrameList.clear();
    mCondAudio->Unlock();
}

/*
 * add an audio output stream
 */
void VideoFileWriter::add_audio_stream(OutputStream *ost, AVFormatContext *oc,
                                                AVCodec **codec,
                                                enum AVCodecID codec_id)
{
    AVCodecContext *aCodecCtx;
    int i;

    /* find the video encoder */
    *codec = avcodec_find_encoder(codec_id);
    if (!codec) {
        fprintf(stderr, "codec not found\n");
        exit(1);
    }

    ost->st = avformat_new_stream(oc, NULL);
    if (!ost->st) {
        fprintf(stderr, "Could not alloc stream\n");
        exit(1);
    }

    ost->st->id = oc->nb_streams-1;

    const AVCodec* aCodec = *codec;

    aCodecCtx = avcodec_alloc_context3(aCodec);
    if (!aCodecCtx)
    {
        fprintf(stderr, "Could not alloc an encoding context\n");
        exit(1);
    }

    ///先用这句话找出 aac编码器支持的 sample_fmt
    /// 我找出的是 AV_SAMPLE_FMT_FLTP
    const enum AVSampleFormat *p = aCodec->sample_fmts;
    fprintf(stderr, "aac encoder sample format is: %s \n",av_get_sample_fmt_name(*p));

    ost->enc = aCodecCtx;

//    aCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO;
    aCodecCtx->sample_fmt = AV_SAMPLE_FMT_FLTP;
    aCodecCtx->sample_rate= 44100;
    aCodecCtx->channels = 2;
    aCodecCtx->channel_layout=av_get_default_channel_layout(aCodecCtx->channels);

//    aCodecCtx->channels       = av_get_channel_layout_nb_channels(aCodecCtx->channel_layout);
//    aCodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;

//    aCodecCtx->profile=FF_PROFILE_AAC_LOW; //（可参考AAC格式简介）
//    aCodecCtx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

//    aCodecCtx->bit_rate = 64000;

#if 0
    if ((*codec)->supported_samplerates)
    {
        aCodecCtx->sample_rate = (*codec)->supported_samplerates[0];

        for (i = 0; (*codec)->supported_samplerates[i]; i++)
        {
            if ((*codec)->supported_samplerates[i] == 44100)
                aCodecCtx->sample_rate = 44100;
        }
    }

    if ((*codec)->channel_layouts)
    {
        aCodecCtx->channel_layout = (*codec)->channel_layouts[0];
        for (i = 0; (*codec)->channel_layouts[i]; i++)
        {
            if ((*codec)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
                aCodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
        }
    }
    aCodecCtx->channels        = av_get_channel_layout_nb_channels(aCodecCtx->channel_layout);
#endif

    ost->st->time_base.num = 1; // = (AVRational){ 1, c->sample_rate };
    ost->st->time_base.den = aCodecCtx->sample_rate;

}

void VideoFileWriter::open_audio(AVFormatContext *oc, AVCodec *codec, OutputStream *ost)
{
    AVCodecContext *aCodecCtx = ost->enc;

    /* open it */
    if (avcodec_open2(aCodecCtx, codec, NULL) < 0)
    {
        qDebug("could not open codec\n");
        exit(1);
    }

    mONEFrameSize = av_samples_get_buffer_size(NULL, aCodecCtx->channels, aCodecCtx->frame_size, aCodecCtx->sample_fmt, 1);

    ost->frame           = av_frame_alloc();
    ost->frameBuffer     = (uint8_t *)av_malloc(mONEFrameSize);
    ost->frameBufferSize = mONEFrameSize;

    ///这句话必须要(设置这个frame里面的采样点个数)
    int oneChannelBufferSize = mONEFrameSize / aCodecCtx->channels; //计算出一个声道的数据
    int nb_samplesize = oneChannelBufferSize / av_get_bytes_per_sample(aCodecCtx->sample_fmt); //计算出采样点个数
    ost->frame->nb_samples = nb_samplesize;

    ///这2种方式都可以
//    avcodec_fill_audio_frame(ost->frame, aCodecCtx->channels, aCodecCtx->sample_fmt,(const uint8_t*)ost->frameBuffer, mONEFrameSize, 0);
    av_samples_fill_arrays(ost->frame->data, ost->frame->linesize, ost->frameBuffer, aCodecCtx->channels, ost->frame->nb_samples, aCodecCtx->sample_fmt, 0);

    ost->tmp_frame = nullptr;

    /* copy the stream parameters to the muxer */
    int ret = avcodec_parameters_from_context(ost->st->codecpar, aCodecCtx);
    if (ret < 0)
    {
        fprintf(stderr, "Could not copy the stream parameters\n");
        exit(1);
    }

}

/*
 * encode one audio frame and send it to the muxer
 * return 1 when encoding is finished, 0 otherwise
 */
//static int write_audio_frame(AVFormatContext *oc, OutputStream *ost)
bool VideoFileWriter::write_audio_frame(AVFormatContext *oc, OutputStream *ost)
{
    AVCodecContext *aCodecCtx = ost->enc;

    AVPacket pkt;
    av_init_packet(&pkt);

    AVPacket *packet = &pkt;

    AVFrame *aFrame = nullptr;


    PCMFramePtr pcmFrame = readPcmFrame();

    if (pcmFrame != nullptr)
    {
		memset(ost->frameBuffer, 0x0, ost->frameBufferSize);
        memcpy(ost->frameBuffer, pcmFrame->getBuffer(), pcmFrame->getSize());

		aFrame = ost->frame;
        aFrame->pts = ost->next_pts;
        ost->next_pts  += aFrame->nb_samples;

        mLastGetAudioTime = av_gettime();
    }
    else
    {
        ///超过3秒没有获取到外部来的数据，则直接用静音数据写入文件
        if ((av_gettime() - mLastGetAudioTime) >= 3000000)
        {
            memset(ost->frameBuffer, 0x0, ost->frameBufferSize);

			aFrame = ost->frame;
			aFrame->pts = ost->next_pts;
			ost->next_pts += aFrame->nb_samples;
        }
        else
        {
            return false;
        }
    }

    if (aFrame)
    {
        AVRational rational;
        rational.num = 1;
        rational.den = aCodecCtx->sample_rate;
        aFrame->pts = av_rescale_q(ost->samples_count, rational, aCodecCtx->time_base);
        ost->samples_count += aFrame->nb_samples;
    }

    /* send the frame for encoding */
    int ret = avcodec_send_frame(aCodecCtx, aFrame);
    if (ret < 0)
    {
        fprintf(stderr, "Error sending the frame to the audio encoder\n");
        return false;
    }

    /* read all the available output packets (in general there may be any
     * number of them */
    while (ret >= 0)
    {
        ret = avcodec_receive_packet(aCodecCtx, packet);

        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF || ret < 0)
        {
            char errstr[AV_ERROR_MAX_STRING_SIZE] = {0};
            av_make_error_string(errstr, AV_ERROR_MAX_STRING_SIZE, ret);
            QString logStr = QString("!!!!!!!!!! Error encoding audio frame: %1 ret=%2")
                        .arg(QString(errstr))
                        .arg(ret);
            AppConfig::WriteLog(logStr);
            return false;
        }

#if 0 ///写入aac文件
        uint8_t * aac_buf = (uint8_t *)malloc(packet->size+7);
        addADTStoPacket(aac_buf, 7+packet->size);
        memcpy(aac_buf+7, packet->data, packet->size);
        static FILE *aacFp = fopen("out22.aac", "wb");
        fwrite(aac_buf,1,packet->size+7,aacFp);
#endif

        ////
        /* rescale output packet timestamp values from codec to stream timebase */
        av_packet_rescale_ts(&pkt, aCodecCtx->time_base, ost->st->time_base);
        pkt.stream_index = ost->st->index;

//        audio_pts = pkt.pts;

        ///由于MP4文件的时间基不是1/1000，因此这里转成毫秒的形式，方便显示和计算。
        ///将Pts转换成毫秒的形式，这里pts仅仅用于显示，不会修改写入文件的pts
        audio_pts = av_rescale_q(pkt.pts, ost->st->time_base, {1, 1000});

        /* Write the compressed frame to the media file. */
        ret = av_interleaved_write_frame(oc, &pkt);
        if (ret < 0)
        {
            char errstr[AV_ERROR_MAX_STRING_SIZE] = {0};
            av_make_error_string(errstr, AV_ERROR_MAX_STRING_SIZE, ret);
            QString logStr = QString("!!!!!!!!!! Error while writing audio frame: %1 ret=%2")
                        .arg(QString(errstr))
                        .arg(ret);
            AppConfig::WriteLog(logStr);
        }

        av_packet_unref(packet);
        break;
    }

    return true;

}

void VideoFileWriter::close_audio(AVFormatContext *oc, OutputStream *ost)
{
    avcodec_free_context(&ost->enc);
    av_frame_free(&ost->frame);

    if (ost->tmp_frame != nullptr)
    av_frame_free(&ost->tmp_frame);

    if (ost->frameBuffer != NULL)
    {
        av_free(ost->frameBuffer);
        ost->frameBuffer = NULL;
    }
}


/* add a video output stream */
void VideoFileWriter::add_video_stream(OutputStream *ost, AVFormatContext *oc,
                       AVCodec **codec,
                       enum AVCodecID codec_id)
{
    AVCodecContext *c;

    /* find the video encoder */
    *codec = avcodec_find_encoder(codec_id);
    if (!codec) {
        fprintf(stderr, "codec not found\n");
        exit(1);
    }

    ost->st = avformat_new_stream(oc, NULL);
    if (!ost->st) {
        fprintf(stderr, "Could not alloc stream\n");
        exit(1);
    }

    ost->st->id = oc->nb_streams-1;

    c = avcodec_alloc_context3(*codec);
    if (!c) {
        fprintf(stderr, "Could not alloc an encoding context\n");
        exit(1);
    }
    ost->enc = c;

    c->codec_id = codec_id;

qDebug()<<__FUNCTION__<<c<<c->codec<<c->codec_id<<codec_id;

    /* resolution must be a multiple of two */
    c->width = WIDTH;
    c->height = HEIGHT;
    /* time base: this is the fundamental unit of time (in seconds) in terms
       of which frame timestamps are represented. for fixed-fps content,
       timebase should be 1/framerate and timestamp increments should be
       identically 1. */
//    c->time_base.den = m_videoFrameRate;
//    c->time_base.num = 1;
//    c->gop_size = 12; /* emit one intra frame every twelve frames at most */
    c->gop_size = mVideoFrameRate;
    c->pix_fmt = AV_PIX_FMT_YUV420P;

//    视频编码器常用的码率控制方式包括abr(平均码率)，crf(恒定码率)，cqp(恒定质量)，
//    ffmpeg中AVCodecContext显示提供了码率大小的控制参数，但是并没有提供其他的控制方式。
//    ffmpeg中码率控制方式分为以下几种情况：
//    1.如果设置了AVCodecContext中bit_rate的大小，则采用abr的控制方式；
//    2.如果没有设置AVCodecContext中的bit_rate，则默认按照crf方式编码，crf默认大小为23（此值类似于qp值，同样表示视频质量）；
//    3.如果用户想自己设置，则需要借助av_opt_set函数设置AVCodecContext的priv_data参数。下面给出三种控制方式的实现代码：

    c->bit_rate = mBitRate;

#if 0
    ///平均码率
    //目标的码率，即采样的码率；显然，采样码率越大，视频大小越大
    c->bit_rate = mBitRate;

#elif 1
    ///恒定码率
//    量化比例的范围为0~51，其中0为无损模式，23为缺省值，51可能是最差的。该数字越小，图像质量越好。从主观上讲，18~28是一个合理的范围。18往往被认为从视觉上看是无损的，它的输出视频质量和输入视频一模一样或者说相差无几。但从技术的角度来讲，它依然是有损压缩。
//    若Crf值加6，输出码率大概减少一半；若Crf值减6，输出码率翻倍。通常是在保证可接受视频质量的前提下选择一个最大的Crf值，如果输出视频质量很好，那就尝试一个更大的值，如果看起来很糟，那就尝试一个小一点值。
//    av_opt_set(c->priv_data, "crf", "31.000", AV_OPT_SEARCH_CHILDREN);

#else
    ///qp的值和crf一样
//    av_opt_set(c->priv_data, "qp", "31.000",AV_OPT_SEARCH_CHILDREN);
#endif

#if TEST_OPT

    av_opt_set(pCodecCtx,"b","400000",0);		//bitrate
    //Another method
    //av_opt_set_int(pCodecCtx,"b",400000,0);	//bitrate

    av_opt_set(pCodecCtx,"time_base","1/25",0);	//time_base
    av_opt_set(pCodecCtx,"bf","5",0);			//max b frame
    av_opt_set(pCodecCtx,"g","25",0);			//gop
    av_opt_set(pCodecCtx,"qmin","10",0);		//qmin/qmax
    av_opt_set(pCodecCtx,"qmax","51",0);

#else

//    pCodecCtx->time_base.num = 1;
//    pCodecCtx->time_base.den = 25;
//    pCodecCtx->max_b_frames=5;
//    pCodecCtx->bit_rate = 400000;
//    pCodecCtx->gop_size=25;
//    pCodecCtx->qmin = 10;
//    pCodecCtx->qmax = 51;

    /* time base: this is the fundamental unit of time (in seconds) in terms
       of which frame timestamps are represented. for fixed-fps content,
       timebase should be 1/framerate and timestamp increments should be
       identically 1. */
    ost->st->time_base = { 1, mVideoFrameRate };
    c->time_base       = ost->st->time_base;
//    c->gop_size = 12; /* emit one intra frame every twelve frames at most */
    c->gop_size = mVideoFrameRate * 2; ///I帧间隔

//    //固定允许的码率误差，数值越大，视频越小
//    c->bit_rate_tolerance = mBitRate;

//    //H264 还可以设置很多参数 自行研究吧
////    pCodecCtx->me_range = 16;
////    pCodecCtx->max_qdiff = 1;
//    c->qcompress = 0.85;


    c->qmin = 16+(10-mQuality)*2;
    c->qmax = 31+(10-mQuality)*2;

////    //采用（qmin/qmax的比值来控制码率，1表示局部采用此方法，0表示全局）
////    c->rc_qsquish = 0;

////    //因为我们的量化系数q是在qmin和qmax之间浮动的，
////    //qblur表示这种浮动变化的变化程度，取值范围0.0～1.0，取0表示不削减
////    c->qblur = 1.0;

//std::cout<<"mBitRate"<<mBitRate<<m_videoFrameRate;

////    ///b_frame_strategy
////    ///如果为true，则自动决定什么时候需要插入B帧，最高达到设置的最大B帧数。
////    ///如果设置为false,那么最大的B帧数被使用。
////    c->b_frame_strategy = 1;
////    c->max_b_frames = 5;

#endif


    if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
        /* just for testing, we also add B frames */
        c->max_b_frames = 2;
    }
    if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO){
        /* Needed to avoid using macroblocks in which some coeffs overflow.
           This does not happen with normal video, it just happens here as
           the motion of the chroma plane does not match the luma plane. */
        c->mb_decision=2;
    }

}

static AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
{
    AVFrame *picture;
    int ret;

    picture = av_frame_alloc();
    if (!picture)
        return NULL;

    picture->format = pix_fmt;
    picture->width  = width;
    picture->height = height;

    /* allocate the buffers for the frame data */
    ret = av_frame_get_buffer(picture, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate frame data.\n");
        exit(1);
    }

    return picture;
}

void VideoFileWriter::open_video(AVFormatContext *oc, AVCodec *codec, OutputStream *ost)
{
    AVCodecContext *c = ost->enc;

    // Set Option
    AVDictionary *param = 0;
    //H.264
    //av_dict_set(&param, "preset", "slow", 0);
    av_dict_set(&param, "preset", "superfast", 0);
    av_dict_set(&param, "tune", "zerolatency", 0);  //实现实时编码

    c->thread_count = 16;
    c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

qDebug()<<__FUNCTION__<<"333";
    int ret = 0;
    if (ret = avcodec_open2(c, codec,&param) < 0)
    {
        char errstr[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_make_error_string(errstr, AV_ERROR_MAX_STRING_SIZE, ret);
        fprintf(stderr, "Failed to open video encoder!: %s \n", errstr);
      exit(1);
    }
qDebug()<<__FUNCTION__<<"333"<<c->pix_fmt<<AV_PIX_FMT_YUV420P;

    /* allocate the encoded raw picture */
    {
        ost->frame = av_frame_alloc();

        ost->frame->format = c->pix_fmt;
        ost->frame->width  = c->width;
        ost->frame->height = c->height;

        int numBytes_yuv = avpicture_get_size(AV_PIX_FMT_YUV420P, c->width,c->height);

        uint8_t * out_buffer_yuv = (uint8_t *) av_malloc(numBytes_yuv * sizeof(uint8_t));

        avpicture_fill((AVPicture *) ost->frame, out_buffer_yuv, AV_PIX_FMT_YUV420P,
                c->width, c->height);

        ost->frameBuffer = out_buffer_yuv;
        ost->frameBufferSize = numBytes_yuv;

//        /* allocate the buffers for the frame data */
//        int ret = av_frame_get_buffer(ost->frame, 0);
//        if (ret < 0) {
//            fprintf(stderr, "Could not allocate frame data.\n");
//            exit(1);
//        }

    }
//    ost->frame = alloc_picture(c->pix_fmt, c->width, c->height);
//    if (!ost->frame) {
//        fprintf(stderr, "Could not allocate picture\n");
//        exit(1);
//    }

    /* If the output format is not YUV420P, then a temporary YUV420P
     * picture is needed too. It is then converted to the required
     * output format. */
    ost->tmp_frame = NULL;
    if (c->pix_fmt != AV_PIX_FMT_YUV420P) {
        ost->tmp_frame = alloc_picture(AV_PIX_FMT_YUV420P, c->width, c->height);
        if (!ost->tmp_frame) {
            fprintf(stderr, "Could not allocate temporary picture\n");
            exit(1);
        }
    }

    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(ost->st->codecpar, c);
    if (ret < 0) {
        fprintf(stderr, "Could not copy the stream parameters\n");
        exit(1);
    }

}

bool VideoFileWriter::write_video_frame(AVFormatContext *oc, OutputStream *ost, int64_t time, bool &isNeedNewFile)
{
    isNeedNewFile = false;

    int ret = 0;
    AVCodecContext *c;

    c = ost->enc;

    VideoRawFramePtr yuvFrame = readYuvFrame(time);

    if (yuvFrame == nullptr)
    {
        return false;
    }
    else
    {
        mLastVideoFrame = yuvFrame;

        ost->frame->pts = ost->next_pts++;
    }

    ///分辨率发生改变的时候，重新生成一个新的视频文件
    if (yuvFrame->getWidth() != WIDTH || yuvFrame->getHeight() != HEIGHT)
    {
qDebug()<<__FUNCTION__<<"new file:"<<time<<yuvFrame->getWidth()<<yuvFrame->getHeight()<<WIDTH<<HEIGHT;
//        this->setWidth(yuvFrame->getWidth(), yuvFrame->getHeight());
        WIDTH = yuvFrame->getWidth();
        HEIGHT = yuvFrame->getHeight();
        isNeedNewFile = true;
        return false;
    }

    std::list<VideoEncodedFramePtr> encodedVideoFrameList = mVideoEncoder->encode(mLastVideoFrame, ost->frame->pts);

    if (encodedVideoFrameList.size() > 1)
    qDebug()<<__FUNCTION__<<"1111"<<time<<encodedVideoFrameList.size();

    if (encodedVideoFrameList.size() > 0)
    {
        AVPacket *pkt = mVideoEncoder->getLastEncodePacket();

        /* rescale output packet timestamp values from codec to stream timebase */
        av_packet_rescale_ts(pkt, c->time_base, ost->st->time_base);
        pkt->stream_index = ost->st->index;

        video_pts = pkt->pts;

        ///由于MP4文件的时间基不是1/1000，因此这里转成毫秒的形式，方便显示和计算。
        ///将Pts转换成毫秒的形式，这里pts仅仅用于显示，不会修改写入文件的pts
        video_pts = av_rescale_q(pkt->pts, ost->st->time_base, {1, 1000});

        /* Write the compressed frame to the media file. */
        ret = av_interleaved_write_frame(oc, pkt);
        if (ret < 0)
        {
            char errstr[AV_ERROR_MAX_STRING_SIZE] = {0};
            av_make_error_string(errstr, AV_ERROR_MAX_STRING_SIZE, ret);
            QString logStr = QString("!!!!!!!!!! Error while writing video frame: %1 ret=%2")
                        .arg(QString(errstr))
                        .arg(ret);
            AppConfig::WriteLog(logStr);
        }

//        av_packet_unref(&pkt);
    }

    for (VideoEncodedFramePtr framePtr : encodedVideoFrameList)
    {
        if (mCallBackFunc != nullptr)
        {
            mCallBackFunc(framePtr, mCallBackFuncParam);
        }
    }

    return true;

}

void VideoFileWriter::close_video(AVFormatContext *oc, OutputStream *ost)
{
    avcodec_free_context(&ost->enc);
    av_frame_free(&ost->frame);

    if (ost->tmp_frame != NULL)
        av_frame_free(&ost->tmp_frame);

    if (ost->frameBuffer != NULL)
    {
        av_free(ost->frameBuffer);
        ost->frameBuffer = NULL;
    }

}

int64_t VideoFileWriter::getVideoPts()
{
    return video_pts + mLastFileVideoPts;
}

int64_t VideoFileWriter::getAudioPts()
{
    return audio_pts;
}

void VideoFileWriter::sig_StartWriteFile(const std::string & filePath)
{
    if (AppConfig::gIsDebugMode)
    {
        AppConfig::WriteLog(QString("%1 startWriteFile %1... \n").arg(__FUNCTION__).arg(QString::fromStdString(filePath)));
    }
}

void VideoFileWriter::sig_StopWriteFile(const std::string & filePath)
{
    if (AppConfig::gIsDebugMode)
    {
        AppConfig::WriteLog(QString("%1 stopWriteFile %1... \n").arg(__FUNCTION__).arg(QString::fromStdString(filePath)));
    }
}

void VideoFileWriter::run()
{
    mIsThreadRunning = true;

    mLastFileVideoPts = 0;
    mLastVideoFrame = nullptr;

    int writeFileIndex = 0;

    mVideoFileList.clear();

while(1)
{
    if (mIsStop)
    {
        break;
    }

    if (WIDTH <= 0 || HEIGHT <= 0)
    {
        MoudleConfig::mSleep(100);
        continue;
    }

    int currentVideoWidth = WIDTH;
    int currentVideoHeight = HEIGHT;

    std::string filePath;

    if (writeFileIndex > 0)
    {
        QFileInfo fileInfo(QString::fromStdString(mFilePath));

//        filePath = QString("%1/%2_%3.%4")
//                .arg(fileInfo.absoluteDir().path())
//                .arg(fileInfo.baseName())
//                .arg(writeFileIndex)
//                .arg(fileInfo.suffix());

        filePath = QString("%1/%2_%3.%4")
                .arg(fileInfo.absoluteDir().path())
                .arg(fileInfo.fileName())
                .arg(writeFileIndex)
                .arg(fileInfo.suffix()).toStdString();

    }
    else
    {
        filePath = mFilePath;
    }

    sig_StartWriteFile(filePath);

    const char *filename = filePath.c_str();
    writeFileIndex++;

    OutputStream video_st = { 0 }, audio_st = { 0 };
    AVOutputFormat *fmt;
    AVFormatContext *oc;
    AVCodec *audio_codec, *video_codec;
    int have_video = 0, have_audio = 0;

    int i;

    /* allocate the output media context */
    avformat_alloc_output_context2(&oc, NULL, NULL, filename);
    if (!oc)
    {
        printf("Could not deduce output format from file extension: using MPEG.\n");
        avformat_alloc_output_context2(&oc, NULL, "mpeg", filename);
    }

    if (!oc)
    {
        fprintf(stderr,"Could not deduce output format from file extension: using MPEG.\n");

//        QString logStr = QString("!!!!!!!!!! Could not deduce output format from file extension ... %1").arg(filePath);
//        AppConfig::WriteLog(logStr);

        MoudleConfig::mSleep(1000);

        continue;
    }

    fmt = oc->oformat;

    if (m_containsVideo)
    {
//        qDebug()<<__FUNCTION__<<"fmt->video_codec:"<<fmt->video_codec<<AV_CODEC_ID_H264<<WIDTH<<HEIGHT;
        if (fmt->video_codec != AV_CODEC_ID_NONE)
        {
            add_video_stream(&video_st, oc, &video_codec, AV_CODEC_ID_H264);
            have_video = 1;
        }
        qDebug()<<"333";
    }

    if (m_containsAudio)
    {
        if (fmt->audio_codec != AV_CODEC_ID_NONE)
        {
            add_audio_stream(&audio_st, oc, &audio_codec, AV_CODEC_ID_AAC);
            have_audio = 1;
        }
    }

    /* Now that all the parameters are set, we can open the audio and
     * video codecs and allocate the necessary encode buffers. */
    if (have_video)
        open_video(oc, video_codec, &video_st);

    if (have_audio)
        open_audio(oc, audio_codec, &audio_st);

    av_dump_format(oc, 0, filename, 1);

    /* open the output file, if needed */
    if (!(fmt->flags & AVFMT_NOFILE))
    {
        if (avio_open(&oc->pb, filename, AVIO_FLAG_WRITE) < 0)
        {
            qDebug()<<"Could not open "<<filename;
//            return;

//            QString logStr = QString("!!!!!!!!!! Could not open %1").arg(filePath);
//            AppConfig::WriteLog(logStr);

            MoudleConfig::mSleep(1000);

            continue;

        }
    }

    /* write the stream header, if any */
//    av_write_header(oc);
    avformat_write_header(oc, NULL);

    mLastGetAudioTime = av_gettime();

    video_pts = 0;
    audio_pts = 0;

    while(1)
    {
//        qDebug()<<__FUNCTION__<<video_st.next_pts<<audio_st.next_pts<<video_pts<<audio_pts;

        /* select the stream to encode */
        if (!have_audio || (av_compare_ts(video_st.next_pts, video_st.enc->time_base, audio_st.next_pts, audio_st.enc->time_base) <= 0))
        {
            bool isNeedNewFile = false;

//            if (!write_video_frame(oc, &video_st, video_pts+mLastFileVideoPts, isNeedNewFile))
//            {
//                if (mIsStop || isNeedNewFile)
//                {
//                    if (isNeedNewFile)
//                    {
//                        mLastFileVideoPts += video_pts;
//                    }
//                    break;
//                }
//                MoudleConfig::mSleep(1);
//            }
            if (!write_video_frame(oc, &video_st, video_pts+mLastFileVideoPts, isNeedNewFile))
            {
                if (mIsStop)
                {
                    break;
                }
                MoudleConfig::mSleep(1);
            }
        }
        else
        {
            if (!write_audio_frame(oc, &audio_st))
            {
                if (mIsStop)
                {
                    break;
                }
                MoudleConfig::mSleep(1);
            }
        }
    }

//    QString logStr = QString("!!!!!!!!!! av_write_trailer ... %1").arg(filePath);
//    AppConfig::WriteLog(logStr);

    av_write_trailer(oc);

//    logStr = QString("!!!!!!!!!! av_write_trailer finised! %1").arg(filePath);
//    AppConfig::WriteLog(logStr);

    sig_StopWriteFile(filePath);

    qDebug()<<"void RTMPPushThread::run() finished!";

    /* close each codec */
    if (have_video)
        close_video(oc, &video_st);
    if (have_audio)
        close_audio(oc, &audio_st);

    /* free the streams */
    for(i = 0; i < oc->nb_streams; i++) {
        av_freep(&oc->streams[i]->codec);
        av_freep(&oc->streams[i]);
    }

    if (!(fmt->flags & AVFMT_NOFILE)) {
        /* close the output file */
        avio_close(oc->pb);
    }

    /* free the stream */
    avformat_free_context(oc);

    VideoFileInfo fileInfo;
    fileInfo.filePath = filePath;
    fileInfo.length   = video_pts;
    fileInfo.width = currentVideoWidth;
    fileInfo.height = currentVideoHeight;

    mVideoFileList.push_back(fileInfo);

    mVideoEncoder->closeEncoder();

    video_pts = 0;
    audio_pts = 0;

//    mLastVideoFrame = nullptr;

//    clearYuvFrame();
//    clearPcmFrame();

}

    mLastFileVideoPts = 0;
    mLastVideoFrame = nullptr;

    clearYuvFrame();
    clearPcmFrame();

    qDebug()<<"void RTMPPushThread::run() finished! 222";

    mIsThreadRunning = false;
}

void VideoFileWriter::setWidth(int width,int height)
{
    if (!mIsThreadRunning)
    {
        WIDTH = width;
        HEIGHT = height;
    }
}

bool VideoFileWriter::isThreadRunning()
{
    return mIsThreadRunning;
}

bool VideoFileWriter::startEncode(std::function<void (VideoEncodedFramePtr videoFramePtr, void *param)> func, void *param)
{
    mIsStop = false;

    mCallBackFunc      = func;
    mCallBackFuncParam = param;

    std::thread([=]
    {
        this->run();

    }).detach();

    return true;
}

bool VideoFileWriter::stopEncode()
{
    mIsStop = true;

    while(mIsThreadRunning)
    {
        AppConfig::mSleep(100);
    }

    return true;
}
