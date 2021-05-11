/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

#include <QWidget>
#include <QPropertyAnimation>

#include <QtWebSockets/QWebSocket>

#include <QListWidgetItem>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonArray>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>

#include "AppConfig.h"
#include "Media/MediaManager.h"

#include "Camera/ShowCameraWidget.h"
#include "CaptureTask/CaptureTaskManager.h"

#include "AppConfig.h"

class MainWindow :public QWidget, public VideoRecorderEventHandle
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    /***系统托盘****/
    QSystemTrayIcon *mTrayicon;
    QMenu *mPopMenu;
    QAction *mQuitAction;

    void setRecorderState(const RecoderState &state);

    ///录屏功能相关
    void startRecord();
    void pauseRecord();
    void restoreRecord();
    void stopRecord();

    void setAlphaValue(int value);

    ///动画控制控件
    QPropertyAnimation* mAnimation_TopTool;

    bool mIsTopToolShowingOut;   //顶部控件是否显示
    qint64 mLastMouseOnTopToolTime; //最后一次鼠标放在区域内的时间

    ShowCameraWidget  *mShowCameraWidget;  //显示摄像头画面
    CaptureTaskManager *mCaptureTaskManager; //设置捕捉窗体的控件

    void showOutTopTool();
    void hideTopTool();

    QTimer *mTimer_GetTime; //定时器-获取时间
    QTimer *mTimer_checkExe; //用于检测是否同时运行多个EXE

    void showMicVolume(const int &value);

private slots:
    void slotIconActivated(QSystemTrayIcon::ActivationReason);

    void slotBtnClicked(bool isChecked);

    void slotTimerTimeOut();
    void slotTimerTimeOut_checkExe();  //检测同时运行多个exe

protected:
    /**
     * @brief 输出音频音量
     * @param volume 音量大小:0~100
     */
    void OnAudioVolumeUpdated(const int &volumeL, const int &volumeR) override;

protected:
    bool eventFilter(QObject *obj, QEvent *event);
//    void closeEvent(QCloseEvent *event);

};

#endif // MAINWINDOW_H
