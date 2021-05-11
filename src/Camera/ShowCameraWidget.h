/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef SHOWCAMERAWIDGET_H
#define SHOWCAMERAWIDGET_H

#include <QWidget>

#include "Media/MediaManager.h"

#include "Widget/CustomWidget/MyCustomerWidget.h"

namespace Ui {
class ShowCameraWidget;
}

class ShowCameraWidget : public MyCustomerWidget
{
    Q_OBJECT

public:
    explicit ShowCameraWidget(QWidget *parent = 0);
    ~ShowCameraWidget();

    void showOut();
    void hideIn();

    void inputVideoFrame(VideoRawFramePtr videoFramePtr);

signals:
    void sigViewStateChanged(const bool &isShow);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::ShowCameraWidget *ui;

    ///记录实际摄像头的分辨率
    int mVideoWidth  = 0;
    int mVideoHeight = 0;

    bool mIsCameraOpend;

    void initDevice();

    void doOpenCamera();
    void openCamera(const QString &cameraName);
    void closeCamera();

private slots:
    void slotCurrentIndexChanged(int index);
    void slotBtnClicked(bool isChecked);

};

#endif // SHOWCAMERAWIDGET_H
