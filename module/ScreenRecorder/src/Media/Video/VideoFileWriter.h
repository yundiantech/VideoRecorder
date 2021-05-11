/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef VIDEOFILEWRITER_H
#define VIDEOFILEWRITER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <functional>

#include <QMutex>
#include <QDebug>

extern"C"
{
    #include <libavutil/avassert.h>
    #include <libavutil/channel_layout.h>
    #include <libavutil/opt.h>
    #include <libavutil/mathematics.h>
    #include <libavutil/time.h>
//    #include <libavutil/timestamp.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
    #include <libavutil/imgutils.h>
}

#include "Mutex/Cond.h"
#include "VideoEncoder.h"
#include "VideoFrame/VideoRawFrame.h"
#include "VideoFrame/VideoEncodedFrame.h"
#include "AudioFrame/PCMFrame.h"

#include "Media/Video/VideoFileInfoTypes.h"

#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio

// a wrapper around a single output AVStream
typedef struct OutputStream
{
    AVStream *st;
    AVCodecContext *enc;

    /* pts of the next frame that will be generated */
    int64_t next_pts;
    int samples_count;

    AVFrame *frame;
    AVFrame *tmp_frame;

    /// 如果是视频这是yuv420p数据
    /// 如果是音频这是存放pcm数据，用来取出刚好的一帧数据传给编码器编码
    uint8_t *frameBuffer;
    int frameBufferSize;

} OutputStream;

class VideoFileWriter
{
public:
    VideoFileWriter();
    ~VideoFileWriter();

    void setFileName(const std::string &filePath);
    std::list<VideoFileInfo> getVideoFileList(); //获取文件列表

    void setQuality(const int &value);
    void setWidth(int width,int height);
    bool startEncode(std::function<void (VideoEncodedFramePtr videoFramePtr, void *param)> func, void *param = nullptr);
    bool stopEncode();

    bool isThreadRunning();

    void inputYuvFrame(VideoRawFramePtr yuvFrame);
    VideoRawFramePtr readYuvFrame(const int64_t &time); //time是毫秒
    void clearYuvFrame();

    void inputPcmFrame(PCMFramePtr pcmFrame);
    PCMFramePtr readPcmFrame();
    void clearPcmFrame();

    int getONEFrameSize(){return mONEFrameSize;}

    void setContainsVideo(bool);
    void setContainsAudio(bool);

    void setVideoFrameRate(int value);

    ///获取时间戳（毫秒）
    int64_t getVideoPts();
    int64_t getAudioPts();

    void sig_StartWriteFile(const std::string & filePath);
    void sig_StopWriteFile(const std::string & filePath);

protected:
    void run();

private:
    std::string mFilePath;

    int mVideoFrameRate;

    bool mIsStop;
    bool mIsThreadRunning;

    int64_t audio_pts, video_pts; //当前文件的时间（毫秒）
    int64_t mLastFileVideoPts;     //上一个文件结束的时候的时间
    std::list<VideoFileInfo> mVideoFileList; //记录最终生成的视频文件列表
    bool mIsNewVideoFile = false;  //输入的图像分辨率改变时，是否重新生成一个视频文件

    int mBitRate; //video bitRate
    int mQuality;

    int mONEFrameSize;

    int WIDTH;
    int HEIGHT;

    bool m_containsVideo;
    bool m_containsAudio;

    int64_t mLastGetAudioTime; //上一次获取到音频的时间
    bool mIsUseMuteAudio; //是否使用静音模式（直接传0x0的数据拿去编码）

    VideoEncoder *mVideoEncoder;

    Cond *mCondAudio;
    std::list<PCMFramePtr> mPcmFrameList;

    Cond *mCondVideo;
    std::list<VideoRawFramePtr> mVideoFrameList;
    VideoRawFramePtr mLastVideoFrame; //上一次的帧（帧不足的时候用上一次的帧来补全）

    void open_audio(AVFormatContext *oc, AVCodec *codec, OutputStream *ost);
    void close_audio(AVFormatContext *oc, OutputStream *ost);

    void open_video(AVFormatContext *oc, AVCodec *codec, OutputStream *ost);
    void close_video(AVFormatContext *oc, OutputStream *ost);

    void add_video_stream(OutputStream *ost, AVFormatContext *oc, AVCodec **codec, AVCodecID codec_id);
    void add_audio_stream(OutputStream *ost, AVFormatContext *oc, AVCodec **codec, AVCodecID codec_id);

    bool write_audio_frame(AVFormatContext *oc, OutputStream *ost);
    bool write_video_frame(AVFormatContext *oc, OutputStream *ost, int64_t time, bool &isNeedNewFile);


    ///回调函数(用于编码完成后数据输出给父类)
    std::function<void (VideoEncodedFramePtr videoFramePtr, void *param)> mCallBackFunc; //回调函数
    void *mCallBackFuncParam; //回调函数用户参数

};

#endif // VIDEOFILEWRITER_H
