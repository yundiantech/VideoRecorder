/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */
#include <stdint.h>

/* force a int [b,c] */
#define COD_CLAMP(a,b,c) \
	(a) < (b) ? (b) : ((a) > (c) ? (c) : (a))

#define COD_ROUND_UP_1(x) (x)
#define COD_ROUND_UP_2(x) ((x) & ~1)
#define COD_ROUND_UP_4(x) ((x) & ~3)

#define COD_PTR_OFFSET(ptr,offset) ((void *)(((unsigned char *)(ptr)) + (offset)))
typedef char cod_int8;
typedef unsigned char cod_uint8;
typedef short cod_int16;
typedef unsigned short cod_uint16;
typedef int cod_int32;
typedef unsigned int cod_uint32;
typedef long long cod_int64;
typedef unsigned long long cod_uint64;


typedef union 
{
	cod_int16	i;
	cod_int8	x2[2];
} cod_union16;

typedef union
{
	cod_int32	i; 
	float		f; 
	cod_int16	x2[2]; 
	cod_int8	x4[4];
} cod_union32;

typedef union
{
	cod_int64	i;
	double		f;
	cod_int32	x2[2];
	float		x2f[2];
	cod_int16	x4[4];
} cod_union64;

/* define supported pixel format */
typedef enum
{
	cod_fmt_unknown,
	cod_fmt_rgb,
	cod_fmt_rgba,
	cod_fmt_argb,
	cod_fmt_rgbx,
	cod_fmt_xrgb,

	cod_fmt_bgr,
	cod_fmt_bgra,
	cod_fmt_abgr,
	cod_fmt_bgrx,
	cod_fmt_xbgr,

	cod_fmt_yuy2,
	cod_fmt_i420,
	cod_fmt_yv12,
	cod_fmt_ayuv,
	cod_fmt_nv12,
	cod_fmt_nv21,
	cod_fmt_yuyv,
	cod_fmt_yvyu,
	cod_fmt_uyvy,
	cod_fmt_y444,
	cod_fmt_y42b,
	cod_fmt_y41b
}cod_fmt;


/* define videoframe */
typedef struct tag_codImageFrame
{
	unsigned char	*data;						// memory pointer
	int				width;						// width of image
	int				height;						// height of image
	long			stride;						// stride of image
	cod_fmt			pixfmt;						// pixel format of image

	tag_codImageFrame()
	{
		data = 0;
		width = height = stride = 0;

		pixfmt = cod_fmt_unknown;
	}
}codImageFrame;

void blend_420p_planar (codImageFrame * srcframe, int xpos, int ypos, double src_alpha, codImageFrame * destframe);

///yuv420裁剪
void Yuv420Cut(int x,int y,int desW,int desH,int srcW,int srcH,uint8_t *srcBuffer,uint8_t *desBuffer);
