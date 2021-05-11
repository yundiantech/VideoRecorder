/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "CaptureTaskManager.h"
#include "ui_CaptureTaskManager.h"

#include <QDesktopWidget>
#include <QFileDialog>

#define CAMERA_TASKID 11
#define MAX_PICTURE_RATE_VALUE 90
#define MAX_CAPTURE_RATE_VALUE 90

#include "AppConfig.h"
#include "Media/Video/CaptureWindowThread.h"

#include "MainWindow.h"
#include "Widget/mymessagebox_withTitle.h"

#include "DeviceTest/DeviceSettingDialog.h"

CaptureTaskManager::CaptureTaskManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CaptureTaskManager)
{
qDebug() <<__FUNCTION__<< "000";
    ui->setupUi(this);
//    ui->setupUi(this->getContainWidget());
qDebug() <<__FUNCTION__<< "111";
//    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setWindowFlags(Qt::FramelessWindowHint); //无b边框窗口
//    setWindowFlags(Qt::WindowStaysOnTopHint|Qt::FramelessWindowHint|Qt::Tool|Qt::X11BypassWindowManagerHint); //使窗口置顶
qDebug() <<__FUNCTION__<< "222";

//    mPlayVideoTaskWidget = new PlayVideoTaskWidget;
//    ui->verticalLayout_videoPlayer->addWidget(mPlayVideoTaskWidget);

    mIsLeftBtnPressed = false;

    mTaskId = 0;
    mWidgetToScreenRate = 1.0f;

    mIsShowPicture = false;
    mIsPlayingVideo = false;

    mCurrentCaptrueProgramHandle = NULL;
    mCurrentCaptureProgramRect = QRect(0, 0, 0, 0);
    mCurrentCaptureProgramIsCoveredByOtherWindow = false;

    mTimerUpdateSelectArea = new QTimer(this);
    mTimerUpdateSelectArea->setInterval(100);
    connect(mTimerUpdateSelectArea, &QTimer::timeout, this, &CaptureTaskManager::slotTimerTimeOut);

    mTimerUpdateSlider = new QTimer(this);
    mTimerUpdateSlider->setInterval(100);
    connect(mTimerUpdateSlider, &QTimer::timeout, this, &CaptureTaskManager::slotTimerTimeOut);

    mTimer_GetTime = new QTimer(this);
    connect(mTimer_GetTime, &QTimer::timeout, this, &CaptureTaskManager::slotTimerTimeOut);
    mTimer_GetTime->setInterval(500);
    mTimer_GetTime->start();

    mSelectedCaptureRect = QRect(0,0,0,0);
    mSelectAreaWidget = new SelectAreaWidget();
    connect(mSelectAreaWidget, &SelectAreaWidget::sig_SelectFinished, this, &CaptureTaskManager::slotSelectRectFinished);

    mCaptureWindowWidget = new CaptureWindowWidget(ui->widget_capture_final); //捕获的窗体画面

    mCapturePictureWidget = new CapturePictureWidget(ui->widget_capture_final); //显示图片
    mCapturePictureWidget->hide();

    mCameraVideoSize = QSize(0, 0);
    mShowCameraWidget = static_cast<ShowVideoWidget*>(new ShowVideoWidget(ui->widget_capture_final)); //摄像头画面
    static_cast<ShowVideoWidget*>(mShowCameraWidget)->hide();

//    {
//        QMenu *m_menu = new QMenu(this);
//        QAction *action = new QAction(QStringLiteral("选择固定区域"), m_menu);
//        action->setEnabled(false);
//        m_menu->addAction(action);
//        m_menu->addSeparator();

//        mVideoResolutionList.append(QSize(1920, 1080));
//        mVideoResolutionList.append(QSize(1600, 900));
//        mVideoResolutionList.append(QSize(1280, 720));
//        mVideoResolutionList.append(QSize(960, 540));
//        mVideoResolutionList.append(QSize(640, 360));
//        mVideoResolutionList.append(QSize(480, 270));

//        auto selectAreaFunc = [=](int w, int h)
//        {
//            mCurrentCaptrueProgramHandle = NULL;
//            mTimerUpdateSelectArea->stop();

//            mSelectAreaWidget->setInTop(true);
//            mSelectAreaWidget->setColor(QColor(250, 145, 25));
//            mSelectAreaWidget->setShowMoveWidget(true);
//            mSelectAreaWidget->setSelectFixedSize(QSize(w, h));
//        };

//        for (int i=1; i < mVideoResolutionList.size(); i++)
//        {
//            QSize size = mVideoResolutionList.at(i);
//            QAction *action = new QAction(QStringLiteral("%1x%2").arg(size.width()).arg(size.height()), m_menu);
//            m_menu->addAction(action);

//            connect(action, &QAction::triggered, [=]()
//            {
//                int w = size.width();
//                int h = size.height();
//                selectAreaFunc(w, h);
//            });
//        }
////        ui->pushButton_selectedArea_expand->setPopupMode(QToolButton::InstantPopup);
//        ui->pushButton_selectedArea_expand->setMenu(m_menu);
//    }

    connect(ui->toolButton_record_start,   &QToolButton::clicked, this, &CaptureTaskManager::slotBtnClicked);
    connect(ui->toolButton_record_restore, &QToolButton::clicked, this, &CaptureTaskManager::slotBtnClicked);
    connect(ui->toolButton_record_pause,   &QToolButton::clicked, this, &CaptureTaskManager::slotBtnClicked);
    connect(ui->toolButton_record_stop,    &QToolButton::clicked, this, &CaptureTaskManager::slotBtnClicked);

    connect(ui->toolButton_fullScreen,   &QToolButton::clicked, this, &CaptureTaskManager::slotBtnClicked);
    connect(ui->toolButton_programe,     &QToolButton::clicked, this, &CaptureTaskManager::slotBtnClicked);
    connect(ui->toolButton_selectedArea, &QToolButton::clicked, this, &CaptureTaskManager::slotBtnClicked);
    connect(ui->toolButton_videofile,    &QToolButton::clicked, this, &CaptureTaskManager::slotBtnClicked);

    connect(ui->pushButton_audioTest,    &QToolButton::clicked, this, &CaptureTaskManager::slotBtnClicked);
    connect(ui->pushButton_picture,      &QToolButton::clicked, this, &CaptureTaskManager::slotBtnClicked);

    connect(ui->checkBox_camera,         &QToolButton::clicked, this, &CaptureTaskManager::slotBtnClicked);
    connect(ui->checkBox_picture,        &QToolButton::clicked, this, &CaptureTaskManager::slotBtnClicked);

    connect(ui->pushButton_close,        &QToolButton::clicked, this, &CaptureTaskManager::slotBtnClicked);

    connect(ui->pushButton_expand,     &QToolButton::clicked, this, &CaptureTaskManager::slotBtnClicked);

    connect(ui->comboBox_videoDevice, SIGNAL(activated(int)), this, SLOT(slotCurrentIndexChanged(int)));

//qDebug()<<__FUNCTION__<<"AppConfig::gVideoQuality="<<AppConfig::gVideoQuality;

    //video
    mMediaManager = new MediaManager();

    AppConfig::gMediaManager = mMediaManager;

    auto getCameraVideoFrameFunc = [=](VideoRawFramePtr yuvFramePtr, VideoRawFramePtr rgbFramePtr, void *param)
    {
        if (mCameraFrameCallBackFunc != nullptr)
        {
            mCameraFrameCallBackFunc(yuvFramePtr, rgbFramePtr, mCameraFrameCallBackFuncParam);
        }

        this->inputCameraVideoFrame(yuvFramePtr);
    };

    auto getFinalVideoFrameFunc = [=](VideoRawFramePtr yuvFramePtr, void *param)
    {
        this->inputFinalVideoFrame(yuvFramePtr);
    };

    mMediaManager->setCameraFrameCallBackFunc(getCameraVideoFrameFunc, this);
    mMediaManager->setFinalVideoFrameCallBackFunc(getFinalVideoFrameFunc, this);
    mMediaManager->setQuality(AppConfig::gVideoQuality);

    mPictureRateMaxValue = MAX_PICTURE_RATE_VALUE;

    ui->horizontalSlider_Size_Horizontal->setValue(AppConfig::gCaptureAreaRate);
    ui->horizontalSlider_Size_Picture->setValue(AppConfig::gPictureAreaRate);

    connect(ui->horizontalSlider_Size_Horizontal, &QSlider::valueChanged, this, &CaptureTaskManager::slotSliderValueChanged);
    connect(ui->horizontalSlider_Size_Picture,    &QSlider::valueChanged, this, &CaptureTaskManager::slotSliderValueChanged);

    mLastSelectedBtn = nullptr;

    ui->widget_capture_back->installEventFilter(this);

    mLastSelectedBtn = ui->toolButton_fullScreen;
    mLastSelectedBtn->setChecked(true);

    ui->checkBox_camera->setChecked(false);
    ui->toolButton_view->setChecked(true);
    ui->toolButton_view->hide();

    mShowVideoWidget = nullptr;
    mShowVideoWidget = static_cast<ShowVideoWidget*>(ui->widget_capture_video);

    ui->lineEdit_picture->hide();

qDebug()<<__FUNCTION__<<"555";
    {
        QDesktopWidget* desktopWidget = QApplication::desktop();
        //获取可用桌面大小
        QRect deskRect = desktopWidget->availableGeometry();
        //获取设备屏幕大小
        QRect screenRect = desktopWidget->screenGeometry();

        int width = screenRect.width() * 0.5;
        int height = 500;//ui->widget_left->height(); //screenRect.height() * 0.5;

        width = height * screenRect.width() / screenRect.height();

qDebug()<<__FUNCTION__<<width<<height;

        ui->widget_capture_back->setMinimumSize(width, height);
//        this->resize(width, height);

        setShowViewWidget(false);
    }

qDebug()<<__FUNCTION__<<"666";
    addCapFullScreen(); // 这句一定要放在最后，不然后面的大小计算一开始会有问题

    setVideoPlayMode(false);
qDebug()<<__FUNCTION__<<"777";

    setRecorderState(State_Stop);

    QString titleStr = QStringLiteral("我的录屏软件-V%1").arg(AppConfig::VERSION_NAME);

    this->setWindowTitle(titleStr);
    ui->label_title->setText(titleStr);

}

CaptureTaskManager::~CaptureTaskManager()
{
    mMediaManager->stopAll();
    delete mMediaManager;
    mMediaManager = nullptr;

    delete ui;
}

void CaptureTaskManager::setCameraFrameCallBackFunc(std::function<void (VideoRawFramePtr yuvFrame, VideoRawFramePtr rgbFrame, void *param)> func, void *param)
{
    mCameraFrameCallBackFunc = func;
    mCameraFrameCallBackFuncParam = param;
}

bool CaptureTaskManager::muteMicroPhone(bool isMute)
{
    return mMediaManager->muteMicroPhone(isMute);
}

int64_t CaptureTaskManager::getVideoFileCurrentTime()
{
    return mMediaManager->getVideoFileCurrentTime();
}

void CaptureTaskManager::mousePressEvent(QMouseEvent * event)
{
qDebug()<<__FUNCTION__<<"000";
    if (event->button() == Qt::LeftButton)
    {
        mIsLeftBtnPressed = true;
        dragPosition=event->globalPos()-frameGeometry().topLeft();
        event->accept();
    }
    else if (event->button() == Qt::RightButton)
    {
//         close(); //关闭窗口
    }
}

void CaptureTaskManager::mouseMoveEvent(QMouseEvent * event)
{  //实现鼠标移动窗口
    if (mIsLeftBtnPressed)
    {
        if (event->buttons() & Qt::LeftButton)
        {
             move(event->globalPos() - dragPosition);
             event->accept();
        }
    }

}

void CaptureTaskManager::mouseReleaseEvent(QMouseEvent *event)
{
    qDebug()<<__FUNCTION__<<"000";
    if (event->button() == Qt::LeftButton)
    {
         mIsLeftBtnPressed = false;
         event->accept();
    }
}

void CaptureTaskManager::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_V : //三组合键 CTRL + SHIFT + V
        if (event->modifiers() == (Qt::ShiftModifier|Qt::ControlModifier|Qt::AltModifier))
        {
            ui->toolButton_view->setChecked(!ui->toolButton_view->isChecked());
        }
        break;
    default:
        break;
    }
}

bool CaptureTaskManager::eventFilter(QObject *obj, QEvent *event)
{
//qDebug()<<__FUNCTION__<<"000";
    if (obj == ui->widget_capture_back)
    {
        if (event->type() == QEvent::Resize)
        {
qDebug()<<__FUNCTION__<<"111";
            QResizeEvent *e = (QResizeEvent*)event;

            int containWidgetWidth  = e->size().width();
            int containWidgetHeight = e->size().height();

            //屏幕大小
            QRect screenRect = QApplication::desktop()->screenGeometry();

            int widgetWidth  = screenRect.width();
            int widgetheight = screenRect.height();

            if (widgetWidth > containWidgetWidth)
            {
                int w = containWidgetWidth;
                int h = w * widgetheight / widgetWidth;

                widgetWidth  = w;
                widgetheight = h;
            }

            if (widgetheight > containWidgetHeight)
            {
                int h = containWidgetHeight;
                int w = h * widgetWidth / widgetheight;

                widgetWidth  = w;
                widgetheight = h;
            }

            int x = (containWidgetWidth - widgetWidth) / 2;
            int y = (containWidgetHeight - widgetheight) / 2;

            ui->widget_capture_contain->move(x, y);
            ui->widget_capture_contain->resize(widgetWidth, widgetheight);
qDebug()<<__FUNCTION__<<"ui->widget_capture_contain size:"<<widgetWidth<<widgetheight;
            mWidgetToScreenRate = widgetWidth * 1.0 / screenRect.width();
qDebug()<<__FUNCTION__<<"999";
            reSetCaptureWidgetWidth();
        }
    }

    return QObject::eventFilter(obj, event);
}

void CaptureTaskManager::hideAll()
{
    this->hide();
    mSelectAreaWidget->hide();
}

void CaptureTaskManager::setRecorderState(const RecoderState &state)
{
    if (state == State_Recording)
    {
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
    }

    emit sig_RecorderStateChanged(state);
}

void CaptureTaskManager::startRecord()
{
    ui->label_time_record->clear();

//    int videoFileWidth  = AppConfig::gVideoFileSize.width();
//    int videoFileHeight = AppConfig::gVideoFileSize.height();

//    if (videoFileWidth <= 0 || videoFileHeight <= 0)
//    {
//        DeviceSettingDialog dialog; //主要是为了获取视频文件分辨率
//        QSize videoFileSize = dialog.getVideoFileSize();

//        videoFileWidth  = videoFileSize.width();
//        videoFileHeight = videoFileSize.height();
//    }

//    mMediaManager->setVideoFileSize(videoFileWidth, videoFileHeight);
//    mMediaManager->setQuantity(AppConfig::gVideoQuality);

    QString filePath = QString("%1/%2.mp4").arg(AppConfig::gVideoDirPath).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hhmmss"));
    mMediaManager->openFile(filePath.toStdString());

    mCurrentVideoFilePath = filePath;

    this->setRecorderState(State_Recording);

    std::thread([=]
    {
        bool isSucceed = mMediaManager->startCapture(AppConfig::gEnableVirtualAudioCapture);

        if (!isSucceed)
        {
            FunctionTransfer::runInMainThread([=]()
            {
                int ret = MyMessageBox_WithTitle::showWarningText(QStringLiteral("警告"),
                                                                  QStringLiteral("打开声卡录制失败，以管理员身份运行(bin/插件)目录下的reg.bat即可解决。"),
                                                                  NULL,
                                                                  QStringLiteral("确定"));

                stopRecord();
            });
        }

    }).detach();

    resetMic();
}

void CaptureTaskManager::pauseRecord()
{
    mMediaManager->stopCapture(false);
    mMediaManager->stopCaptureMic(false);

    this->setRecorderState(State_Pause);
}

void CaptureTaskManager::restoreRecord()
{
    mMediaManager->startCapture(AppConfig::gEnableVirtualAudioCapture);
    resetMic();

    this->setRecorderState(State_Recording);
}

void CaptureTaskManager::stopRecord()
{
//    std::thread([=]
//    {
//        AppConfig::mSleep(1000);
//        FunctionTransfer::runInMainThread([=]()
//        {
//            ui->label_time_2->clear();
////            ui->label_time_2->setPixmap(QPixmap(":/image/pull_top.png"));
//            ui->label_time_2->setText(QStringLiteral("我的录屏"));
//        });
//    }).detach();

    mMediaManager->stopCaptureMic();
    mMediaManager->stopCapture();
    mMediaManager->closeFile();

    this->setRecorderState(State_Stop);

//    std::list<VideoFileInfo> videoFileList = mMediaManager->getVideoFileList(); //获取文件列表

//    if (!videoFileList.empty())
//    {
//        MergeVideoDialog dialog;
//        bool isNeedWait = dialog.mergeVideo(videoFileList, mCurrentVideoFilePath, true, false);
//        if (isNeedWait)
//        {
//            if (dialog.exec() == QDialog::Accepted)
//            {

//            }
//        }
//    }
}

void CaptureTaskManager::addCapPicture(const QString &filePath)
{
    QRect capRect;

    CapturePictureWidget *widget = mCapturePictureWidget;
    widget->setTask(++mTaskId, capRect);
    widget->setFilePath(filePath);
//    widget->show();

    mIsShowPicture = true;
    mPictureImage  = QImage(filePath);

    reSetCaptureWidgetWidth();
}

void CaptureTaskManager::addCapWindow(HWND hwnd, QRect capRect)
{
qDebug()<<__FUNCTION__<<hwnd<<capRect;
    CaptureWindowWidget *widget = mCaptureWindowWidget;

    int taskID = widget->getId();

    if (taskID < 0)
    {
        taskID = ++mTaskId;
    }

    widget->move(0, 0);
    widget->show();
    widget->resize(capRect.width(), capRect.height());
    widget->setTask(taskID, capRect);
    widget->setHWND(hwnd);
    widget->startCap();

    reSetCaptureWidgetWidth();
}

void CaptureTaskManager::addCapFullScreen()
{
    QDesktopWidget* desktopWidget = QApplication::desktop();
    //获取可用桌面大小
    QRect deskRect = desktopWidget->availableGeometry();
    //获取设备屏幕大小
    QRect screenRect = desktopWidget->screenGeometry();

    addCapWindow(NULL, screenRect);

    mSelectAreaWidget->showFullScreen();
    mSelectAreaWidget->setRect(screenRect);
    mSelectAreaWidget->setEditMode(false);
}

void CaptureTaskManager::doDeviceTest()
{
    DeviceSettingDialog dialog;
    connect(&dialog, &DeviceSettingDialog::sig_ChangeMic, [=](int state)
    {
        if (state == 0) //关闭麦克风
        {
            mMediaManager->stopCaptureMic();
        }
        else if (state == 1) //打开麦克风
        {
            resetMic();
        }
    });

    if (dialog.exec() == QDialog::Accepted)
    {

    }

    {
        mMediaManager->muteVirtualAudio(!AppConfig::gEnableVirtualAudioCapture);
    }

    resetMic();
}

void CaptureTaskManager::resetMic()
{
    QStringList deviceNameList = DeviceSettingDialog::mCurrentDevicesName;
qDebug()<<__FUNCTION__<<"000";
    if (deviceNameList.isEmpty())
    {
        DeviceSettingDialog dialog; //主要是为了获取默认的设备名称
        deviceNameList = dialog.getCurrentDevicesName();
    }
qDebug()<<__FUNCTION__<<"111";
    QString audioInDeviceName = deviceNameList.at(0);
    QString audioOutDeviceName = deviceNameList.at(1);
    QString cameraDeviceName = deviceNameList.at(2);
qDebug()<<__FUNCTION__<<"222";
    mMediaManager->setMicroPhone(audioInDeviceName.toStdString());
}

void CaptureTaskManager::initDevice()
{
    ///获取设备信息

    std::list<DeviceNode> videoDeviceList;
    std::list<DeviceNode> audioDeviceList;

    MediaManager::getDeviceList(videoDeviceList, audioDeviceList);

    int index = 0;
    int i=0;

    ui->comboBox_videoDevice->clear();
    for (const DeviceNode &node : videoDeviceList)
    {
        QString deviceName = QString::fromStdString(node.deviceName);
        if (AppConfig::gLocalCameraDeviceName == deviceName)
        {
            index=i;
        }
        ui->comboBox_videoDevice->addItem(deviceName);

        i++;
    }

    ui->comboBox_videoDevice->setCurrentIndex(index);

//    foreach (const QCameraInfo &cameraInfo, QCameraInfo::availableCameras())
//    {qDebug()<<cameraInfo.description();
//        ui->comboBox_videoDevice->addItem(cameraInfo.description(), qVariantFromValue(cameraInfo));
//    }

}

void CaptureTaskManager::openCamera(const QString &cameraName)
{
    AppConfig::gLocalCameraDeviceName = cameraName;
    AppConfig::gMediaManager->openCameraCaptureMode(cameraName.toStdString());
}

void CaptureTaskManager::doOpenCamera()
{
    QString cameraDeviceName;

    for (int i=0;i<ui->comboBox_videoDevice->count();i++)
    {
        QString deviceName = ui->comboBox_videoDevice->itemText(i);

        if (deviceName == AppConfig::gLocalCameraDeviceName)
        {
            cameraDeviceName = deviceName;
        }
    }

    if (cameraDeviceName.isEmpty())
    {
        if (ui->comboBox_videoDevice->count() > 0)
        {
            cameraDeviceName = ui->comboBox_videoDevice->itemText(0);
        }
    }

    if (!cameraDeviceName.isEmpty())
    {
        openCamera(cameraDeviceName);
    }
}

#if 0
void CaptureTaskManager::reSetCaptureWidgetWidth()
{
qDebug()<<__FUNCTION__<<"000"<<mIsPlayingVideo;

    if (mIsPlayingVideo)
    {
        return;
    }

    bool isShowCamera = ui->checkBox_camera->isChecked();

    int leftWidgetWidth  = 0; //左边的采集窗口区域宽度
    int leftWidgetHeight = 0;

    int rightWidgetWidth  = 0; //右边的摄像头区域宽度
    int rightWidgetHeight = 0;

    ///先调整左边的窗口捕获区域，调整到显示区域内
    {
        CaptureWindowWidget *widget = mCaptureWindowWidget;

        QRect capRect = widget->getCaptureRect();

        int containWidgetWidth  = ui->widget_capture_contain->width();
        int containWidgetHeight = ui->widget_capture_contain->height();

        if (isShowCamera)// || mIsShowPicture)
        {
            float rate = ui->horizontalSlider_Size->value() * 1.0 / 100;

            containWidgetWidth =    rate * ui->widget_capture_contain->width();
            rightWidgetWidth  = (1-rate) * ui->widget_capture_contain->width();
        }

        int widgetWidth  = capRect.width();
        int widgetheight = capRect.height();

        if (widgetWidth > containWidgetWidth)
        {
            int w = containWidgetWidth;
            int h = w * widgetheight / widgetWidth;

            widgetWidth  = w;
            widgetheight = h;
        }

        if (widgetheight > containWidgetHeight)
        {
            int h = containWidgetHeight;
            int w = h * widgetWidth / widgetheight;

            widgetWidth  = w;
            widgetheight = h;
        }

        widget->move(0, 0);
        widget->resize(widgetWidth, widgetheight);

        leftWidgetWidth = widgetWidth;

//        finalVideoWidth  = widgetWidth + rightWidgetWidth;
//        finalVideoHeight = widgetheight;

        leftWidgetHeight = widgetheight;
    }

    ///最终输出的视频分辨率
    int finalVideoWidth  = leftWidgetWidth + rightWidgetWidth;
    int finalVideoHeight = max(leftWidgetHeight, rightWidgetHeight);

    if (isShowCamera)// || mIsShowPicture)
    {
        rightWidgetHeight = leftWidgetHeight;

        int pictureWidgetBackHeight = rightWidgetHeight * 0.6;

        if (mIsShowPicture)
        {
            int pictureWidth  = rightWidgetWidth;
            int pictureHeight = 0;

            pictureWidth  = rightWidgetWidth;
            pictureHeight = pictureWidth * mPictureImage.height() / mPictureImage.width();

            if (pictureHeight > pictureWidgetBackHeight)
            {
                pictureHeight = pictureWidgetBackHeight;
                pictureWidth  = pictureHeight * mPictureImage.width() / mPictureImage.height();
            }

            int x = (rightWidgetWidth - pictureWidth) / 2;
            int y = (pictureWidgetBackHeight - pictureHeight) / 2;
            int w = rightWidgetWidth;
            int h = pictureHeight;

            x += leftWidgetWidth;
            y += pictureWidgetBackHeight;

            mCapturePictureWidget->move(x, y);
            mCapturePictureWidget->resize(w, h);
            mCapturePictureWidget->show();
        }

        if (isShowCamera && mCameraVideoSize.width() > 0)
        {

            int cameraWidgetHeightBack = rightWidgetHeight - pictureWidgetBackHeight;

            int cameraWidgetWidth  = rightWidgetWidth;
            int cameraWidgetHeight = cameraWidgetWidth * mCameraVideoSize.height() / mCameraVideoSize.width();

            if (cameraWidgetHeight > cameraWidgetHeightBack)
            {
                cameraWidgetHeight = cameraWidgetHeightBack;
                cameraWidgetWidth  = cameraWidgetHeight * mCameraVideoSize.width() / mCameraVideoSize.height();
            }

            int x = (rightWidgetWidth - cameraWidgetWidth) / 2;
            int y = (cameraWidgetHeightBack - cameraWidgetHeight) / 2;

            x += leftWidgetWidth;

            mShowCameraWidget->move(x, y);
            mShowCameraWidget->resize(cameraWidgetWidth, cameraWidgetHeight);

            mShowCameraWidget->show();
        }

    }
    else
    {
        mShowCameraWidget->hide();
        mCapturePictureWidget->hide();
    }

qDebug()<<__FUNCTION__<<finalVideoWidth<<finalVideoHeight<<isShowCamera<<rightWidgetWidth<<rightWidgetHeight;
    ///设置最终显示位置
    {
        if (finalVideoWidth % 2 != 0) finalVideoWidth++;
        if (finalVideoHeight % 2 != 0) finalVideoHeight++;

        int x = (ui->widget_capture_contain->width() - finalVideoWidth) / 2 ;
        int y = (ui->widget_capture_contain->height() - finalVideoHeight) / 2 ;

        ui->widget_capture_final->resize(finalVideoWidth, finalVideoHeight);
        ui->widget_capture_final->move(x, y);

        ui->widget_capture_view->resize(finalVideoWidth, finalVideoHeight);
        ui->widget_capture_view->move(x, y);
    }

    ///输出最终结果
    {

        //实际输出的视频分辨率
        int videoFileWidth  = finalVideoWidth  / mWidgetToScreenRate;
        int videoFileHeight = finalVideoHeight / mWidgetToScreenRate;

        AppConfig::gMediaManager->setVideoSize(videoFileWidth, videoFileHeight);

        ///添加采集屏幕区域
        {
            int captureWidgetWidth  = leftWidgetWidth / mWidgetToScreenRate;
            int captureWidgetHeight = leftWidgetHeight / mWidgetToScreenRate;

            int left = mCaptureWindowWidget->x() / mWidgetToScreenRate;
            int top = mCaptureWindowWidget->y() / mWidgetToScreenRate;
            int right = (mCaptureWindowWidget->geometry().right()+1) / mWidgetToScreenRate;
            int bottom = (mCaptureWindowWidget->geometry().bottom()+1) / mWidgetToScreenRate;

            if (left % 2 != 0) left++;
            if (top % 2 != 0) top++;
            if (right % 2 != 0) right++;
            if (bottom % 2 != 0) bottom++;

            QRect capRect = mCaptureWindowWidget->getCaptureRect();
    qDebug()<<__FUNCTION__<<capRect<<videoFileWidth<<videoFileHeight<<mCaptureWindowWidget->geometry()<<left<<top<<right<<bottom;
            AppConfig::gMediaManager->addCaptureWindowTask(mCaptureWindowWidget->getId(),
                                                           mCaptureWindowWidget->getHWND(),
                                                           RECT{capRect.x(), capRect.y(), capRect.x()+capRect.width(), capRect.y()+capRect.height()},
                                                           RECT{left, top, right, bottom});

        }

        ///添加摄像头区域
        if (isShowCamera)
        {
            int cameraVideoWidth  = rightWidgetWidth / mWidgetToScreenRate;
            int cameraVideoHeight = rightWidgetHeight / mWidgetToScreenRate;

            int left = mShowCameraWidget->x() / mWidgetToScreenRate;
            int top = mShowCameraWidget->y() / mWidgetToScreenRate;
            int right = (mShowCameraWidget->geometry().right()+1) / mWidgetToScreenRate;
            int bottom = (mShowCameraWidget->geometry().bottom()+1) / mWidgetToScreenRate;

            if (left % 2 != 0) left++;
            if (top % 2 != 0) top++;
            if (right % 2 != 0) right++;
            if (bottom % 2 != 0) bottom++;

            AppConfig::gMediaManager->addCaptureCameraTask(CAMERA_TASKID, RECT{left, top, right, bottom}, 1.0);
        }

        ///添加图片区域
        if (isShowCamera && mIsShowPicture)
        {
            int left = mCapturePictureWidget->x() / mWidgetToScreenRate;
            int top = mCapturePictureWidget->y() / mWidgetToScreenRate;
            int right = (mCapturePictureWidget->geometry().right()+1) / mWidgetToScreenRate;
            int bottom = (mCapturePictureWidget->geometry().bottom()+1) / mWidgetToScreenRate;

            if (left % 2 != 0) left++;
            if (top % 2 != 0) top++;
            if (right % 2 != 0) right++;
            if (bottom % 2 != 0) bottom++;

            AppConfig::gMediaManager->addCapturePictureTask(21,
                                                            mCapturePictureWidget->getFilePath().toStdString(),
                                                            RECT{left, top, right, bottom}, 1.0);
        }

    }
}
#else
void CaptureTaskManager::reSetCaptureWidgetWidth()
{
qDebug()<<__FUNCTION__<<"000"<<mIsPlayingVideo;

    if (mIsPlayingVideo)
    {
        return;
    }

    bool isShowCamera = ui->checkBox_camera->isChecked();
    float leftWidgetRate = 1.0;

    if (isShowCamera)// || mIsShowPicture)
    {
        leftWidgetRate = ui->horizontalSlider_Size_Horizontal->value() * 1.0 / 100;
    }

    int leftAreaWidth_Real = 0; //左边区域的宽度（实际的像素大小）
    int leftAreaHeight_Real = 0;

    int rightAreaWidth_Real = 0; //右边区域的宽度（实际的像素大小）
    int rightAreaHeight_Real = 0;

    int totalAreaWidth_Real = 0; //整个区域的宽度（实际的像素大小）
    int totalAreaHeight_Real = 0;
    float totalAreaScaleRate = 1.0f; //整体缩放比例

    float cameraHeightRate = 1.0; //摄像头占右边区域的高度
    int cameraAreaHeightBack_Real = 0; //期望的摄像头区域高度
    int cameraAreaX_Real = 0; //摄像头区域位置 (实际像素)
    int cameraAreaY_Real = 0;
    int cameraAreaWidth_Real = 0; //摄像头区域宽度 (实际像素)
    int cameraAreaHeight_Real = 0;

    int pictureAreaHeightBack_Real = 0; //期望的摄像头区域高度
    int pictureAreaX_Real = 0; //图片区域位置 (实际像素)
    int pictureAreaY_Real = 0;
    int pictureAreaWidth_Real = 0; //图片区域宽度 (实际像素)
    int pictureAreaHeight_Real = 0;

    ///先计算总视频实际像素大小
    {
        QRect capRect = mCaptureWindowWidget->getCaptureRect();

        leftAreaWidth_Real = capRect.width();
        leftAreaHeight_Real = capRect.height();

        if (isShowCamera)
        {
            totalAreaWidth_Real = leftAreaWidth_Real / leftWidgetRate;
            totalAreaHeight_Real = leftAreaHeight_Real;

            rightAreaWidth_Real = totalAreaWidth_Real - leftAreaWidth_Real;
            rightAreaHeight_Real = totalAreaHeight_Real;

        }
        else
        {
            totalAreaWidth_Real = leftAreaWidth_Real;
            totalAreaHeight_Real = leftAreaHeight_Real;
        }

        ///计算显示图像区域的实际大小
        if (isShowCamera && mIsShowPicture)
        {
//            cameraHeightRate = 0.3; //摄像头占右边区域的高度
            cameraHeightRate = (1 - (ui->horizontalSlider_Size_Picture->value() * 1.0 / 100));

            pictureAreaHeightBack_Real = totalAreaHeight_Real*(1-cameraHeightRate); //期望的摄像头区域高度

            pictureAreaWidth_Real = rightAreaWidth_Real; //图片区域宽度 (实际像素)
            pictureAreaHeight_Real = pictureAreaWidth_Real * mPictureImage.height() / mPictureImage.width();

            ///如果图片实际大小比期望的高度大，则将图片压缩到期望高度
            if (pictureAreaHeight_Real > pictureAreaHeightBack_Real)
            {
                pictureAreaHeight_Real = pictureAreaHeightBack_Real;
                pictureAreaWidth_Real = pictureAreaHeight_Real * mPictureImage.width() / mPictureImage.height();
            }

            ///如果图片实际大小比期望的高度小，则将摄像头整体下移
            if (pictureAreaHeight_Real < pictureAreaHeightBack_Real)
            {
                cameraAreaHeightBack_Real = totalAreaHeight_Real - pictureAreaHeight_Real;
                cameraHeightRate = cameraAreaHeightBack_Real * 1.0 / rightAreaHeight_Real; //重新生成摄像头占右边的高度比例
            }

            pictureAreaX_Real = leftAreaWidth_Real;
            pictureAreaY_Real = rightAreaHeight_Real * cameraHeightRate;


            ///计算广告图片高度占比的最大值，既：达到此值图片宽度就铺满右边区域了
            {
                int width = rightAreaWidth_Real;
                int height = width * mPictureImage.height() / mPictureImage.width();

                if (height > rightAreaHeight_Real)
                {
                    height = rightAreaHeight_Real;
                }

                int rate = height * 100 / rightAreaHeight_Real;

                if (rate > MAX_PICTURE_RATE_VALUE)
                {
                    rate = MAX_PICTURE_RATE_VALUE;
                }

                if (rate < mPictureRateMaxValue)
                {
                    mPictureRateMaxValue = rate;
                    ui->horizontalSlider_Size_Picture->setValue(rate);
                }
                else
                {
                    mPictureRateMaxValue = rate;
                }
            }
        }

        ///计算摄像头实际区域
        if (isShowCamera)
        {
            cameraAreaHeightBack_Real = totalAreaHeight_Real * cameraHeightRate; //期望的摄像头区域高度

            cameraAreaHeight_Real = rightAreaHeight_Real * cameraHeightRate; //摄像头区域宽度 (实际像素)

            if (mCameraVideoSize.width() > 0)
            {
                cameraAreaWidth_Real = rightAreaWidth_Real; //图片区域宽度 (实际像素)
                cameraAreaHeight_Real = cameraAreaWidth_Real * mCameraVideoSize.height() / mCameraVideoSize.width();

                if (cameraAreaHeight_Real > cameraAreaHeightBack_Real)
                {
                    cameraAreaHeight_Real = cameraAreaHeightBack_Real;
                    cameraAreaWidth_Real = cameraAreaHeight_Real * mCameraVideoSize.width() / mCameraVideoSize.height(); //重新生成摄像头占右边的高度比例
                }
            }
            else
            {
                cameraAreaWidth_Real = rightAreaWidth_Real; //图片区域宽度 (实际像素)
                cameraAreaHeight_Real = cameraAreaHeightBack_Real;
            }

            int x = (rightAreaWidth_Real - cameraAreaWidth_Real) / 2;
            int y = (cameraAreaHeightBack_Real - cameraAreaHeight_Real) / 2;

            cameraAreaX_Real = leftAreaWidth_Real + x;
            cameraAreaY_Real = y;
        }
    }

    ///输出最终结果
    {

        //实际输出的视频分辨率
        int videoFileWidth = totalAreaWidth_Real;
        int videoFileHeight = totalAreaHeight_Real;

        QRect screenRect = QApplication::desktop()->screenGeometry();
        if (videoFileWidth > screenRect.width())
        {
            videoFileWidth = screenRect.width();
            videoFileHeight = videoFileWidth * totalAreaHeight_Real / totalAreaWidth_Real;
        }

        totalAreaScaleRate = videoFileWidth * 1.0 / totalAreaWidth_Real;

        ///最终的视频的宽度必须是4的倍数，否则opengl显示的时候会有问题（暂时解决不了），因此这里直接给他限制掉，防止后面自己的播放器播放出问题
        if ((videoFileWidth % 4) != 0)
        {
            videoFileWidth += 4 - (videoFileWidth % 4);
        }

        if ((videoFileHeight % 2) != 0)
        {
            videoFileHeight += 1;
        }

        AppConfig::gMediaManager->setVideoSize(videoFileWidth, videoFileHeight);

        ///添加采集屏幕区域
        {
            QRect capRect = mCaptureWindowWidget->getCaptureRect();

            int left = 0;
            int top = 0;
            int right = leftAreaWidth_Real * totalAreaScaleRate;
            int bottom = leftAreaHeight_Real * totalAreaScaleRate;

            if (left % 2 != 0) left++;
            if (top % 2 != 0) top++;
            if (right % 2 != 0) right++;
            if (bottom % 2 != 0) bottom++;

            AppConfig::gMediaManager->addCaptureWindowTask(mCaptureWindowWidget->getId(),
                                                           mCaptureWindowWidget->getHWND(),
                                                           RECT{capRect.x(), capRect.y(), capRect.x()+capRect.width(), capRect.y()+capRect.height()},
                                                           RECT{left, top, right, bottom});
        }

        ///添加摄像头区域
        if (isShowCamera)
        {
            int left = cameraAreaX_Real * totalAreaScaleRate;
            int top = cameraAreaY_Real  * totalAreaScaleRate;
            int right  = left + (cameraAreaWidth_Real  * totalAreaScaleRate);
            int bottom = top  + (cameraAreaHeight_Real * totalAreaScaleRate);

            if (left % 2 != 0) left++;
            if (top % 2 != 0) top++;
            if (right % 2 != 0) right++;
            if (bottom % 2 != 0) bottom++;

            AppConfig::gMediaManager->addCaptureCameraTask(CAMERA_TASKID, RECT{left, top, right, bottom}, 1.0);
        }

        ///添加图片区域
        if (isShowCamera && mIsShowPicture)
        {
            int left = pictureAreaX_Real * totalAreaScaleRate;
            int top = pictureAreaY_Real  * totalAreaScaleRate;
            int right = left + (pictureAreaWidth_Real  * totalAreaScaleRate);
            int bottom = top + (pictureAreaHeight_Real * totalAreaScaleRate);

            if (left % 2 != 0) left++;
            if (top % 2 != 0) top++;
            if (right % 2 != 0) right++;
            if (bottom % 2 != 0) bottom++;

            AppConfig::gMediaManager->addCapturePictureTask(mCapturePictureWidget->getId(),
                                                            mCapturePictureWidget->getFilePath().toStdString(),
                                                            RECT{left, top, right, bottom}, 1.0);
        }
    }

    ///计算出界面上显示的大小
    {
        int leftWidgetWidth  = 0; //左边的采集窗口区域宽度
        int leftWidgetHeight = 0;

        int rightWidgetWidth  = 0; //右边的区域宽度
        int rightWidgetHeight = 0;

        int cameraWidgetWidth = 0; //右边的摄像头区域宽度
        int cameraWidgetHeight = 0;

        int pictureWidgetWidth = 0; //右边的图片区域宽度
        int pictureWidgetHeight = 0;

        int totalWidgetWidth = 0;
        int totalWidgetHeight = 0;

        int width = ui->widget_capture_contain->width();
        int height = ui->widget_capture_contain->height();

        totalWidgetWidth = width;
        totalWidgetHeight = totalWidgetWidth * totalAreaHeight_Real / totalAreaWidth_Real;

        if (totalWidgetHeight > height)
        {
            totalWidgetHeight = height;
            totalWidgetWidth = totalWidgetHeight * totalAreaWidth_Real / totalAreaHeight_Real;
        }

        ///如果区域的实际像素比界面小，则直接按实际像素大小显示
        if (totalAreaWidth_Real < totalWidgetWidth)
        {
            totalWidgetWidth = totalAreaWidth_Real;
            totalWidgetHeight = totalAreaHeight_Real;
        }

        ///设置最终显示位置
        {
            int x = (width - totalWidgetWidth) / 2;
            int y = (height - totalWidgetHeight) / 2;

            ui->widget_capture_final->resize(totalWidgetWidth, totalWidgetHeight);
            ui->widget_capture_final->move(x, y);

            ui->widget_capture_view->resize(totalWidgetWidth, totalWidgetHeight);
            ui->widget_capture_view->move(x, y);
        }

        ///调整左边的窗口捕获区域，调整到显示区域内
        {
            leftWidgetWidth  = totalWidgetWidth * leftWidgetRate;
            leftWidgetHeight = totalWidgetHeight;

            mCaptureWindowWidget->move(0, 0);
            mCaptureWindowWidget->resize(leftWidgetWidth, leftWidgetHeight);
        }

        ///调整摄像头区域
        if (isShowCamera)
        {
            rightWidgetWidth = totalWidgetWidth - leftWidgetWidth;
            rightWidgetHeight = totalWidgetHeight;

            int cameraWidgetHeightBack = rightWidgetHeight * cameraHeightRate; //期望显示摄像头的高度

            cameraWidgetWidth = rightWidgetWidth;
            cameraWidgetHeight = cameraWidgetWidth * cameraAreaHeight_Real / cameraAreaWidth_Real;

            if (cameraWidgetHeight > cameraWidgetHeightBack)
            {
                cameraWidgetHeight = cameraWidgetHeightBack;
                cameraWidgetWidth = cameraWidgetHeight * cameraAreaWidth_Real / cameraAreaHeight_Real;
            }

            int x = leftWidgetWidth + (rightWidgetWidth - cameraWidgetWidth) / 2;
            int y = (cameraWidgetHeightBack - cameraWidgetHeight) / 2;

            static_cast<ShowVideoWidget*>(mShowCameraWidget)->move(x, y);
            static_cast<ShowVideoWidget*>(mShowCameraWidget)->resize(cameraWidgetWidth, cameraWidgetHeight);
            static_cast<ShowVideoWidget*>(mShowCameraWidget)->show();

qDebug()<<__FUNCTION__<<"camera:"<<cameraHeightRate<<x<<y<<pictureAreaWidth_Real<<pictureAreaHeight_Real<<pictureWidgetWidth<<pictureWidgetHeight<<mPictureImage;

        }

        ///调整右边的图像显示区域
        if (isShowCamera && mIsShowPicture)
        {
            int x = leftWidgetWidth;
            int y = cameraHeightRate * rightWidgetHeight;

            pictureWidgetWidth = rightWidgetWidth;
            pictureWidgetHeight = pictureWidgetWidth * pictureAreaHeight_Real / pictureAreaWidth_Real;

            int pictureWidgetHeightBack = totalWidgetHeight - (cameraHeightRate * rightWidgetHeight);

            if (pictureWidgetHeight > pictureWidgetHeightBack)
            {
                pictureWidgetHeight = pictureWidgetHeightBack;
                pictureWidgetWidth = pictureWidgetHeight * pictureAreaWidth_Real / pictureAreaHeight_Real;
            }

            mCapturePictureWidget->move(x, y);
            mCapturePictureWidget->resize(pictureWidgetWidth, pictureWidgetHeight);
            mCapturePictureWidget->show();

            qDebug()<<__FUNCTION__<<"picture:"<<x<<y<<pictureAreaWidth_Real<<pictureAreaHeight_Real<<pictureWidgetWidth<<pictureWidgetHeight<<mPictureImage;
        }
        else
        {
            mCapturePictureWidget->hide();
        }
    }

qDebug()<<__FUNCTION__<<"total area   size:"<<totalAreaWidth_Real<<totalAreaHeight_Real<<leftAreaWidth_Real<<leftAreaHeight_Real<<rightAreaWidth_Real<<rightAreaHeight_Real;
//qDebug()<<__FUNCTION__<<"total widget size:"<<totalWidgetWidth<<totalWidgetHeight<<leftWidgetWidth<<leftWidgetHeight<<rightWidgetWidth<<rightWidgetHeight;

}
#endif

void CaptureTaskManager::setVideoPlayMode(const bool &isVideoPlayMode)
{
    mIsPlayingVideo = isVideoPlayMode;

    if (isVideoPlayMode)
    {
        ui->stackedWidget->setCurrentIndex(1);
        AppConfig::gMediaManager->stopCapture();
    }
    else
    {
//        mPlayVideoTaskWidget->stopPlay();
//        mPlayVideoTaskWidget->clear();
        ui->stackedWidget->setCurrentIndex(0);
qDebug()<<__FUNCTION__<<"111";
        AppConfig::gMediaManager->startCapture(AppConfig::gEnableVirtualAudioCapture);
qDebug()<<__FUNCTION__<<"222";
    }
}

void CaptureTaskManager::setShowViewWidget(const bool &isShow, const bool &isWaitResize)
{
    int width = this->width();
    int height = 0;

    int left = 0, top = 0, right = 0, bottom = 0;
    this->getContentsMargins(&left, &top, &right, &bottom);

    if (isShow)
    {
        ui->widget_expand->show();
        ui->widget_capture_back->show();
        ui->pushButton_expand->setChecked(true);

        height = ui->widget_top->height() +
                ui->widget_controlBtn->height() +
                ui->widget_capture_back->height() +
                ui->widget_expand->height() + top + bottom;
    }
    else
    {
        ui->widget_expand->hide();
        ui->widget_capture_back->hide();
        ui->pushButton_expand->setChecked(false);

        height = ui->widget_top->height() + ui->widget_controlBtn->height() + top + bottom;
    }

    QDesktopWidget* desktopWidget = QApplication::desktop();
    //获取可用桌面大小
    QRect deskRect = desktopWidget->availableGeometry();
    //获取设备屏幕大小
    QRect screenRect = desktopWidget->screenGeometry();

    int x = this->x();
    int y = this->y();

    if (y <= 0)
    {
        y = (screenRect.height() - height) / 2;
    }

    if ((y + height) > deskRect.height())
    {
        y = deskRect.height() - height - 30;
    }
qDebug()<<__FUNCTION__<<y<<deskRect<<width<<height<<isShow;

    if (isWaitResize)
    {
        std::thread([=]
        {
            AppConfig::mSleep(50);

            FunctionTransfer::runInMainThread([=]()
            {
                this->resize(width, height);

                if (isShow)
                {
                    this->move(x, y);
                }
            });

        }).detach();
    }
    else
    {
        this->resize(width, height);

        if (isShow)
        {
            this->move(x, y);
        }
    }
}

void CaptureTaskManager::changeToFullScreenMode()
{
    setVideoPlayMode(false);

    mCurrentCaptrueProgramHandle = NULL;
    mTimerUpdateSelectArea->stop();

    addCapFullScreen();

    if (mLastSelectedBtn != nullptr)
    {
        mLastSelectedBtn->setChecked(false);
    }
    mLastSelectedBtn = ui->toolButton_fullScreen;
    mLastSelectedBtn->setChecked(true);

    mSelectAreaWidget->setInTop(true);
    mSelectAreaWidget->setColor(QColor(255, 0, 0));
//        mSelectRectWidget->hide();
}

void CaptureTaskManager::slotCurrentIndexChanged(int index)
{
    qDebug()<<__FUNCTION__<<index;
    QString deviceName = ui->comboBox_videoDevice->currentText();
    openCamera(deviceName);
}

void CaptureTaskManager::slotBtnClicked(bool isChecked)
{
    if (QObject::sender() == ui->toolButton_record_start)
    {
        this->startRecord();
    }
    else if (QObject::sender() == ui->toolButton_record_restore)
    {
        this->restoreRecord();
    }
    else if (QObject::sender() == ui->toolButton_record_pause)
    {
        this->pauseRecord();
    }
    else if (QObject::sender() == ui->toolButton_record_stop)
    {
        this->stopRecord();
    }
    else if (QObject::sender() == ui->pushButton_close)
    {
        this->close();
    }
    else if (QObject::sender() == ui->toolButton_fullScreen)
    {
        changeToFullScreenMode();
    }
    else if (QObject::sender() == ui->toolButton_programe)
    {
        SelectRunningProgramDialog dialog;

        dialog.addIgnorHandle((HWND)this->winId());
        dialog.addIgnorHandle((HWND)AppConfig::gMainWindow->winId());
        dialog.init();

        if (dialog.exec() == QDialog::Accepted)
        {
            setVideoPlayMode(false);
            mCurrentCaptureProgramIsCoveredByOtherWindow = false;
            mSelectAreaWidget->setColor(QColor(255, 0, 0));
            mCurrentCaptureProgramRect = QRect(0, 0, 0, 0);

            HWND hWnd = dialog.getSelectedHandle();
            QRect rect = dialog.getProgramRect();

//            ShowWindow(hWnd, SW_SHOWNOACTIVATE); //显示窗口
            SwitchToThisWindow(hWnd, TRUE);

//            std::thread([=]
//            {
//                AppConfig::mSleep(500);
//                FunctionTransfer::runInMainThread([=]()
//                {
//                    SwitchToThisWindow((HWND)this->winId(), TRUE);
//                });
//            }).detach();

            mCurrentCaptrueProgramHandle = hWnd;
qDebug()<<__FUNCTION__<<hWnd<<rect;
            addCapWindow(hWnd, rect);

            if (mLastSelectedBtn != nullptr)
            {
                mLastSelectedBtn->setChecked(false);
            }
            mLastSelectedBtn = (QToolButton*)QObject::sender();
            mLastSelectedBtn->setChecked(true);

            mTimerUpdateSelectArea->stop();
            mTimerUpdateSelectArea->start();

            mSelectAreaWidget->setInTop(false);
//            mSelectRectWidget->showFullScreen();
//            mSelectRectWidget->setRect(rect);
//            mSelectRectWidget->setPointHide();
        }
        else if (isChecked)
        {
            ui->toolButton_programe->setChecked(false);
        }
        else
        {
            ui->toolButton_programe->setChecked(true);
        }

    }
    else if (QObject::sender() == ui->toolButton_selectedArea)
    {
        mCurrentCaptrueProgramHandle = NULL;
        mTimerUpdateSelectArea->stop();

        setVideoPlayMode(false);

        if (isChecked)
        {
            mSelectAreaWidget->getReadyToSelect();
        }
        else
        {
            mSelectAreaWidget->showFullScreen();
            mSelectAreaWidget->setEditMode(true);
        }

        if (mLastSelectedBtn != nullptr)
        {
            mLastSelectedBtn->setChecked(false);
        }
        mLastSelectedBtn = (QToolButton*)QObject::sender();
        mLastSelectedBtn->setChecked(true);

        mSelectAreaWidget->setInTop(true);
        mSelectAreaWidget->setColor(QColor(255, 0, 0));
    }
//    else if (QObject::sender() == ui->toolButton_videofile)
//    {
//        if (isChecked)
//        {
//            QStringList fileList = QFileDialog::getOpenFileNames(
//                       this, QStringLiteral("选择要播放的文件"),
//                        AppConfig::gVideoFilePath,//初始目录
//                        QStringLiteral("视频文件 (*.flv *.rmvb *.avi *.MP4 *.mkv);;")
//                        +QStringLiteral("音频文件 (*.mp3 *.wma *.wav);;")
//                        +QStringLiteral("所有文件 (*.*)"));

//            if (!fileList.isEmpty())
//            {
//                setVideoPlayMode(true);

////                mPlayVideoTaskWidget->addVideoFiles(fileList);
////                mPlayVideoTaskWidget->startPlay();

//                if (mLastSelectedBtn != nullptr)
//                {
//                    mLastSelectedBtn->setChecked(false);
//                }

//                mLastSelectedBtn = (QToolButton*)QObject::sender();
//                mLastSelectedBtn->setChecked(true);
//            }
//            else
//            {
//                ui->toolButton_videofile->setChecked(false);
//            }
//        }
//        else
//        {
//            ui->toolButton_videofile->setChecked(true);
//        }
//    }
    else if (QObject::sender() == ui->pushButton_audioTest)
    {
        if (mIsPlayingVideo)
        {
            int ret = MyMessageBox_WithTitle::showWarningText(QStringLiteral("警告"),
                                                              QStringLiteral("当前正在播放视频，无法设置麦克风！"),
                                                                       NULL,
                                                                       QStringLiteral("关闭"));
        }
        else
        {
            this->doDeviceTest();
        }
    }
    else if (QObject::sender() == ui->checkBox_camera)
    {
        if (isChecked)
        {
            initDevice();
            doOpenCamera();
            static_cast<ShowVideoWidget*>(mShowCameraWidget)->show();
            reSetCaptureWidgetWidth();
        }
        else
        {
            static_cast<ShowVideoWidget*>(mShowCameraWidget)->hide();
            mCameraVideoSize  = QSize(0, 0);

            AppConfig::gMediaManager->removeTask(CAMERA_TASKID);
            AppConfig::gMediaManager->closeCameraCaptureMode();

            reSetCaptureWidgetWidth();
        }
    }
    else if (QObject::sender() == ui->checkBox_picture)
    {
        if (isChecked)
        {
            QString filePath = ui->lineEdit_picture->text();

            if (!filePath.isEmpty())
            {
                AppConfig::gMediaManager->removeTask(mCapturePictureWidget->getId());
                addCapPicture(filePath);
            }
        }
        else
        {
            mIsShowPicture = false;
            mCapturePictureWidget->hide();
            AppConfig::gMediaManager->removeTask(mCapturePictureWidget->getId());
            reSetCaptureWidgetWidth();
        }
    }
    else if (QObject::sender() == ui->pushButton_picture)
    {
        QString filePath = QFileDialog::getOpenFileName(this,
                                                        QStringLiteral("选择图片"),
                                                        "",
                                                        QStringLiteral("图片文件(*png *jpg *jpeg *bmp);;"));

        if (!filePath.isEmpty())
        {
            ui->lineEdit_picture->setText(filePath);

            AppConfig::gMediaManager->removeTask(mCapturePictureWidget->getId());
            addCapPicture(filePath);
        }
    }
    else if (QObject::sender() == ui->pushButton_expand)
    {
        setShowViewWidget(isChecked);
    }
}

void CaptureTaskManager::slotSliderValueChanged(int value)
{
    if (QObject::sender() == ui->horizontalSlider_Size_Horizontal)
    {
        if (value < 30)
        {
            ui->horizontalSlider_Size_Horizontal->setValue(30);
        }
        else if (value > MAX_CAPTURE_RATE_VALUE)
        {
            ui->horizontalSlider_Size_Horizontal->setValue(MAX_CAPTURE_RATE_VALUE);
        }
        else
        {
            AppConfig::gCaptureAreaRate = value;

            mTimerUpdateSlider->stop();
            mTimerUpdateSlider->start();
        }
    }
    else if (QObject::sender() == ui->horizontalSlider_Size_Picture)
    {
        if (value < 10)
        {
            ui->horizontalSlider_Size_Picture->setValue(10);
        }
        else if (value > mPictureRateMaxValue)
        {
            ui->horizontalSlider_Size_Picture->setValue(mPictureRateMaxValue);
        }
        else
        {
            AppConfig::gPictureAreaRate = value;

            mTimerUpdateSlider->stop();
            mTimerUpdateSlider->start();
        }
    }

}

void CaptureTaskManager::slotSelectRectFinished(QRect re)
{
    /// 1.传给ffmpeg编码的图像宽高必须是偶数。
    /// 2.图像裁剪的起始位置和结束位置也必须是偶数
    /// 而手动选择的区域很有可能会是奇数，因此需要处理一下 给他弄成偶数
    /// 处理的方法很简答：其实就是往前或者往后移一个像素
    /// 一个像素的大小肉眼基本也看不出来啥区别。

    int x = re.x() + RECT_WIDTH;
    int y = re.y() + RECT_WIDTH;
    int w = re.width() - RECT_WIDTH*2;
    int h = re.height() - RECT_WIDTH*2;

    if (x % 2 != 0)
    {
        x++;
        w--;
    }

    if (y % 2 != 0)
    {
        y++;
        h--;
    }

    if (w % 2 != 0)
    {
        w--;
    }

    if (h % 2 != 0)
    {
        h--;
    }

    mSelectedCaptureRect = QRect(x,y,w,h);

    addCapWindow(NULL, mSelectedCaptureRect);

//    QString str = QStringLiteral("==当前区域==\n\n起点(%1,%2)\n\n大小(%3 x %4)")
//            .arg(rect.left()).arg(rect.left()).arg(rect.width()).arg(rect.height());

//    ui->showRectInfoLabel->setText(str);

//    ui->startButton->setEnabled(true);
//    ui->editRectButton->setEnabled(true);
//    ui->hideRectButton->setEnabled(true);
//    ui->hideRectButton->setText(QStringLiteral("隐藏"));

//    saveConfigFile();

}

void CaptureTaskManager::slotTimerTimeOut()
{
    if (QObject::sender() == mTimerUpdateSelectArea)
    {
        if (mCurrentCaptrueProgramHandle != NULL)
        {
            HWND hWnd = mCurrentCaptrueProgramHandle;

//            qDebug()<<__FUNCTION__<<IsWindowVisible(hWnd)<<IsIconic(hWnd);

            if (IsWindowVisible(hWnd))
            {
                if (!IsIconic(hWnd))
                {
                    RECT rect = CaptureWindowThread::getWindowRect(hWnd);

                    int x = rect.left;
                    int y = rect.top;
                    int w = rect.right - rect.left;
                    int h = rect.bottom - rect.top;

                    if (w > 0 && h > 0)
                    {
                        bool isCoveredByOther = CaptureWindowThread::IsCoveredByOtherWindow(hWnd);

                        if (mCurrentCaptureProgramIsCoveredByOtherWindow != isCoveredByOther)
                        {
                            mCurrentCaptureProgramIsCoveredByOtherWindow = isCoveredByOther;

                            if (mCurrentCaptureProgramIsCoveredByOtherWindow)
                            {
                                mSelectAreaWidget->setColor(QColor(230, 168, 69));
                            }
                            else
                            {
                                SwitchToThisWindow((HWND)mSelectAreaWidget->winId(), TRUE);
    //                                ::BringWindowToTop((HWND)mSelectAreaWidget->winId());
                                mSelectAreaWidget->setColor(QColor(255, 0, 0));
                            }
                        }

                        QRect tmpRect = QRect(x, y, w, h);
                        if (tmpRect != mCurrentCaptureProgramRect)
                        {
                            mSelectAreaWidget->setRect(QRect(x-RECT_WIDTH, y-RECT_WIDTH, w+RECT_WIDTH*2, h+RECT_WIDTH*2));
                            mSelectAreaWidget->setEditMode(false);

                            if (!isCoveredByOther)
                            {
                                mCurrentCaptureProgramRect = tmpRect;
                                addCapWindow(hWnd, QRect(0, 0, w, h));
                            }
                        }
                    }
    //                qDebug()<<__FUNCTION__<<hWnd<<(HWND)mSelectRectWidget->winId()<<(HWND)AppConfig::gMainWindow->winId()<<IsCoveredByOtherWindow(hWnd);
                }
            }
            else
            {
                ///窗口已经关闭了，切回全屏
                changeToFullScreenMode();
            }
        }
    }
    else if (QObject::sender() == mTimerUpdateSlider)
    {
        mTimerUpdateSlider->stop();

        reSetCaptureWidgetWidth();
        AppConfig::saveConfigInfoToFile();
    }
    else if (QObject::sender() == mTimer_GetTime)
    {
        ///获取录屏时间
        {
            int64_t currentTime = 0;

            currentTime = mMediaManager->getVideoFileCurrentTime();
            currentTime /= 1000;

            int minutes = currentTime / 60;
            int hours = minutes / 60;
            int minute = minutes % 60;
            int second = currentTime % 60;

            char timeCh[16] = {0};
            sprintf(timeCh, "%02d:%02d:%02d",hours, minute, second);

            ui->label_time_record->setText(QString(timeCh));

//qDebug()<<__FUNCTION__<<timeCh;
        }
    }
}

void CaptureTaskManager::inputCameraVideoFrame(VideoRawFramePtr videoFramePtr)
{
//    qDebug()<<__FUNCTION__<<videoFramePtr->getWidth()<<videoFramePtr->getHeight();

    FunctionTransfer::runInMainThread([=]()
    {
        if(mCameraVideoSize.width() == 0)
        {
            if (ui->checkBox_camera->isChecked())
            {
                mCameraVideoSize = QSize(videoFramePtr->getWidth(), videoFramePtr->getHeight());
                reSetCaptureWidgetWidth();
            }
        }
    });

    static_cast<ShowVideoWidget*>(mShowCameraWidget)->inputOneFrame(videoFramePtr);
}

void CaptureTaskManager::inputFinalVideoFrame(VideoRawFramePtr videoFramePtr)
{
//    qDebug()<<__FUNCTION__<<(videoFramePtr==nullptr)<<videoFramePtr.get();
    if (videoFramePtr->getWidth() <= 0)
qDebug()<<__FUNCTION__<<videoFramePtr->getWidth()<<videoFramePtr->getHeight()<<ui->widget_capture_view->size();
    if (ui->toolButton_view->isChecked())
    {
        static_cast<ShowVideoWidget*>(mShowVideoWidget)->inputOneFrame(videoFramePtr);
    }
}
