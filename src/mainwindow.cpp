/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>
#include <QUuid>
#include <QDesktopWidget>
#include <QAudioDeviceInfo>
#include <QDebug>
#include <QColorDialog>
#include <QSharedMemory>

#include <QWindow>
#include <QScreen>
#include <QJsonObject>
#include <QJsonDocument>

#include <QProcess>
#include <QTextCodec>

#include <QSslConfiguration>

#if defined(WIN32)
    #include <WinSock2.h>
    #include <Windows.h>
#else

#endif

#include "AppConfig.h"
#include "Base/FunctionTransfer.h"
#include "Base64/Base64.h"

#include "Widget/mymessagebox_withTitle.h"

#include "ShareMemery/ShareMemery.h"

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    FunctionTransfer::init();

    AppConfig::gMainWindow = this;

    this->setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::WindowStaysOnTopHint|Qt::FramelessWindowHint|Qt::Tool|Qt::X11BypassWindowManagerHint); //使窗口置顶
//    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint); //使窗口标题栏隐藏

    ui->label_erro_tips_10000->hide();

    ///系统托盘
    mPopMenu    = new QMenu;
    mQuitAction = new QAction(QIcon("images/open.png"), QStringLiteral("退出"), this);

    connect(mQuitAction, &QAction::triggered, [=](bool isChecked)
    {
        int ret = MyMessageBox_WithTitle::showWarningText(QStringLiteral("警告"),
                                                           QStringLiteral("确定要要退出程序么？"),
                                                           QStringLiteral("确定"),
                                                           QStringLiteral("取消"));
        if (ret == QDialog::Accepted)
        {
            stopRecord();
            QApplication::quit();
        }
    });

    mPopMenu->addAction(mQuitAction);
//    mPopMenu->addSeparator();       //添加分离器

    mTrayicon = new QSystemTrayIcon(this);
    QIcon ico(":/img/logo.png");
    mTrayicon->setIcon(ico);  //设置图标
    mTrayicon->setToolTip(QStringLiteral("视频录制软件"));
    mTrayicon->setContextMenu(mPopMenu);  //设置菜单(QMenu *trayiconMenu;)
    mTrayicon->show(); //显示托盘

    connect(mTrayicon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this,SLOT(slotIconActivated(QSystemTrayIcon::ActivationReason)));//图标被激活时

    mIsTopToolShowingOut   = false;

    mAnimation_TopTool   = new QPropertyAnimation(ui->widget_top,   "geometry");

    connect(mAnimation_TopTool,   &QPropertyAnimation::finished, [=]
    {
        if (mAnimation_TopTool->endValue().toRect().y() == 0) //展出
        {
            ui->widget_expand_top->hide();
        }
        else
        {
            ui->widget_expand_top->show();
        }
    });

    mShowCameraWidget  = new ShowCameraWidget();
    mShowCameraWidget->setGeometry(AppConfig::gLocalCameraWidgetRect);
    mShowCameraWidget->hide();

    connect(mShowCameraWidget, &ShowCameraWidget::sigViewStateChanged, [=](const bool &isShow)
    {
        ui->toolButton_camera->setChecked(isShow);
    });

    //rect
    QDesktopWidget* desktopWidget = QApplication::desktop();
    QRect screenRect = desktopWidget->screenGeometry();

    connect(ui->toolButton_camera, SIGNAL(clicked(bool)), this, SLOT(slotBtnClicked(bool)));
    connect(ui->toolButton_view,  SIGNAL(clicked(bool)), this, SLOT(slotBtnClicked(bool)));

    connect(ui->toolButton_micro, SIGNAL(clicked(bool)), this, SLOT(slotBtnClicked(bool)));
    connect(ui->toolButton_exit, SIGNAL(clicked(bool)),this, SLOT(slotBtnClicked(bool)));

    connect(ui->toolButton_record_start,   &QToolButton::clicked, this, &MainWindow::slotBtnClicked);
    connect(ui->toolButton_record_restore, &QToolButton::clicked, this, &MainWindow::slotBtnClicked);
    connect(ui->toolButton_record_pause,   &QToolButton::clicked, this, &MainWindow::slotBtnClicked);
    connect(ui->toolButton_record_stop,    &QToolButton::clicked, this, &MainWindow::slotBtnClicked);

    mCaptureTaskManager = new CaptureTaskManager();
    connect(mCaptureTaskManager, &CaptureTaskManager::sig_RecorderStateChanged, [=](const RecoderState &state)
    {
        this->setRecorderState(state);
    });


    auto getCameraVideoFrameFunc = [=](VideoRawFramePtr yuvFramePtr, VideoRawFramePtr rgbFramePtr, void *param)
    {
        mShowCameraWidget->inputVideoFrame(yuvFramePtr);
    };

    mCaptureTaskManager->setCameraFrameCallBackFunc(getCameraVideoFrameFunc, this);

    mTimer_GetTime = new QTimer(this);
    connect(mTimer_GetTime, &QTimer::timeout, this, &MainWindow::slotTimerTimeOut);
    mTimer_GetTime->setInterval(500);
    mTimer_GetTime->start();

    mTimer_checkExe = new QTimer;
    mTimer_checkExe->setInterval(1000);
    connect(mTimer_checkExe, &QTimer::timeout, this, &MainWindow::slotTimerTimeOut_checkExe);
    mTimer_checkExe->start();

    showMicVolume(0);

    {
        ui->toolButton_record_start->show();
        ui->toolButton_record_stop->hide();
        ui->toolButton_record_restore->hide();
        ui->toolButton_record_pause->hide();
    }

    ui->pushButton_expand_top->installEventFilter(this);

    ui->label_time_2->clear();
//    ui->label_time_2->setPixmap(QPixmap(":/image/pull_top.png"));
    ui->label_time_2->setText(QStringLiteral("我的录屏"));

    showOutTopTool();

//    std::thread([=]
//    {
//        AppConfig::mSleep(2000);
//        FunctionTransfer::runInMainThread([=]()
//        {
//            this->hideTopTool();
//            this->hideRightTool();
//        });
//    }).detach();

    bool IsAudioInDeviceExist  = false;
    bool IsAudioOutDeviceExist = false;

    if (QAudioDeviceInfo::availableDevices(QAudio::AudioInput).size() > 0)
    {
        IsAudioInDeviceExist = true;
    }

    if (QAudioDeviceInfo::availableDevices(QAudio::AudioOutput).size() > 0)
    {
        IsAudioOutDeviceExist = true;
    }

    if (IsAudioInDeviceExist)
    {
        mCaptureTaskManager->show();
        mCaptureTaskManager->raise();
    }
    else
    {
        QString tipStr;

        if (!IsAudioInDeviceExist)
        {
            tipStr = QStringLiteral("未检测到麦克风，无法录制！");
        }

        int ret = MyMessageBox_WithTitle::showWarningText(QStringLiteral("警告"),
                                                                   tipStr,
                                                                   NULL,
                                                                   QStringLiteral("退出程序"));
        std::thread([=]
        {
            AppConfig::mSleep(10);
            FunctionTransfer::runInMainThread([=]()
            {
                QApplication::quit();
            });
        }).detach();

    }

}

MainWindow::~MainWindow()
{
    qDebug()<<__FUNCTION__;

    AppConfig::gLocalCameraWidgetRect = mShowCameraWidget->geometry();
    AppConfig::saveConfigInfoToFile();
    AppConfig::removeDirectory(AppConfig::AppDataPath_Tmp);

    delete ui;
}

void MainWindow::showOutTopTool()
{
    QDesktopWidget* desktopWidget = QApplication::desktop();
    QRect screenRect = desktopWidget->screenGeometry();//获取设备屏幕大小
    QRect deskRect = desktopWidget->screenGeometry();//获取设备屏幕大小

    int x = screenRect.width() / 2 - ui->widget_top->width() / 2;
    int y = 0 - ui->widget_top_tool_back->height();
    int w = ui->widget_top->width();
    int h = ui->widget_top->height();

    mAnimation_TopTool->setDuration(600);
    mAnimation_TopTool->setStartValue(QRect(x, y, w, h));
    mAnimation_TopTool->setEndValue(QRect(x, 0, w, h));
//    mAnimation_TopTool->setEasingCurve(QEasingCurve::CosineCurve);/*  设置动画效果  */
    mAnimation_TopTool->start();

    std::thread([=]
    {
        AppConfig::mSleep(300);
        FunctionTransfer::runInMainThread([=]()
        {
            ui->widget_expand_top->hide();
        });
    }).detach();


    mLastMouseOnTopToolTime = QDateTime::currentMSecsSinceEpoch();
    mIsTopToolShowingOut = true;
}

void MainWindow::hideTopTool()
{
    mAnimation_TopTool->stop();

    QDesktopWidget* desktopWidget = QApplication::desktop();
    QRect screenRect = desktopWidget->screenGeometry();//获取设备屏幕大小
    QRect deskRect = desktopWidget->screenGeometry();//获取设备屏幕大小

    int x = screenRect.width() / 2 - ui->widget_top->width() / 2;
    int y = 0 - ui->widget_top_tool_back->height();
    int w = ui->widget_top->width();
    int h = ui->widget_top->height();

    mAnimation_TopTool->setDuration(600);
    mAnimation_TopTool->setStartValue(QRect(x, 0, w, h));
    mAnimation_TopTool->setEndValue(QRect(x, y, w, h));
//    mAnimation_TopTool->setEasingCurve(QEasingCurve::CosineCurve);/*  设置动画效果  */
    mAnimation_TopTool->start();

//    ui->pushButton_expand_top->show();
    mIsTopToolShowingOut = false;

}

void MainWindow::setRecorderState(const RecoderState &state)
{
    if (state == State_Recording)
    {
        ui->label_time_2->clear();

        ui->toolButton_record_start->hide();
        ui->toolButton_record_pause->show();
        ui->toolButton_record_restore->hide();
        ui->toolButton_record_stop->show();
    }
    else if (state == State_Pause)
    {
        ui->toolButton_record_start->hide();
        ui->toolButton_record_pause->hide();
        ui->toolButton_record_restore->show();
        ui->toolButton_record_stop->show();
    }
    else if (state == State_Stop)
    {
        ui->toolButton_record_start->show();
        ui->toolButton_record_pause->hide();
        ui->toolButton_record_restore->hide();
        ui->toolButton_record_stop->hide();

        std::thread([=]
        {
            AppConfig::mSleep(1000);
            FunctionTransfer::runInMainThread([=]()
            {
                ui->label_time_2->clear();
    //            ui->label_time_2->setPixmap(QPixmap(":/image/pull_top.png"));
                ui->label_time_2->setText(QStringLiteral("我的录屏"));
            });
        }).detach();
    }
}

void MainWindow::startRecord()
{
    mCaptureTaskManager->startRecord();
}

void MainWindow::pauseRecord()
{
    mCaptureTaskManager->pauseRecord();
}

void MainWindow::restoreRecord()
{
    mCaptureTaskManager->restoreRecord();
}

void MainWindow::stopRecord()
{
    mCaptureTaskManager->stopRecord();
}

void MainWindow::showMicVolume(const int &value)
{
//    qDebug()<<__FUNCTION__<<value;
    int height = value / 100.0 * ui->label_Micro_green->height();
    height = ui->label_Micro_green->height() - height;
    ui->label_Micro_black->resize(ui->label_Micro_black->width(), height);
};

void MainWindow::OnAudioVolumeUpdated(const int &volumeL, const int &volumeR)
{
//    qDebug()<<__FUNCTION__<<volumeL<<volumeR;
    FunctionTransfer::runInMainThread([=]()
    {
        showMicVolume(volumeL);
    });
}

void MainWindow::slotTimerTimeOut()
{
    if (QObject::sender() == mTimer_GetTime)
    {
        ///设置时间
        {
            int64_t currentTime = 0;

            {
                currentTime = mCaptureTaskManager->getVideoFileCurrentTime();
            }

            currentTime /= 1000;

            int minutes = currentTime / 60;
            int hours = minutes / 60;
            int minute = minutes % 60;
            int second = currentTime % 60;

            char timeCh[16] = {0};
            sprintf(timeCh, "%02d:%02d:%02d",hours, minute, second);

            ui->label_time->setText(QString(timeCh));

            if (currentTime != 0)
            {
                ui->label_time_2->setText(QString(timeCh));
            }

//qDebug()<<__FUNCTION__<<timeCh;
        }

        ///检测是否需要缩回顶部控件
        if (mIsTopToolShowingOut)
        {
            QPoint globalPoint(ui->widget_top_tool_back->mapToGlobal(QPoint(0, 0)));

            QRect widgetRect;
            widgetRect.setTopLeft(globalPoint);
            widgetRect.setWidth(ui->widget_top_tool_back->width());
            widgetRect.setHeight(ui->widget_top_tool_back->height() + ui->widget_time->height());
//qDebug()<<__FUNCTION__<<ui->widget_pentype->hasFocus()<<ui->widget_pentype_back->hasFocus()<<ui->toolButton_pen_type_arrow->hasFocus()<<ui->toolButton_pen_type_rect->hasFocus()<<ui->toolButton_pen_type_ellipse->hasFocus()<<ui->toolButton_pen_type_pen->hasFocus();
            if (widgetRect.contains(QCursor::pos()))
            {
                mLastMouseOnTopToolTime = QDateTime::currentMSecsSinceEpoch();
            }

            if ((QDateTime::currentMSecsSinceEpoch() - mLastMouseOnTopToolTime) >= 3000)
            {
                hideTopTool();
            }
        }

    }
}

void MainWindow::setAlphaValue(int value)
{
    QString rgbaStr = QString("rgba(255, 255, 255, %1)").arg((float)value/100);

    QString styleStr = QString("QWidget#widget_top {background-color: %1;}").arg(rgbaStr);

    ui->widget_top->setStyleSheet(styleStr);
}

void MainWindow::slotIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
    case QSystemTrayIcon::Trigger://单击
    case QSystemTrayIcon::DoubleClick://双击
        this->showOutTopTool();
        break;
    case QSystemTrayIcon::MiddleClick:
        mTrayicon->showMessage(tr("视频录制软件"),tr("欢迎访问：blog.yundiantech.com"),QSystemTrayIcon::Information,10000);
        break;
    default:
        ;
    }
}

void MainWindow::slotBtnClicked(bool isChecked)
{
    if (QObject::sender() == ui->toolButton_camera)
    {
        if (isChecked)
        {
            mShowCameraWidget->showOut();
        }
        else
        {
            mShowCameraWidget->hideIn();
        }
    }
    else if (QObject::sender() == ui->toolButton_view)
    {
        mCaptureTaskManager->show();
        mCaptureTaskManager->raise();
    }
    else if (QObject::sender() == ui->toolButton_micro)
    {
//        qDebug() << "pushButton_Micro:" << isChecked;
        mCaptureTaskManager->muteMicroPhone(isChecked);
    }
    else if (QObject::sender() == ui->toolButton_record_start)
    {
        startRecord();
    }
    else if (QObject::sender() == ui->toolButton_record_restore)
    {
        restoreRecord();
    }
    else if (QObject::sender() == ui->toolButton_record_pause)
    {
        pauseRecord();
    }
    else if (QObject::sender() == ui->toolButton_record_stop)
    {
        stopRecord();
    }
    else if (QObject::sender() == ui->toolButton_exit)
    {
        int ret = MyMessageBox_WithTitle::showWarningText(QStringLiteral("警告"),
                                                           QStringLiteral("确定要要退出程序么？"),
                                                           QStringLiteral("确定"),
                                                           QStringLiteral("取消"));
        if (ret == QDialog::Accepted)
        {
            stopRecord();
            QApplication::quit();
        }
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->pushButton_expand_top)
    {
        if (event->type() == QEvent::Enter)
        {
            if (mAnimation_TopTool->state() != QAbstractAnimation::Running)
            {
                showOutTopTool();
            }
        }
    }

    return QObject::eventFilter(obj, event);
}

///检测同时运行多个exe - begin
void MainWindow::slotTimerTimeOut_checkExe()
{
    QSharedMemory *sharedmem = new QSharedMemory(AppConfig::Memory_KEY_NAME);
    if (sharedmem->create(1024))
    {
        sharedmem->lock();
        char * to = static_cast<char*>(sharedmem->data());
        const char * from = "from dbzhang800-shared.";
        ::memcpy(to, from, 24);
        sharedmem->unlock();
    }
    else if (sharedmem->attach())
    {
//        qDebug("shared memory attached.");
        sharedmem->lock();
        const char * data = static_cast<const char*>(sharedmem->constData());
        QString str = QString(data);
//        qDebug(data);
        sharedmem->unlock();

        if (str == "show out")
        {
            sharedmem->lock();
            char * to = static_cast<char*>(sharedmem->data());

            ::memset(to,0,1024);
            sharedmem->unlock();

            {
                qDebug()<<str<<this->isMinimized();

                {
                    if (this->isMinimized())
                    {
                        this->showNormal();
                        this->raise();
                    }
                    else
                    {
                        this->show();
                        this->raise();
                        this->showOutTopTool();
                    }
                }
            }
        }
    }
    else {
//        QMessageBox::information(NULL,"3333","erro");

//        qDebug("error.");
    }
    delete sharedmem;
}

///检测同时运行多个exe - end
