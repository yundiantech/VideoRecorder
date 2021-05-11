/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */
#ifndef GetVideoThread_H
#define GetVideoThread_H

#include <thread>

#include "VideoFrame/VideoRawFrame.h"
#include "Mutex/Cond.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <libavutil/imgutils.h>
}

/**
 * @brief The GetVideoThread class  此类主要负责采集屏幕
 */

class GetVideoThread
{

public:
    enum ErroCode
    {
        AudioOpenFailed = 0,
        VideoOpenFailed,
        AudioDecoderOpenFailed,
        VideoDecoderOpenFailed,
        SUCCEED
    };

    explicit GetVideoThread();
    ~GetVideoThread();

    GetVideoThread::ErroCode openCamera(const std::string &deviceName);
    void closeCamera(bool isBlock = false);

    void startRecord(std::function<void (VideoRawFramePtr yuvFrame, VideoRawFramePtr rgbFrame, void *param)> func = nullptr, void *param = nullptr);
    void pauseRecord();
    void restoreRecord();
    void stopRecord(const bool &isBlock);

protected:
    void run();

private:

    std::function<void (VideoRawFramePtr yuvFrame, VideoRawFramePtr rgbFrame, void *param)> mCallBackFunc; //回调函数
    void *mCallBackFuncParam; //回调函数用户参数


    AVFormatContext	*pFormatCtx;
    int				i, videoindex;
    AVCodecContext	*pCodecCtx;

    AVFrame	*pFrame,*pFrameYUV,*pFrameRGB;
    uint8_t *out_buffer;
    uint8_t *out_buffer_rgb24;

    bool m_pause;

    bool mIsCloseCamera;
    bool mIsStop;
    bool mIsThreadRuning;
    bool mIsReadingVideo;

    bool m_getFirst; //是否获取到了时间基准
    int64_t mLastReadFailedTime; //记录上一次视频读取失败的时间，因为是非阻塞模式，因此需要通过判断一定时间内没有读取过数据才可以认为真的读取失败了

    Cond *mCond;
    std::string mDeviceName;

    /**
     * @brief init 初始化打开录屏设备
     * @param videoDevName
     * @return
     */
    ErroCode init(const std::string deviceName);
    void deInit();

};

#endif // GetVideoThread_H
