/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef GetAudioThread_H
#define GetAudioThread_H

#include <QThread>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
}

#include "AACEncoder.h"

/**
 * @brief The GetAudioThread class  此类主要负责获取录音数据 并使用ffmpeg编码成AAC
 */

enum ErroCode
{
    AudioOpenFailed = 0,
    VideoOpenFailed,
    AudioDecoderOpenFailed,
    VideoDecoderOpenFailed,
    SUCCEED
};


class GetAudioThread : public QThread
{
    Q_OBJECT

public:
    explicit GetAudioThread();
    ~GetAudioThread();

    /**
     * @brief init 初始化打开录屏设备
     * @param videoDevName
     * @return
     */
    ErroCode init(QString audioDevName);
    void deInit();

    void startRecord();
    void pauseRecord();
    void restoreRecord();
    void stopRecord();

protected:
    void run();

private:
    AVFormatContext	*pFormatCtx;
    int				i, audioindex;
    AVCodecContext	*aCodecCtx;

    AVFrame	*aFrame;
    uint8_t *out_buffer;

    bool m_isRun;
    bool m_pause;

    AACEncoder *mAACEncoder;

};

#endif // GetAudioThread_H
