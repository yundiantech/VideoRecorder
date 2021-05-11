/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */
#ifndef IMAGEREADER_H
#define IMAGEREADER_H

#include <stdint.h>

class ImageReader
{
public:
    ImageReader();

    static int ReadRgb24Buffer(const char* pFileName, uint8_t* pRgbBuffer, int nBufSize, int* pnWidth, int* pnHeight, int nDepth);
    static int ReadYuv420pBuffer(const char* pFileName, uint8_t* pYuv420pBuffer, int nBufSize, int* pnWidth, int* pnHeight);


};

#endif // IMAGEREADER_H
