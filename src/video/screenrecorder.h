
/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef SCREENRECORDER_H
#define SCREENRECORDER_H

#include <QThread>

#include "getvideothread.h"
#include "savevideofile.h"

class ScreenRecorder : public QObject
{
    Q_OBJECT

public:
    explicit ScreenRecorder();
    ~ScreenRecorder();

    void setFileName(QString filePath);

    ErroCode init(QString videoDevName,bool useVideo,QString audioDevName,bool useAudio);

    void startRecord();
    void pauseRecord();
    void restoreRecord();
    void stopRecord();

    void setPicRange(int x,int y,int w,int h); //设置录制屏幕的区域
    void setVideoFrameRate(int value);

    double getVideoPts();
    double getAudioPts();

private:
    SaveVideoFileThread * m_saveVideoFileThread; //保存成视频文件的线程

    /// 把视频和音频放到一起获取
    /// avformat_free_context释放的时候会奔溃
    /// 无奈，只能把他们放到2个线程中执行
    GetVideoThread *m_videoThread; //获取视频的线程
    GetVideoThread *m_audioThread; //获取音频的线程

    bool m_useVideo;
    bool m_useAudio;

};

#endif // SCREENRECORDER_H
