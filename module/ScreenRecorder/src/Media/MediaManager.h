/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef MEDIAMANAGER_H
#define MEDIAMANAGER_H

#include <list>
#include <string>

#include "Video/CaptureWindowThread.h"
#include "Video/GetVideoThread.h"
#include "Audio/GetAudioThread.h"

#include "Audio/AudioEncoder.h"
#include "Video/VideoFileWriter.h"

#include "EventHandle/VideoRecorderEventHandle.h"

struct DeviceNode
{
    std::string deviceName;
    std::string deviceID;
};

struct VideoRECT
{
    RECT destRect;
};

enum TaskType
{
    TaskType_Window = 0,
    TaskType_Camera,
    TaskType_Picture,
};

class MediaManager
{
public:
    MediaManager();
    ~MediaManager();

    /**
     * @brief 添加捕获窗口的任务
     * @param id       [in] 任务ID
     * @param hWnd     [in] 窗口句柄
     * @param srcRect  [in] 捕获的窗口区域
     * @param destRect [in] 最终图像贴到主图的区域
     * @param alpha    [in] 图像透明度0~1.0
     */
    void addCaptureWindowTask(const int &id, const HWND &hWnd,
                              const RECT &srcRect, const RECT &destRect,
                              const float &alpha=1.0f);

    void addCaptureCameraTask(const int &id,
                              const RECT &destRect,
                              const float &alpha=1.0f);

    void addCapturePictureTask(const int &id, const std::string &filePath,
                               const RECT &destRect,
                               const float &alpha=1.0f);

    bool removeTask(const int &id);
    void clearTask();

    void stopAll(); //只能在释放之前调用此函数，否则会影响基本的功能

    /**
     * @brief startCapture
     * @param [in] hWnd
     * @param [in] audioDeviceName
     * @param [in] enableVirtualAudio 是否捕获虚拟声卡声音
     * @return
     */
    bool startCapture(const bool &enableVirtualAudio = true);
    bool stopCapture(const bool &isBlock = true);

    bool setMicroPhone(const std::string &deviceName);
    bool startCaptureMic(const std::string &deviceName);
    bool stopCaptureMic(const bool &isBlock = true);

    bool muteMicroPhone(bool isMute); //静音麦克风
    bool muteVirtualAudio(bool isMute); //静音声卡捕获的声音

    void setVideoRecorderEventHandle(VideoRecorderEventHandle *handle);

    int64_t getVideoFileCurrentTime(); //获取录屏的时间戳(毫秒)

    ///设置最终生成的视频文件的分辨率（如果此值和采集桌面区域不一致，那么图像将是经过压缩后再编码写入文件）
    void setVideoSize(const int &width, const int &height);
    void setQuality(const int &quality); //设置质量 0-10
    void setFrameRate(const int &frameRate);

    ///摄像头相关
    bool openCameraCaptureMode(const std::string &deviceName); //在直播设置界面打开的摄像头
    bool openCameraWindowMode(const std::string &deviceName); //在窗口上执行的打开摄像头
    bool closeCameraCaptureMode(); //在直播设置界面打开的摄像头
    bool closeCameraWindowMode(); //在窗口上执行的打开摄像头

    bool isRecording(); //是否正在录制
    bool openFile(const std::string &filePath);
    bool closeFile();
    std::list<VideoFileInfo> getVideoFileList(); //获取文件列表

    ///设置视频数据回调函数
    void setCameraFrameCallBackFunc(std::function<void (VideoRawFramePtr yuvFrame, VideoRawFramePtr rgbFrame, void *param)> func = nullptr, void *param = nullptr);
    void setFinalVideoFrameCallBackFunc(std::function<void (VideoRawFramePtr yuvFrame, void *param)> func = nullptr, void *param = nullptr);

    ///输入外部音视频数据
    void inputExternalYuvFrame(VideoRawFramePtr videoFrame);
    void inputExternalPcmFrame(PCMFramePtr pcmFrame);

    /**
     * @brief 获取设备列表
     * @param [out] videoDeviceList 存放获取到的视频设备列表
     * @param [out] audioDeviceList 存放获取到的音频设备列表
     * @return 获取成功为true
     */
    static  bool getDeviceList(std::list<DeviceNode> &videoDeviceList, std::list<DeviceNode> &audioDeviceList);

protected:
    bool mIsAudioBufferManagerStop;
    bool mIsAudioBufferManagerThreadRunning;
    void audioBufferManagerThreadFunc();

    bool mIsVideoBufferManagerStop;
    bool mIsVideoBufferManagerThreadRunning;
    void videoBufferManagerThreadFunc();

private:
    int64_t mStartWriteVideoTime; //开始写视频文件的时间，用于计算传给视频保存的时间戳
    int64_t mStartCaptureTime; //开始采集的时间，用于计算传给视频保存的时间戳
    bool mIsCaptureNow; //当前处理音频和视频的线程是否正在允许

    GetVideoThread *mGetCameraVideoThread;      //获取摄像头画面的线程
    GetAudioThread *mGetAudioThread;  //捕获系统输出音频
    GetAudioThread *mGetAudioThread_MicroPhone; //捕获麦克风音频

    bool mIsMicroPhoneMute; //静音麦克风捕获
    bool mIsVirtualAudioMute; //静音声卡捕获

    int mVideoFileWidth;  //最终视频文件的分辨率
    int mVideoFileHeight; //最终视频文件的分辨率
    VideoFileWriter *mVideoFileWriter;

    VideoRecorderEventHandle *mVideoRecorderEventHandle;

    ///摄像头数据回调函数
    std::function<void (VideoRawFramePtr yuvFrame, VideoRawFramePtr rgbFrame, void *param)> mCameraFrameCallBackFunc = nullptr; //摄像头数据回调函数
    void *mCameraFrameCallBackFuncParam = nullptr; //回调函数用户参数

    void startCaptureCamera(); //打开相机捕获线程
    void stopCaptureCamera();  //停止相机捕获线程

    ///实时数据回调函数
    std::function<void (VideoRawFramePtr yuvFrame, void *param)> mFinalVideoFrameCallBackFunc = nullptr; //摄像头数据回调函数
    void *mFinalVideoFrameCallBackFuncParam = nullptr; //回调函数用户参数

    int64_t mLastInputRtmpVideoTime; //记录上一次往rtmp推入视频的时间
    int mRtmpFrameRate;

    ///存放采集到的音频数据（用于混音）
    struct AudioManagerNode
    {
        GetAudioThread* thread;
        std::list<PCMFramePtr> pcmFrameList;
        int64_t lastGetFrameTime; //最近一次获取

        AudioManagerNode()
        {
            lastGetFrameTime = 0;
        }
    };

    VideoRawFramePtr mVideoFrameBackGround; //背景图

    ///存放采集到的视频数据（用于图片叠加）
    struct VideoManagerNode
    {
        int id;
        void* thread;
        TaskType type;
        VideoRawFramePtr videoFrame;
        VideoRECT rect;
        float alpha;
        int64_t lastGetFrameTime; //最近一次获取的时间

        VideoManagerNode()
        {
            videoFrame = nullptr;
            alpha = 1.0f;
            lastGetFrameTime = 0;
        }

        bool operator == (const VideoManagerNode & node)//重载运算符函数的具体实现
        {
            bool isSame = false;
            if (node.id == this->id)
            {
                isSame = true;
            }
            return isSame;
        }
    };

    Cond *mCond_Audio;
    std::list<AudioManagerNode> mAudioManagerList;

    Cond *mCond_Video;
    std::list<VideoManagerNode> mVideoManagerList;

    AudioEncoder *mAudioEncoder;
//    VideoEncoder *mVideoEncoder;

    void inputPcmFrame(PCMFramePtr pcmFrame, void *param);
    void inputVideoFrame(VideoRawFramePtr videoFrame, void *param);

    void startAudioBufferMangerThread();
    void stopAudioBufferMangerThread();

    bool mIsCameraOpenCaptureMode = false;
    bool mIsCameraOpenWindowMode  = false;
    bool openCamera(const std::string &deviceName);
    bool closeCamera();

};


#endif // MEDIAMANAGER_H
