/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "yuv420p.h"

#include <string.h>

void cod_video_blend_u8 (unsigned char *  d1, int d1_stride,
	const unsigned char *  s1, int s1_stride, int a, int w, int h)
{
	int i;
	int j;
	cod_int8 * ptr0;
	const cod_int8 * ptr4;
	cod_int8 var34;
	cod_int8 var35;
	cod_union16 var36;
	cod_int8 var37;
	cod_union16 var38;
	cod_union16 var39;
	cod_union16 var40;
	cod_union16 var41;
	cod_union16 var42;
	cod_union16 var43;
	cod_union16 var44;

	for (j = 0; j < h; j++)
	{
		ptr0 = (cod_int8 *)COD_PTR_OFFSET (d1, d1_stride * j);
		ptr4 = (cod_int8 *)COD_PTR_OFFSET (s1, s1_stride * j);

		/* 5: loadpw */
		var36.i = a;

		for (i = 0; i < w; i++)
		{
			/* 0: loadb */
			var34 = ptr0[i];
			/* 1: convubw */
			var38.i = (cod_uint8) var34;
			/* 2: loadb */
			var35 = ptr4[i];
			/* 3: convubw */
			var39.i = (cod_uint8) var35;
			/* 4: subw */
			var40.i = var39.i - var38.i;
			/* 6: mullw */
			var41.i = (var40.i * var36.i) & 0xffff;
			/* 7: shlw */
			var42.i = ((cod_uint16) var38.i) << 8;
			/* 8: addw */
			var43.i = var42.i + var41.i;
			/* 9: shruw */
			var44.i = ((cod_uint16) var43.i) >> 8;
			/* 10: convsuswb */
			var37 = COD_CLAMP (var44.i, 0, 255);
			/* 11: storeb */
			ptr0[i] = var37;
		}
	}

}


/*************************************************************************************/
/* Y444, Y42B, I420, YV12, Y41B */
inline void _blend_420p_planar (const unsigned char * src, unsigned char * dest,
	int src_stride, int dest_stride, int src_width, int src_height, double src_alpha)
{
	/* If it's completely transparent... we just return */
	if (src_alpha == 0.0)
	{
		return;
	}

	/* If it's completely opaque, we do a fast copy */
	if (src_alpha == 1.0)
	{
		for (int i = 0; i < src_height; i++)
		{
			memcpy (dest, src, src_width);
			src += src_stride;
			dest += dest_stride;
		}
		return;
	}
	// do alpha blend
	int b_alpha = COD_CLAMP ((int) (src_alpha * 256), 0, 256);
	cod_video_blend_u8(dest, dest_stride, src, src_stride, b_alpha, src_width, src_height);
}

void blend_420p_planar (codImageFrame * srcframe, int xpos, int ypos, double src_alpha, codImageFrame * destframe)
{
	const unsigned char *b_src;
	unsigned char *b_dest;
	int b_src_width;
	int b_src_height;
	int xoffset = 0;
	int yoffset = 0;
	int src_comp_rowstride, dest_comp_rowstride;
	int src_comp_height;
	int src_comp_width;
	int comp_ypos, comp_xpos;
	int comp_yoffset, comp_xoffset;

	int src_width = srcframe->width;
	int src_height = srcframe->height;

	int dest_width = destframe->width;
	int dest_height = destframe->height;

	switch(srcframe->pixfmt)
	{
	case cod_fmt_yv12:
	case cod_fmt_i420:
		xpos = COD_ROUND_UP_2 (xpos);
		ypos = COD_ROUND_UP_2 (ypos);
		break;

	case cod_fmt_y444:
		xpos = COD_ROUND_UP_1 (xpos);
		ypos = COD_ROUND_UP_1 (ypos);
		break;

	case cod_fmt_y42b:
		xpos = COD_ROUND_UP_2 (xpos);
		ypos = COD_ROUND_UP_1 (ypos);
		break;

	case cod_fmt_y41b:
		xpos = COD_ROUND_UP_4 (xpos);
		ypos = COD_ROUND_UP_2 (ypos);
		break;

	default:
		xpos = COD_ROUND_UP_2 (xpos);
		ypos = COD_ROUND_UP_2 (ypos);
		break;
	}


	b_src_width = src_width;
	b_src_height = src_height;

	/* adjust src pointers for negative sizes */
	if (xpos < 0)
	{
		xoffset = -xpos;
		b_src_width -= -xpos;
		xpos = 0;
	}
	if (ypos < 0)
	{
		yoffset += -ypos;
		b_src_height -= -ypos;
		ypos = 0;
	}
	/* If x or y offset are larger then the source it's outside of the picture */
	if (xoffset > src_width || yoffset > src_height)
	{
		return;
	}

	/* adjust width/height if the src is bigger than dest */
	if (xpos + src_width > dest_width)
	{
		b_src_width = dest_width - xpos;
	}
	if (ypos + src_height > dest_height)
	{
		b_src_height = dest_height - ypos;
	}
	if (b_src_width < 0 || b_src_height < 0)
	{
		return;
	}

	/* First mix Y, then U, then V */
	b_src = srcframe->data;
	b_dest = destframe->data;
	src_comp_rowstride = src_width;
	dest_comp_rowstride = dest_width;
	src_comp_width = b_src_width;
	src_comp_height = b_src_height;
	comp_xpos = xpos;
	comp_ypos = ypos;
	comp_xoffset = xoffset;
	comp_yoffset = yoffset;
	_blend_420p_planar (b_src + comp_xoffset + comp_yoffset * src_comp_rowstride,
		b_dest + comp_xpos + comp_ypos * dest_comp_rowstride,
		src_comp_rowstride,
		dest_comp_rowstride, src_comp_width, src_comp_height,
		src_alpha);


	// Blend V
	b_src = (unsigned char *)COD_PTR_OFFSET (srcframe->data, src_width * src_height);
	b_dest = (unsigned char *)COD_PTR_OFFSET (destframe->data, dest_width * dest_height);
	src_comp_rowstride = src_width >> 1;
	dest_comp_rowstride = dest_width >> 1;
	src_comp_width = b_src_width >> 1;
	src_comp_height = b_src_height >> 1;
	comp_xpos = xpos >> 1;
	comp_ypos = ypos >> 1;
	comp_xoffset = xoffset >> 1;
	comp_yoffset = yoffset >> 1;
	_blend_420p_planar (b_src + comp_xoffset + comp_yoffset * src_comp_rowstride,
		b_dest + comp_xpos + comp_ypos * dest_comp_rowstride,
		src_comp_rowstride,
		dest_comp_rowstride, src_comp_width, src_comp_height,
		src_alpha);

	// Blend U, other information same to V
	b_src = (unsigned char *)COD_PTR_OFFSET (srcframe->data, src_width * (src_height + (src_height >> 2)));
	b_dest = (unsigned char *)COD_PTR_OFFSET (destframe->data, dest_width * (dest_height + (dest_height >> 2)));
	_blend_420p_planar (b_src + comp_xoffset + comp_yoffset * src_comp_rowstride,
		b_dest + comp_xpos + comp_ypos * dest_comp_rowstride,
		src_comp_rowstride,
		dest_comp_rowstride, src_comp_width, src_comp_height,
		src_alpha);
}

void Yuv420Cut(int x,int y,int desW,int desH,int srcW,int srcH,uint8_t *srcBuffer,uint8_t *desBuffer)
{
    int tmpRange;
    int bufferIndex;

    int yIndex = 0;
    bufferIndex = 0 + x + y*srcW;
    tmpRange = srcW * desH;
    for (int i=0;i<tmpRange;) //逐行拷贝Y分量数据
    {
        memcpy(desBuffer+yIndex,srcBuffer+bufferIndex+i,desW);
        i += srcW;
        yIndex += desW;
    }

    int uIndex = desW * desH;
    int uIndexStep = srcW/2;
    int uWidthCopy = desW/2;
    bufferIndex = srcW * srcH+x/2 + y /2 *srcW / 2;
    tmpRange = srcW * desH / 4;
    for (int i=0;i<tmpRange;) //逐行拷贝U分量数据
    {
        memcpy(desBuffer+uIndex,srcBuffer+bufferIndex+i,uWidthCopy);
        i += uIndexStep;
        uIndex += uWidthCopy;
    }


    int vIndex = desW * desH +  desW * desH /4;
    int vIndexStep = srcW/2;
    int vWidthCopy = desW/2;
    bufferIndex = srcW*srcH + srcW*srcH/4 + x/2 + y /2 *srcW / 2;
    tmpRange = srcW * desH / 4;
    for (int i=0;i<tmpRange;) //逐行拷贝V分量数据
    {
        memcpy(desBuffer+vIndex,srcBuffer+bufferIndex+i,vWidthCopy);
        i += vIndexStep;
        vIndex += vWidthCopy;
    }
}
