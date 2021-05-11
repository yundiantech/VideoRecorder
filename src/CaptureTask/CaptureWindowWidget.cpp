/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "CaptureWindowWidget.h"
#include "ui_CaptureWindowWidget.h"

#include <QScreen>

CaptureWindowWidget::CaptureWindowWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CaptureWindowWidget)
{
    ui->setupUi(this);

    mHwnd = NULL;
    mId = -1;

    mTimer = new QTimer(this);
    mTimer->setInterval(1000);
    connect(mTimer, &QTimer::timeout, this, &CaptureWindowWidget::slotTimerTimeout);

}

CaptureWindowWidget::~CaptureWindowWidget()
{
    delete ui;
}

void CaptureWindowWidget::setTask(const int &id, const QRect &capRect)
{
    mId      = id;
    mCapRect = capRect;
}

void CaptureWindowWidget::setHWND(HWND hwnd)
{
    mHwnd = hwnd;
}

void CaptureWindowWidget::startCap()
{
    doUpdate();
    mTimer->start();
}

void CaptureWindowWidget::doUpdate()
{
    if (IsIconic(mHwnd) || CaptureWindowThread::IsCoveredByOtherWindow(mHwnd))
    {
        return;
    }

    QScreen *screen = QGuiApplication::primaryScreen();
//    QPixmap pixmap = screen->grabWindow((WId)mHwnd, mCapRect.x(), mCapRect.y(), mCapRect.width(), mCapRect.height());

    QRect capRect = mCapRect;

    if (mHwnd != NULL)
    {
        HWND hWnd = mHwnd;

        int width = 0;
        int height = 0;

//        RECT rect;
////        GetClientRect(hWnd, &rect);
//        GetWindowRect(hWnd, &rect);

        RECT rect = CaptureWindowThread::getWindowRect(hWnd);

        width  = rect.right - rect.left;
        height = rect.bottom - rect.top;

        capRect = QRect(rect.left, rect.top, width, height);
    }

    QPixmap pixmap = screen->grabWindow(NULL, capRect.x(), capRect.y(), capRect.width(), capRect.height());

    ui->label->setPixmap(pixmap);
}

void CaptureWindowWidget::slotTimerTimeout()
{
    doUpdate();
}
