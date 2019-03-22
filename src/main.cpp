
/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <QDebug>

extern"C"
{
    #include "libavutil/mathematics.h"
    #include "libavformat/avformat.h"
    #include "libswscale/swscale.h"

    #include "SDL.h"
    #include "SDL_thread.h"
    #include "SDL_events.h"

}

#define WIDTH  176
#define HEIGHT 144

#define PCM_FILE_NAME "in.pcm"
#define YUV_FILE_NAME "in.yuv"
#define OUT_VIDEO_FILENAME "out.mp4"

uint8_t picture_buf[WIDTH*HEIGHT*4];
bool isStop = false;

static float t, tincr, tincr2;
static int16_t *samples;
static uint8_t *audio_outbuf;
static int audio_outbuf_size;
int audio_input_frame_size;

#define STREAM_DURATION   200.0  //视频总长度单位秒
#define STREAM_FRAME_RATE 25 /* 25 images/s */
#define STREAM_NB_FRAMES  ((int)(STREAM_DURATION * STREAM_FRAME_RATE))
#define STREAM_PIX_FMT PIX_FMT_YUV420P /* default pix_fmt */

/**************************************************************/
/* audio output */

struct BufferDataNode
{
    uint8_t * buffer;
    int bufferSize;
    BufferDataNode * next;
};

SDL_mutex *videoMutex = SDL_CreateMutex();
BufferDataNode * videoDataQueneHead = NULL;
BufferDataNode * videoDataQueneTail = NULL;

SDL_mutex *audioMutex = SDL_CreateMutex();
BufferDataNode * AudioDataQueneHead = NULL;
BufferDataNode * AudioDataQueneTail = NULL;

void videoDataQuene_Input(uint8_t * buffer,int size)
{
    BufferDataNode * node = (BufferDataNode*)malloc(sizeof(BufferDataNode));
    node->buffer = (uint8_t *)malloc(size);
    node->bufferSize = size;
    node->next = NULL;

    memcpy(node->buffer,buffer,size);

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

    SDL_UnlockMutex(videoMutex);
}

static BufferDataNode *videoDataQuene_get()
{
    BufferDataNode * node = NULL;

    SDL_LockMutex(videoMutex);

    if (videoDataQueneHead != NULL)
    {
        node = videoDataQueneHead;

        if (videoDataQueneTail == videoDataQueneHead)
        {
            videoDataQueneTail = NULL;
        }

        videoDataQueneHead = videoDataQueneHead->next;
    }

    SDL_UnlockMutex(videoMutex);

    return node;
}


void audioDataQuene_Input(uint8_t * buffer,int size)
{
    BufferDataNode * node = (BufferDataNode*)malloc(sizeof(BufferDataNode));
    node->buffer = (uint8_t *)malloc(size);
    node->bufferSize = size;
    node->next = NULL;

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

    SDL_UnlockMutex(audioMutex);
}

static BufferDataNode *audioDataQuene_get()
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
    }

    SDL_UnlockMutex(audioMutex);

    return node;
}

/*
 * add an audio output stream
 */
static AVStream *add_audio_stream(AVFormatContext *oc, AVCodecID codec_id)
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
    c->bit_rate = 44100;
    c->sample_rate = 44100;
    c->channels = 2;

    // some formats want stream headers to be separate
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;

    return st;
}

static void open_audio(AVFormatContext *oc, AVStream *st)
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

static void write_audio_frame(AVFormatContext *oc, AVStream *st)
{
    AVCodecContext *c;
    AVPacket pkt;
    av_init_packet(&pkt);

    c = st->codec;

    BufferDataNode *node = audioDataQuene_get();

    if (node == NULL)
    {
        SDL_Delay(1); //延时1ms
        return;
    }
    else
    {
        memcpy(samples,node->buffer, node->bufferSize);

        free(node->buffer);
        free(node);
    }

//    fread(samples, 1, audio_input_frame_size*4, pcmInFp);

    pkt.size = avcodec_encode_audio(c, audio_outbuf, audio_outbuf_size, samples);

    if (c->coded_frame && c->coded_frame->pts != AV_NOPTS_VALUE)
        pkt.pts= av_rescale_q(c->coded_frame->pts, c->time_base, st->time_base);
    pkt.flags |= AV_PKT_FLAG_KEY;
    pkt.stream_index = st->index;
    pkt.data = audio_outbuf;

    /* write the compressed frame in the media file */
    if (av_interleaved_write_frame(oc, &pkt) != 0) {
        fprintf(stderr, "Error while writing audio frame\n");
        exit(1);
    }
}

static void close_audio(AVFormatContext *oc, AVStream *st)
{
    avcodec_close(st->codec);

    av_free(samples);
    av_free(audio_outbuf);
}

/**************************************************************/
/* video output */

static AVFrame *picture, *tmp_picture;
static uint8_t *video_outbuf;
static int video_outbuf_size;

/* add a video output stream */
static AVStream *add_video_stream(AVFormatContext *oc, AVCodecID codec_id)
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
    c->bit_rate = 400000;
    /* resolution must be a multiple of two */
    c->width = WIDTH;
    c->height = HEIGHT;
    /* time base: this is the fundamental unit of time (in seconds) in terms
       of which frame timestamps are represented. for fixed-fps content,
       timebase should be 1/framerate and timestamp increments should be
       identically 1. */
    c->time_base.den = STREAM_FRAME_RATE;
    c->time_base.num = 1;
    c->gop_size = 12; /* emit one intra frame every twelve frames at most */
    c->pix_fmt = STREAM_PIX_FMT;
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

static void open_video(AVFormatContext *oc, AVStream *st)
{
    AVCodec *codec;
    AVCodecContext *c;

    c = st->codec;

    /* find the video encoder */
    codec = avcodec_find_encoder(c->codec_id);
    if (!codec) {
        fprintf(stderr, "codec not found\n");
        exit(1);
    }

    /* open the codec */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "could not open codec\n");
        exit(1);
    }

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

static void write_video_frame(AVFormatContext *oc, AVStream *st)
{

    int out_size, ret;
    AVCodecContext *c;

    c = st->codec;

    BufferDataNode *node = videoDataQuene_get();

    if (node == NULL)
    {
        SDL_Delay(1); //延时1ms
        return;
    }
    else
    {
        int y_size = c->width * c->height;
        memcpy(picture_buf,node->buffer, y_size*3/2);

        free(node->buffer);
        free(node);

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

}

static void close_video(AVFormatContext *oc, AVStream *st)
{
    avcodec_close(st->codec);
    av_free(picture->data[0]);
    av_free(picture);
    if (tmp_picture) {
        av_free(tmp_picture->data[0]);
        av_free(tmp_picture);
    }
    av_free(video_outbuf);
}

int encode_thread(void *arg)
{

    const char *filename;
    AVOutputFormat *fmt;
    AVFormatContext *oc;
    AVStream *audio_st, *video_st;
    double audio_pts, video_pts;
    int i;

    /* initialize libavcodec, and register all codecs and formats */
    av_register_all();

    filename = OUT_VIDEO_FILENAME;

    /* allocate the output media context */
    avformat_alloc_output_context2(&oc, NULL, NULL, filename);
    if (!oc) {
        printf("Could not deduce output format from file extension: using MPEG.\n");
        avformat_alloc_output_context2(&oc, NULL, "mpeg", filename);
    }
    if (!oc) {
        return 1;
    }
    fmt = oc->oformat;

    /* add the audio and video streams using the default format codecs
       and initialize the codecs */
    video_st = NULL;
    audio_st = NULL;
    if (fmt->video_codec != CODEC_ID_NONE) {
        video_st = add_video_stream(oc, fmt->video_codec);
    }
    if (fmt->audio_codec != CODEC_ID_NONE) {
        audio_st = add_audio_stream(oc, fmt->audio_codec);
    }

    av_dump_format(oc, 0, filename, 1);

    /* now that all the parameters are set, we can open the audio and
       video codecs and allocate the necessary encode buffers */
    if (video_st)
        open_video(oc, video_st);
    if (audio_st)
        open_audio(oc, audio_st);

    /* open the output file, if needed */
    if (!(fmt->flags & AVFMT_NOFILE)) {
        if (avio_open(&oc->pb, filename, AVIO_FLAG_WRITE) < 0) {
            fprintf(stderr, "Could not open '%s'\n", filename);
            return 1;
        }
    }

    /* write the stream header, if any */
//    av_write_header(oc);
    avformat_write_header(oc,NULL);

    picture->pts = 0;
    while(!isStop)
    {
        /* compute current audio and video time */
        if (audio_st)
            audio_pts = (double)audio_st->pts.val * audio_st->time_base.num / audio_st->time_base.den;
        else
            audio_pts = 0.0;

        if (video_st)
            video_pts = (double)video_st->pts.val * video_st->time_base.num / video_st->time_base.den;
        else
            video_pts = 0.0;

        qDebug()<<audio_pts<<video_pts;

        if ((!audio_st || audio_pts >= STREAM_DURATION) &&
            (!video_st || video_pts >= STREAM_DURATION))
            break;

        /* write interleaved audio and video frames */
        if (!video_st || (video_st && audio_st && audio_pts < video_pts)) {
            write_audio_frame(oc, audio_st);
        } else {
            write_video_frame(oc, video_st);
            picture->pts++;
        }
    }



    av_write_trailer(oc);

    /* close each codec */
    if (video_st)
        close_video(oc, video_st);
    if (audio_st)
        close_audio(oc, audio_st);

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

    return 0;
}

bool startEncode()
{
    SDL_Thread *encodeThreadId = SDL_CreateThread(encode_thread, "parse_thread", NULL);

    if (!encodeThreadId)
    {
        return false;
    }

    return true;
}


int putVideoThread(void*arg);
int putAudioThread(void*arg);

FILE *pcmInFp = fopen(PCM_FILE_NAME,"rb");
FILE *yuv420InFp = fopen(YUV_FILE_NAME,"rb");

#undef main

int main()
{
    if (pcmInFp == NULL || yuv420InFp == NULL)
    {
        ///请把pcm和yuv文件拷贝到构建目录下
        fprintf(stderr,"please copy the yuv and pcm file to the Qt Build folder!\n");
        exit(-1);
    }

    startEncode(); //开启编码并写入文件的线程
    SDL_CreateThread(putAudioThread, "parse_thread", NULL); //开启读取PCM的线程
    SDL_CreateThread(putVideoThread, "parse_thread", NULL); //开启读取YUV的线程

    while(1); //主线程停住等待

    return 0;
}



///读取输入文件相关 - Begin
int putVideoThread(void*arg)
{
    int y_size = WIDTH * HEIGHT;

    while(1)
    {

        char buffer[WIDTH*HEIGHT*3];

        int size = fread(buffer, 1, y_size*3/2, yuv420InFp);

        if (size < 0){
          printf("Failed to read YUV data! 文件读取错误\n");
          return 0;
        }
        else if(feof(yuv420InFp))
        {
            qDebug()<<"read video finished!";

            fclose(yuv420InFp);
            yuv420InFp = fopen(YUV_FILE_NAME,"rb");

            continue;
        }

        if (size == y_size*3/2)
        {
            videoDataQuene_Input((uint8_t*)buffer,y_size*3/2);
        }

        SDL_Delay(10);
    }

    return 0;
}


int putAudioThread(void*arg)
{
    while(1)
    {

        char buffer[WIDTH*HEIGHT*3];
        int size = fread(buffer, 1, audio_input_frame_size*4, pcmInFp);

        if (size < 0){
          printf("Failed to read YUV data! 文件读取错误\n");
          return 0;
        }
        else if(feof(pcmInFp))
        {
            qDebug()<<"read audio finished!";

            fclose(pcmInFp);
            pcmInFp = fopen(PCM_FILE_NAME,"rb");

            continue;
        }

        if (size == audio_input_frame_size*4)
        {
            audioDataQuene_Input((uint8_t*)buffer,audio_input_frame_size*4);
        }

        SDL_Delay(1);
    }

    return 0;
}
///读取输入文件相关 - End
