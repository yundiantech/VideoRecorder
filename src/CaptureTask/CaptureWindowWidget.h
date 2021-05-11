/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef CAPTUREWINDOWWIDGET_H
#define CAPTUREWINDOWWIDGET_H

#include <QWidget>
#include <QTimer>

//#include "widget/customWidget/MyCustomerWidget.h"
#include "Media/MediaManager.h"

namespace Ui {
class CaptureWindowWidget;
}

class CaptureWindowWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CaptureWindowWidget(QWidget *parent = nullptr);
    ~CaptureWindowWidget();

    void setTask(const int &id, const QRect &capRect);
    int getId(){return mId;}

    QRect getCaptureRect(){return mCapRect;}

    void setHWND(HWND hwnd);
    HWND getHWND(){return mHwnd;}

    void startCap();

private:
    Ui::CaptureWindowWidget *ui;

    int mId;

    HWND mHwnd; //窗口句柄

    QRect mCapRect; //采集的区域

    QTimer *mTimer;

    void doUpdate();

private slots:
    void slotTimerTimeout();

};

#endif // CAPTUREWINDOWWIDGET_H
