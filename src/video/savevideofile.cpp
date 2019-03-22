
/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "savevideofile.h"

#define STREAM_PIX_FMT PIX_FMT_YUV420P /* default pix_fmt */

static int writeVideoThreadFunc(void *arg);

SaveVideoFileThread::SaveVideoFileThread()
{
    isStop = false;

    m_containsVideo = true;
    m_containsAudio = true;

    videoMutex = SDL_CreateMutex();
    videoDataQueneHead = NULL;
    videoDataQueneTail = NULL;

    audioMutex = SDL_CreateMutex();
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

void SaveVideoFileThread::setFileName(char *str)
{
    memset(filename,0x0,128);
    strcpy(filename,str);
}

void SaveVideoFileThread::setQuantity(int value)
{
    mBitRate = 450000 + (value - 5) * 50000;
}

void SaveVideoFileThread::videoDataQuene_Input(uint8_t * buffer,int size,long time)
{
//    qDebug()<<"void SaveVideoFileThread::videoDataQuene_Input(uint8_t * buffer,int size,long time)"<<time;
    BufferDataNode * node = (BufferDataNode*)malloc(sizeof(BufferDataNode));
//    node->buffer = (uint8_t *)malloc(size);
    node->bufferSize = size;
    node->next = NULL;
    node->time = time;

    node->buffer = buffer;
//    memcpy(node->buffer,buffer,size);

    SDL_LockMutex(videoMutex);

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

    SDL_UnlockMutex(videoMutex);
}

//BufferDataNode *SaveVideoFileThread::videoDataQuene_get(double time)
//{
//    BufferDataNode * node = NULL;

//    SDL_LockMutex(videoMutex);

//    if (videoDataQueneHead != NULL)
//    {
//        node = videoDataQueneHead;

////qDebug()<<"111:"<<time<<node->time<<videoBufferCount;
//        if (time >= node->time)
//        {
////            qDebug()<<"000";
//            if (videoDataQueneHead->next != NULL)
//            {
////                qDebug()<<"111";
//                while(node != NULL)
//                {
////                    qDebug()<<"222";
//                    if (node->next == NULL)
//                    {
//                        break;
//                    }
////qDebug()<<"333"<<time << node->next->time;
//                    if (time < node->next->time)
//                    {
//                        break;
//                    }

//                    BufferDataNode * tmp = node;
////qDebug()<<"222:"<<node->time<<time;
//                    node = node->next;

//                    videoBufferCount--;

//                    av_free(tmp->buffer);
//                    free(tmp);
//                }
//            }
//        }
//        else
//        {
//            node = lastVideoNode;
//        }

//        if (videoDataQueneTail == node)
//        {
//            videoDataQueneTail = NULL;
//        }

//        if (node != NULL && node != lastVideoNode)
//        {
////            qDebug()<<"00000000";
//            videoDataQueneHead = node->next;
//            videoBufferCount--;
//        }

//    }
////    qDebug()<<videoBufferCount;
//    SDL_UnlockMutex(videoMutex);
////qDebug()<<"00000000999";
//    return node;
//}

/**
 * @brief SaveVideoFileThread::videoDataQuene_get
 * 由于采集屏幕获取到的图像每秒钟的张数是不固定的
 * 而我们是用固定帧率的方式来写视频
 * 因此需要保证每秒钟的视频图像张数都是固定的
 * 下面这个函数就是做了这个操作：
 * 1.视频数据不足的时候重复使用上一帧
 * 2.视频数据太多的时候 丢掉一些
 * @param time
 * @return
 */

BufferDataNode *SaveVideoFileThread::videoDataQuene_get(double time)
{
    BufferDataNode * node = NULL;

    SDL_LockMutex(videoMutex);

    if (videoDataQueneHead != NULL)
    {
        node = videoDataQueneHead;
        if (time >= node->time)
        {
            while(node != NULL)
            {
                if (node->next == NULL)
                {
                    if (isStop)
                    {
                        break;
                    }
                    else
                    {
                        //队列里面才一帧数据 先不处理
                        SDL_UnlockMutex(videoMutex);
                        return NULL;
                    }
                }

                if (time < node->next->time)
                {
                    break;
                }

                BufferDataNode * tmp = node;
                node = node->next;

                videoDataQueneHead = node;

                videoBufferCount--;
                av_free(tmp->buffer);
                free(tmp);
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

    SDL_UnlockMutex(videoMutex);

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

    SDL_LockMutex(audioMutex);

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

    SDL_UnlockMutex(audioMutex);
}

BufferDataNode *SaveVideoFileThread::audioDataQuene_get()
{
    BufferDataNode * node = NULL;

    SDL_LockMutex(audioMutex);

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

    SDL_UnlockMutex(audioMutex);

    return node;
}


/*
 * add an audio output stream
 */
AVStream *SaveVideoFileThread::add_audio_stream(AVFormatContext *oc, AVCodecID codec_id)
{
    AVCodecContext *c;
    AVStream *st;

    st = avformat_new_stream(oc, NULL);
    if (!st) {
        fprintf(stderr, "Could not alloc stream\n");
        exit(1);
    }
    st->id = 1;

    c = st->codec;
    c->codec_id = codec_id;
    c->codec_type = AVMEDIA_TYPE_AUDIO;

    /* put sample parameters */
    c->sample_fmt = AV_SAMPLE_FMT_S16;
    c->bit_rate = 64000;
    c->sample_rate = 22050;
    c->channels = 2;

//    c->bit_rate = 9600;
//    c->sample_rate = 11025;
//    c->channels = 1;

    // some formats want stream headers to be separate
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;

    return st;
}

void SaveVideoFileThread::open_audio(AVFormatContext *oc, AVStream *st)
{
    AVCodecContext *c;
    AVCodec *codec;

    c = st->codec;

    /* find the audio encoder */
    codec = avcodec_find_encoder(c->codec_id);
    if (!codec) {
        fprintf(stderr, "codec not found\n");
        exit(1);
    }

    /* open it */
    if (avcodec_open2(c, codec,NULL) < 0) {
        fprintf(stderr, "could not open codec\n");
        exit(1);
    }

    /* init signal generator */
    t = 0;
    tincr = 2 * M_PI * 110.0 / c->sample_rate;
    /* increment frequency by 110 Hz per second */
    tincr2 = 2 * M_PI * 110.0 / c->sample_rate / c->sample_rate;

    audio_outbuf_size = 10000;
    audio_outbuf = (uint8_t *)av_malloc(audio_outbuf_size);


    if (c->frame_size <= 1) {
        audio_input_frame_size = audio_outbuf_size / c->channels;
        switch(st->codec->codec_id) {
        case CODEC_ID_PCM_S16LE:
        case CODEC_ID_PCM_S16BE:
        case CODEC_ID_PCM_U16LE:
        case CODEC_ID_PCM_U16BE:
            audio_input_frame_size >>= 1;
            break;
        default:
            break;
        }
    } else {
        audio_input_frame_size = c->frame_size;
    }
    samples = (int16_t *)av_malloc(audio_input_frame_size * 2 * c->channels);
}

bool SaveVideoFileThread::write_audio_frame(AVFormatContext *oc, AVStream *st)
{
    AVCodecContext *c;
    AVPacket pkt;
    av_init_packet(&pkt);

    c = st->codec;

    BufferDataNode *node = audioDataQuene_get();

    if (node == NULL)
    {
        return false;
    }
    else
    {
        memcpy(samples,node->buffer, node->bufferSize);
//    qDebug()<<"size:"<<node->bufferSize<<audio_input_frame_size;
        free(node->buffer);
        free(node);
    }

//    memset(samples,0,audio_input_frame_size * 2 * c->channels); //静音

//    fread(samples, 1, audio_input_frame_size*4, pcmInFp);

    pkt.size = avcodec_encode_audio(c, audio_outbuf, audio_outbuf_size, samples);

    if (c->coded_frame && c->coded_frame->pts != AV_NOPTS_VALUE)
        pkt.pts= av_rescale_q(c->coded_frame->pts, c->time_base, st->time_base);
    pkt.flags |= AV_PKT_FLAG_KEY;
    pkt.stream_index = st->index;
    pkt.dts = pkt.pts;
    pkt.data = audio_outbuf;

//    pkt.pts = 0;
//qDebug()<<"audio pts"<<pkt.duration<<pkt.pos<<pkt.dts<<pkt.pts;
    /* write the compressed frame in the media file */
    if (av_interleaved_write_frame(oc, &pkt) != 0) {
        fprintf(stderr, "Error while writing audio frame\n");
        exit(1);
    }

    return true;
}

void SaveVideoFileThread::close_audio(AVFormatContext *oc, AVStream *st)
{
    avcodec_close(st->codec);

    av_free(samples);
    av_free(audio_outbuf);
}


/* add a video output stream */
AVStream *SaveVideoFileThread::add_video_stream(AVFormatContext *oc, AVCodecID codec_id)
{
    AVCodecContext *c;
    AVStream *st;
    AVCodec *codec;

    st = avformat_new_stream(oc, NULL);
    if (!st) {
        fprintf(stderr, "Could not alloc stream\n");
        exit(1);
    }

    c = st->codec;

    /* find the video encoder */
    codec = avcodec_find_encoder(codec_id);
    if (!codec) {
        fprintf(stderr, "codec not found\n");
        exit(1);
    }
    avcodec_get_context_defaults3(c, codec);

    c->codec_id = codec_id;

    /* put sample parameters */
    c->bit_rate = mBitRate;
    /* resolution must be a multiple of two */
    c->width = WIDTH;
    c->height = HEIGHT;
    /* time base: this is the fundamental unit of time (in seconds) in terms
       of which frame timestamps are represented. for fixed-fps content,
       timebase should be 1/framerate and timestamp increments should be
       identically 1. */
    c->time_base.den = m_videoFrameRate;
    c->time_base.num = 1;
//    c->gop_size = 12; /* emit one intra frame every twelve frames at most */
    c->gop_size = m_videoFrameRate;
    c->pix_fmt = STREAM_PIX_FMT;

    c->b_frame_strategy = 0;

    if (c->codec_id == CODEC_ID_MPEG2VIDEO) {
        /* just for testing, we also add B frames */
        c->max_b_frames = 2;
    }
    if (c->codec_id == CODEC_ID_MPEG1VIDEO){
        /* Needed to avoid using macroblocks in which some coeffs overflow.
           This does not happen with normal video, it just happens here as
           the motion of the chroma plane does not match the luma plane. */
        c->mb_decision=2;
    }
    // some formats want stream headers to be separate
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;

    return st;
}

static AVFrame *alloc_picture(enum PixelFormat pix_fmt, int width, int height)
{
    AVFrame *picture;
    uint8_t *picture_buf;
    int size;

    picture = avcodec_alloc_frame();
    if (!picture)
        return NULL;
    size = avpicture_get_size(pix_fmt, width, height);
    picture_buf = (uint8_t *)av_malloc(size);
    if (!picture_buf) {
        av_free(picture);
        return NULL;
    }
    avpicture_fill((AVPicture *)picture, picture_buf,
                   pix_fmt, width, height);
    return picture;
}

void SaveVideoFileThread::open_video(AVFormatContext *oc, AVStream *st)
{
    AVCodec *codec;
    AVCodecContext *c;

    c = st->codec;

    // Set Option
    AVDictionary *param = 0;
    //H.264
    //av_dict_set(&param, "preset", "slow", 0);
    av_dict_set(&param, "preset", "superfast", 0);
    av_dict_set(&param, "tune", "zerolatency", 0);  //实现实时编码

    codec = avcodec_find_encoder(c->codec_id);
    if (!codec){
      fprintf(stderr,"Can not find video encoder!\n");
      exit(1);
    }

int ret = 0;
    if (ret = avcodec_open2(c, codec,&param) < 0){
      fprintf(stderr,"Failed to open video encoder!\n");
      exit(1);
    }

//    /* find the video encoder */
//    codec = avcodec_find_encoder(c->codec_id);
//    if (!codec) {
//        fprintf(stderr, "codec not found\n");
//        exit(1);
//    }

//    /* open the codec */
//    if (avcodec_open2(c, codec, NULL) < 0) {
//        fprintf(stderr, "could not open codec\n");
//        exit(1);
//    }

    video_outbuf = NULL;
    if (!(oc->oformat->flags & AVFMT_RAWPICTURE)) {
        video_outbuf_size = 200000;
        video_outbuf = (uint8_t *)av_malloc(video_outbuf_size);
    }

    /* allocate the encoded raw picture */
    picture = alloc_picture(c->pix_fmt, c->width, c->height);
    if (!picture) {
        fprintf(stderr, "Could not allocate picture\n");
        exit(1);
    }

    /* if the output format is not YUV420P, then a temporary YUV420P
       picture is needed too. It is then converted to the required
       output format */
    tmp_picture = NULL;
    if (c->pix_fmt != PIX_FMT_YUV420P) {
        tmp_picture = alloc_picture(PIX_FMT_YUV420P, c->width, c->height);
        if (!tmp_picture) {
            fprintf(stderr, "Could not allocate temporary picture\n");
            exit(1);
        }
    }
}

bool SaveVideoFileThread::write_video_frame(AVFormatContext *oc, AVStream *st, double time)
{

    int out_size, ret;
    AVCodecContext *c;

    c = st->codec;

    BufferDataNode *node = videoDataQuene_get(time);

    if (node != NULL)
    {
        if (node != lastVideoNode)
        {
            if (lastVideoNode != NULL)
            {
                av_free(lastVideoNode->buffer);
                free(lastVideoNode);
            }

            lastVideoNode = node;
        }
    }

    ///没有视频数据 则先返回 等待视频数据
    if (node == NULL)
    {
        return false;
    }
    else
    {
        int y_size = c->width * c->height;

        memcpy(picture_buf,node->buffer, y_size*3/2);

        picture->data[0] = picture_buf;  // 亮度Y
        picture->data[1] = picture_buf+ y_size;  // U
        picture->data[2] = picture_buf+ y_size*5/4; // V
    }

    if (oc->oformat->flags & AVFMT_RAWPICTURE) {
        /* raw video case. The API will change slightly in the near
           future for that. */
        AVPacket pkt;
        av_init_packet(&pkt);

        pkt.flags |= AV_PKT_FLAG_KEY;
        pkt.stream_index = st->index;
        pkt.data = (uint8_t *)picture;
        pkt.size = sizeof(AVPicture);
        ret = av_interleaved_write_frame(oc, &pkt);
    } else {
        /* encode the image */
        out_size = avcodec_encode_video(c, video_outbuf, video_outbuf_size, picture);
        /* if zero size, it means the image was buffered */
        if (out_size > 0) {
            AVPacket pkt;
            av_init_packet(&pkt);

            if (c->coded_frame->pts != AV_NOPTS_VALUE)
                pkt.pts= av_rescale_q(c->coded_frame->pts, c->time_base, st->time_base);
            if(c->coded_frame->key_frame)
                pkt.flags |= AV_PKT_FLAG_KEY;
            pkt.stream_index = st->index;

            pkt.data = video_outbuf;
            pkt.size = out_size;

            /* write the compressed frame in the media file */
            ret = av_interleaved_write_frame(oc, &pkt);

        } else {
            ret = 0;
        }
    }

    if (ret != 0) {
        fprintf(stderr, "Error while writing video frame\n");
        exit(1);
    }

    return true;

}

void SaveVideoFileThread::close_video(AVFormatContext *oc, AVStream *st)
{
    avcodec_close(st->codec);
//    av_free(picture_buf);
    av_free(picture);
    if (tmp_picture) {
        av_free(tmp_picture->data[0]);
        av_free(tmp_picture);
    }
    av_free(video_outbuf);
}

double SaveVideoFileThread::getVideoPts()
{
    return video_pts;
}

double SaveVideoFileThread::getAudioPts()
{
    return audio_pts;
}

void SaveVideoFileThread::setWidth(int width,int height)
{
    WIDTH = width;
    HEIGHT = height;
}


bool SaveVideoFileThread::startWrite()
{
    isStop = false;

    ///创建一个线程专门用来解码视频
    SDL_CreateThread(writeVideoThreadFunc, "writeVideoThreadFunc", this);

    return true;
}

bool SaveVideoFileThread::stopWrite()
{
    isStop = true;

    return true;
}

int writeVideoThreadFunc(void *arg)
{
    SaveVideoFileThread *pointer = (SaveVideoFileThread *) arg;

//    const char *filename;
    AVOutputFormat *fmt;
    AVFormatContext *oc;
    AVStream *audio_st, *video_st;

    int i;

    /// ffmpegg可以根据文件名称自动获取视频格式
    /// 如需换别的格式 只需要改文件名和对应的format_name（就是下面的Mp4）即可
    /// 因此无需了解具体的视频封装格式

    /* allocate the output media context */
    avformat_alloc_output_context2(&oc, NULL, "mp4", pointer->filename);
    if (!oc) {
        fprintf(stderr,"Could not deduce output format from file extension: using MPEG.\n");
        avformat_alloc_output_context2(&oc, NULL, "mpeg", pointer->filename);
    }
    if (!oc) {
        return -1;
    }
    fmt = oc->oformat;

    /* add the audio and video streams using the default format codecs
       and initialize the codecs */
    video_st = NULL;
    audio_st = NULL;

    pointer->picture = NULL;
    pointer->isStop = false;

    if (pointer->m_containsVideo)
    {
        if (fmt->video_codec != CODEC_ID_NONE) {
            video_st = pointer->add_video_stream(oc, AV_CODEC_ID_H264);
        }
    }

    if (pointer->m_containsAudio)
    {
        if (fmt->audio_codec != CODEC_ID_NONE) {
            audio_st = pointer->add_audio_stream(oc, AV_CODEC_ID_AAC);
        }
    }

    av_dump_format(oc, 0, pointer->filename, 1);

    /* now that all the parameters are set, we can open the audio and
       video codecs and allocate the necessary encode buffers */
    if (video_st)
        pointer->open_video(oc, video_st);
    if (audio_st)
        pointer->open_audio(oc, audio_st);

    /* open the output file, if needed */
    if (!(fmt->flags & AVFMT_NOFILE)) {
        if (avio_open(&oc->pb, pointer->filename, AVIO_FLAG_WRITE) < 0) {
            fprintf(stderr,"Could not open %s",pointer->filename);
            return -1;
        }
    }

    /* write the stream header, if any */
//    av_write_header(oc);
    avformat_write_header(oc,NULL);

    if (pointer->picture)
    {
        pointer->picture->pts = 0;
    }

    while(1)
    {
        /* compute current audio and video time */
        if (audio_st)
            pointer->audio_pts = (double)audio_st->pts.val * audio_st->time_base.num / audio_st->time_base.den;
        else
            pointer->audio_pts = 0.0;

        if (video_st)
            pointer->video_pts = (double)video_st->pts.val * video_st->time_base.num / video_st->time_base.den;
        else
            pointer->video_pts = 0.0;

        /* write interleaved audio and video frames */
        if (!video_st || (video_st && audio_st && pointer->audio_pts < pointer->video_pts)) {
            if (!pointer->write_audio_frame(oc, audio_st))
            {
                if (pointer->isStop)
                {
                    break;
                }
                SDL_Delay(1); //延时1ms
            }
        } else {
            if (!pointer->write_video_frame(oc, video_st,pointer->video_pts*1000))
            {
                if (pointer->isStop)
                {
                    break;
                }
                SDL_Delay(1); //延时1ms
            }
        }
    }

    av_write_trailer(oc);

    /* close each codec */
    if (video_st)
        pointer->close_video(oc, video_st);
    if (audio_st)
        pointer->close_audio(oc, audio_st);

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
    av_free(oc);

    pointer->audio_pts = 0;
    pointer->video_pts = 0;

}

