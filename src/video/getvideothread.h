
/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef GetVideoThread_H
#define GetVideoThread_H

#include <QThread>

extern "C"
{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libswscale/swscale.h"
    #include "libavdevice/avdevice.h"
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

class GetVideoThread : public QThread
{
    Q_OBJECT

public:
    explicit GetVideoThread();
    ~GetVideoThread();

    ErroCode init(QString videoDevName,bool useVideo,QString audioDevName,bool useAudio);
    void deInit();

    void startRecord();
    void pauseRecord();
    void restoreRecord();
    void stopRecord();

    void setPicRange(int x,int y,int w,int h);

    void setSaveVideoFileThread(SaveVideoFileThread * p);

protected:
    void run();

private:

    AVFormatContext	*pFormatCtx;
    int				i, videoindex ,audioindex;
    AVCodecContext	*pCodecCtx,*aCodecCtx;

    AVFrame	*pFrame,*aFrame,*pFrameYUV;
    uint8_t *out_buffer;

    int pic_x;
    int pic_y;
    int pic_w;
    int pic_h;

    bool m_isRun;
    bool m_pause;

    SaveVideoFileThread * m_saveVideoFileThread;

    ///用于存储读取到的音频数据
    DECLARE_ALIGNED(16, uint8_t, audio_buf) [AVCODEC_MAX_AUDIO_FRAME_SIZE * 4];
    unsigned int audio_buf_size;

};

#endif // GetVideoThread_H
