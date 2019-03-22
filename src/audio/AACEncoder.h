/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef AACENCODER_H
#define AACENCODER_H

#include <QThread>
#include <QMutex>

extern "C"
{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libswscale/swscale.h"
    #include "libavdevice/avdevice.h"
}

/// 编码aac的线程  这里把编码和采集分开 放到单独的线程 是因为编码也比较耗时

struct FrameDataNode
{
    uint8_t * buffer;
    int size;

    FrameDataNode()
    {
        buffer = NULL;
        size = 0;
    }
};

class AACEncoder : public QThread
{
    Q_OBJECT

public:
    explicit AACEncoder();
    ~AACEncoder();

    void startEncode();

    /**
     * @brief inputPcmBuffer 输入需要编码的PCM数据
     * @param buffer
     * @param size
     * @param time
     */
    void inputPcmBuffer(uint8_t *buffer, int size);

protected:
    void run();

private:
    QList<FrameDataNode> mPcmBufferList; //PCM数据队列
    QMutex mMutex;

};

#endif // AACENCODER_H
