/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "CaptureWindowThread.h"

#include "MoudleConfig.h"
#include <QDebug>
#if defined(WIN32)
    #include <WinSock2.h>
    #include <Windows.h>
#else

#endif

enum window_search_mode {
    INCLUDE_MINIMIZED,
    EXCLUDE_MINIMIZED
};

static bool check_window_valid(HWND window, enum window_search_mode mode)
{
    DWORD styles, ex_styles;
    RECT  rect;

    if (/*!IsWindowVisible(window) ||*/
        (mode == EXCLUDE_MINIMIZED && IsIconic(window)))
        return false;

    GetClientRect(window, &rect);
    styles    = (DWORD)GetWindowLongPtr(window, GWL_STYLE);
    ex_styles = (DWORD)GetWindowLongPtr(window, GWL_EXSTYLE);

    if (ex_styles & WS_EX_TOOLWINDOW)
        return false;
    if (styles & WS_CHILD)
        return false;
    if (mode == EXCLUDE_MINIMIZED && (rect.bottom == 0 || rect.right == 0))
        return false;

    return true;
}

static inline HWND next_window(HWND window, enum window_search_mode mode)
{
    while (true) {
        window = GetNextWindow(window, GW_HWNDNEXT);
        if (!window || check_window_valid(window, mode))
            break;
    }

    return window;
}

static inline HWND first_window(enum window_search_mode mode)
{
    HWND window = GetWindow(GetDesktopWindow(), GW_CHILD);
    if (!check_window_valid(window, mode))
        window = next_window(window, mode);
    return window;
}

/////////////
static BOOL IsMainWindow(HWND handle)
{
    return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
}

/************************************************************************/
/* hBitmap    为刚才的屏幕位图句柄
/* lpFileName 为需要保存的位图文件名
/************************************************************************/
int SaveBitmapToFile(HBITMAP hBitmap,LPSTR lpFileName)
{
    HDC            hDC; //设备描述表
    int            iBits;//当前显示分辨率下每个像素所占字节数
    WORD           wBitCount;//位图中每个像素所占字节数
    DWORD          dwPaletteSize=0;//定义调色板大小
    DWORD          dwBmBitsSize;//位图中像素字节大小
    DWORD          dwDIBSize;// 位图文件大小
    DWORD          dwWritten;//写入文件字节数
    BITMAP         Bitmap;//位图结构
    BITMAPFILEHEADER   bmfHdr;   //位图属性结构
    BITMAPINFOHEADER   bi;       //位图文件头结构
    LPBITMAPINFOHEADER lpbi;     //位图信息头结构     指向位图信息头结构
    HANDLE          fh;//定义文件句柄
    HANDLE            hDib;//分配内存句柄
    HANDLE            hPal;//分配内存句柄
    HANDLE          hOldPal=NULL;//调色板句柄

    //计算位图文件每个像素所占字节数
    hDC = CreateDC(L"DISPLAY",NULL,NULL,NULL);
    iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
    DeleteDC(hDC);

    if (iBits <= 1)
        wBitCount = 1;
    else if (iBits <= 4)
        wBitCount = 4;
    else if (iBits <= 8)
        wBitCount = 8;
    else if (iBits <= 24)
        wBitCount = 24;
    else if (iBits<=32)
        wBitCount = 24;


    //计算调色板大小
    if (wBitCount <= 8)
        dwPaletteSize = (1 << wBitCount) *sizeof(RGBQUAD);



    //设置位图信息头结构
    GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);
    bi.biSize            = sizeof(BITMAPINFOHEADER);
    bi.biWidth           = Bitmap.bmWidth;
    bi.biHeight          = Bitmap.bmHeight;
    bi.biPlanes          = 1;
    bi.biBitCount         = wBitCount;
    bi.biCompression      = BI_RGB;
    bi.biSizeImage        = 0;
    bi.biXPelsPerMeter     = 0;
    bi.biYPelsPerMeter     = 0;
    bi.biClrUsed         = 0;
    bi.biClrImportant      = 0;
    dwBmBitsSize = ((Bitmap.bmWidth *wBitCount+31)/32)* 4*Bitmap.bmHeight ;

    //为位图内容分配内存
    hDib  = GlobalAlloc(GHND,dwBmBitsSize+dwPaletteSize+sizeof(BITMAPINFOHEADER));
    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
    if (lpbi==NULL)
    {
        return 0;
    }

    *lpbi = bi;
    // 处理调色板
    hPal = GetStockObject(DEFAULT_PALETTE);
    if (hPal)
    {
        hDC  = GetDC(NULL);
        hOldPal = ::SelectPalette(hDC, (HPALETTE)hPal, FALSE);
        RealizePalette(hDC);
    }
    // 获取该调色板下新的像素值
    GetDIBits(hDC, hBitmap, 0, (UINT) Bitmap.bmHeight,
        (LPSTR)lpbi + sizeof(BITMAPINFOHEADER)+dwPaletteSize,
        (LPBITMAPINFO)lpbi, DIB_RGB_COLORS);
    //恢复调色板
    if (hOldPal)
    {
        SelectPalette(hDC, (HPALETTE)hOldPal, TRUE);
        RealizePalette(hDC);
        ReleaseDC(NULL, hDC);
    }
    //创建位图文件
    fh = CreateFileA(lpFileName, GENERIC_WRITE,
        0, NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (fh == INVALID_HANDLE_VALUE)
        return FALSE;

    // 设置位图文件头
    bmfHdr.bfType = 0x4D42;  // "BM"
    dwDIBSize    = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)+ dwPaletteSize + dwBmBitsSize;
    bmfHdr.bfSize = dwDIBSize;
    bmfHdr.bfReserved1 = 0;
    bmfHdr.bfReserved2 = 0;
    bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER)+ dwPaletteSize;

    // 写入位图文件头
    WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);

    // 写入位图文件其余内容
    WriteFile(fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL);

    //清除
    GlobalUnlock(hDib);
    GlobalFree(hDib);
    CloseHandle(fh);

    return 1;
}

CaptureWindowThread::CaptureWindowThread()
{
    pFrameRGB = nullptr;
    pFrameYUV = nullptr;

    outBufferYUV = nullptr;
    outBufferRGB = nullptr;
    img_convert_ctx = nullptr;

    mCallBackFunc      = nullptr;
    mCallBackFuncParam = nullptr;

    mIsStop = false;
    mIsThreadRunning = false;

    mIsPause = false;
    mStartTime = 0; //第一次获取到数据的时间
    mCurrentTime = 0;

    mLastGetVideoTime = 0;
    mFrameRate = 20;

    mIsFollowMouseMode = false;

    mX = 0;
    mY = 0;
    mW = 0;
    mH = 0;
}

CaptureWindowThread::~CaptureWindowThread()
{

}

void CaptureWindowThread::setHWND(const HWND &hWnd)
{
    mHWnd = hWnd;
}

void CaptureWindowThread::setQuantity(const int &value)
{
//    mVideoEncoder->setQuantity(value);
}

void CaptureWindowThread::setFrameRate(const int &frameRate)
{
    mFrameRate = frameRate;
}

void CaptureWindowThread::setRect(const int &x, const int &y, const int &width, const int &height)
{
    mX = x;
    mY = y;
    mW = width;
    mH = height;
}

void CaptureWindowThread::setFollowMouseMode(const bool &value)
{
    mIsFollowMouseMode = value;
}

bool CaptureWindowThread::init(const int &width, const int &height)
{
    int y_size = 0;
    int yuvSize = 0;

    pFrameRGB = av_frame_alloc();
    pFrameYUV = av_frame_alloc();

    ///将数据转成YUV420P格式
    img_convert_ctx = sws_getContext(width, height, AV_PIX_FMT_BGR24,
                                     width, height, AV_PIX_FMT_YUV420P,
                                     SWS_BICUBIC, nullptr, nullptr, nullptr);

    y_size = width * height;
//        yuvSize = avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
    yuvSize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, width, height, 1);  //按1字节进行内存对齐,得到的内存大小最接近实际大小
//        image_buf_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 0);  //按0字节进行内存对齐，得到的内存大小是0
//        image_buf_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 4);   //按4字节进行内存对齐，得到的内存大小稍微大一些

    ///理论上 这里的 yuvSize = y_size * 3 / 2
    unsigned int numBytes = static_cast<unsigned int>(yuvSize);
    outBufferYUV = static_cast<uint8_t *>(av_malloc(numBytes * sizeof(uint8_t)));
//    avpicture_fill((AVPicture *) pFrameYUV, outBufferYUV, AV_PIX_FMT_YUV420P,pCodecCtx->width, pCodecCtx->height);
    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, outBufferYUV, AV_PIX_FMT_YUV420P, width, height, 1);

    ///理论上 这里的 yuvSize = y_size * 3 / 2
    unsigned int numBytesRgb = av_image_get_buffer_size(AV_PIX_FMT_BGR24, width, height, 1);
    outBufferRGB = static_cast<uint8_t *>(av_malloc(numBytesRgb * sizeof(uint8_t)));
    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, outBufferRGB, AV_PIX_FMT_BGR24, width, height, 1);

    {
        ///反转图像 ，否则生成的rgb24图像是上下颠倒的
        pFrameYUV->data[0] += pFrameYUV->linesize[0] * (height - 1);
        pFrameYUV->linesize[0] *= -1;
        pFrameYUV->data[1] += pFrameYUV->linesize[1] * (height / 2 - 1);
        pFrameYUV->linesize[1] *= -1;
        pFrameYUV->data[2] += pFrameYUV->linesize[2] * (height / 2 - 1);
        pFrameYUV->linesize[2] *= -1;
    }

    return true;
}

void CaptureWindowThread::deInit()
{
    if (pFrameRGB != nullptr)
    {
        av_free(pFrameRGB);
    }

    if (pFrameYUV != nullptr)
    {
        av_free(pFrameYUV);
    }

    if (outBufferYUV != nullptr)
    {
        av_free(outBufferYUV);
    }

    if (outBufferRGB != nullptr)
    {
        av_free(outBufferRGB);
    }

    if (img_convert_ctx != nullptr)
    {
        sws_freeContext(img_convert_ctx);
    }

    pFrameRGB = nullptr;
    pFrameYUV = nullptr;

    outBufferYUV = nullptr;
    outBufferRGB = nullptr;
    img_convert_ctx = nullptr;
}

void CaptureWindowThread::startRecord(std::function<void (VideoRawFramePtr videoFramePtr, void *param)> func,
                                      void *param, const int64_t &startPts)
{
    mIsStop = false;

    mCallBackFunc      = func;
    mCallBackFuncParam = param;

    mStartTime = 0;

    if (startPts > 0)
    {
        mStartTime = MoudleConfig::getTimeStamp_MilliSecond() - startPts;
    }
qDebug()<<__FUNCTION__<<mIsStop<<mIsThreadRunning<<startPts;

    mIsThreadRunning = true;

    //启动新的线程
    std::thread([=](CaptureWindowThread *pointer)
    {
        pointer->run();

    }, this).detach();

}

void CaptureWindowThread::pauseRecord()
{
    mIsPause = true;
}

void CaptureWindowThread::restoreRecord()
{
    mStartTime = MoudleConfig::getTimeStamp_MilliSecond() - mCurrentTime;
    mIsPause = false;
}

void CaptureWindowThread::stopRecord(const bool &isBlock)
{
qDebug()<<__FUNCTION__<<mIsStop<<mIsThreadRunning;

    mIsStop = true;

    if (isBlock)
    {
        while(mIsThreadRunning)
        {
            MoudleConfig::mSleep(10);
        }
    }

qDebug()<<__FUNCTION__<<mIsStop<<mIsThreadRunning<<"finished!";
}

void CaptureWindowThread::run()
{
    mIsThreadRunning = true;
qDebug()<<__FUNCTION__<<"starting...";
    if (mStartTime <= 0)
    {
        mStartTime = MoudleConfig::getTimeStamp_MilliSecond();
    }

    int VideoWidth  = 0;
    int VideoHeight = 0;

    while(!mIsStop)
    {
        if ((MoudleConfig::getTimeStamp_MilliSecond() - mLastGetVideoTime) <= (1000 / mFrameRate))
        {
            MoudleConfig::mSleep(5);
            continue;
        }

        if (mIsPause)
        {
            MoudleConfig::mSleep(10);
            continue;
        }

        HWND hWnd = mHWnd;

        int width = 0;
        int height = 0;

        if (hWnd != nullptr)
        {
            if (IsIconic(hWnd) || IsCoveredByOtherWindow(hWnd))
            {
                MoudleConfig::mSleep(100);
                continue;
            }

//            RECT rect;
//    //        GetClientRect(hWnd, &rect);
//            GetWindowRect(hWnd, &rect);

            RECT rect = CaptureWindowThread::getWindowRect(hWnd);

            width  = rect.right - rect.left;
            height = rect.bottom - rect.top;

            mX = rect.left;
            mY = rect.top;
            mW = width;
            mH = height;

    //fprintf(stderr, "record starting %d %d... \n", width, height);
        }
        else
        {
            width  = GetSystemMetrics(SM_CXVIRTUALSCREEN); /// 获取桌面总高度
            height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

            int xScreen = ::GetSystemMetrics(SM_XVIRTUALSCREEN); // 获取桌面x坐标，可以为负值
            int yScreen = ::GetSystemMetrics(SM_YVIRTUALSCREEN); // 获取桌面y坐标，可以为负值
//            int nScreenCount = ::GetSystemMetrics(SM_CMONITORS); ///获取屏幕数量

            if (mIsFollowMouseMode)
            {
                POINT point;
                GetCursorPos(&point);            // 获取鼠标指针位置（屏幕坐标）
                ScreenToClient(hWnd, &point);    // 将鼠标指针位置转换为窗口坐标

                int x = point.x - (mW / 2);
                int y = point.y - (mH / 2);

                if (x < xScreen) x = xScreen;
                if (y < yScreen) y = yScreen;

                if ((x + mW) > width)
                {
                    x = width - mW;
                }

                if ((y + mH) > height)
                {
                    y = height - mH;
                }

                mX = x;
                mY = y;

//qDebug()<<__FUNCTION__<<point.x<<point.y<<x<<y<<width<<height;
            }
        }

        if (mW > 0)
        {
            width = mW;
        }

        if (mH > 0)
        {
            height = mH;
        }

        /// 传给编码器的图形宽高必须是偶数

        ///这里bitmap的宽度必须是4的倍数，否则data里面取到的rgb24数据有问题
        if ((width % 4) != 0)
        {
            width += 4 - (width % 4);
        }

        if ((height % 2) != 0)
        {
            height -= 1;
        }

        if (VideoWidth != width || VideoHeight != height)
        {
            deInit();

            init(width, height);

            VideoWidth = width;
            VideoHeight = height;
        }

//fprintf(stderr, "record starting %d %d... \n", width, height);

        {
            HDC     hDC;
            HDC     MemDC;
            BYTE*   Data;
            HBITMAP   hBmp;
            BITMAPINFO   bi;

            memset(&bi,   0,   sizeof(bi));
            bi.bmiHeader.biSize     = sizeof(BITMAPINFO);
            bi.bmiHeader.biWidth    = width;//GetSystemMetrics(SM_CXSCREEN);
            bi.bmiHeader.biHeight   = height;//GetSystemMetrics(SM_CYSCREEN);
            bi.bmiHeader.biPlanes   = 1;
            bi.bmiHeader.biBitCount = 24;

//            hDC   =   GetDC(hWnd);
            hDC   =   GetDC(NULL); //只采集桌面，然后裁图
            MemDC =   CreateCompatibleDC(hDC);
            hBmp  =   CreateDIBSection(MemDC,   &bi, DIB_RGB_COLORS,   (void**)&Data,   NULL,   0);
            SelectObject(MemDC,   hBmp);
            BitBlt(MemDC,   0,   0,   bi.bmiHeader.biWidth,   bi.bmiHeader.biHeight,hDC,   mX,   mY,   SRCCOPY);
            ReleaseDC(NULL,   hDC);
            DeleteDC(MemDC);

            {

                POINT point;
                GetCursorPos(&point);            // 获取鼠标指针位置（屏幕坐标）
                ScreenToClient(hWnd, &point);    // 将鼠标指针位置转换为窗口坐标

                HDC hBmpDC=CreateCompatibleDC(NULL); //建立兼容DC
                HBITMAP hBmpOld = (HBITMAP)SelectObject(hBmpDC, hBmp); //原图选入兼容DC

                // Get information about the global cursor.
                CURSORINFO ci;
                ci.cbSize = sizeof(ci);
                GetCursorInfo(&ci);

                // Draw the cursor into the canvas.
                DrawIcon(hBmpDC, point.x, point.y, ci.hCursor);

//                LineTo(hBmpDC,100,100);  //画线
        //        HBITMAP hBmpRet= (HBITMAP)SelectObject(hBmpDC,hBmpOld); //返回画线后的位图

                DeleteObject(hBmpOld);
                DeleteDC(hBmpDC);
            }

//            fwrite(Data, 1, width * height * 3, fp);

            mCurrentTime = MoudleConfig::getTimeStamp_MilliSecond() - mStartTime;
//fprintf(stderr, "mCurrentTime %d... \n", mCurrentTime);
            {
                mLastGetVideoTime = MoudleConfig::getTimeStamp_MilliSecond();

                memcpy(outBufferRGB, Data, width*height*3);

                /// 转换成YUV420
                /// 由于解码后的数据不一定是yuv420p，比如硬件解码后会是yuv420sp，因此这里统一转成yuv420p
                sws_scale(img_convert_ctx, (const uint8_t* const*)pFrameRGB->data, pFrameRGB->linesize, 0, height, pFrameYUV->data, pFrameYUV->linesize);

                VideoRawFramePtr yuvFrame = std::make_shared<VideoRawFrame>();

                yuvFrame->initBuffer(width, height, VideoRawFrame::FRAME_TYPE_YUV420P, mCurrentTime);
                yuvFrame->setFramebuf(outBufferYUV);

                if (mCallBackFunc != nullptr)
                {
                    mCallBackFunc(yuvFrame, mCallBackFuncParam);
                }

            }

//            SaveBitmapToFile(hBmp, (char*)"out.bmp");
//fwrite(Data, 1, width*height*3, fp);
            DeleteObject(hBmp);
        }
    }

    fprintf(stderr, "%s record stopping... \n", __FUNCTION__);

    deInit();

    fprintf(stderr, "%s record finished! \n", __FUNCTION__);

    mIsPause = false;

    mIsThreadRunning = false;

    qDebug()<<__FUNCTION__<<"stopping...";
}

//FILE *fp = fopen("out.rgb", "wb");
//void CaptureWindowThread::run()
//{
//    mIsThreadRunning = true;
//qDebug()<<__FUNCTION__<<"starting...";
//    if (mStartTime <= 0)
//    {
//        mStartTime = MoudleConfig::getTimeStamp_MilliSecond();
//    }

//    int VideoWidth  = 0;
//    int VideoHeight = 0;

//    while(!mIsStop)
//    {
//        if ((MoudleConfig::getTimeStamp_MilliSecond() - mLastGetVideoTime) <= (1000 / mFrameRate))
//        {
//            MoudleConfig::mSleep(5);
//            continue;
//        }

//        if (mIsPause)
//        {
//            MoudleConfig::mSleep(10);
//            continue;
//        }

//        HWND hWnd = mHWnd;

//        int width = 0;
//        int height = 0;

//        if (hWnd != nullptr)
//        {
//            RECT rect;
//    //        GetClientRect(hWnd, &rect);
//            GetWindowRect(hWnd, &rect);

//            width  = rect.right - rect.left;
//            height = rect.bottom - rect.top;
//    //fprintf(stderr, "record starting %d %d... \n", width, height);
//        }
//        else
//        {
//            width  = GetSystemMetrics(SM_CXSCREEN);
//            height = GetSystemMetrics(SM_CYSCREEN);
//        }

//        if (mW > 0)
//        {
//            width = mW;
//        }

//        if (mH > 0)
//        {
//            height = mH;
//        }

//        /// 传给编码器的图形宽高必须是偶数

//        ///这里bitmap的宽度必须是4的倍数，否则data里面取到的rgb24数据有问题
//        if ((width % 4) != 0)
//        {
//            width += 4 - (width % 4);
//        }

//        if ((height % 2) != 0)
//        {
//            height -= 1;
//        }

//        if (VideoWidth != width || VideoHeight != height)
//        {
//            deInit();

//            init(width, height);

//            VideoWidth = width;
//            VideoHeight = height;
//        }

////fprintf(stderr, "record starting %d %d... \n", width, height);

////        if (hWnd != nullptr)
//        {
//            HDC     hDC;
//            HDC     MemDC;
//            BYTE*   Data;
//            HBITMAP   hBmp;
//            BITMAPINFO   bi;

//            memset(&bi,   0,   sizeof(bi));
//            bi.bmiHeader.biSize     = sizeof(BITMAPINFO);
//            bi.bmiHeader.biWidth    = width;//GetSystemMetrics(SM_CXSCREEN);
//            bi.bmiHeader.biHeight   = height;//GetSystemMetrics(SM_CYSCREEN);
//            bi.bmiHeader.biPlanes   = 1;
//            bi.bmiHeader.biBitCount = 24;

//            hDC   =   GetDC(hWnd);
//            MemDC =   CreateCompatibleDC(hDC);
//            hBmp  =   CreateDIBSection(MemDC,   &bi, DIB_RGB_COLORS,   (void**)&Data,   NULL,   0);
//            SelectObject(MemDC,   hBmp);
//            BitBlt(MemDC,   0,   0,   bi.bmiHeader.biWidth,   bi.bmiHeader.biHeight,hDC,   mX,   mY,   SRCCOPY);
//            ReleaseDC(NULL,   hDC);
//            DeleteDC(MemDC);

//            {

//                POINT point;
//                GetCursorPos(&point);            // 获取鼠标指针位置（屏幕坐标）
//                ScreenToClient(hWnd, &point);    // 将鼠标指针位置转换为窗口坐标

//                HDC hBmpDC=CreateCompatibleDC(NULL); //建立兼容DC
//                HBITMAP hBmpOld = (HBITMAP)SelectObject(hBmpDC, hBmp); //原图选入兼容DC

//                // Get information about the global cursor.
//                CURSORINFO ci;
//                ci.cbSize = sizeof(ci);
//                GetCursorInfo(&ci);

//                // Draw the cursor into the canvas.
//                DrawIcon(hBmpDC, point.x, point.y, ci.hCursor);

////                LineTo(hBmpDC,100,100);  //画线
//        //        HBITMAP hBmpRet= (HBITMAP)SelectObject(hBmpDC,hBmpOld); //返回画线后的位图

//                DeleteObject(hBmpOld);
//                DeleteDC(hBmpDC);
//            }

////            fwrite(Data, 1, width * height * 3, fp);

//            mCurrentTime = MoudleConfig::getTimeStamp_MilliSecond() - mStartTime;
////fprintf(stderr, "mCurrentTime %d... \n", mCurrentTime);
//            {
//                mLastGetVideoTime = MoudleConfig::getTimeStamp_MilliSecond();

//                memcpy(outBufferRGB, Data, width*height*3);

//                /// 转换成YUV420
//                /// 由于解码后的数据不一定是yuv420p，比如硬件解码后会是yuv420sp，因此这里统一转成yuv420p
//                sws_scale(img_convert_ctx, (const uint8_t* const*)pFrameRGB->data, pFrameRGB->linesize, 0, height, pFrameYUV->data, pFrameYUV->linesize);

//                VideoRawFramePtr yuvFrame = std::make_shared<VideoRawFrame>();

//                yuvFrame->initBuffer(width, height, VideoRawFrame::FRAME_TYPE_YUV420P, mCurrentTime);
//                yuvFrame->setFramebuf(outBufferYUV);

//                if (mCallBackFunc != nullptr)
//                {
//                    mCallBackFunc(yuvFrame, mCallBackFuncParam);
//                }

//            }

////            SaveBitmapToFile(hBmp, (char*)"out.bmp");
////fwrite(Data, 1, width*height*3, fp);
//            DeleteObject(hBmp);
//        }
//    }

//    fprintf(stderr, "%s record stopping... \n", __FUNCTION__);

//    deInit();

//    fprintf(stderr, "%s record finished! \n", __FUNCTION__);

//    mIsPause = false;
//qDebug()<<__FUNCTION__<<"stopping...";
//    mIsThreadRunning = false;
//}

RECT CaptureWindowThread::getWindowRect(HWND hWnd)
{
    RECT outRect = RECT{0, 0, 0, 0};

    RECT rect;
    if (GetWindowRect(hWnd, &rect))
    {

        int x = 0;
        int y = 0;
        int w = 0;
        int h = 0;

        RECT rect2;
        GetClientRect(hWnd, &rect2);

        int width  = rect.right - rect.left;
        int height = rect.bottom - rect.top;

        int width2  = rect2.right - rect2.left;
        int height2 = rect2.bottom - rect2.top;

        x = rect.left;
        y = rect.top;
        w = rect.right - rect.left;
        h = rect.bottom - rect.top;

        int value = width - width2;

        w -= value;
        h -= (value/2);
        x += (value/2);

//    qDebug()<<__FUNCTION__<<x<<y<<w<<h<<value;

        ///这里把宽度计算成4的整数倍，方便后面使用
        if ((w % 4) != 0)
        {
//            w += 4 - (w % 4);
            w -= (w % 4);
        }

        outRect = RECT{x, y, x+w, y+h};
    }

    return outRect;
}

std::list<HWND> CaptureWindowThread::getCaptureWindowList()
{
    std::list<HWND> handleList;

    HWND pWnd = first_window(INCLUDE_MINIMIZED); //得到第一个窗口句柄

    if (IsWindowAvailable(pWnd))
        handleList.push_back(pWnd);

    while(pWnd != nullptr)
    {
        pWnd = next_window(pWnd, INCLUDE_MINIMIZED);//得到下一个窗口句柄

        if (IsWindowAvailable(pWnd))
            handleList.push_back(pWnd);
    }

    return handleList;

}

bool CaptureWindowThread::IsWindowAvailable(HWND hWnd)
{
    bool isAvailable = false;

    if (IsMainWindow(hWnd))
    {
//                if(GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)
//                {
//qDebug()<<true;
//                }

//            RECT rect;
//            GetWindowRect(hWnd, &rect);

//        WCHAR szTitle[256] = {0};
//        GetWindowText(hWnd, szTitle, 256 );
//        QString title = QString::fromWCharArray(szTitle);

        char className[512] = {0};
        GetClassNameA(hWnd, className, 512);

        if (strcmp(className, "Windows.UI.Core.CoreWindow") == 0
                || strcmp(className, "ApplicationFrameWindow") == 0)
        {

        }
        else
        {
            isAvailable = true;
        }
    }

    return isAvailable;
}

bool CaptureWindowThread::IsCoveredByOtherWindow(HWND hWnd)
{
    std::list<HWND> availableHandleList = getCaptureWindowList();

    RECT rcTarget;
    ::GetWindowRect(hWnd, &rcTarget);

    bool isChild = (WS_CHILD == (::GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD));

    if (::GetDesktopWindow() == hWnd)
        hWnd = ::GetWindow(::GetTopWindow(hWnd), GW_HWNDLAST);

    do{
        HWND hCurWnd = hWnd;

        while(NULL != (hWnd = ::GetNextWindow(hWnd, GW_HWNDPREV)))
        {
            if (std::find(availableHandleList.begin(), availableHandleList.end(), hWnd) != availableHandleList.end())
            {
                //存在
            }
            else
            {
                //不存在
                continue;
            }

            if(IsWindowAvailable(hWnd))
            {
                RECT rcWnd;
                ::GetWindowRect(hWnd, &rcWnd);

                if(!((rcWnd.right < rcTarget.left) || (rcWnd.left > rcTarget.right) ||
                    (rcWnd.bottom < rcTarget.top) || (rcWnd.top > rcTarget.bottom)))
                {
                    return true;
                }
            }
        }

        if(isChild)
        {
            hWnd = ::GetParent(hCurWnd);
            isChild = hWnd ? (WS_CHILD == (::GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD)) : false;
        }
        else
        {
            break;
        }

    }while(true);

 return false;
}
