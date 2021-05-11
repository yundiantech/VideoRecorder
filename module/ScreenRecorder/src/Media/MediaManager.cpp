/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "MediaManager.h"

#if defined(WIN32)
    #include<direct.h>
#else
    #include<unistd.h>
#endif

extern "C"
{
    #include "libyuv.h"
};

#include "Image/yuv420p.h"
#include "Image/ImageReader.h"

#include "MoudleConfig.h"
#include "Mix/PcmMix.h"

static bool gIsInited = false;

void doInit()
{
    if (!gIsInited)
    {
    #ifdef WIN32
        WSADATA dat;
        WSAStartup(MAKEWORD(2, 2), &dat);
    #endif // WIN32

        av_register_all();
        avformat_network_init();
        avdevice_register_all();

        gIsInited = true;
    }

}

MediaManager::MediaManager()
{
    doInit();

    mGetCameraVideoThread = nullptr;
    mGetAudioThread = nullptr;
    mGetAudioThread_MicroPhone = nullptr;

    mGetCameraVideoThread = new GetVideoThread(); //获取摄像头画面的线程
    mGetAudioThread = new GetAudioThread();
    mGetAudioThread_MicroPhone = new GetAudioThread();

//    mGetAudioThread->setIsNeedReOpenWhenReadFailed(false);
//    mGetAudioThread_MicroPhone->setIsNeedReOpenWhenReadFailed(true);

    mCond_Audio = new Cond();
    mCond_Video = new Cond();

//    mVideoEncoder     = new VideoEncoder();
    mAudioEncoder     = new AudioEncoder();
//    mVideoEncoder->openEncoder();
    mAudioEncoder->openEncoder();

    mIsAudioBufferManagerStop = true;
    mIsAudioBufferManagerThreadRunning = false;

    mIsVideoBufferManagerStop = true;
    mIsVideoBufferManagerThreadRunning = false;

    mIsCaptureNow = false;
    mStartCaptureTime = 0;
    mStartWriteVideoTime = 0;

    mIsMicroPhoneMute   = false;
    mIsVirtualAudioMute = false;

    mVideoRecorderEventHandle = nullptr;

    mCameraFrameCallBackFunc      = nullptr; //摄像头数据回调函数
    mCameraFrameCallBackFuncParam = nullptr; //回调函数用户参数

    mVideoFileWidth  = 0;
    mVideoFileHeight = 0;
    mVideoFileWriter = new VideoFileWriter();

    mLastInputRtmpVideoTime = 0; //记录上一次往rtmp推入视频的时间
    mRtmpFrameRate = 10;

    ///记录获取音频的2个线程，用于混音
    {
        {
            AudioManagerNode node;
            node.thread = mGetAudioThread;
            node.pcmFrameList.clear();
            node.lastGetFrameTime = MoudleConfig::getTimeStamp_MilliSecond();

            mAudioManagerList.push_back(node);
        }

        {
            AudioManagerNode node;
            node.thread = mGetAudioThread_MicroPhone;
            node.pcmFrameList.clear();
            node.lastGetFrameTime = MoudleConfig::getTimeStamp_MilliSecond();

            mAudioManagerList.push_back(node);
        }
    }

    setFrameRate(15);

    startCaptureCamera();
    startAudioBufferMangerThread();

//    {
//        addCaptureWindowTask(1, NULL, RECT{0, 0, 1920, 1080},   RECT{0, 0, 1920, 1080},    1.0);
//        addCaptureWindowTask(2, NULL, RECT{300, 300, 940, 980}, RECT{500, 500, 800, 900}, 0.5);

//        addCaptureCameraTask(11, "USB2.0 PC CAMERA", RECT{1600, 0, 1920, 480}, 1.0);
//        addCaptureCameraTask(12, "USB2.0 PC CAMERA", RECT{100, 0, 620, 480}, 1.0);

//    }
}

MediaManager::~MediaManager()
{
qDebug()<<__FUNCTION__;
    stopAll();
qDebug()<<__FUNCTION__<<"finished!";
}

void MediaManager::stopAll()
{
    stopCapture();
    stopCaptureCamera();
    stopAudioBufferMangerThread();

    clearTask();
}

void MediaManager::setCameraFrameCallBackFunc(std::function<void (VideoRawFramePtr yuvFrame, VideoRawFramePtr rgbFrame, void *param)> func, void *param)
{
    mCameraFrameCallBackFunc = func;
    mCameraFrameCallBackFuncParam = param;
}

void MediaManager::setFinalVideoFrameCallBackFunc(std::function<void (VideoRawFramePtr yuvFrame, void *param)> func, void *param)
{
    mFinalVideoFrameCallBackFunc = func;
    mFinalVideoFrameCallBackFuncParam = param;
}

void MediaManager::startAudioBufferMangerThread()
{
    if (!mIsAudioBufferManagerThreadRunning)
    {
        std::thread([=]
        {
            mIsAudioBufferManagerStop = false;

            this->audioBufferManagerThreadFunc();

        }).detach();
    }
}

void MediaManager::stopAudioBufferMangerThread()
{
    mIsAudioBufferManagerStop = true;

    while(mIsAudioBufferManagerThreadRunning)
    {
        MoudleConfig::mSleep(100);
    }
}

bool MediaManager::startCapture(const bool &enableVirtualAudio)
{
    if (mIsCaptureNow)
    {
        return true;
    }

    auto inputPcmBufferFunc = [&](PCMFramePtr pcmFrame, void *param)
    {
        inputPcmFrame(pcmFrame, param);
    };

    bool isSucceed = false;

    if (enableVirtualAudio)
    {
        if (mGetAudioThread->openDevice("virtual-audio-capturer"))
        {
            isSucceed = true;
        }
    }
    else
    {
        isSucceed = true;
    }

    if (isSucceed)
    {
        mIsCaptureNow = true;
        mStartCaptureTime = MoudleConfig::getTimeStamp_MilliSecond();

        int64_t currentTime = mVideoFileWriter->getVideoPts();
        mStartWriteVideoTime = MoudleConfig::getTimeStamp_MilliSecond() - currentTime;
        qDebug()<<__FUNCTION__<<"currentTime:"<<currentTime;

//        std::thread([=]
//        {
//            mIsAudioBufferManagerStop = false;

//            this->audioBufferManagerThreadFunc();

//        }).detach();

        std::thread([=]
        {
            mIsVideoBufferManagerStop = false;

            this->videoBufferManagerThreadFunc();

        }).detach();

        if (enableVirtualAudio)
        {
            mGetAudioThread->startRecord(mAudioEncoder->getONEFrameSize(), inputPcmBufferFunc, mGetAudioThread);
        }
    }

    return isSucceed;
}

bool MediaManager::stopCapture(const bool &isBlock)
{
    stopCaptureMic();

    if (mGetAudioThread != nullptr)
    {
        mGetAudioThread->stopRecord(isBlock);
    }

    {
//        mIsAudioBufferManagerStop = true;
        mIsVideoBufferManagerStop = true;

//        while(mIsAudioBufferManagerThreadRunning)
//        {
//            MoudleConfig::mSleep(100);
//        }

        while(mIsVideoBufferManagerThreadRunning)
        {
            MoudleConfig::mSleep(100);
        }
    }

    mIsCaptureNow = false;

    return true;
}

bool MediaManager::muteMicroPhone(bool isMute)
{
    mIsMicroPhoneMute = isMute;
    return true;
}

bool MediaManager::muteVirtualAudio(bool isMute)
{
    mIsVirtualAudioMute = isMute;
    return true;
}

bool MediaManager::setMicroPhone(const std::string &deviceName)
{
    bool ret = startCaptureMic(deviceName);
    return ret;
}

bool MediaManager::startCaptureMic(const std::string &deviceName)
{
    auto inputPcmBufferFunc = [&](PCMFramePtr pcmFrame, void *param)
    {
        inputPcmFrame(pcmFrame, param);
    };
qDebug()<<__FUNCTION__<<"000";
    stopCaptureMic();
qDebug()<<__FUNCTION__<<"111";
    bool isSucceed = false;

    if (mGetAudioThread_MicroPhone->openDevice(deviceName))
    {
        qDebug()<<__FUNCTION__<<"222";
        mGetAudioThread_MicroPhone->startRecord(mAudioEncoder->getONEFrameSize(), inputPcmBufferFunc, mGetAudioThread_MicroPhone);

        isSucceed = true;
    }
qDebug()<<__FUNCTION__<<"333";
    return isSucceed;
}

bool MediaManager::stopCaptureMic(const bool &isBlock)
{
    if (mGetAudioThread_MicroPhone != nullptr)
    {
        mGetAudioThread_MicroPhone->stopRecord(isBlock);
    }

    return true;
}

void MediaManager::startCaptureCamera()
{
    auto getCameraVideoFrameFunc = [=](VideoRawFramePtr yuvFramePtr, VideoRawFramePtr rgbFramePtr, void *param)
    {
        if (mCameraFrameCallBackFunc != nullptr)
        {
            mCameraFrameCallBackFunc(yuvFramePtr, rgbFramePtr, param);
        }

        inputVideoFrame(yuvFramePtr, param);
    };

    if (mGetCameraVideoThread != nullptr)
    {
        mGetCameraVideoThread->startRecord(getCameraVideoFrameFunc, mGetCameraVideoThread);
    }
}

void MediaManager::stopCaptureCamera()
{
    if (mGetCameraVideoThread != nullptr)
    {
        mGetCameraVideoThread->stopRecord(true);
    }
}

bool MediaManager::openCamera(const std::string &deviceName)
{
    bool isSucceed = false;

    if (mGetCameraVideoThread->openCamera(deviceName) == GetVideoThread::SUCCEED)
    {
        isSucceed = true;
    }

    return isSucceed;
}

bool MediaManager::closeCamera()
{
    mGetCameraVideoThread->closeCamera();

    return true;
}

bool MediaManager::openCameraCaptureMode(const std::string &deviceName)
{
    mIsCameraOpenCaptureMode = true;
    bool isSucceed = openCamera(deviceName);
    return isSucceed;
}

bool MediaManager::openCameraWindowMode(const std::string &deviceName)
{
    mIsCameraOpenWindowMode = true;
    bool isSucceed = openCamera(deviceName);
    return isSucceed;
}

bool MediaManager::closeCameraCaptureMode()
{
    mIsCameraOpenCaptureMode = false;

    if (!mIsCameraOpenCaptureMode && !mIsCameraOpenWindowMode)
    {
        closeCamera();
    }

    return true;

}

bool MediaManager::closeCameraWindowMode()
{
    mIsCameraOpenWindowMode = false;

    if (!mIsCameraOpenCaptureMode && !mIsCameraOpenWindowMode)
    {
        closeCamera();
    }

    return true;
}

void MediaManager::setQuality(const int &quality)
{
    mVideoFileWriter->setQuality(quality);
}

void MediaManager::setFrameRate(const int &frameRate)
{
    mRtmpFrameRate = frameRate + 5; //每秒钟传给写视频的线程多几针，因为写视频的线程里面还有自己的丢帧策略
    mVideoFileWriter->setVideoFrameRate(frameRate);
}

bool MediaManager::isRecording()
{
    return mVideoFileWriter->isThreadRunning();
}

bool MediaManager::openFile(const std::string &filePath)
{
    mStartWriteVideoTime = MoudleConfig::getTimeStamp_MilliSecond();

    auto videoEncodedFunc = [=](VideoEncodedFramePtr videoFramePtr, void *param)
    {
//        qDebug()<<__FUNCTION__<<videoFramePtr->getNalu()->nalu.h264Nalu.len;
//        if (param == mVideoEncoder)
//        {
//            mRtmpSender->inputVideoFrame(videoFramePtr);
//        }
    };
qDebug()<<__FUNCTION__<<"111";
    bool ret = true;

    mVideoFileWriter->setFileName(filePath);
qDebug()<<__FUNCTION__<<"222";
    mVideoFileWriter->startEncode(videoEncodedFunc, nullptr);
qDebug()<<__FUNCTION__<<"333";
qDebug()<<__FUNCTION__<<filePath.c_str();
    return ret;
}

bool MediaManager::closeFile()
{
    mVideoFileWriter->stopEncode();
    return true;
}

std::list<VideoFileInfo> MediaManager::getVideoFileList()
{
    return mVideoFileWriter->getVideoFileList();
}

int64_t MediaManager::getVideoFileCurrentTime()
{
    int64_t time = mVideoFileWriter->getVideoPts();
    return time;
}

void MediaManager::setVideoSize(const int &width, const int &height)
{
    int w = width;
    int h = height;

    if (w % 2 != 0)
    {
        w++;
    }

    if (h % 2 != 0)
    {
        h++;
    }

    mCond_Video->Lock();

    mVideoFileWidth  = w;
    mVideoFileHeight = h;

    mVideoFileWriter->setWidth(mVideoFileWidth, mVideoFileHeight);

    mVideoFrameBackGround = std::make_shared<VideoRawFrame>();
    mVideoFrameBackGround->initBuffer(mVideoFileWidth, mVideoFileHeight, VideoRawFrame::FRAME_TYPE_YUV420P);

    int YSize = mVideoFileWidth * mVideoFileHeight;
    int USize = mVideoFileWidth * mVideoFileHeight / 4;
    int VSize = USize;

    ///黑色Yuv是(0,128,128);
    memset(mVideoFrameBackGround->getBuffer() + YSize, 128, USize+VSize);

    mCond_Video->Unlock();

}

//void MediaManager::setCaptureWindowRect(const int &x, const int &y, const int &width, const int &height)
//{
//    mGetScreenVideoThread->setRect(x, y, width, height);
//}

void MediaManager::setVideoRecorderEventHandle(VideoRecorderEventHandle *handle)
{
    mVideoRecorderEventHandle = handle;
//    mVideoFileWriter->setEventHandle(handle);
}

void MediaManager::addCaptureWindowTask(const int &id, const HWND &hWnd, const RECT &srcRect, const RECT &destRect, const float &alpha)
{
qDebug()<<__FUNCTION__<<id<<hWnd<<srcRect.left<<srcRect.top<<srcRect.right<<srcRect.bottom<<destRect.left<<destRect.top<<destRect.right<<destRect.bottom<<alpha;

    VideoManagerNode node;
    node.id = id;
//    node.thread = thread;
    node.videoFrame = nullptr;
    node.type = TaskType_Window;
    node.rect.destRect = destRect;
    node.alpha = alpha;
    node.lastGetFrameTime = MoudleConfig::getTimeStamp_MilliSecond();

    CaptureWindowThread *thread = nullptr;
qDebug()<<__FUNCTION__<<"000";
    mCond_Video->Lock();
qDebug()<<__FUNCTION__<<"111";
    for (VideoManagerNode tmpNode : mVideoManagerList)
    {
        if (tmpNode.id == id)
        {
            thread = (CaptureWindowThread*)tmpNode.thread;
            node.videoFrame = tmpNode.videoFrame;
            break;
        }
    }

    if (thread == nullptr)
    {
        thread = new CaptureWindowThread(); //捕获窗口图像的线程;
    }

    node.thread = thread;

    mVideoManagerList.remove(node);
    mVideoManagerList.push_back(node);

    mCond_Video->Unlock();

    auto getScreenVideoFrameFunc = [=](VideoRawFramePtr videoFramePtr, void *param)
    {
        inputVideoFrame(videoFramePtr, param);
    };
    int64_t currentTime = mVideoFileWriter->getVideoPts();

    thread->stopRecord(true);

    int x = srcRect.left;
    int y = srcRect.top;
    int w = srcRect.right - srcRect.left;
    int h = srcRect.bottom - srcRect.top;
qDebug()<<__FUNCTION__<<"222"<<x<<y<<w<<h;
    thread->setHWND(hWnd);
    thread->setRect(x, y, w, h);

    thread->startRecord(getScreenVideoFrameFunc, thread, currentTime);

    qDebug()<<__FUNCTION__<<"111"<<id<<hWnd<<srcRect.left<<srcRect.top<<srcRect.right<<srcRect.bottom<<destRect.left<<destRect.top<<destRect.right<<destRect.bottom<<alpha;

}

void MediaManager::addCaptureCameraTask(const int &id, const RECT &destRect, const float &alpha)
{
    VideoManagerNode node;
    node.id = id;
    node.thread = mGetCameraVideoThread;
    node.videoFrame = nullptr;
    node.type = TaskType_Camera;
    node.rect.destRect = destRect;
    node.alpha = alpha;
    node.lastGetFrameTime = MoudleConfig::getTimeStamp_MilliSecond();

    mCond_Video->Lock();
    mVideoManagerList.remove(node);
    mVideoManagerList.push_back(node);
    mCond_Video->Unlock();
}

void MediaManager::addCapturePictureTask(const int &id, const std::string &filePath, const RECT &destRect, const float &alpha)
{
    VideoRawFramePtr videoFrame = nullptr;

    {
        int width  = 0;
        int height = 0;
        int ret = ImageReader::ReadYuv420pBuffer(filePath.c_str(), NULL, 0, &width, &height);
qDebug()<<__FUNCTION__<<filePath.c_str()<<ret<<width<<height<<destRect.left<<destRect.top<<destRect.right<<destRect.bottom;
        if (ret == 0 && width > 0 && height > 0)
        {
            videoFrame = std::make_shared<VideoRawFrame>();
            videoFrame->initBuffer(width, height, VideoRawFrame::FRAME_TYPE_YUV420P);

            ImageReader::ReadYuv420pBuffer(filePath.c_str(), videoFrame->getBuffer(), videoFrame->getSize(), &width, &height);
        }
    }

//    FILE *fp = fopen("out-pic.yuv", "wb");
//    fwrite(videoFrame->getBuffer(), 1, videoFrame->getSize(), fp);
//    fclose(fp);

    VideoManagerNode node;
    node.id = id;
    node.thread = nullptr;
    node.videoFrame = videoFrame;
    node.type = TaskType_Picture;
    node.rect.destRect = destRect;
    node.alpha = alpha;
    node.lastGetFrameTime = MoudleConfig::getTimeStamp_MilliSecond();

    mCond_Video->Lock();
    mVideoManagerList.remove(node);
    mVideoManagerList.push_back(node);
    mCond_Video->Unlock();
}

bool MediaManager::removeTask(const int &id)
{
    bool isExist = false;

    std::list<CaptureWindowThread *> needStopThreadList; //记录需要调用stopRecord(true) 的任务指针。

    mCond_Video->Lock();

    for (VideoManagerNode node : mVideoManagerList)
    {
        if (node.id == id)
        {
            isExist = true;

            if (node.type == TaskType_Window)
            {
                CaptureWindowThread * thread = (CaptureWindowThread*)node.thread;
                thread->stopRecord(false);
                needStopThreadList.push_back(thread);
                ///这里不能等待，因为线程里面传数据过来的时候，调用了inputVideoFrame,里面也调用了mCond_Video->Lock()，这样就会造成死锁。
                /// 因此放到后面等锁释放后再来阻塞等待。
//                thread->stopRecord(true);
//                delete thread;
            }
            else if (node.type == TaskType_Camera)
            {
                GetVideoThread * thread = (GetVideoThread*)node.thread;
    //            thread->startRecord(getCameraVideoFrameFunc, thread);
            }
            else if (node.type == TaskType_Picture)
            {

            }
        }
    }
qDebug()<<__FUNCTION__<<id<<isExist;
    if (isExist)
    {
        VideoManagerNode node;
        node.id = id;

        mVideoManagerList.remove(node);
    }

    mCond_Video->Unlock();

    for (CaptureWindowThread * thread : needStopThreadList)
    {
        thread->stopRecord(true);
        delete thread;
    }

    return isExist;
}

void MediaManager::clearTask()
{
    std::list<CaptureWindowThread *> needStopThreadList; //记录需要调用stopRecord(true) 的任务指针。

    mCond_Video->Lock();

    for (VideoManagerNode node : mVideoManagerList)
    {
        {
            if (node.type == TaskType_Window)
            {
                CaptureWindowThread * thread = (CaptureWindowThread*)node.thread;
                thread->stopRecord(false);
                needStopThreadList.push_back(thread);
                ///这里不能等待，因为线程里面传数据过来的时候，调用了inputVideoFrame,里面也调用了mCond_Video->Lock()，这样就会造成死锁。
                /// 因此放到后面等锁释放后再来阻塞等待。
//                thread->stopRecord(true);
//                delete thread;
            }
            else if (node.type == TaskType_Camera)
            {
                GetVideoThread * thread = (GetVideoThread*)node.thread;
//                thread->stopRecord(true);
//                delete thread;
            }
            else if (node.type == TaskType_Picture)
            {

            }
        }
    }

    mVideoManagerList.clear();

    mCond_Video->Unlock();


    for (CaptureWindowThread * thread : needStopThreadList)
    {
        thread->stopRecord(true);
        delete thread;
    }
}

void MediaManager::inputPcmFrame(PCMFramePtr pcmFrame, void *param)
{
    mCond_Audio->Lock();
    ///将数据存入队列，然后在后面的线程中，取出数据，混音后再拿去编码。
    std::list<AudioManagerNode>::iterator iter;
    for (iter=mAudioManagerList.begin();iter!=mAudioManagerList.end();iter++)
    {
        if ((*iter).thread == param)
        {
            bool isNeedPush = false;
            if ((*iter).thread == mGetAudioThread && !mIsVirtualAudioMute)
            {
//                fprintf(stderr, ">> %s\n", __FUNCTION__);
                isNeedPush = true;
            }

            if ((*iter).thread == mGetAudioThread_MicroPhone && !mIsMicroPhoneMute)
            {
//                fprintf(stderr, ".. %s\n", __FUNCTION__);
                isNeedPush = true;
            }

            if (isNeedPush)
            {
                (*iter).pcmFrameList.push_back(pcmFrame);
                (*iter).lastGetFrameTime = MoudleConfig::getTimeStamp_MilliSecond();
            }
//            else
//            {
//                fprintf(stderr, "none... %s\n", __FUNCTION__);
//            }
        }
    }

    if (param == mGetAudioThread_MicroPhone)
    {
        if (!mIsMicroPhoneMute)
        {
            #define PCMTYPE short
            #define MAXPCMVALUE 32767

            PCMTYPE *buffer = (PCMTYPE *)pcmFrame->getBuffer();

            int volumeL = abs(buffer[0] * 100.0 / MAXPCMVALUE);
            int volumeR = abs(buffer[1] * 100.0 / MAXPCMVALUE);

//            fprintf(stderr, "%s %d %d %f\n", __FUNCTION__, volumeL, volumeR, *((float*)buffer));

            volumeL = abs(*((float*)buffer) * 100);
            volumeR = abs(*((float*)buffer) * 100);

            volumeL *= 3;

//            if (mRtmpSenderEventHandle != nullptr)
//            {
//                mRtmpSenderEventHandle->OnAudioVolumeUpdated(volumeL, volumeR);
//            }
        }
    }

    mCond_Audio->Unlock();
    mCond_Audio->Signal();
}

void MediaManager::inputVideoFrame(VideoRawFramePtr videoFrame, void *param)
{
    mCond_Video->Lock();
    ///将数据存入队列，然后在后面的线程中，取出数据，混音后再拿去编码。
    std::list<VideoManagerNode>::iterator iter;
    for (iter=mVideoManagerList.begin();iter!=mVideoManagerList.end();iter++)
    {
        if ((*iter).thread == param)
        {
            (*iter).videoFrame = videoFrame;
            (*iter).lastGetFrameTime = MoudleConfig::getTimeStamp_MilliSecond();
        }
    }

    mCond_Video->Unlock();
    mCond_Video->Signal();
}

void MediaManager::inputExternalYuvFrame(VideoRawFramePtr videoFrame)
{
    int64_t pts = (MoudleConfig::getTimeStamp_MilliSecond() - mStartWriteVideoTime);
    videoFrame->setPts(pts);
//qDebug()<<__FUNCTION__<<isRecording()<<destYuvFrame->getWidth()<<destYuvFrame->getHeight()<<pts;
    if (isRecording())
    {
        mVideoFileWriter->inputYuvFrame(videoFrame);
    }
}

void MediaManager::inputExternalPcmFrame(PCMFramePtr pcmFrame)
{
    inputPcmFrame(pcmFrame, mGetAudioThread);
}

void MediaManager::audioBufferManagerThreadFunc()
{
    mIsAudioBufferManagerThreadRunning = true;

    mAudioEncoder->openEncoder();

    while(!mIsAudioBufferManagerStop)
    {
        std::list<PCMFramePtr> waitEncodeFrameList;

        mCond_Audio->Lock();

        do
        {
            ///判断队列里面是否有数据
            bool hasBuffer = true;

//            int i=0;

            for (std::list<AudioManagerNode>::iterator iter = mAudioManagerList.begin(); iter!=mAudioManagerList.end(); iter++)
            {
                ///由于读取声卡数据的时候，当声卡没有输出的时候，采集就不会获取到数据，因此需要判断采集声卡的线程是否有数据。
                if ( ((MoudleConfig::getTimeStamp_MilliSecond() - (*iter).lastGetFrameTime) < 1000)
                     && ((*iter).pcmFrameList.size() <= 0)) //一秒内获取过数据，且队列是空的，那么继续等待
                {
                    hasBuffer = false;
                    break;
                }
//qDebug()<<__FUNCTION__<<i++<<((*iter).thread==mGetAudioThread_MicroPhone)<<(*iter).pcmFrameList.size();
            }

            if (hasBuffer)
            {
                waitEncodeFrameList.clear();
                for (std::list<AudioManagerNode>::iterator iter =mAudioManagerList.begin(); iter!=mAudioManagerList.end(); iter++)
                {
                    std::list<PCMFramePtr> &tmpFrameList = (*iter).pcmFrameList;
                    if (!tmpFrameList.empty())
                    {
                        waitEncodeFrameList.push_back(tmpFrameList.front());
                        tmpFrameList.pop_front();
                    }
                }
//                fprintf(stderr, "%s waitEncodeFrameList size = %d \n",__FUNCTION__, waitEncodeFrameList.size());
                break;
            }
            else
            {
                mCond_Audio->Wait(1000);
            }

            if (mIsAudioBufferManagerStop) break;

        }while(1);

        mCond_Audio->Unlock();

        if (waitEncodeFrameList.size() > 0)
        {
            ///这里的PCM数据格式为：AV_SAMPLE_FMT_FLTP

            ///实现混音
            float *srcData[10] = {0};
            int number=0;
            int bufferSize = 0;

            int64_t pts = 0;

            for (PCMFramePtr & pcmFrame : waitEncodeFrameList)
            {
                srcData[number++] = (float*)pcmFrame->getBuffer();
                bufferSize = pcmFrame->getSize(); //由于采集的时候做了处理，因此这里每一帧的size都是一样的。

                pts = pcmFrame->getPts();
            }

            uint8_t * pcmBuffer = (uint8_t*)malloc(bufferSize);
            PcmMix::NormalizedRemix(srcData, number, bufferSize, (float*)pcmBuffer);

            PCMFramePtr pcmFramePtr = std::make_shared<PCMFrame>();
            pcmFramePtr->setFrameBuffer(pcmBuffer, bufferSize, pts);

            if (isRecording())
            {
                mVideoFileWriter->inputPcmFrame(pcmFramePtr);
            }

//            #define PCMTYPE short
//            #define MAXPCMVALUE 32767

//            PCMTYPE *buffer = (PCMTYPE *)pcmBuffer;

//            int volumeL = abs(buffer[0] * 100.0 / MAXPCMVALUE);
//            int volumeR = abs(buffer[1] * 100.0 / MAXPCMVALUE);

//            AACFramePtr aacFrame = mAudioEncoder->encode(pcmBuffer, bufferSize);
//            free(pcmBuffer);

//            if (aacFrame != nullptr && aacFrame.get() != nullptr)
//            {
//#if 0 ///写入aac文件
//                static FILE *aacFp = fopen("out.aac", "wb");
//                fwrite(aacFrame->getBuffer(), 1, aacFrame->getSize(), aacFp);
//#endif
//                aacFrame->setPts(pts);

//                mRtmpSender->inputAudioFrame(aacFrame);

//                if (mRtmpSenderEventHandle != nullptr)
//                {
//                    mRtmpSenderEventHandle->OnAudioVolumeUpdated(volumeL, volumeR);
//                }
//            }
        }
    }

    mAudioEncoder->closeEncoder();

    mIsAudioBufferManagerThreadRunning = false;
}

void scaleI420(uint8_t *src_i420_data, int width, int height, uint8_t *dst_i420_data, int dst_width, int dst_height)
{

    int src_i420_y_size = width * height;
    int src_i420_u_size = (width >> 1) * (height >> 1);
    uint8_t *src_i420_y_data = src_i420_data;
    uint8_t *src_i420_u_data = src_i420_data + src_i420_y_size;
    uint8_t *src_i420_v_data = src_i420_data + src_i420_y_size + src_i420_u_size;

    int dst_i420_y_size = dst_width * dst_height;
    int dst_i420_u_size = (dst_width >> 1) * (dst_height >> 1);
    uint8_t *dst_i420_y_data = dst_i420_data;
    uint8_t *dst_i420_u_data = dst_i420_data + dst_i420_y_size;
    uint8_t *dst_i420_v_data = dst_i420_data + dst_i420_y_size + dst_i420_u_size;

    libyuv::I420Scale((const uint8_t *) src_i420_y_data, width,
                      (const uint8_t *) src_i420_u_data, width >> 1,
                      (const uint8_t *) src_i420_v_data, width >> 1,
                      width, height,
                      (uint8_t *) dst_i420_y_data, dst_width,
                      (uint8_t *) dst_i420_u_data, dst_width >> 1,
                      (uint8_t *) dst_i420_v_data, dst_width >> 1,
                      dst_width, dst_height,
                      libyuv::kFilterNone);
}

void yuvMerge(uint8_t *mainYuv420pBuffer, int mainWidth, int mainHeight,
              uint8_t *childYuv420pBuffer, int childWidth, int childHeight,
              int posX, int posY)
{
    uint8_t *mainYBuffer = mainYuv420pBuffer;
    uint8_t *mainUBuffer = mainYBuffer + mainWidth * mainHeight;
    uint8_t *mainVBuffer = mainUBuffer + (mainWidth * mainHeight / 4);

    uint8_t *childYBuffer = childYuv420pBuffer;
    uint8_t *childUBuffer = childYBuffer + childWidth * childHeight;
    uint8_t *childVBuffer = childUBuffer + (childWidth * childHeight / 4);

qDebug()<<__FUNCTION__<<mainWidth<<mainHeight<<childWidth<<childHeight;

    for (int y = 0; y < childHeight; y++)
    {
        uint8_t *yBuffer = mainYBuffer + ((y+posY)*mainWidth) + posX;
        uint8_t *yBufferChild = childYBuffer + (y*childWidth);

        int len = childWidth;

        memcpy(yBuffer, yBufferChild, len);
    }

    for (int y = 0; y < (childHeight/4); y++)
    {
        uint8_t *uBuffer = mainUBuffer + ((y+posY)*mainWidth) + posX;
        uint8_t *uBufferChild = childUBuffer + (y*childWidth);

        int len = childWidth;

        memcpy(uBuffer, uBufferChild, len);
    }

    for (int y = 0; y < (childHeight/4); y++)
    {
        uint8_t *vBuffer = mainVBuffer + ((y+posY)*mainWidth) + posX;
        uint8_t *vBufferChild = childVBuffer + (y*childWidth);

        int len = childWidth;

        memcpy(vBuffer, vBufferChild, len);
    }
}

void MediaManager::videoBufferManagerThreadFunc()
{
    mIsVideoBufferManagerThreadRunning = true;

    while(!mIsVideoBufferManagerStop)
    {
        int64_t currentSystemTime = MoudleConfig::getTimeStamp_MilliSecond();
        if ((currentSystemTime - mLastInputRtmpVideoTime) >= (1000.0 / mRtmpFrameRate))
        {
            mLastInputRtmpVideoTime = currentSystemTime;

            mCond_Video->Lock();

            if (mVideoFrameBackGround == nullptr)
            {
                mCond_Video->Unlock();
                continue;
            }

            VideoRawFramePtr destYuvFrame = std::make_shared<VideoRawFrame>();
            destYuvFrame->initBuffer(mVideoFrameBackGround->getWidth(), mVideoFrameBackGround->getHeight(), VideoRawFrame::FRAME_TYPE_YUV420P);
            destYuvFrame->setFramebuf(mVideoFrameBackGround->getBuffer());

//            int64_t pts = 0;

            for (VideoManagerNode node : mVideoManagerList)
            {
                VideoRawFramePtr videoFrameTmp = node.videoFrame;
                VideoRawFramePtr videoFrame = videoFrameTmp;
//qDebug()<<__FUNCTION__<<node.id<<(node.videoFrame==nullptr);
                if (videoFrame != nullptr && videoFrame->getBuffer() != nullptr)
                {
//                    if (videoFrame->getPts() != 0)
//                    {
//                        pts = videoFrame->getPts();
//                    }

                    int destFrameWidth  = node.rect.destRect.right - node.rect.destRect.left;
                    int destFrameHeight = node.rect.destRect.bottom - node.rect.destRect.top;

                    ///捕获到的分辨率 与最终要的不一致，则执行一次压缩（主要是摄像头才会有这个现象）
                    if (destFrameWidth != videoFrameTmp->getWidth() || destFrameHeight != videoFrameTmp->getHeight())
                    {
                        videoFrame = std::make_shared<VideoRawFrame>();
                        videoFrame->initBuffer(destFrameWidth, destFrameHeight, VideoRawFrame::FRAME_TYPE_YUV420P, videoFrameTmp->getPts());

                        scaleI420(videoFrameTmp->getBuffer(), videoFrameTmp->getWidth(), videoFrameTmp->getHeight(),
                                  videoFrame->getBuffer(), destFrameWidth, destFrameHeight);
////qDebug()<<__FUNCTION__<<videoFrameTmp->getWidth()<<videoFrameTmp->getHeight()<<destFrameWidth<<destFrameHeight<<node.rect.destRect.left<<node.rect.destRect.top;
//                        FILE *fp = fopen("out-0.yuv", "wb");
//                        fwrite(videoFrame->getBuffer(), 1, videoFrame->getSize(), fp);
//                        fclose(fp);
                    }

                    codImageFrame frame;
                    frame.data   = videoFrame->getBuffer();				// memory pointer
                    frame.width  = videoFrame->getWidth();				// width of image
                    frame.height = videoFrame->getHeight();				// height of image
                    frame.stride = videoFrame->getWidth();				// stride of image
                    frame.pixfmt = cod_fmt_i420;						// pixel format of image

                    codImageFrame dstFrame;
                    dstFrame.data   = destYuvFrame->getBuffer();		// memory pointer
                    dstFrame.width  = destYuvFrame->getWidth();			// width of image
                    dstFrame.height = destYuvFrame->getHeight();		// height of image
                    dstFrame.stride = destYuvFrame->getWidth();			// stride of image
                    dstFrame.pixfmt = cod_fmt_i420;						// pixel format of image

                    blend_420p_planar (&frame, node.rect.destRect.left, node.rect.destRect.top, node.alpha, &dstFrame);

                }
            }

            mCond_Video->Unlock();

            int64_t pts = (MoudleConfig::getTimeStamp_MilliSecond() - mStartWriteVideoTime);
            destYuvFrame->setPts(pts);
//qDebug()<<__FUNCTION__<<isRecording()<<destYuvFrame->getWidth()<<destYuvFrame->getHeight()<<pts;
            if (isRecording())
            {
                mVideoFileWriter->inputYuvFrame(destYuvFrame);
            }

            if (mFinalVideoFrameCallBackFunc != nullptr)
            {
                mFinalVideoFrameCallBackFunc(destYuvFrame, mFinalVideoFrameCallBackFuncParam);
            }

//            FILE *fp = fopen("out.yuv", "wb");
//            fwrite(destYuvFrame->getBuffer(), 1, destYuvFrame->getSize(), fp);
//            fclose(fp);
        }
        else
        {
            MoudleConfig::mSleep(5);
        }
    }

    mIsVideoBufferManagerThreadRunning = false;
}

#if 0

#if defined(WIN32)
std::string UTF8ToGB(const char* str)
{
    std::string result;
    WCHAR *strSrc;
    LPSTR szRes;

    //获得临时变量的大小
    int i = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    strSrc = new WCHAR[i + 1];
    MultiByteToWideChar(CP_UTF8, 0, str, -1, strSrc, i);

    //获得临时变量的大小
    i = WideCharToMultiByte(CP_ACP, 0, strSrc, -1, NULL, 0, NULL, NULL);
    szRes = new CHAR[i + 1];
    WideCharToMultiByte(CP_ACP, 0, strSrc, -1, szRes, i, NULL, NULL);

    result = szRes;
    delete[]strSrc;
    delete[]szRes;

    return result;
}
#endif

bool MediaManager::getDeviceList(std::list<DeviceNode> &videoDeviceList, std::list<DeviceNode> &audioDeviceList)
{
    bool isSucceed = false;

    /// 执行ffmpeg命令行 获取音视频设备
    /// 请将ffmpeg.exe和程序放到同一个目录下

    char dirPath[512] = {0};
    getcwd(dirPath, sizeof (dirPath));

#ifdef WIN32

    std::string ffmpegPath = std::string(dirPath) + "/ffmpeg.exe";
    ffmpegPath = MoudleConfig::stringReplaceAll(ffmpegPath, "/","\\\\");

    #if 0
        std::string cmdStr = AppConfig::stringFormat(" /c \"%s\" -list_devices true -f dshow -i dummy 2>ffmpeg_device_out.txt", ffmpegPath.c_str());

        std::wstring str;
        {
            char * c = (char*)cmdStr.c_str();
            size_t m_encode = CP_ACP;
            int len = MultiByteToWideChar(m_encode, 0, c, strlen(c), NULL, 0);
            wchar_t*	m_wchar = new wchar_t[len + 1];
            MultiByteToWideChar(m_encode, 0, c, strlen(c), m_wchar, len);
            m_wchar[len] = '\0';
            str = m_wchar;
            delete m_wchar;
        }

        fprintf(stderr, "%s %s \n", __FUNCTION__, str.c_str());

        SHELLEXECUTEINFO ShExecInfo = {0};
        ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
        ShExecInfo.fMask = SEE_MASK_FLAG_NO_UI;
        ShExecInfo.hwnd = NULL;
        ShExecInfo.lpVerb = NULL;
        ShExecInfo.lpFile = L"cmd.exe";//调用的程序名
    //    ShExecInfo.lpParameters = L" /c ffmpeg.exe -list_devices true -f dshow -i dummy 2>D:/a.txt";//调用程序的命令行参数
        ShExecInfo.lpParameters = str.data();
        ShExecInfo.lpDirectory = NULL;
        ShExecInfo.nShow = SW_SHOWMINIMIZED;//窗口状态为隐藏
        ShExecInfo.hInstApp = NULL;
        int ret = ShellExecuteEx(&ShExecInfo);
        WaitForSingleObject(ShExecInfo.hProcess, INFINITE);////等到该进程结束
    #else
        std::string cmdStr = MoudleConfig::stringFormat("cmd.exe /c \"%s\" -list_devices true -f dshow -i dummy 2>ffmpeg_device_out.txt", ffmpegPath.c_str());

        int ret = WinExec(cmdStr.c_str(), SW_SHOWMINIMIZED);
    #endif

#else

//    int ret = system(cmdStr.c_str());
#endif

    MoudleConfig::mSleep(2000);

    for (int i=0;i<10;i++)
    {
        std::string deviceName;
        std::string deviceID;

        FILE *fp = fopen("ffmpeg_device_out.txt", "r");
        if (fp != nullptr)
        {
            bool isVideoBegin = false;
            bool isAudioBegin = false;

            while (!feof(fp))
            {
                char ch[1024] = {0};
                char*p = fgets(ch, 1024, fp);

#if defined(WIN32)
//                std::string str = UTF8ToGB(ch); //ffmpeg生成的文件是UTF8编码的
                std::string str = (ch); //ffmpeg生成的文件是UTF8编码的
#else
                std::string str = std::string(ch);
#endif
//                fprintf(stderr, "[%s] %s [end]\n", str.c_str(), ch);

                if ((str.find("DirectShow video devices") != std::string::npos) && (str.find("[dshow @") != std::string::npos))
                {
                    isVideoBegin = true;
                    isAudioBegin = false;
                    continue;
                }

                if ((str.find("DirectShow audio devices") != std::string::npos) && (str.find("[dshow @") != std::string::npos))
                {
                    isAudioBegin = true;
                    isVideoBegin = false;
                    continue;
                }

                if (str.find("[dshow @") != std::string::npos)
                {
                    std::string tmpStr = str;

                    int index = str.find("\"");
                    str = str.erase(0, index);

                    str = MoudleConfig::stringReplaceAll(str, "\"", "");
                    str = MoudleConfig::stringReplaceAll(str, "\n", "");
                    str = MoudleConfig::stringReplaceAll(str, "\r", "");

                    if (tmpStr.find("Alternative name") == std::string::npos)
                    {
                        ///是设备名字
//                        if (str.find("virtual-audio-capturer") != std::string::npos)
                        deviceName = str;
                    }
                    else
                    {
                        deviceID = str;

                        DeviceNode deviceNode{deviceName, deviceID};

                        ///是设备ID
                        if (isVideoBegin)
                        {
    //                        fprintf(stderr, ">>>>>>>video>>>>>>> %s\n", str.c_str());
                            if (!deviceName.empty())
                                videoDeviceList.push_back(deviceNode);
                        }
                        else if (isAudioBegin)
                        {
//                            fprintf(stderr, ">>>>>>>audio>>>>>>> %s\n", str.c_str());
                            if (!deviceName.empty())
                                audioDeviceList.push_back(deviceNode);
                        }
                    }
                }
            }
            fclose(fp);

            isSucceed = true;
            break;
        }
        else
        {
            MoudleConfig::mSleep(1000); //等待一秒再试一次
        }
//        fprintf(stderr, "####=======================###\n");
    }

    std::thread([]
    {

        MoudleConfig::mSleep(10000);
        MoudleConfig::removeFile("ffmpeg_device_out.txt");

    }).detach();

    return isSucceed;
}

#else

#include <initguid.h>
#include <WinSock2.h>
#include <Windows.h>
#include <dshow.h>
#include <stdio.h>
#include <stdarg.h>  //定义成一个可变参数列表的指针
//Strmiids.lib oleaut32.lib

static void wcharTochar(const wchar_t *wchar, char *chr, int length)
{
    WideCharToMultiByte( CP_ACP, 0, wchar, -1,chr, length, NULL, NULL );
}


static char* GuidToString(const GUID &guid)
{
    int buf_len=64;
    char *buf =(char *)malloc(buf_len);
    StringCbPrintfA(
        buf,
        buf_len,
        "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        guid.Data1, guid.Data2, guid.Data3,
        guid.Data4[0], guid.Data4[1],
        guid.Data4[2], guid.Data4[3],
        guid.Data4[4], guid.Data4[5],
        guid.Data4[6], guid.Data4[7]);
    //printf("%s\n",buf);
    return buf;
}

bool MediaManager::getDeviceList(std::list<DeviceNode> &videoDeviceList, std::list<DeviceNode> &audioDeviceList)
{
    bool isSucceed = false;

    do
    {
        // Init COM
        HRESULT hr=NULL;
        hr= CoInitialize(NULL);
        if (FAILED(hr)){
            fprintf(stderr,"Error, Can not init COM.");
            break;
        }
        printf("===============Directshow Filters ===============\n");
        ICreateDevEnum *pSysDevEnum = NULL;
        hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
            IID_ICreateDevEnum, (void **)&pSysDevEnum);
        if (FAILED(hr)){
            break;
        }


        //    HRESULT status= S_OK;

        //    // create System Device Enumerator
        //    ICreateDevEnum *pSystemDeviceEnumerator= NULL;
        //    status= CoCreateInstance(  CLSID_SystemDeviceEnum,
        //                                NULL,
        //                                CLSCTX_INPROC,
        //                                IID_ICreateDevEnum,
        //                                (void**)&pSystemDeviceEnumerator);
        //    if( FAILED(status))
        //    {
        ////        MessageBoxEx( NULL, "Creating System Device Enumerator failed!", __FUNCTION__, MB_ICONERROR, 0);
        //        return false;
        //    }

        //    // create Class Enumerator that lists alls video input devices among the system devices
        //    IEnumMoniker *pVideoInputDeviceEnumerator= NULL;
        //    status= pSystemDeviceEnumerator->CreateClassEnumerator( CLSID_VideoInputDeviceCategory,
        //                                                            &pVideoInputDeviceEnumerator,
        //                                                            0);

        IEnumMoniker *pEnumCat = NULL;
        //Category
        /************************************************************************
        Friendly Name                         CLSID
        -------------------------------------------------------------------------
        Audio Capture Sources                 CLSID_AudioInputDeviceCategory
        Audio Compressors                     CLSID_AudioCompressorCategory
        Audio Renderers                       CLSID_AudioRendererCategory
        Device Control Filters                CLSID_DeviceControlCategory
        DirectShow Filters                    CLSID_LegacyAmFilterCategory
        External Renderers                    CLSID_TransmitCategory
        Midi Renderers                        CLSID_MidiRendererCategory
        Video Capture Sources                 CLSID_VideoInputDeviceCategory
        Video Compressors                     CLSID_VideoCompressorCategory
        WDM Stream Decompression Devices      CLSID_DVDHWDecodersCategory
        WDM Streaming Capture Devices         AM_KSCATEGORY_CAPTURE
        WDM Streaming Crossbar Devices        AM_KSCATEGORY_CROSSBAR
        WDM Streaming Rendering Devices       AM_KSCATEGORY_RENDER
        WDM Streaming Tee/Splitter Devices    AM_KSCATEGORY_SPLITTER
        WDM Streaming TV Audio Devices        AM_KSCATEGORY_TVAUDIO
        WDM Streaming TV Tuner Devices        AM_KSCATEGORY_TVTUNER
        WDM Streaming VBI Codecs              AM_KSCATEGORY_VBICODEC
        ************************************************************************/
    //    hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoCompressorCategory, &pEnumCat, 0);
        hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);
        //hr = pSysDevEnum->CreateClassEnumerator(CLSID_AudioCompressorCategory, &pEnumCat, 0);
    //    hr = pSysDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &pEnumCat, 0);
        //hr = pSysDevEnum->CreateClassEnumerator(CLSID_MediaMultiplexerCategory, &pEnumCat, 0);
        //hr = pSysDevEnum->CreateClassEnumerator(CLSID_LegacyAmFilterCategory, &pEnumCat, 0);

        if (hr != S_OK) {
            pSysDevEnum->Release();
            break;
        }

        isSucceed = true;

        IMoniker *pMoniker = NULL;
        ULONG monikerFetched;
        //Filter
        while(pEnumCat->Next(1, &pMoniker, &monikerFetched) == S_OK){
            IPropertyBag *pPropBag;
            VARIANT varName;
            IBaseFilter *pFilter;
            hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag,(void **)&pPropBag);
            if (FAILED(hr)){
                pMoniker->Release();
                continue;
            }
            VariantInit(&varName);
            hr = pPropBag->Read(L"FriendlyName", &varName, 0);
            //"FriendlyName": The name of the device.
            //"Description": A description of the device.
            //Filter Info================
    //        printf("[%s]\n",W2A(varName.bstrVal));

            //wchar_t to char
            char chr[128]={0};
            wcharTochar(varName.bstrVal, chr, sizeof(chr));

            DeviceNode node;
            node.deviceName = chr;
            videoDeviceList.push_back(node);

//            fprintf(stderr,"[%s]\n", chr);

            VariantClear(&varName);
            //========================
    #if OUTPUT_PIN
            hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter,(void**)&pFilter);
            if (!pFilter){
                continue;
            }

            IEnumPins * pinEnum = NULL;
            IPin * pin = NULL;
            ULONG pinFetched = 0;
            if (FAILED(pFilter->EnumPins(&pinEnum))){
                pinEnum->Release();
                continue;
            }
            pinEnum->Reset();
            //Pin Info
            while (SUCCEEDED(pinEnum->Next(1, &pin, &pinFetched)) && pinFetched){
                if (!pin){
                    continue;
                }
                PIN_INFO pinInfo;
                if (FAILED(pin->QueryPinInfo(&pinInfo))){
                    continue;
                }
                printf("\t[Pin] ");
                    switch(pinInfo.dir){
                    case PINDIR_INPUT:printf("Dir:Input  \t");break;
                    case PINDIR_OUTPUT:printf("Dir:Output \t");break;
                    default:printf("Dir:Unknown\n");break;
                }
    //            printf("Name:%s\n",W2A(pinInfo.achName));
                    char     chr[128]={0};
                    wcharTochar(pinInfo.achName, chr, sizeof(chr));
                    printf("Name:%s\n", chr);

                //MediaType
    #if OUTPUT_MEDIATYPE
                IEnumMediaTypes *mtEnum=NULL;
                AM_MEDIA_TYPE   *mt=NULL;
                if( FAILED( pin->EnumMediaTypes( &mtEnum )) )
                    break;
                mtEnum->Reset();

                ULONG mtFetched = 0;

                while (SUCCEEDED(mtEnum->Next(1, &mt, &mtFetched)) && mtFetched){

                    printf("\t\t[MediaType]\n");
                    //Video
                    char *MEDIATYPE_Video_str=GuidToString(MEDIATYPE_Video);
                    //Audio
                    char *MEDIATYPE_Audio_str=GuidToString(MEDIATYPE_Audio);
                    //Stream
                    char *MEDIATYPE_Stream_str=GuidToString(MEDIATYPE_Stream);
                    //Majortype
                    char *majortype_str=GuidToString(mt->majortype);
                    //Subtype
                    char *subtype_str=GuidToString(mt->subtype);

                    printf("\t\t  Majortype:");
                    if(strcmp(majortype_str,MEDIATYPE_Video_str)==0){
                        printf("Video\n");
                    }else if(strcmp(majortype_str,MEDIATYPE_Audio_str)==0){
                        printf("Audio\n");
                    }else if(strcmp(majortype_str,MEDIATYPE_Stream_str)==0){
                        printf("Stream\n");
                    }else{
                        printf("Other\n");
                    }
                    printf("\t\t  Subtype GUID:%s",subtype_str);

                    free(MEDIATYPE_Video_str);
                    free(MEDIATYPE_Audio_str);
                    free(MEDIATYPE_Stream_str);
                    free(subtype_str);
                    free(majortype_str);
                    printf("\n");

                }
    #endif
                pin->Release();

            }
            pinEnum->Release();

            pFilter->Release();
    #endif
            pPropBag->Release();
            pMoniker->Release();
        }
        pEnumCat->Release();
        pSysDevEnum->Release();
        printf("=================================================\n");
        CoUninitialize();

    }while(0);

    return isSucceed;
}
#endif
