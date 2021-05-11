#include "PcmMix.h"

PcmMix::PcmMix()
{

}

/**
 * 归一化混音算法
 * 要求两个音频参数一样
 */
void PcmMix::NormalizedRemix(float **src_data, const int &number, const int &buffer_size, float *out_data)
{
    //归一化混音
    int i = 0,j = 0;

    for (i=0; i < (buffer_size / sizeof (float)); i++)
    {
        //将所有音轨的值相加
        float temp = 0;
        for (j = 0; j < number; j++)
        {
            temp += *(float*)(src_data[j] + i);
        }

        *(float*)(out_data + i) = temp;
    }
}

/**
 * 归一化混音算法
 * 要求两个音频参数一样
 */
void PcmMix::NormalizedRemix(short **src_data, const int &number, const int &buffer_size, short *out_data)
{
    //归一化混音
    int const MAX=32767;
    int const MIN=-32768;

    double f=1;
    int output;
    int i = 0,j = 0;

    for (i=0; i < (buffer_size / sizeof (short)); i++)
    {
        //将所有音轨的值相加
        int temp = 0;
        for (j = 0; j < number; j++)
        {
            temp += *(short*)(src_data[j] + i);
        }
        output=(int)(temp*f);
        if (output > MAX)
        {
            f = (double)MAX / (double)(output);
            output = MAX;
        }
        if (output < MIN)
        {
            f = (double)MIN / (double)(output);
            output = MIN;
        }
        if (f < 1)
        {
            f += ((double)1 - f) / (double)32;
        }

        *(short*)(out_data + i) = (short)output;
    }
}
