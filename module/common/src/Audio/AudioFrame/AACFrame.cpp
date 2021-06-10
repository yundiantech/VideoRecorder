/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "AACFrame.h"

AACFrame::AACFrame()
{
    mFrameBuffer = nullptr;
    mFrameBufferSize = 0;

    mPts = 0;

}

AACFrame::~AACFrame()
{
    if (mFrameBuffer != nullptr)
    {
        free(mFrameBuffer);

        mFrameBuffer = nullptr;
        mFrameBufferSize = 0;
    }
}

void AACFrame::setAdtsHeader(const ADTS_HEADER &adts)
{
    mAdtsHeader = adts;
}

void AACFrame::setFrameBuffer(const uint8_t * const buffer, const unsigned int &size)
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
}
