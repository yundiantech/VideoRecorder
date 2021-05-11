/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "AudioEncoder.h"

AudioEncoder::AudioEncoder()
{
    aCodec = nullptr;
    aCodecCtx = nullptr;

    aFrame = nullptr;

    mFrameBuffer = nullptr;
    mFrameBufferSize = 0;

}

bool AudioEncoder::openEncoder()
{
    if (aCodec == nullptr)
    {
        ///打开音频编码器
        //find the encoder
        aCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);

        if (aCodec == nullptr)
        {
            fprintf(stderr, "audio Codec not found.\n");
            return false;
        }
        else
        {
            aCodecCtx = avcodec_alloc_context3(aCodec);
        }

        aCodecCtx->codec_type  = AVMEDIA_TYPE_AUDIO;
        aCodecCtx->sample_fmt  = AV_SAMPLE_FMT_FLTP;
        aCodecCtx->sample_rate = 44100;
        aCodecCtx->channels    = 2;
        aCodecCtx->channel_layout = av_get_default_channel_layout(aCodecCtx->channels);

    //    aCodecCtx->channels       = av_get_channel_layout_nb_channels(aCodecCtx->channel_layout);
    //    aCodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;

    //    aCodecCtx->profile=FF_PROFILE_AAC_LOW; //（可参考AAC格式简介）
    //    aCodecCtx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

    //    aCodecCtx->bit_rate = 64000;

        aCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        aCodecCtx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

        if(avcodec_open2(aCodecCtx, aCodec, nullptr)<0)
        {
            printf("Could not open audio codec.\n");
            return false;
        }

        mONEFrameSize = av_samples_get_buffer_size(NULL, aCodecCtx->channels, aCodecCtx->frame_size, aCodecCtx->sample_fmt, 1);

        aFrame           = av_frame_alloc();
        mFrameBuffer     = (uint8_t *)av_malloc(mONEFrameSize);
        mFrameBufferSize = mONEFrameSize;

        ///这句话必须要(设置这个frame里面的采样点个数)
        int oneChannelBufferSize = mONEFrameSize / aCodecCtx->channels; //计算出一个声道的数据
        int nb_samplesize = oneChannelBufferSize / av_get_bytes_per_sample(aCodecCtx->sample_fmt); //计算出采样点个数
        aFrame->nb_samples = nb_samplesize;

        ///填充数据  下面这2种方式都可以
    //    avcodec_fill_audio_frame(ost->frame, aCodecCtx->channels, aCodecCtx->sample_fmt,(const uint8_t*)ost->frameBuffer, mONEFrameSize, 0);
        av_samples_fill_arrays(aFrame->data, aFrame->linesize, mFrameBuffer, aCodecCtx->channels, aFrame->nb_samples, aCodecCtx->sample_fmt, 0);
    }

    return true;
}

void AudioEncoder::closeEncoder()
{
    avcodec_close(aCodecCtx);
    av_free(aCodecCtx);
    av_free(aFrame);

    aCodec = nullptr;
    aCodecCtx = nullptr;

    aFrame = nullptr;
}

/**
*  Add ADTS header at the beginning of each and every AAC packet.
*  This is needed as MediaCodec encoder generates a packet of raw
*  AAC data.
*
*  Note the packetLen must count in the ADTS header itself !!! .
*注意，这里的packetLen参数为raw aac Packet Len + 7; 7 bytes adts header
**/
void addADTStoPacket(uint8_t* packet, int packetLen)
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

AACFramePtr AudioEncoder::encode(uint8_t *inputbuf, int bufferSize)
{
    AACFramePtr framePtr = nullptr;

    AVPacket pkt;
    av_init_packet(&pkt);

    AVPacket *packet = &pkt;

    memcpy(mFrameBuffer, inputbuf, bufferSize);

    int ret = avcodec_send_frame(aCodecCtx, aFrame);
    if (ret != 0)
    {
        char buff[128]={0};
        av_strerror(ret, buff, 128);

        fprintf(stderr, "Error sending a frame for encoding! (%s)\n", buff);
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
            fprintf(stderr, "!!!!!!!!!! Error encoding audio frame: %s ret=%d", errstr, ret);

            return false;
        }

        uint8_t * aacBuf = (uint8_t *)malloc(packet->size+7);
        addADTStoPacket(aacBuf, 7+packet->size);
        memcpy(aacBuf+7, packet->data, packet->size);

#if 0 ///写入aac文件
        static FILE *aacFp = fopen("out22.aac", "wb");
        fwrite(aac_buf,1,packet->size+7,aacFp);
#endif

        framePtr = std::make_shared<AACFrame>();
        AACFrame *frame = framePtr.get();
        frame->setFrameBuffer(aacBuf, 7+packet->size);

        free(aacBuf);

        av_packet_unref(packet);
        break;
    }

    return framePtr;
}
