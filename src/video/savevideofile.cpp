#include "savevideofile.h"
#include "AppConfig.h"

#include <QFileInfo>
#include <QDir>

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

SaveVideoFileThread::SaveVideoFileThread()
{
    isStop = false;

    m_containsVideo = true;
    m_containsAudio = true;

    videoDataQueneHead = NULL;
    videoDataQueneTail = NULL;

    AudioDataQueneHead = NULL;
    AudioDataQueneTail = NULL;

    videoBufferCount = 0;
    audioBufferCount = 0;

    m_videoFrameRate = 15;

    lastVideoNode = NULL;

    mBitRate = 450000;

    audio_pts = 0;
    video_pts = 0;

}

SaveVideoFileThread::~SaveVideoFileThread()
{

}

void SaveVideoFileThread::setContainsVideo(bool value)
{
    m_containsVideo = value;
}

void SaveVideoFileThread::setContainsAudio(bool value)
{
    m_containsAudio = value;
}

void SaveVideoFileThread::setVideoFrameRate(int value)
{
    m_videoFrameRate = value;
}

void SaveVideoFileThread::setFileName(QString filePath)
{
    mFilePath = filePath;
}

void SaveVideoFileThread::setQuantity(int value)
{
    mBitRate = 450000 + (value - 5) * 50000;
}

void SaveVideoFileThread::videoDataQuene_Input(uint8_t * buffer, int size, int64_t time)
{
//    qDebug()<<"void SaveVideoFileThread::videoDataQuene_Input(uint8_t * buffer,int size,long time)"<<time;
    BufferDataNode * node = (BufferDataNode*)malloc(sizeof(BufferDataNode));
    node->bufferSize = size;
    node->next = NULL;
    node->time = time;

    node->buffer = buffer;
//    node->buffer = (uint8_t *)malloc(size);
//    memcpy(node->buffer,buffer,size);

    mVideoMutex.lock();

    if (videoDataQueneHead == NULL)
    {
        videoDataQueneHead = node;
    }
    else
    {
        videoDataQueneTail->next = node;
    }

    videoDataQueneTail = node;

    videoBufferCount++;

    mVideoMutex.unlock();
//qDebug()<<__FUNCTION__<<videoBufferCount<<time;
    if (videoBufferCount >= 30)
    {
        QString logStr = QString("!!!!!!!!!! encode too slow! count=%1")
                    .arg(videoBufferCount);
        AppConfig::WriteLog(logStr);
    }

}

BufferDataNode *SaveVideoFileThread::videoDataQuene_get(int64_t time)
{
    BufferDataNode * node = NULL;

    mVideoMutex.lock();

    if (videoDataQueneHead != NULL)
    {
        node = videoDataQueneHead;

//qDebug()<<"111:"<<time<<node->time<<videoBufferCount;
        if (time >= node->time)
        {
//            qDebug()<<"000";
            if (videoDataQueneHead->next != NULL)
            {
//                qDebug()<<"111";
                while(node != NULL)
                {
//                    qDebug()<<"222";
                    if (node->next == NULL)
                    {
                        videoDataQueneHead = node;
                        node = NULL;
                        break;
                    }
//qDebug()<<"333"<<time << node->next->time;
                    if (time < node->next->time)
                    {
                        break;
                    }

                    BufferDataNode * tmp = node;
//qDebug()<<"222:"<<node->time<<time;
                    node = node->next;
                    videoBufferCount--;
                    free(tmp->buffer);
                    free(tmp);
                }
            }
            else
            {
                node = NULL;
            }
        }
        else
        {
            node = lastVideoNode;
        }

        if (videoDataQueneTail == node)
        {
            videoDataQueneTail = NULL;
        }

        if (node != NULL && node != lastVideoNode)
        {
            videoDataQueneHead = node->next;
            videoBufferCount--;
        }

    }
//    qDebug()<<__FUNCTION__<<videoBufferCount<<node;
    mVideoMutex.unlock();

    return node;
}

void SaveVideoFileThread::audioDataQuene_Input(uint8_t * buffer,int size)
{
    BufferDataNode * node = (BufferDataNode*)malloc(sizeof(BufferDataNode));
//    node->buffer = buffer;
    node->bufferSize = size;
    node->next = NULL;

    node->buffer = (uint8_t *)malloc(size);
    memcpy(node->buffer,buffer,size);

    mAudioMutex.lock();

    if (AudioDataQueneHead == NULL)
    {
        AudioDataQueneHead = node;
    }
    else
    {
        AudioDataQueneTail->next = node;
    }

    AudioDataQueneTail = node;

    audioBufferCount++;
//qDebug()<<__FUNCTION__<<audioBufferCount<<size;
    mAudioMutex.unlock();

}

BufferDataNode *SaveVideoFileThread::audioDataQuene_get()
{
    BufferDataNode * node = NULL;

    mAudioMutex.lock();

    if (AudioDataQueneHead != NULL)
    {
        node = AudioDataQueneHead;

        if (AudioDataQueneTail == AudioDataQueneHead)
        {
            AudioDataQueneTail = NULL;
        }

        AudioDataQueneHead = AudioDataQueneHead->next;

        audioBufferCount--;

    }
//qDebug()<<__FUNCTION__<<audioBufferCount;

    mAudioMutex.unlock();

    return node;
}


/*
 * add an audio output stream
 */
void SaveVideoFileThread::add_audio_stream(OutputStream *ost, AVFormatContext *oc,
                                                AVCodec **codec,
                                                enum AVCodecID codec_id)
{
    AVCodecContext *c;
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
    c = avcodec_alloc_context3(*codec);
    if (!c) {
        fprintf(stderr, "Could not alloc an encoding context\n");
        exit(1);
    }
    ost->enc = c;

    c->sample_fmt  = (*codec)->sample_fmts ? (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
    c->bit_rate    = 64000;
    c->sample_rate = 44100;

    if ((*codec)->supported_samplerates) {
        c->sample_rate = (*codec)->supported_samplerates[0];
        for (i = 0; (*codec)->supported_samplerates[i]; i++) {
            if ((*codec)->supported_samplerates[i] == 44100)
                c->sample_rate = 44100;
        }
    }
    c->channels        = av_get_channel_layout_nb_channels(c->channel_layout);
    c->channel_layout = AV_CH_LAYOUT_STEREO;
    if ((*codec)->channel_layouts)
    {
        c->channel_layout = (*codec)->channel_layouts[0];
        for (i = 0; (*codec)->channel_layouts[i]; i++)
        {
            if ((*codec)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
                c->channel_layout = AV_CH_LAYOUT_STEREO;
        }
    }
    c->channels        = av_get_channel_layout_nb_channels(c->channel_layout);
    ost->st->time_base.num = 1; // = (AVRational){ 1, c->sample_rate };
    ost->st->time_base.den = c->sample_rate;

//qDebug()<<__FUNCTION__<<c<<c->codec<<c->codec_id;
//qDebug()<<__FUNCTION__<<ost->enc<<ost->enc->codec<<ost->enc->codec_id;
//    c->codec_id = codec_id;
//    c->codec_type = AVMEDIA_TYPE_AUDIO;

//    /* put sample parameters */
//    c->sample_fmt = AV_SAMPLE_FMT_S16;
//    c->bit_rate = 64000;
//    c->sample_rate = 44100;
//    c->channels = 2;
//qDebug()<<__FUNCTION__<<ost->enc<<ost->enc->codec<<ost->enc->codec_id;
////    c->bit_rate = 9600;
////    c->sample_rate = 11025;
////    c->channels = 1;

}

static AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt,
                                  uint64_t channel_layout,
                                  int sample_rate, int nb_samples)
{
    AVFrame *frame = av_frame_alloc();
    int ret;

    if (!frame) {
        fprintf(stderr, "Error allocating an audio frame\n");
        exit(1);
    }

    frame->format = sample_fmt;
    frame->channel_layout = channel_layout;
    frame->sample_rate = sample_rate;
    frame->nb_samples = nb_samples;

    if (nb_samples) {
        ret = av_frame_get_buffer(frame, 0);
        if (ret < 0) {
            fprintf(stderr, "Error allocating an audio buffer\n");
            exit(1);
        }
    }

    return frame;
}

void SaveVideoFileThread::open_audio(AVFormatContext *oc, AVCodec *codec, OutputStream *ost)
{
    AVCodecContext *c;
    int nb_samples;
    int ret;

    c = ost->enc;

    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        qDebug("could not open codec\n");
        exit(1);
    }

    /* init signal generator */
    ost->t     = 0;
    ost->tincr = 2 * M_PI * 110.0 / c->sample_rate;
    /* increment frequency by 110 Hz per second */
    ost->tincr2 = 2 * M_PI * 110.0 / c->sample_rate / c->sample_rate;

//    if (c->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
//        nb_samples = 10000;
//    else
//        nb_samples = c->frame_size;

    if (codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
        nb_samples = 10000;
    else
        nb_samples = c->frame_size;

    ost->frame     = alloc_audio_frame(c->sample_fmt, c->channel_layout,
                                       c->sample_rate, nb_samples);
    ost->tmp_frame = alloc_audio_frame(AV_SAMPLE_FMT_S16, c->channel_layout,
                                       c->sample_rate, nb_samples);

    ost->frameBuffer = (uint8_t *) av_malloc(AVCODEC_MAX_AUDIO_FRAME_SIZE);
    ost->frameBufferSize = 0;

    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(ost->st->codecpar, c);
    if (ret < 0) {
        fprintf(stderr, "Could not copy the stream parameters\n");
        exit(1);
    }


    /* create resampler context */
    ost->swr_ctx = swr_alloc();
    if (!ost->swr_ctx) {
        fprintf(stderr, "Could not allocate resampler context\n");
        exit(1);
    }

    /* set options */
    av_opt_set_int       (ost->swr_ctx, "in_channel_count",   c->channels,       0);
    av_opt_set_int       (ost->swr_ctx, "in_sample_rate",     c->sample_rate,    0);
    av_opt_set_sample_fmt(ost->swr_ctx, "in_sample_fmt",      AV_SAMPLE_FMT_S16, 0);
    av_opt_set_int       (ost->swr_ctx, "out_channel_count",  c->channels,       0);
    av_opt_set_int       (ost->swr_ctx, "out_sample_rate",    c->sample_rate,    0);
    av_opt_set_sample_fmt(ost->swr_ctx, "out_sample_fmt",     c->sample_fmt,     0);

    /* initialize the resampling context */
    if ((ret = swr_init(ost->swr_ctx)) < 0) {
        fprintf(stderr, "Failed to initialize the resampling context\n");
        exit(1);
    }

    audio_input_frame_size = nb_samples * 4;

}

/* Prepare a 16 bit dummy audio frame of 'frame_size' samples and
 * 'nb_channels' channels. */
static AVFrame *get_audio_frame(OutputStream *ost)
{
    AVFrame *frame = ost->tmp_frame;
    int j, i, v;
    int16_t *q = (int16_t*)frame->data[0];

    for (j = 0; j <frame->nb_samples; j++) {
        v = (int)(sin(ost->t) * 10000);
        for (i = 0; i < ost->enc->channels; i++)
            *q++ = v;
        ost->t     += ost->tincr;
        ost->tincr += ost->tincr2;
    }

    frame->pts = ost->next_pts;
    ost->next_pts  += frame->nb_samples;

    return frame;
}

/*
 * encode one audio frame and send it to the muxer
 * return 1 when encoding is finished, 0 otherwise
 */
//static int write_audio_frame(AVFormatContext *oc, OutputStream *ost)
bool SaveVideoFileThread::write_audio_frame(AVFormatContext *oc, OutputStream *ost)
{
    AVCodecContext *c;
    AVPacket pkt = { 0 }; // data and size must be 0;
    AVFrame *frame;
    int ret = 0;
    int got_packet;
    int dst_nb_samples;

    c = ost->enc;

#if 1
    BufferDataNode *node = audioDataQuene_get();

    if (node == NULL)
    {
        return false;
    }

    frame = ost->frame;
//        memset(frame->data[0], 0x0, frame->nb_samples);
    memcpy(frame->data[0], node->buffer, frame->nb_samples);
//            memcpy(frame->data[0], node->buffer, node->bufferSize);
    free(node->buffer);
    free(node);

    frame->pts = ost->next_pts;
    ost->next_pts  += frame->nb_samples;

#else
    frame = get_audio_frame(ost); //自动生成音频数据
#endif

    if (frame)
    {
        /* convert samples from native format to destination codec format, using the resampler */
            /* compute destination number of samples */
            dst_nb_samples = av_rescale_rnd(swr_get_delay(ost->swr_ctx, c->sample_rate) + frame->nb_samples,
                                            c->sample_rate, c->sample_rate, AV_ROUND_UP);
            av_assert0(dst_nb_samples == frame->nb_samples);

//        /* when we pass a frame to the encoder, it may keep a reference to it
//         * internally;
//         * make sure we do not overwrite it here
//         */
//        ret = av_frame_make_writable(ost->frame);
//        if (ret < 0)
//            exit(1);

        /* convert to destination format */
        ret = swr_convert(ost->swr_ctx,
                          ost->frame->data, dst_nb_samples,
                          (const uint8_t **)frame->data, frame->nb_samples);
        if (ret < 0)
        {
            char errstr[AV_ERROR_MAX_STRING_SIZE] = {0};
            av_make_error_string(errstr, AV_ERROR_MAX_STRING_SIZE, ret);
            QString logStr = QString("!!!!!!!!!! Error while converting: %1 ret=%2")
                        .arg(QString(errstr))
                        .arg(ret);
            AppConfig::WriteLog(logStr);
        }
        frame = ost->frame;

        AVRational rational;
        rational.num = 1;
        rational.den = c->sample_rate;
        frame->pts = av_rescale_q(ost->samples_count, rational, c->time_base);
        ost->samples_count += dst_nb_samples;
    }

    ret = avcodec_encode_audio2(c, &pkt, frame, &got_packet);

    if (ret < 0)
    {
        char errstr[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_make_error_string(errstr, AV_ERROR_MAX_STRING_SIZE, ret);
        QString logStr = QString("!!!!!!!!!! Error encoding audio frame: %1 ret=%2")
                    .arg(QString(errstr))
                    .arg(ret);
        AppConfig::WriteLog(logStr);
    }

    if (got_packet)
    {
        /* rescale output packet timestamp values from codec to stream timebase */
        av_packet_rescale_ts(&pkt, c->time_base, ost->st->time_base);
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

        av_packet_unref(&pkt);
    }

//qDebug()<<__FUNCTION__<<"1111 99999";

    if (ret < 0)
    {
        return false;
    }
    else
    {
        return (frame || got_packet) ? 0 : 1;
    }

}

void SaveVideoFileThread::close_audio(AVFormatContext *oc, OutputStream *ost)
{
    avcodec_free_context(&ost->enc);
    av_frame_free(&ost->frame);
    av_frame_free(&ost->tmp_frame);
    sws_freeContext(ost->sws_ctx);
    swr_free(&ost->swr_ctx);

    if (ost->frameBuffer != NULL)
    {
        av_free(ost->frameBuffer);
        ost->frameBuffer = NULL;
    }
}


/* add a video output stream */
void SaveVideoFileThread::add_video_stream(OutputStream *ost, AVFormatContext *oc,
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
    c->gop_size = m_videoFrameRate;
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
    ost->st->time_base = { 1, m_videoFrameRate };
    c->time_base       = ost->st->time_base;
//    c->gop_size = 12; /* emit one intra frame every twelve frames at most */
    c->gop_size = m_videoFrameRate * 2; ///I帧间隔

//    //固定允许的码率误差，数值越大，视频越小
//    c->bit_rate_tolerance = mBitRate;

//    //H264 还可以设置很多参数 自行研究吧
////    pCodecCtx->me_range = 16;
////    pCodecCtx->max_qdiff = 1;
//    c->qcompress = 0.85;
    c->qmin = 16;
    c->qmax = 31;

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

void SaveVideoFileThread::open_video(AVFormatContext *oc, AVCodec *codec, OutputStream *ost)
{
    AVCodecContext *c = ost->enc;

    // Set Option
    AVDictionary *param = 0;
    //H.264
    //av_dict_set(&param, "preset", "slow", 0);
    av_dict_set(&param, "preset", "superfast", 0);
    av_dict_set(&param, "tune", "zerolatency", 0);  //实现实时编码

    c->thread_count = 16;

qDebug()<<__FUNCTION__<<"333";
    int ret = 0;
    if (ret = avcodec_open2(c, codec,&param) < 0){
      qDebug()<<("Failed to open video encoder!\n")<<ret;
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

bool SaveVideoFileThread::write_video_frame(AVFormatContext *oc, OutputStream *ost, double time)
{
    int out_size, ret = 0;
    AVCodecContext *c;
    int got_packet = 0;

    c = ost->enc;
//qDebug()<<__FUNCTION__<<"0000"<<time;
    BufferDataNode *node = videoDataQuene_get(time);

    if (node == NULL)
    {
//        qDebug()<<__FUNCTION__<<"0000 000"<<time;
        return false;
    }
    else
    {
        if (node != lastVideoNode)
        {
            if (lastVideoNode != NULL)
            {
                free(lastVideoNode->buffer);
                free(lastVideoNode);
            }

            lastVideoNode = node;
        }

        memcpy(ost->frameBuffer, node->buffer, node->bufferSize);

//不止为何下面这两种方式都不行
//        int y_size = c->width * c->height;
//        memcpy(ost->frame->data[0], node->buffer, y_size * 3 / 2);

//        memcpy(ost->frame->data[0], node->buffer, y_size);
//        memcpy(ost->frame->data[1], node->buffer+ y_size, y_size*5/4);
//        memcpy(ost->frame->data[2], node->buffer+ y_size*5/4, y_size*5/4);
//qDebug()<<__FUNCTION__<<"000 1111";

        ost->frame->pts = ost->next_pts++;

    }
//qDebug()<<__FUNCTION__<<"1111";

    AVPacket pkt = { 0 };
//    av_init_packet(&pkt);

    /* encode the image */
    out_size = avcodec_encode_video2(c, &pkt, ost->frame, &got_packet);

    if (got_packet)
    {
//qDebug()<<__FUNCTION__<<"111"<<ost->frame->pts<<pkt.pts<<c->time_base.num<<c->time_base.den<<ost->st->time_base.den<<ost->st->time_base.num;
        /* rescale output packet timestamp values from codec to stream timebase */
        av_packet_rescale_ts(&pkt, c->time_base, ost->st->time_base);
        pkt.stream_index = ost->st->index;
//qDebug()<<__FUNCTION__<<"222"<<ost->frame->pts<<pkt.pts<<time<<node->time;

//        video_pts = pkt.pts;

        ///由于MP4文件的时间基不是1/1000，因此这里转成毫秒的形式，方便显示和计算。
        ///将Pts转换成毫秒的形式，这里pts仅仅用于显示，不会修改写入文件的pts
        video_pts = av_rescale_q(pkt.pts, ost->st->time_base, {1, 1000});


        /* Write the compressed frame to the media file. */
        ret = av_interleaved_write_frame(oc, &pkt);
        if (ret < 0)
        {
            char errstr[AV_ERROR_MAX_STRING_SIZE] = {0};
            av_make_error_string(errstr, AV_ERROR_MAX_STRING_SIZE, ret);
            QString logStr = QString("!!!!!!!!!! Error while writing video frame: %1 ret=%2")
                        .arg(QString(errstr))
                        .arg(ret);
            AppConfig::WriteLog(logStr);
        }

        av_packet_unref(&pkt);
    }

    return true;

}

void SaveVideoFileThread::close_video(AVFormatContext *oc, OutputStream *ost)
{
    avcodec_free_context(&ost->enc);
    av_frame_free(&ost->frame);
    if (ost->tmp_frame != NULL)
        av_frame_free(&ost->tmp_frame);
    sws_freeContext(ost->sws_ctx);
    swr_free(&ost->swr_ctx);

    if (ost->frameBuffer != NULL)
    {
        av_free(ost->frameBuffer);
        ost->frameBuffer = NULL;
    }

}

int64_t SaveVideoFileThread::getVideoPts()
{
    return video_pts;
}

int64_t SaveVideoFileThread::getAudioPts()
{
    return audio_pts;
}

void SaveVideoFileThread::run()
{
    int writeFileIndex = 1;

while(1)
{
    if (isStop)
    {
        break;
    }

    if (WIDTH <= 0 || HEIGHT <= 0)
    {
        msleep(100);
        continue;
    }

    QString filePath;

    if (writeFileIndex > 1)
    {
        QFileInfo fileInfo(mFilePath);

        filePath = QString("%1/%2_%3.%4")
                .arg(fileInfo.absoluteDir().path())
                .arg(fileInfo.baseName())
                .arg(writeFileIndex)
                .arg(fileInfo.suffix());
    }
    else
    {
        filePath = mFilePath;
    }

    emit sig_StartWriteFile(filePath);

    char filename[1280] = {0};
    strcpy(filename, filePath.toLocal8Bit());
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

        QString logStr = QString("!!!!!!!!!! Could not deduce output format from file extension ... %1").arg(filePath);
        AppConfig::WriteLog(logStr);

        msleep(1000);

        continue;
    }

    fmt = oc->oformat;

    if (m_containsVideo)
    {
        qDebug()<<fmt->video_codec;
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

            QString logStr = QString("!!!!!!!!!! Could not open %1").arg(filePath);
            AppConfig::WriteLog(logStr);

            msleep(1000);

            continue;

        }
    }

    /* write the stream header, if any */
//    av_write_header(oc);
    avformat_write_header(oc, NULL);

    video_pts = 0;
    audio_pts = 0;

    while(1)
    {
//        qDebug()<<video_st.next_pts<<audio_st.next_pts<<video_pts<<audio_pts;

        /* select the stream to encode */
        if (!have_audio || (av_compare_ts(video_st.next_pts, video_st.enc->time_base, audio_st.next_pts, audio_st.enc->time_base) <= 0))
        {
            if (!write_video_frame(oc, &video_st, video_pts))
            {
                if (isStop)
                {
                    break;
                }
                msleep(1);
            }
        }
        else
        {
            if (!write_audio_frame(oc, &audio_st))
            {
                if (isStop)
                {
                    break;
                }
                msleep(1);
            }
        }
    }

    QString logStr = QString("!!!!!!!!!! av_write_trailer ... %1").arg(filePath);
    AppConfig::WriteLog(logStr);

    av_write_trailer(oc);

    logStr = QString("!!!!!!!!!! av_write_trailer finised! %1").arg(filePath);
    AppConfig::WriteLog(logStr);

    emit sig_StopWriteFile(filePath);

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
}

    qDebug()<<"void RTMPPushThread::run() finished! 222";
}

void SaveVideoFileThread::setWidth(int width,int height)
{
    WIDTH = width;
    HEIGHT = height;
}

bool SaveVideoFileThread::startEncode()
{
    isStop = false;
    start();

    return true;
}

bool SaveVideoFileThread::stopEncode()
{
    isStop = true;

    return true;
}
