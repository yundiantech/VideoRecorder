/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

extern "C"
{
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <libavutil/imgutils.h>
}

#include "VideoRawFrame.h"

VideoRawFrame::VideoRawFrame()
{
    mFrameBuffer = nullptr;
    mFrameBufferSize = 0;
    mPts = 0;
}

VideoRawFrame::~VideoRawFrame()
{
    if (mFrameBuffer != nullptr)
    {
        free(mFrameBuffer);
        mFrameBuffer = nullptr;
        mFrameBufferSize = 0;
    }
}

void VideoRawFrame::initBuffer(const int &width, const int &height, const FrameType &type, int64_t time)
{
    if (mFrameBuffer != nullptr)
    {
        free(mFrameBuffer);
        mFrameBuffer = nullptr;
    }

    mWidth  = width;
    mHegiht = height;

    mPts = time;

    mType = type;

    int size = 0;
    if (type == FRAME_TYPE_YUV420P)
    {
        //size = width * height * 3 / 2;
		size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, width, height, 1);  //按1字节进行内存对齐,得到的内存大小最接近实际大小
    }
    else if (type == FRAME_TYPE_RGB24)
    {
        size = width * height * 3;
    }

    mFrameBuffer = (uint8_t*)malloc(size);
    mFrameBufferSize = size;
}

void VideoRawFrame::setFramebuf(const uint8_t *buf)
{
    memcpy(mFrameBuffer, buf, mFrameBufferSize);
}

void VideoRawFrame::setYbuf(const uint8_t *buf)
{
    int Ysize = mWidth * mHegiht;
    memcpy(mFrameBuffer, buf, Ysize);
}

void VideoRawFrame::setUbuf(const uint8_t *buf)
{
    int Ysize = mWidth * mHegiht;
    memcpy(mFrameBuffer + Ysize, buf, Ysize / 4);
}

void VideoRawFrame::setVbuf(const uint8_t *buf)
{
    int Ysize = mWidth * mHegiht;
    memcpy(mFrameBuffer + Ysize + Ysize / 4, buf, Ysize / 4);
}

