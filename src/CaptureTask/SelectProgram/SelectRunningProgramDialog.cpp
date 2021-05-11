/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "SelectRunningProgramDialog.h"
#include "ui_SelectRunningProgramDialog.h"

#include <QScreen>
#include <QDebug>

#include "Media/Video/CaptureWindowThread.h"

SelectRunningProgramDialog::SelectRunningProgramDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectRunningProgramDialog)
{
    ui->setupUi(this);

    mFlowLayout = new FlowLayout;
    ui->verticalLayout->addLayout(mFlowLayout);

    mSelectedWidget = nullptr;
}

SelectRunningProgramDialog::~SelectRunningProgramDialog()
{
    for (ShowProgramPictureWidget *widget : mShowProgramPictureWidgetList)
    {
        mFlowLayout->removeWidget(widget);
        widget->disconnect(this);
        widget->deleteLater();
    }
    delete ui;
}

void SelectRunningProgramDialog::addIgnorHandle(HWND handle)
{
    mIgnoreHandleList.append(handle);
}

void SelectRunningProgramDialog::init()
{
    std::list<HWND> handleList = CaptureWindowThread::getCaptureWindowList(); //获取可以捕获的窗体列表

    for (HWND pWnd : handleList)
    {
        if (mIgnoreHandleList.contains(pWnd)) continue;

//        bool isAvailble = CaptureWindowThread::IsWindowAvailable(pWnd);
//        if (isAvailble)
        {
            ShowProgramPictureWidget* widget = new ShowProgramPictureWidget();
            connect(widget, &ShowProgramPictureWidget::sig_Clicked, this, &SelectRunningProgramDialog::slotItemClicked);

            widget->setHandle(pWnd);
            widget->setMaximumSize(240, 160);
            widget->setMinimumSize(240, 160);

            mFlowLayout->addWidget(widget);
            mShowProgramPictureWidgetList.append(widget);

//            ///保存图片
//            {
//                QScreen *screen = QGuiApplication::primaryScreen();
//                QPixmap pixmap = screen->grabWindow((WId)pWnd);

//                static int i=0;
//                pixmap.save(QString("%1.bmp").arg(i++));
//            }
        }
    }
}

HWND SelectRunningProgramDialog::getSelectedHandle()
{
    HWND handle = NULL;

    if (mSelectedWidget != nullptr)
    {
        handle = mSelectedWidget->getHandle();
    }

    return handle;
}

QRect SelectRunningProgramDialog::getProgramRect()
{
    QRect outRect = QRect(0, 0, 0, 0);

    if (mSelectedWidget != nullptr)
    {
        HWND hWnd = mSelectedWidget->getHandle();

        int width = 0;
        int height = 0;

        RECT rect;
//        GetClientRect(hWnd, &rect);
        GetWindowRect(hWnd, &rect);

        width  = rect.right - rect.left;
        height = rect.bottom - rect.top;

        outRect = QRect(0, 0, width, height);
    }

    return outRect;
}

void SelectRunningProgramDialog::slotItemClicked(HWND handle)
{
    mSelectedWidget = (ShowProgramPictureWidget*)QObject::sender();
    this->accept();
}
