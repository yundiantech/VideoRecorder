#ifndef SAVEVIDEOFILE_H
#define SAVEVIDEOFILE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <QThread>
#include <QDebug>


extern"C"
{
    #include <libavutil/avassert.h>
    #include <libavutil/channel_layout.h>
    #include <libavutil/opt.h>
    #include <libavutil/mathematics.h>
    #include <libavutil/time.h>
//    #include <libavutil/timestamp.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
    #include <libavutil/imgutils.h>
}

#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio


// a wrapper around a single output AVStream
typedef struct OutputStream
{
    AVStream *st;
    AVCodecContext *enc;

    /* pts of the next frame that will be generated */
    int64_t next_pts;
    int samples_count;

    AVFrame *frame;
    AVFrame *tmp_frame;

    /// �������Ƶ����yuv420p����
    /// �������Ƶ���Ǵ��pcm���ݣ�����ȡ���պõ�һ֡���ݴ�������������
    uint8_t *frameBuffer;
    int frameBufferSize;

    float t, tincr, tincr2;

    struct SwsContext *sws_ctx;
    struct SwrContext *swr_ctx;
} OutputStream;

struct BufferDataNode
{
    uint8_t * buffer;
    int bufferSize;
    int64_t time;
    BufferDataNode * next;
};

class SaveVideoFileThread : public QThread
{
    Q_OBJECT

public:
    explicit SaveVideoFileThread();
    ~SaveVideoFileThread();

    void setFileName(QString filePath);

    void setQuantity(int value);
    void setWidth(int width,int height);
    bool startEncode();
    bool stopEncode();

    ///time�Ǻ���
    void videoDataQuene_Input(uint8_t * buffer, int size, int64_t time);
    BufferDataNode *videoDataQuene_get(int64_t time);

    void audioDataQuene_Input(uint8_t * buffer,int size);
    BufferDataNode *audioDataQuene_get();

    int audio_input_frame_size;

    void setContainsVideo(bool);
    void setContainsAudio(bool);

    void setVideoFrameRate(int value);

    ///��ȡʱ��������룩
    int64_t getVideoPts();
    int64_t getAudioPts();

signals:
    void sig_StartWriteFile(QString filePath);
    void sig_StopWriteFile(QString filePath);

protected:
    void run();

private:

    QString mFilePath;

    int m_videoFrameRate;

    bool isStop;

    int64_t audio_pts, video_pts; //ʱ�䣨���룩

    int mBitRate; //video bitRate

    int WIDTH;
    int HEIGHT;

    bool m_containsVideo;
    bool m_containsAudio;

    QMutex mVideoMutex;
    BufferDataNode * videoDataQueneHead;
    BufferDataNode * videoDataQueneTail;

    QMutex mAudioMutex;
    BufferDataNode * AudioDataQueneHead;
    BufferDataNode * AudioDataQueneTail;

    BufferDataNode * lastVideoNode; //��һ�ε�֡��֡�����ʱ������һ�ε�֡����ȫ��
    int videoBufferCount;
    int audioBufferCount;

    void open_audio(AVFormatContext *oc, AVCodec *codec, OutputStream *ost);
    void close_audio(AVFormatContext *oc, OutputStream *ost);

    void open_video(AVFormatContext *oc, AVCodec *codec, OutputStream *ost);
    void close_video(AVFormatContext *oc, OutputStream *ost);

    void add_video_stream(OutputStream *ost, AVFormatContext *oc, AVCodec **codec, AVCodecID codec_id);
    void add_audio_stream(OutputStream *ost, AVFormatContext *oc, AVCodec **codec, AVCodecID codec_id);

    bool write_audio_frame(AVFormatContext *oc, OutputStream *ost);
    bool write_video_frame(AVFormatContext *oc, OutputStream *ost, double time);

};



#endif // SAVEVIDEOFILE_H