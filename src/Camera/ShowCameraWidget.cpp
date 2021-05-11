/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "ShowCameraWidget.h"
#include "ui_ShowCameraWidget.h"

#include "AppConfig.h"
#include "Base/FunctionTransfer.h"
#include "Media/Image/yuv420p.h"
#include "Media/MediaManager.h"

#include <QDebug>
#include <QCameraInfo>

Q_DECLARE_METATYPE(QCameraInfo)

ShowCameraWidget::ShowCameraWidget(QWidget *parent) :
    MyCustomerWidget(parent),
    ui(new Ui::ShowCameraWidget)
{
//    ui->setupUi(this);
    ui->setupUi(this->getContainWidget());

    setWindowFlags(Qt::WindowStaysOnTopHint|Qt::FramelessWindowHint|Qt::Tool|Qt::X11BypassWindowManagerHint); //使窗口置顶

    mIsCameraOpend = false;

    this->installEventFilter(this);
    ui->widget_back->installEventFilter(this);

    connect(ui->comboBox_videoDevice, SIGNAL(activated(int)), this, SLOT(slotCurrentIndexChanged(int)));

    connect(ui->pushButton_close, &QPushButton::clicked, this, &ShowCameraWidget::slotBtnClicked);
}

ShowCameraWidget::~ShowCameraWidget()
{
    delete ui;
}

void ShowCameraWidget::showOut()
{
    initDevice();

    doOpenCamera();

    this->show();
}

void ShowCameraWidget::hideIn()
{
    this->hide();
    closeCamera();
}

void ShowCameraWidget::doOpenCamera()
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

//void ShowCameraWidget::openCamera(const QString &cameraName)
//{
//    if (mIsCameraOpend)
//    {
//        closeCamera();
//    }

//    auto getVideoFrameFunc = [=](VideoRawFramePtr videoFramePtr, void *param)
//    {
//        this->inputVideoFrame(videoFramePtr);
//    };
//qDebug()<<__FUNCTION__<<cameraName;
//    if (mReadCameraThead->openCamera(cameraName.toStdString()))
//    {
//        mReadCameraThead->startRecord(getVideoFrameFunc, nullptr);

//        AppConfig::gLocalCameraDeviceName = cameraName;
//        mIsCameraOpend = true;

//        int w = mReadCameraThead->getVideoWidth();
//        int h = mReadCameraThead->getVideoHeight();

//        float rate = h * 1.0 / w;

//        this->setRate(rate);

//        int widgetW = this->width();
//        int widgetH = widgetW * rate;

//        this->resize(widgetW, widgetH);
//    }
//    else
//    {
//    qDebug()<<__FUNCTION__<<cameraName<<"failed!";
//        ui->widget_video->setPlayFailed(true);
//    }

//}

void ShowCameraWidget::initDevice()
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

void ShowCameraWidget::openCamera(const QString &cameraName)
{
    AppConfig::gLocalCameraDeviceName = cameraName;
    AppConfig::gMediaManager->openCameraWindowMode(cameraName.toStdString());
}

void ShowCameraWidget::closeCamera()
{
    AppConfig::gMediaManager->closeCameraWindowMode();
//    mReadCameraThead->stopRecord();
    mIsCameraOpend = false;
}

void ShowCameraWidget::slotCurrentIndexChanged(int index)
{
    qDebug()<<__FUNCTION__<<index;
    QString deviceName = ui->comboBox_videoDevice->currentText();
    openCamera(deviceName);
}

void ShowCameraWidget::slotBtnClicked(bool isChecked)
{
    if (QObject::sender() == ui->pushButton_close)
    {
        this->hideIn();
        emit sigViewStateChanged(false);
    }
}

bool ShowCameraWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->widget_back)
    {
//        qDebug()<<event->type();
        if (event->type() == QEvent::Resize)
        {
            QResizeEvent *e = (QResizeEvent*)event;
            ui->widget_control->resize(e->size());
            ui->widget_video->resize(e->size());

            ui->widget_control->move(0, 0);
            ui->widget_video->move(0, 0);
        }
        else if (event->type() == QEvent::Enter)
        {
//qDebug()<<__FUNCTION__<<"enter";
            if (!ui->widget_control->isVisible())
            {
                ui->widget_control->show();
                ui->widget_control->raise();
            }
        }
        else if (event->type() == QEvent::Leave)
        {
            QPoint globalPoint(ui->widget_control->mapToGlobal(QPoint(0, 0)));

            QRect widgetRect;
            widgetRect.setTopLeft(globalPoint);
            widgetRect.setWidth(ui->widget_control->width());
            widgetRect.setHeight(ui->widget_control->height());

            if (!widgetRect.contains(QCursor::pos()))
            {
                ui->widget_control->hide();
            }

        }
    }

    return QObject::eventFilter(obj, event);
}

void ShowCameraWidget::inputVideoFrame(VideoRawFramePtr videoFramePtr)
{
    ui->widget_video->inputOneFrame(videoFramePtr);
}
