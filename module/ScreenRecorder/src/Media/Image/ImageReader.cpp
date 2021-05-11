/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "ImageReader.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <libavutil/imgutils.h>
}

ImageReader::ImageReader()
{

}

// 读图象文件，支持BMP、JPEG、PNG文件
// 输入参数：
//        pFileName ---- 图象文件名
//        nBufSize ---- 接收图象数据的缓冲区大小，当接收缓冲区大小为0或者接收缓冲区指针为空时，只输出图象的宽度
// 输出参数：
//        pOutDataBuf ---- 接收图象数据，必须分配足够大的缓冲区并且将分配的缓冲区大小通过nBufSize参数传入
//        pnWidth ---- 接收图象宽度
//        pnHeight ---- 接收图象高度
// 返回值：
//        0 ---- 成功
//        -1 ---- 参数错误
//        -2 ---- 打开文件失败
//        其它 ---- 图象解析失败
// 备注：不知道图象分辨率因为不确认应分配的缓冲区大小时，可以先填入空缓冲区指针检测图象分辨率，再分配合适的缓冲区重新调用获得图象数据
int ImageReader::ReadRgb24Buffer(const char* pFileName, uint8_t* pRgbBuffer, int nBufSize, int* pnWidth, int* pnHeight, int nDepth)
{
    int ret = -100;

//fprintf(stderr,"ReadImageFile %s \n", pFileName);

    ///ffmpeg相关变量 用于打开rtsp
     AVFormatContext *pFormatCtx;
     AVCodecContext *pCodecCtx;
     AVCodec *pCodec;

     int videoStream;

     ///解码视频相关
     AVFrame *pFrame, *pFrameRGB_Scaled;
     uint8_t *out_buffer_rgb_scaled; //压缩的rgb数据
     struct SwsContext *img_convert_ctx_RGBScaled;  //用于RGB数据的压缩
     int numBytes_rgb_scaled; //压缩后的rgb数据

     const char *file_path = pFileName;

     ///ffmpeg相关变量 用于打开图片文件
     pFormatCtx = nullptr;
     pCodecCtx  = nullptr;
     pCodec     = nullptr;

     ///解码视频相关
     pFrame    = nullptr;
     pFrameRGB_Scaled = nullptr;
     out_buffer_rgb_scaled = nullptr; //压缩的rgb数据
     img_convert_ctx_RGBScaled = nullptr;  //用于RGB数据的压缩

     AVPacket packet;

     videoStream = -1;

     //Allocate an AVFormatContext.
     pFormatCtx = avformat_alloc_context();

     AVDictionary* opts = nullptr;
     av_dict_set(&opts, "rtsp_transport", "tcp", 0); //设置tcp or udp，默认一般优先tcp再尝试udp
     av_dict_set(&opts, "stimeout", "3000000", 0);//设置超时3秒

     AVStream *video_st = nullptr;

     ///输出的rgb数据 分辨率
     int OutPutWidth = 0;
     int OutPutHeight= 0;

     if (pFormatCtx == nullptr)
     {
         fprintf(stderr, "apFormatCtx == nullptr \n");
         goto end;
     }

     ///打开rtsp流
     if ((ret = avformat_open_input(&pFormatCtx, file_path, nullptr, &opts)) && (ret != 0))
     {
         char buff[128]={0};
         av_strerror(ret, buff, 128);
         fprintf(stderr, "avformat_open_input erro %s\n", buff);
         goto end;
     }

     ///读取出视频流
     if ((ret = avformat_find_stream_info(pFormatCtx, nullptr)) && (ret < 0))
     {
         char buff[128]={0};
         av_strerror(ret, buff, 128);
         fprintf(stderr, "avformat_find_stream_info erro %s\n", buff);

         goto end;
     }
     else
     {
         //find videoindex
         videoStream = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
         if (videoStream < 0)
         {///如果videoStream为-1 说明没有找到视频流
             fprintf(stderr, "av_find_best_stream erro videoStream=%d\n", videoStream);
             ret = -1;
             goto end;
         }
     }

     ///打开视频解码器
     pCodecCtx = avcodec_alloc_context3(nullptr);
     if (pCodecCtx == nullptr)
     {
         fprintf(stderr,"could not allocate AVCodecContext.\n");
         ret = -2;
         goto end;
     }

     avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoStream]->codecpar);

     pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

     if (pCodec == nullptr)
     {
         fprintf(stderr,"Codec not found.\n");
         ret = -3;
         goto end;
     }
     else
     {
//         pCodecCtx->thread_count = 8;

         ///打开解码器
         if ((ret = avcodec_open2(pCodecCtx, pCodec, NULL)) && (ret < 0))
         {
             char buff[128]={0};
             av_strerror(ret, buff, 128);
             fprintf(stderr, "avcodec_open2 erro %s\n", buff);
             avcodec_close(pCodecCtx);
             goto end;
         }
     }

     if (pCodecCtx->pix_fmt == AV_PIX_FMT_NONE)
     {
         fprintf(stderr,"open file failed! pCodecCtx->pix_fmt == AV_PIX_FMT_NONE \n");
         ret = -4;
         goto end;
     }

     video_st = pFormatCtx->streams[videoStream];

     ///输出的rgb数据 分辨率
     OutPutWidth  = pCodecCtx->width;
     OutPutHeight = pCodecCtx->height;

//     if (OutPutWidth > 1920)
//     {
//         int newWidth = 1920;
//         int newHeight;

//         newHeight = OutPutHeight * newWidth / OutPutWidth;

//         fprintf(stderr, "%s PixWidth is Scaled w=%d h=%d newW=%d newH=%d \n", __FUNCTION__, OutPutWidth, OutPutHeight, newWidth, newHeight);

//         OutPutWidth  = newWidth;
//         OutPutHeight = newHeight;

//     }

//     if (OutPutHeight > 1080)
//     {
//         int newWidth;
//         int newHeight = 1080;

//         newWidth = OutPutWidth * newHeight / OutPutHeight;

//         fprintf(stderr, "%s PixHeight is Scaled w=%d h=%d newW=%d newH=%d \n", __FUNCTION__, OutPutWidth, OutPutHeight, newWidth, newHeight);

//         OutPutWidth  = newWidth;
//         OutPutHeight = newHeight;
//     }

     if (OutPutWidth % 2 != 0) OutPutWidth++;
     if (OutPutHeight % 2 != 0) OutPutHeight++;

     ///当传入的指针为空的时候也需要返回图像宽高
     *pnWidth  = OutPutWidth;
     *pnHeight = OutPutHeight;

     if (pRgbBuffer == nullptr)
     {
         ret = 0;
         goto end;
     }

     ///分配解码后的帧的存储空间
     {
         AVPixelFormat tartFormat = AV_PIX_FMT_RGB24;

         pFrame = av_frame_alloc();  //用于保存解码后的帧
         pFrameRGB_Scaled = av_frame_alloc(); //用于保存转换后的rgb24数据

         ///将解码后的YUV数据转换成RGB24 并压缩rgb
         img_convert_ctx_RGBScaled = sws_getContext(pCodecCtx->width, pCodecCtx->height,
                 pCodecCtx->pix_fmt, OutPutWidth, OutPutHeight,
                 tartFormat, SWS_BICUBIC, nullptr, nullptr, nullptr);
         numBytes_rgb_scaled = avpicture_get_size(tartFormat, OutPutWidth, OutPutHeight);
         out_buffer_rgb_scaled = (uint8_t *) av_malloc(numBytes_rgb_scaled * sizeof(uint8_t));
         avpicture_fill((AVPicture *) pFrameRGB_Scaled, out_buffer_rgb_scaled, tartFormat,
                 OutPutWidth, OutPutHeight);

     }

//     ///输出视频信息
//     av_dump_format(pFormatCtx, 0, file_path, 0);

     fprintf(stderr,"open file succeed!\n");

     ret = -200;

     do{
         if (av_read_frame(pFormatCtx, &packet) < 0)
         {
             fprintf(stderr,"read image stream failed!\n");
             break; //这里认为视频读取完了
         }

         if (packet.stream_index == videoStream)
         {
             if (avcodec_send_packet(pCodecCtx, &packet) != 0)
             {
                av_packet_unref(&packet);
                continue;
             }

             while (0 == avcodec_receive_frame(pCodecCtx, pFrame))
             {
                 ///判断解码完毕的帧是否是关键帧
                 bool isKeyFrame = false;

                 if(pFrame->key_frame)
                 {
                     isKeyFrame = true;
                 }

 //                ///反转图像 ，否则生成的rgb24图像是上下颠倒的
 //                pFrame->data[0] += pFrame->linesize[0] * (pCodecCtx->height - 1);
 //                pFrame->linesize[0] *= -1;
 //                pFrame->data[1] += pFrame->linesize[1] * (pCodecCtx->height / 2 - 1);
 //                pFrame->linesize[1] *= -1;
 //                pFrame->data[2] += pFrame->linesize[2] * (pCodecCtx->height / 2 - 1);
 //                pFrame->linesize[2] *= -1;

                 ///将解码后的图像转成rgb24
                 sws_scale(img_convert_ctx_RGBScaled,
                         (uint8_t const * const *) pFrame->data,
                         pFrame->linesize, 0, pCodecCtx->height, pFrameRGB_Scaled->data,
                         pFrameRGB_Scaled->linesize);

                 ///处理rgb数据

                 memcpy(pRgbBuffer, out_buffer_rgb_scaled, numBytes_rgb_scaled);

                 *pnWidth  = OutPutWidth;
                 *pnHeight = OutPutHeight;

                 ret = 0;

//                 FILE * fp = fopen("out.rgb24", "wb");
//                 fwrite(out_buffer_rgb_scaled, 1, numBytes_rgb_scaled, fp);
//                 fclose(fp);

//                 fprintf(stderr,"open file succeed! %d\n", numBytes_rgb_scaled);

             }
         }

         av_packet_unref(&packet);

     }while(0);

//     avcodec_close(pCodecCtx);
//     avformat_close_input(&pFormatCtx);

 end:

     {
         if (pFrame != nullptr)
             av_frame_free(&pFrame);

         if (pFrameRGB_Scaled != nullptr)
             av_frame_free(&pFrameRGB_Scaled);

         if (img_convert_ctx_RGBScaled != nullptr)
             sws_freeContext(img_convert_ctx_RGBScaled);

         if (out_buffer_rgb_scaled != nullptr)
             av_free(out_buffer_rgb_scaled);

         pFrame    = nullptr;
         pFrameRGB_Scaled = nullptr;
         out_buffer_rgb_scaled = nullptr; //压缩的rgb数据
         img_convert_ctx_RGBScaled = nullptr;  //用于RGB数据的压缩

     }

        if (pCodecCtx != nullptr)
        {
         avcodec_close(pCodecCtx);
         av_free(pCodecCtx);
        }

        if (pFormatCtx != nullptr)
        {
          avformat_close_input(&pFormatCtx);
          avformat_free_context(pFormatCtx);
        }

        if (opts != nullptr)
        {
          av_dict_free(&opts);
        }

//fprintf(stderr,"ReadImageFile %s   111 ret = %d \n", pFileName, ret);

     return ret;
}

int ImageReader::ReadYuv420pBuffer(const char* pFileName, uint8_t* pYuv420pBuffer, int nBufSize, int* pnWidth, int* pnHeight)
{
    int ret = -100;

//fprintf(stderr,"ReadImageFile %s \n", pFileName);

    ///ffmpeg相关变量 用于打开rtsp
     AVFormatContext *pFormatCtx;
     AVCodecContext *pCodecCtx;
     AVCodec *pCodec;

     int videoStream;

     ///解码视频相关
     AVFrame *pFrame, *pFrameRGB_Scaled;
     uint8_t *out_buffer_rgb_scaled; //压缩的rgb数据
     struct SwsContext *img_convert_ctx_RGBScaled;  //用于RGB数据的压缩
     int numBytes_rgb_scaled; //压缩后的rgb数据

     const char *file_path = pFileName;

     ///ffmpeg相关变量 用于打开图片文件
     pFormatCtx = nullptr;
     pCodecCtx  = nullptr;
     pCodec     = nullptr;

     ///解码视频相关
     pFrame    = nullptr;
     pFrameRGB_Scaled = nullptr;
     out_buffer_rgb_scaled = nullptr; //压缩的rgb数据
     img_convert_ctx_RGBScaled = nullptr;  //用于RGB数据的压缩

     AVPacket packet;

     videoStream = -1;

     //Allocate an AVFormatContext.
     pFormatCtx = avformat_alloc_context();

     AVDictionary* opts = nullptr;
     av_dict_set(&opts, "rtsp_transport", "tcp", 0); //设置tcp or udp，默认一般优先tcp再尝试udp
     av_dict_set(&opts, "stimeout", "3000000", 0);//设置超时3秒

     AVStream *video_st = nullptr;

     ///输出的rgb数据 分辨率
     int OutPutWidth = 0;
     int OutPutHeight= 0;

     if (pFormatCtx == nullptr)
     {
         fprintf(stderr, "apFormatCtx == nullptr \n");
         goto end;
     }

     ///打开rtsp流
     if ((ret = avformat_open_input(&pFormatCtx, file_path, nullptr, &opts)) && (ret != 0))
     {
         char buff[128]={0};
         av_strerror(ret, buff, 128);
         fprintf(stderr, "avformat_open_input erro %s\n", buff);
         goto end;
     }

     ///读取出视频流
     if ((ret = avformat_find_stream_info(pFormatCtx, nullptr)) && (ret < 0))
     {
         char buff[128]={0};
         av_strerror(ret, buff, 128);
         fprintf(stderr, "avformat_find_stream_info erro %s\n", buff);

         goto end;
     }
     else
     {
         //find videoindex
         videoStream = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
         if (videoStream < 0)
         {///如果videoStream为-1 说明没有找到视频流
             fprintf(stderr, "av_find_best_stream erro videoStream=%d\n", videoStream);
             ret = -1;
             goto end;
         }
     }

     ///打开视频解码器
     pCodecCtx = avcodec_alloc_context3(nullptr);
     if (pCodecCtx == nullptr)
     {
         fprintf(stderr,"could not allocate AVCodecContext.\n");
         ret = -2;
         goto end;
     }

     avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoStream]->codecpar);

     pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

     if (pCodec == nullptr)
     {
         fprintf(stderr,"Codec not found.\n");
         ret = -3;
         goto end;
     }
     else
     {
//         pCodecCtx->thread_count = 8;

         ///打开解码器
         if ((ret = avcodec_open2(pCodecCtx, pCodec, NULL)) && (ret < 0))
         {
             char buff[128]={0};
             av_strerror(ret, buff, 128);
             fprintf(stderr, "avcodec_open2 erro %s\n", buff);
             avcodec_close(pCodecCtx);
             goto end;
         }
     }

     if (pCodecCtx->pix_fmt == AV_PIX_FMT_NONE)
     {
         fprintf(stderr,"open file failed! pCodecCtx->pix_fmt == AV_PIX_FMT_NONE \n");
         ret = -4;
         goto end;
     }

     video_st = pFormatCtx->streams[videoStream];

     ///输出的rgb数据 分辨率
     OutPutWidth  = pCodecCtx->width;
     OutPutHeight = pCodecCtx->height;

//     if (OutPutWidth > 1920)
//     {
//         int newWidth = 1920;
//         int newHeight;

//         newHeight = OutPutHeight * newWidth / OutPutWidth;

//         fprintf(stderr, "%s PixWidth is Scaled w=%d h=%d newW=%d newH=%d \n", __FUNCTION__, OutPutWidth, OutPutHeight, newWidth, newHeight);

//         OutPutWidth  = newWidth;
//         OutPutHeight = newHeight;

//     }

//     if (OutPutHeight > 1080)
//     {
//         int newWidth;
//         int newHeight = 1080;

//         newWidth = OutPutWidth * newHeight / OutPutHeight;

//         fprintf(stderr, "%s PixHeight is Scaled w=%d h=%d newW=%d newH=%d \n", __FUNCTION__, OutPutWidth, OutPutHeight, newWidth, newHeight);

//         OutPutWidth  = newWidth;
//         OutPutHeight = newHeight;
//     }

     if (OutPutWidth % 2 != 0) OutPutWidth++;
     if (OutPutHeight % 2 != 0) OutPutHeight++;

     ///当传入的指针为空的时候也需要返回图像宽高
     *pnWidth  = OutPutWidth;
     *pnHeight = OutPutHeight;

     if (pYuv420pBuffer == nullptr)
     {
         ret = 0;
         goto end;
     }

     ///分配解码后的帧的存储空间
     {
         AVPixelFormat tartFormat = AV_PIX_FMT_YUV420P;

         pFrame = av_frame_alloc();  //用于保存解码后的帧
         pFrameRGB_Scaled = av_frame_alloc(); //用于保存转换后的rgb24数据

         ///将解码后的YUV数据转换成RGB24 并压缩rgb
         img_convert_ctx_RGBScaled = sws_getContext(pCodecCtx->width, pCodecCtx->height,
                 pCodecCtx->pix_fmt, OutPutWidth, OutPutHeight,
                 tartFormat, SWS_BICUBIC, nullptr, nullptr, nullptr);

         numBytes_rgb_scaled = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, OutPutWidth, OutPutHeight, 1);  //按1字节进行内存对齐,得到的内存大小最接近实际大小
		 out_buffer_rgb_scaled = static_cast<uint8_t *>(av_malloc(numBytes_rgb_scaled * sizeof(uint8_t)));
         av_image_fill_arrays(pFrameRGB_Scaled->data, pFrameRGB_Scaled->linesize, out_buffer_rgb_scaled, AV_PIX_FMT_YUV420P, OutPutWidth, OutPutHeight, 1);

		 //numBytes_rgb_scaled = avpicture_get_size(tartFormat, OutPutWidth, OutPutHeight);
		 //out_buffer_rgb_scaled = (uint8_t *)av_malloc(numBytes_rgb_scaled * sizeof(uint8_t));
		 //avpicture_fill((AVPicture *)pFrameRGB_Scaled, out_buffer_rgb_scaled, tartFormat,
			// OutPutWidth, OutPutHeight);

     }

//     ///输出视频信息
     av_dump_format(pFormatCtx, 0, file_path, 0);

     fprintf(stderr,"open file succeed!\n");

     ret = -200;

     do{
         if (av_read_frame(pFormatCtx, &packet) < 0)
         {
             fprintf(stderr,"read image stream failed!\n");
             break; //这里认为视频读取完了
         }

         if (packet.stream_index == videoStream)
         {
             if (avcodec_send_packet(pCodecCtx, &packet) != 0)
             {
                av_packet_unref(&packet);
                continue;
             }

             while (0 == avcodec_receive_frame(pCodecCtx, pFrame))
             {
                 ///判断解码完毕的帧是否是关键帧
                 bool isKeyFrame = false;

                 if(pFrame->key_frame)
                 {
                     isKeyFrame = true;
                 }

 //                ///反转图像 ，否则生成的rgb24图像是上下颠倒的
 //                pFrame->data[0] += pFrame->linesize[0] * (pCodecCtx->height - 1);
 //                pFrame->linesize[0] *= -1;
 //                pFrame->data[1] += pFrame->linesize[1] * (pCodecCtx->height / 2 - 1);
 //                pFrame->linesize[1] *= -1;
 //                pFrame->data[2] += pFrame->linesize[2] * (pCodecCtx->height / 2 - 1);
 //                pFrame->linesize[2] *= -1;

                 ///将解码后的图像转成rgb24
                 sws_scale(img_convert_ctx_RGBScaled,
                         (uint8_t const * const *) pFrame->data,
                         pFrame->linesize, 0, pCodecCtx->height, pFrameRGB_Scaled->data,
                         pFrameRGB_Scaled->linesize);

                 ///处理rgb数据

                 memcpy(pYuv420pBuffer, out_buffer_rgb_scaled, numBytes_rgb_scaled);

                 *pnWidth  = OutPutWidth;
                 *pnHeight = OutPutHeight;

                 ret = 0;

//                 FILE * fp = fopen("out.rgb24", "wb");
//                 fwrite(out_buffer_rgb_scaled, 1, numBytes_rgb_scaled, fp);
//                 fclose(fp);

//                 fprintf(stderr,"open file succeed! %d\n", numBytes_rgb_scaled);

             }
         }

         av_packet_unref(&packet);

     }while(0);

//     avcodec_close(pCodecCtx);
//     avformat_close_input(&pFormatCtx);

 end:

     {
         if (pFrame != nullptr)
             av_frame_free(&pFrame);

         if (pFrameRGB_Scaled != nullptr)
             av_frame_free(&pFrameRGB_Scaled);

         if (img_convert_ctx_RGBScaled != nullptr)
             sws_freeContext(img_convert_ctx_RGBScaled);

         if (out_buffer_rgb_scaled != nullptr)
             av_free(out_buffer_rgb_scaled);

         pFrame    = nullptr;
         pFrameRGB_Scaled = nullptr;
         out_buffer_rgb_scaled = nullptr; //压缩的rgb数据
         img_convert_ctx_RGBScaled = nullptr;  //用于RGB数据的压缩

     }

        if (pCodecCtx != nullptr)
        {
         avcodec_close(pCodecCtx);
         av_free(pCodecCtx);
        }

        if (pFormatCtx != nullptr)
        {
          avformat_close_input(&pFormatCtx);
          avformat_free_context(pFormatCtx);
        }

        if (opts != nullptr)
        {
          av_dict_free(&opts);
        }

//fprintf(stderr,"ReadImageFile %s   111 ret = %d \n", pFileName, ret);

     return ret;
}
