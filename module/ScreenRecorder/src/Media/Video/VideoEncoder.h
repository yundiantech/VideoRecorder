/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef VIDEOENCORDER_H
#define VIDEOENCORDER_H

#include <list>
#include <thread>
#include "Mutex/Cond.h"

#include "VideoFrame/VideoRawFrame.h"
#include "VideoFrame/VideoEncodedFrame.h"

extern "C"
{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libswscale/swscale.h"
    #include "libavdevice/avdevice.h"
}

/// 编码h.264的线程  这里把编码和采集分开 放到单独的线程 是因为编码也比较耗时
class VideoEncoder
{
public:
    explicit VideoEncoder();
    ~VideoEncoder();

    void setWidth(int w, int h);//设置编码后的图像高宽(这个必须和输入的yuv图像数据一样 且必须是偶数)
    void setFrameRate(int value){mFrameRate = value;}
    void setQuality(int value);// 设置编码质量 0~10 10画质最高

    bool openEncoder(); //打开编码器
    bool closeEncoder(); //关闭编码器

    std::list<VideoEncodedFramePtr> encode(VideoRawFramePtr yuvFramePtr, const int64_t &framePts);

    AVPacket *getLastEncodePacket(){return &mPacket;}

private:

    AVCodecContext* pCodecCtx;
    AVCodec* pCodec;

    uint8_t* picture_buf;
    AVFrame* picture;

    AVPacket mPacket;

    int mBitRate; //video bitRate
    int mQuality;
    int mCurrentQuality; //已经打开的编码器质量值

    int mWidth;
    int mHeight;
    int mFrameRate;

    AVDictionary *setEncoderParam(const AVCodecID &codec_id); //设置编码器参数

    bool openVideoEncoder(const AVCodecID &codec_id); //打开视频编码器
    bool openHardEncoder_Cuvid(const AVCodecID &codec_id); //打开硬件编码器（英伟达）
    bool openHardEncoder_Qsv(const AVCodecID &codec_id);   //打开硬件编码器（intel）
    bool openSoftEncoder(const AVCodecID &codec_id);//打开软编码器

    ///回调函数(用于编码完成后数据输出给父类)
    std::function<void (VideoEncodedFramePtr videoFramePtr, void *param)> mCallBackFunc; //回调函数
    void *mCallBackFuncParam; //回调函数用户参数
};

#endif // VIDEOENCORDER_H
