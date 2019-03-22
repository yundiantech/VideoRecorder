
/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef SCREENRECORDER_H
#define SCREENRECORDER_H

#include <QThread>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
//SDL
#include "SDL.h"
#include "SDL_thread.h"
}

#include "savevideofile.h"

enum ErroCode
{
    AudioOpenFailed = 0,
    VideoOpenFailed,
    AudioDecoderOpenFailed,
    VideoDecoderOpenFailed,
    SUCCEED
};

class ScreenRecorder : public QThread
{
    Q_OBJECT

public:
    explicit ScreenRecorder();
    ~ScreenRecorder();

    ErroCode init(QString audioDevName);
    void deInit();

    void startRecord();
    void stopRecord();

protected:
    void run();

private:


    AVFormatContext	*pFormatCtx;
    int				i, videoindex ,audioindex;
    AVCodecContext	*pCodecCtx,*aCodecCtx;


    AVFrame	*pFrame,*aFrame,*pFrameYUV;
    uint8_t *out_buffer;

    bool m_isRun;


};

#endif // SCREENRECORDER_H
