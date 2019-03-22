/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "getaudiothread.h"

#include <QTimer>
#include <QDateTime>
#include <QDebug>

#include <windows.h>

GetAudioThread::GetAudioThread()
{
    m_isRun = false;

    pFormatCtx = NULL;
    out_buffer = NULL;

    m_pause = false;

    mAACEncoder = new AACEncoder;
}

GetAudioThread::~GetAudioThread()
{

}

ErroCode GetAudioThread::init(QString audioDevName)
{

    AVCodec			*aCodec = NULL;

    pFormatCtx = avformat_alloc_context();

    AVInputFormat *ifmt = av_find_input_format("dshow"); //使用dshow

    QString audioDevOption = QString("audio=%1").arg(audioDevName);

    if(avformat_open_input(&pFormatCtx,audioDevOption.toUtf8(),ifmt,NULL)!=0){
        fprintf(stderr,"Couldn't open input stream audio.\n");
        return AudioOpenFailed;
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
        printf("Didn't find a video stream.\n");
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

//    qDebug()<<"audio info:";
//    qDebug()<<"audio info:"<<aCodecCtx->bit_rate<<aCodecCtx->sample_fmt<<aCodecCtx->bit_rate<<aCodecCtx->sample_rate<<aCodecCtx->channels;

    aFrame = avcodec_alloc_frame();

    return SUCCEED;
}

void GetAudioThread::deInit()
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

    avformat_close_input(&pFormatCtx);

    avformat_free_context(pFormatCtx);

}

void GetAudioThread::startRecord()
{
    m_isRun = true;
    start();

    if (mAACEncoder != NULL)
        mAACEncoder->startEncode();

}

void GetAudioThread::pauseRecord()
{
    m_pause = true;
}

void GetAudioThread::restoreRecord()
{
    m_pause = false;
}

void GetAudioThread::stopRecord()
{
    m_pause = false;
    m_isRun = false;
}

FILE *pcmFp = fopen("out.pcm","wb");

void GetAudioThread::run()
{
    int ret, got_frame;

    AVPacket *packet=(AVPacket *)av_malloc(sizeof(AVPacket));

    while(m_isRun)
    {

        if (av_read_frame(pFormatCtx, packet)<0)
        {
            qDebug()<<"read failed!";
            msleep(10);
            continue;
        }

        if (m_pause)
        {
            av_free_packet(packet);
            msleep(10);
            continue;
        }
//qDebug()<<packet->stream_index << audioindex;
        if(packet->stream_index == audioindex)
        {

            ret = avcodec_decode_audio4(aCodecCtx, aFrame, &got_frame, packet);
//            qDebug()<<ret<<got_frame;
            if(ret < 0)
            {
                fprintf(stderr,"video Audio Error.\n");
//                return;
            }

            if (got_frame)
            {
//                int data_size = aFrame->linesize[0];

                int framSize = av_samples_get_buffer_size(NULL,aCodecCtx->channels, aFrame->nb_samples,aCodecCtx->sample_fmt, 1);

                uint8_t * audio_buf = (uint8_t *)malloc(framSize);
                memcpy(audio_buf, aFrame->data[0], framSize);
                mAACEncoder->inputPcmBuffer((uint8_t*)audio_buf,framSize);

                fwrite(audio_buf,1,framSize,pcmFp);
            }
        }
        else
        {
            qDebug()<<"other"<<packet->stream_index;
        }
        av_free_packet(packet);
    }

    qDebug()<<"record stopping...";

    m_pause = false;

    deInit();

    qDebug()<<"record finished!";

}

