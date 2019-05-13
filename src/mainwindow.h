
/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QUdpSocket>
#include <QHostInfo>
#include <QMessageBox>
#include <QScrollBar>
#include <QDateTime>
#include <QNetworkInterface>
#include <QProcess>

#include <QMainWindow>
#include <QPropertyAnimation>

#include "widget/selectrect.h"
#include "video/screenrecorder.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    enum RecoderState
    {
        Recording,
        Pause,
        Stop
    };

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void showOut();

protected:
    void closeEvent(QCloseEvent *);

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

private:
    Ui::MainWindow *ui;

    bool isLeftBtnPressed;
    QPoint dragPosition;

    QPropertyAnimation *animation; //动画类 用来实现窗体从上方慢慢出现

    QString mSaveFileDir; //视频保存目录
    QString mCurrentFilePath; //当前录制的视频文件存放路径

    ScreenRecorder *m_screenRecorder;
    QRect deskRect; //可用桌面大小
    SelectRect *selectRectWidget;  //选择区域的控件
    QRect rect; //当前录制的区域
    float m_rate; //屏幕宽高比

    QTimer * m_timer; //定时器 用于获取时间

    RecoderState m_recordeState;

    void initDev(); //获取录音设备的列表

    void loadConfigFile(); //加载配置文件
    void saveConfigFile(); //写入配置文件

    bool startRecord();
    bool pauseRecord();
    bool stopRecord();

private slots:
    void slotToolBtnToggled(bool);
    void slotBtnClicked();

    ///选择录屏区域相关 - Begin
    void slotSelectRectBtnClick();
    void slotEditRectBtnClick();
    void slotHideRectBtnClick();
    void slotSelectRectFinished(QRect);
    ///选择录屏区域相关 - End

    void slotTimerTimeOut();

    void slotCheckBoxClick(bool checked);
};

#endif // MAINWINDOW_H
