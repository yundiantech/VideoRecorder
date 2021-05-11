/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef CaptureWindowThread_H
#define CaptureWindowThread_H

#include <thread>
#include <stdint.h>
#include <functional>

#include "VideoEncoder.h"

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavdevice/avdevice.h>

    #include <libavutil/imgutils.h>
}

/**
 * @brief The CaptureWindowThread class  通过windows Api采集指定窗口图形
 */

class CaptureWindowThread
{

public:
    explicit CaptureWindowThread();
    ~CaptureWindowThread();

    void setHWND(const HWND &hWnd);

    void setQuantity(const int &value);
    void setFrameRate(const int &frameRate);
    void setRect(const int &x, const int &y, const int &width, const int &height);
    void setFollowMouseMode(const bool &value);

    void startRecord(std::function<void (VideoRawFramePtr videoFramePtr, void *param)> func = nullptr,
                     void *param = nullptr, const int64_t &startPts = 0);
    void pauseRecord();
    void restoreRecord();
    void stopRecord(const bool &isBlock = true);

    static RECT getWindowRect(HWND hWnd);
    static std::list<HWND> getCaptureWindowList(); //获取可以捕获的窗体列表
    static bool IsWindowAvailable(HWND hWnd);
    static bool IsCoveredByOtherWindow(HWND hWnd);

protected:
    void run();

private:

    HWND mHWnd;

    std::list<HWND> mIgnorHandleList; //忽略盖住的窗体句柄

    ///裁图区域
    int mX;
    int mY;
    int mW;
    int mH;

    bool mIsFollowMouseMode; //追随鼠标模式

    bool mIsStop;
    bool mIsThreadRunning;

    bool mIsPause;
    int64_t mStartTime; //第一次获取到数据的时间
    int64_t mCurrentTime; //当前时间戳

    int64_t mLastGetVideoTime; //上一次传入编码的时间(用来控制帧率)
    int mFrameRate;

    AVFrame	*pFrameRGB, *pFrameYUV;
    SwsContext *img_convert_ctx;
    uint8_t *outBufferYUV;
    uint8_t *outBufferRGB;

    ///采集到屏幕数据的回调函数
    std::function<void (VideoRawFramePtr videoFramePtr, void *param)> mCallBackFunc; //回调函数
    void *mCallBackFuncParam; //回调函数用户参数

    bool init(const int &width, const int &height);
    void deInit();
};

#endif // GetVideoThread_H
