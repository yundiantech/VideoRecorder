/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef H264_H
#define H264_H

#include <stdlib.h>

typedef struct
{
  int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
  unsigned len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
  unsigned max_size;            //! Nal Unit Buffer size
  int forbidden_bit;            //! should be always FALSE
  int nal_reference_idc;        //! NALU_PRIORITY_xxxx
  int nal_unit_type;            //! NALU_TYPE_xxxx
  unsigned char *buf;                    //! contains the first byte followed by the EBSP
  unsigned short lost_packets;  //! true, if packet loss is detected
} T_H264_NALU;

#pragma pack (1)
typedef struct {
    //byte 0
    unsigned char TYPE:5;
    unsigned char NRI:2;
    unsigned char F:1;

} T_H264_NALU_HEADER; /**//* 1 BYTES */
#pragma pack ()

#endif // H264_H
