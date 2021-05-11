/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef NALU_H
#define NALU_H

#include <stdint.h>
#include <stdlib.h>

#include "h264.h"
#include "h265.h"

enum T_NALU_TYPE
{
    T_NALU_H264 = 0,
    T_NALU_H265,
};

typedef struct
{
    T_NALU_TYPE type;

    union
    {
        T_H264_NALU h264Nalu;
        T_H265_NALU h265Nalu;
    }nalu;

} T_NALU;

///用于从连续的h264/h265数据中解析出nalu
class NALUParsing
{
public:
    NALUParsing();

    void setVideoType(const T_NALU_TYPE &type){mVideoType = type;}

    int inputH264Data(uint8_t *buf, const int &len); //输入h264数据

    ///从H264数据中查找出一帧数据
    T_NALU* getNextFrame();

private:
    uint8_t *mH264Buffer;
    int mBufferSize;

    T_NALU_TYPE mVideoType; //类型 区分是264还是265

public:
    ///为NALU_t结构体分配内存空间
    static T_NALU *AllocNALU(const int &buffersize, const T_NALU_TYPE &type, const bool &isAllocBuffer = true);

    ///释放
    static void FreeNALU(T_NALU *n);

};


#endif // NALU_H
