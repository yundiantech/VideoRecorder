/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "aacencoder.h"

extern "C"
{
#include "libavcodec\avcodec.h"
#include "libavformat\avformat.h"
#include "libswscale\swscale.h"
}

AACEncoder::AACEncoder()
{

}

AACEncoder::~AACEncoder()
{

}

void AACEncoder::startEncode()
{
    start();
}

void AACEncoder::inputPcmBuffer(uint8_t *buffer, int size)
{
    FrameDataNode node;
    node.size = size;
    node.buffer = buffer;

    mMutex.lock();
    mPcmBufferList.append(node);
    mMutex.unlock();
}

void AACEncoder::run()
{
    AVCodecContext* pCodecCtx;
    AVCodec* aCodec;

    uint8_t* frame_buf;
    AVFrame* frame;
    int ONEFrameSize;

    //查找h264编码器
    aCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if(!aCodec)
    {
      fprintf(stderr, "aac codec not found\n");
      exit(1);
    }

    pCodecCtx = avcodec_alloc_context3(aCodec);
    pCodecCtx->codec_id = AV_CODEC_ID_AAC;
    pCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO;
    pCodecCtx->sample_fmt = AV_SAMPLE_FMT_S16;
    pCodecCtx->sample_rate= 44100;
    pCodecCtx->channel_layout=AV_CH_LAYOUT_STEREO;
    pCodecCtx->channels = av_get_channel_layout_nb_channels(pCodecCtx->channel_layout);
    pCodecCtx->bit_rate = 64000;


    aCodec = avcodec_find_encoder(pCodecCtx->codec_id);
    if (!aCodec)
    {
        printf("没有找到合适的编码器！\n");
        return;
    }
    if (avcodec_open2(pCodecCtx, aCodec,NULL) < 0)
    {
        printf("编码器打开失败！\n");
        return;
    }
    frame = avcodec_alloc_frame();
    frame->nb_samples = pCodecCtx->frame_size;
    frame->format = pCodecCtx->sample_fmt;

    ONEFrameSize = av_samples_get_buffer_size(NULL, pCodecCtx->channels,pCodecCtx->frame_size,pCodecCtx->sample_fmt, 1);
    frame_buf = (uint8_t *)av_malloc(ONEFrameSize);
    avcodec_fill_audio_frame(frame, pCodecCtx->channels, pCodecCtx->sample_fmt,(const uint8_t*)frame_buf, ONEFrameSize, 1);


    AVPacket pkt;
    av_new_packet(&pkt,ONEFrameSize);

    FILE *aacFp = fopen("out.aac","wb");

    uint8_t * mAacBuffer = (uint8_t * )malloc(4096*100);
    int mAacBufferIndex = 0;
    int mAacBufferSize = 0;

    while(1)
    {
        if (mPcmBufferList.isEmpty())
        {
            msleep(10);
            continue;
        }

        mMutex.lock();
        FrameDataNode node = mPcmBufferList.takeFirst(); //取出1帧yuv数据
        mMutex.unlock();

        memcpy(mAacBuffer+mAacBufferSize,node.buffer,node.size);
        mAacBufferSize += node.size;
        free(node.buffer);

        /// 每次传递给编码器的数据大小都要是 上面获取到的 "ONEFrameSize"
        /// 因此需要下面的循环
        while(1)
        {
            int size = mAacBufferSize - mAacBufferIndex;
            if (size < ONEFrameSize) //不够编码1帧了
            {
                memcpy(mAacBuffer,mAacBuffer+mAacBufferIndex,size);
                mAacBufferIndex = 0;
                mAacBufferSize = size;
                break;
            }

            frame->data[0] = mAacBuffer+mAacBufferIndex;  //采样信号
            mAacBufferIndex += ONEFrameSize;

            int got_frame=0;

            //编码
            int ret = avcodec_encode_audio2(pCodecCtx, &pkt,frame, &got_frame);
            if (got_frame==1)
            {
                /// 编码后的数据是带ADTS头的 因此写入文件后 可以直接用播放器播放
                fwrite(pkt.data,1,pkt.size,aacFp);

                av_free_packet(&pkt);
            }
        }
    }

    //清理
    av_free(frame);
    av_free(frame_buf);

    free(mAacBuffer);

    fclose(aacFp);

    avcodec_close(pCodecCtx);

}

