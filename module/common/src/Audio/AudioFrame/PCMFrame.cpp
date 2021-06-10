/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "PCMFrame.h"

PCMFrame::PCMFrame()
{
    mFrameBuffer = nullptr;
    mFrameBufferSize = 0;
}

PCMFrame::~PCMFrame()
{
    if (mFrameBuffer != nullptr)
    {
        free(mFrameBuffer);

        mFrameBuffer = nullptr;
        mFrameBufferSize = 0;
    }
}

void PCMFrame::setFrameBuffer(const uint8_t * const buffer, const unsigned int &size, const int64_t &time)
{
    if (mFrameBufferSize < size)
    {
        if (mFrameBuffer != nullptr)
        {
            free(mFrameBuffer);
        }

        mFrameBuffer = static_cast<uint8_t*>(malloc(size));
    }

    memcpy(mFrameBuffer, buffer, size);
    mFrameBufferSize = size;
    mPts = time;
}
