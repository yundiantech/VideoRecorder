/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef CAPTURETASKMANAGER_H
#define CAPTURETASKMANAGER_H

#include <QWidget>
#include <QToolButton>

#include "Media/MediaManager.h"

#include "CaptureWindowWidget.h"
#include "CapturePictureWidget.h"

#include "SelectAreaWidget/SelectAreaWidget.h"
#include "SelectProgram/SelectRunningProgramDialog.h"
#include "Widget/Video/ShowVideoWidget.h"

namespace Ui {
class CaptureTaskManager;
}

enum BtnIndex{
    Btn_StartRecord = 1, //开始按钮
    Btn_PauseRecord,     //暂停按钮
    Btn_RestoreRecord,   //恢复按钮
    Btn_StopRecord       //停止按钮
};

enum RecoderState
{
    State_Recording,
    State_Pause,
    State_Stop
};

class CaptureTaskManager : public QWidget
{
    Q_OBJECT

public:
    explicit CaptureTaskManager(QWidget *parent = nullptr);
    ~CaptureTaskManager();

    ///录屏功能相关
    void startRecord();
    void pauseRecord();
    void restoreRecord();
    void stopRecord();

    ///设置视频数据回调函数
    void setCameraFrameCallBackFunc(std::function<void (VideoRawFramePtr yuvFrame, VideoRawFramePtr rgbFrame, void *param)> func = nullptr, void *param = nullptr);

    bool muteMicroPhone(bool isMute); //静音麦克风

    int64_t getVideoFileCurrentTime(); //获取录屏的时间戳(毫秒)

    void hideAll();

signals:
    void sig_RecorderStateChanged(const RecoderState &state);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::CaptureTaskManager *ui;

    QPoint dragPosition; //鼠标拖动
    bool mIsLeftBtnPressed;

    bool mIsPlayingVideo; //释放正在播放视频
//    PlayVideoTaskWidget *mPlayVideoTaskWidget; //播放视频的任务

    int mTaskId; //记录已经使用的任务号
    float mWidgetToScreenRate; //当前显示画面的区域与屏幕实际大小的比例

    SelectAreaWidget *mSelectAreaWidget;  //选择区域的控件
    QRect mSelectedCaptureRect; //当前录制的区域

    QSize mCameraVideoSize;
    ///这里声明用ShowVideoWidget*的话会崩溃，莫名其妙，因此改用void*
//    ShowVideoWidget *mShowCameraWidget; //显示摄像头画面的窗体
    void *mShowCameraWidget;

    QToolButton *mLastSelectedBtn;
    CaptureWindowWidget *mCaptureWindowWidget; //捕获窗体画面的类

    HWND mCurrentCaptrueProgramHandle;
    QRect mCurrentCaptureProgramRect;
    bool mCurrentCaptureProgramIsCoveredByOtherWindow;
    QTimer *mTimerUpdateSelectArea; //更新红框
    QTimer *mTimerUpdateSlider; //滑块移动后，用于重新设置分辨率

    MediaManager *mMediaManager; //获取屏幕图像的线程
    QTimer *mTimer_GetTime; //定时器-获取时间

    QString mCurrentVideoFilePath; //记录当前保持的视频文件路径

    ///摄像头数据回调函数
    std::function<void (VideoRawFramePtr yuvFrame, VideoRawFramePtr rgbFrame, void *param)> mCameraFrameCallBackFunc = nullptr; //摄像头数据回调函数
    void *mCameraFrameCallBackFuncParam = nullptr; //回调函数用户参数

    ///这里声明用ShowVideoWidget*的话会崩溃，莫名其妙，因此改用void*
//	ShowVideoWidget *mShowVideoWidget;
    void *mShowVideoWidget;

    bool mIsShowPicture;
    QImage mPictureImage;
    CapturePictureWidget *mCapturePictureWidget; //显示图片

    int mPictureRateMaxValue; //广告图片占比最大值

    void addCapPicture(const QString &filePath);
    void addCapWindow(HWND hwnd, QRect capRect);
    void addCapFullScreen(); //添加采集全屏

    void changeToFullScreenMode(); //切换到全屏捕捉模式

    void doDeviceTest();
    void resetMic();

    void setRecorderState(const RecoderState &state);

    void initDevice();
    void openCamera(const QString &cameraName);
    void doOpenCamera();

    void setVideoPlayMode(const bool &isVideoPlayMode);
    void reSetCaptureWidgetWidth();

    void setShowViewWidget(const bool &isShow, const bool &isWaitResize = true); //设置是否显示视频预览

    void inputCameraVideoFrame(VideoRawFramePtr videoFramePtr);
    void inputFinalVideoFrame(VideoRawFramePtr videoFramePtr);

private slots:
    void slotCurrentIndexChanged(int index);
    void slotBtnClicked(bool isChecked);
    void slotSliderValueChanged(int value);
    void slotSelectRectFinished(QRect re);
    void slotTimerTimeOut();

};

#endif // CAPTURETASKMANAGER_H
