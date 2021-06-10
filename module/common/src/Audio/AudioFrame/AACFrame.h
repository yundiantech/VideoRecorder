﻿/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef AACFRAME_H
#define AACFRAME_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <memory>

/*
sampling_frequency_index sampling frequeny [Hz]
0x0                           96000
0x1                           88200
0x2                           64000
0x3                           48000
0x4                           44100
0x5                           32000
0x6                           24000
0x7                           22050
0x8                           16000
0x9                           2000
0xa                           11025
0xb                           8000
0xc                           reserved
0xd                           reserved
0xe                           reserved
0xf                           reserved
*/
typedef struct
{
    unsigned int syncword;  //12 bslbf 同步字The bit string ‘1111 1111 1111’，说明一个ADTS帧的开始
    unsigned int id;        //1 bslbf   MPEG 标示符, 设置为1
    unsigned int layer;     //2 uimsbf Indicates which layer is used. Set to ‘00’
    unsigned int protection_absent;  //1 bslbf  表示是否误码校验
    unsigned int profile;            //2 uimsbf  表示使用哪个级别的AAC，如01 Low Complexity(LC)--- AACLC
    unsigned int sf_index;           //4 uimsbf  表示使用的采样率下标
    unsigned int private_bit;        //1 bslbf
    unsigned int channel_configuration;  //3 uimsbf  表示声道数
    unsigned int original;               //1 bslbf
    unsigned int home;                   //1 bslbf
    /*下面的为改变的参数即每一帧都不同*/
    unsigned int copyright_identification_bit;   //1 bslbf
    unsigned int copyright_identification_start; //1 bslbf
    unsigned int aac_frame_length;               // 13 bslbf  一个ADTS帧的长度包括ADTS头和raw data block
    unsigned int adts_buffer_fullness;           //11 bslbf     0x7FF 说明是码率可变的码流

    /*no_raw_data_blocks_in_frame 表示ADTS帧中有number_of_raw_data_blocks_in_frame + 1个AAC原始帧.
    所以说number_of_raw_data_blocks_in_frame == 0
    表示说ADTS帧中有一个AAC数据块并不是说没有。(一个AAC原始帧包含一段时间内1024个采样及相关数据)
    */
    unsigned int no_raw_data_blocks_in_frame;    //2 uimsfb
} ADTS_HEADER;


#define AACFramePtr std::shared_ptr<AACFrame>

class AACFrame
{
public:
    AACFrame();
    ~AACFrame();

    void setAdtsHeader(const ADTS_HEADER &adts);
    void setFrameBuffer(const uint8_t * const buffer, const unsigned int &size);

    uint8_t *getBuffer(){return mFrameBuffer;}
    unsigned int getSize(){return  mFrameBufferSize;}

    void setPts(const int64_t &time){mPts = time;}
    int64_t getPts(){return mPts;}

private:
    ADTS_HEADER mAdtsHeader;

    uint8_t *mFrameBuffer; //aac数据（包括adts头）
    unsigned int mFrameBufferSize; //aac数据长度（包括adts头的大小）

    int64_t mPts;
};

#endif // AACFRAME_H
