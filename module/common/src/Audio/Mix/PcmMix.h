#ifndef PCMMIX_H
#define PCMMIX_H

#include <list>

class PcmMix
{
public:
    PcmMix();


    /**
     *
     * 归一化混音算法
     * 要求两个音频参数一样(只能处理16bit的音频)
     *
     * @param src_data    [in] 需要混音的原始数据
     * @param number      [in] 输入的音频音轨数量
     * @param buffer_size [in] 每一个音轨数据长度
     * @param out_data    [out] 输出数据
     */
    ///输入为float的pcm
    static void NormalizedRemix(float **src_data, const int &number, const int &buffer_size, float *out_data);
    ///输入为16bit的pcm
    static void NormalizedRemix(short **src_data, const int &number, const int &buffer_size, short *out_data);

};

#endif // PCMMIX_H
