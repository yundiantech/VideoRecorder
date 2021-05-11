/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef AUDIOENCODER_H
#define AUDIOENCODER_H

#include "AudioFrame/PCMFrame.h"
#include "AudioFrame/AACFrame.h"

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavdevice/avdevice.h>
    #include <libavformat/avformat.h>
    #include <libswresample/swresample.h>
    #include <libavutil/imgutils.h>
}

class AudioEncoder
{
public:
    AudioEncoder();

    bool openEncoder();
    void closeEncoder();

    int getONEFrameSize(){return mONEFrameSize;}

    AACFramePtr encode(uint8_t *inputbuf, int bufferSize);

private:
    AVCodec         *aCodec;
    AVCodecContext  *aCodecCtx;

    AVFrame *aFrame;
    uint8_t *mFrameBuffer;/// 存放pcm数据，用来取出刚好的一帧数据传给编码器编码
    int mFrameBufferSize;

    int mONEFrameSize;
};

#endif // AUDIOENCODER_H
