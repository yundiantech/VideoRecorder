/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef VIDEORAWFRAME_H
#define VIDEORAWFRAME_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <memory>

#define VideoRawFramePtr std::shared_ptr<VideoRawFrame>


class VideoRawFrame
{
public:
    enum FrameType
    {
        FRAME_TYPE_NONE = -1,
        FRAME_TYPE_YUV420P,   ///< planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples)
        FRAME_TYPE_RGB24,     ///< packed RGB 8:8:8, 24bpp, RGBRGB...
    };

    VideoRawFrame();
    ~VideoRawFrame();

    void initBuffer(const int &width, const int &height, const FrameType &type, int64_t time = 0);

    void setFramebuf(const uint8_t *buf);
    void setYbuf(const uint8_t *buf);
    void setUbuf(const uint8_t *buf);
    void setVbuf(const uint8_t *buf);

    uint8_t * getBuffer(){return mFrameBuffer;}
    int getWidth(){return mWidth;}
    int getHeight(){return mHegiht;}
    int getSize(){return mFrameBufferSize;}

    void setPts(const int64_t &pts){mPts=pts;}
    int64_t getPts(){return mPts;}

protected:
    FrameType mType;

    uint8_t *mFrameBuffer;
    int mFrameBufferSize;

    int mWidth;
    int mHegiht;

    int64_t mPts;
};

#endif // VIDEOFRAME_H
