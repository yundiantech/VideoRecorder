/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "ShowProgramPictureWidget.h"
#include "ui_ShowProgramPictureWidget.h"

#if defined(WIN32)
    #include <WinSock2.h>
    #include <Windows.h>
#else

#endif

#include <QScreen>

static QString get_window_title(HWND hwnd)
{
    QString retStr;
    wchar_t *temp;
    int len;

    len = GetWindowTextLengthW(hwnd);
    if (!len)
        return retStr;

    temp = (wchar_t *)malloc(sizeof(wchar_t) * (len+1));
    if (GetWindowTextW(hwnd, temp, len+1))
    {
        retStr = QString::fromWCharArray(temp);
    }
    free(temp);
    return retStr;
}

QString get_window_class(HWND hwnd)
{
    QString retStr;
    wchar_t temp[256];

    temp[0] = 0;
    if (GetClassNameW(hwnd, temp, sizeof(temp) / sizeof(wchar_t)))
    {
        retStr = QString::fromWCharArray(temp);
    }
    return retStr;
}

ShowProgramPictureWidget::ShowProgramPictureWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShowProgramPictureWidget)
{
    ui->setupUi(this);
}

ShowProgramPictureWidget::~ShowProgramPictureWidget()
{
    delete ui;
}

void ShowProgramPictureWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit sig_Clicked(mHWnd);
    }
}

void ShowProgramPictureWidget::setHandle(HWND handle)
{
    mHWnd = handle;

    QString title = get_window_title(handle);
    setTitle(title);

    QScreen *screen = QGuiApplication::primaryScreen();
    QPixmap pixmap = screen->grabWindow((WId)handle);

    ui->label->setPixmap(pixmap);

    mRect = QRect(0, 0, pixmap.width(), pixmap.height());

//    qDebug()<<__FUNCTION__<<i<<pWnd<<isMainWindow<<map;

//            if (!map.isNull() && map.width() >= 2 && map.height() >= 2)
//    map.save(QString("image/%1.jpg").arg(i++));

}

void ShowProgramPictureWidget::setTitle(const QString &title)
{
    ui->label_title->setText(title);
}
