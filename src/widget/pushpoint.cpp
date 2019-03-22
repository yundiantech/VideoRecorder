
/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "pushpoint.h"

PushPoint::PushPoint(QWidget *parent)
{
    setParent(parent);
    setWindowFlags(Qt::FramelessWindowHint);  //使窗口标题栏隐藏
    setMouseTracking(true);
    QPixmap pixmap = QPixmap(QSize(POINT_SIZE,POINT_SIZE));  //构建一个QPixmap
    pixmap.fill(Qt::red);   //用红色填充这个pixmap
    setPixmap(pixmap);
    show();
}

PushPoint::~PushPoint()
{

}

void PushPoint::setLocation(LocationPoint Loa)
{
    location = Loa;
    setMouseCursor();
}

void PushPoint::mousePressEvent(QMouseEvent * event)
{
    if (event->button() == Qt::LeftButton)
    {
         dragPosition=event->globalPos()-frameGeometry().topLeft();
         event->accept();
    }
}
void PushPoint::mouseMoveEvent(QMouseEvent * event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        QPoint point;
        int x;
        int y;
        switch (location)
        {
        case TopLeft:
        case BottomRight:
        case TopRight:
        case BottomLeft:
            point = event->globalPos() - dragPosition;
            break;
        case TopMid:
        case BottomMid:
            y = (event->globalPos() - dragPosition).y();
            x = pos().x();
            point = QPoint(x,y);
            break;
        case MidLeft:
        case MidRight:
            x = (event->globalPos() - dragPosition).x();
            y = pos().y();
            point = QPoint(x,y);
            break;
        default:
            point = event->globalPos() - dragPosition;
            break;
        }
        move(point);
        emit moved(point);
        event->accept();
    }
}

void PushPoint::setMouseCursor()
{
    switch (location)
    {
    case TopLeft:
    case BottomRight:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case TopRight:
    case BottomLeft:
        setCursor(Qt::SizeBDiagCursor);
        break;
    case TopMid:
    case BottomMid:
        setCursor(Qt::SizeVerCursor);
        break;
    case MidLeft:
    case MidRight:
        setCursor(Qt::SizeHorCursor);
        break;
    default:
        setCursor(Qt::ArrowCursor);
        break;
    }
}
