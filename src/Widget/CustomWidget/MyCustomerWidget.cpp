/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "mycustomerwidget.h"
#include "ui_mycustomerwidget.h"

#include <QDebug>

enum Direction { UP=0, DOWN, LEFT, RIGHT, LEFTTOP, LEFTBOTTOM, RIGHTBOTTOM, RIGHTTOP, NONE };

MyCustomerWidget::MyCustomerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MyCustomerWidget)
{
    ui->setupUi(this);

///改变窗体大小相关
    isMax = false;

    mLocation = this->geometry();

    isLeftPressDown = false;
    this->dir = NONE;
    this->setMinimumHeight(100);
    this->setMinimumWidth(150);
//    this->setWindowFlags(Qt::FramelessWindowHint|Qt::WindowSystemMenuHint); // 设置成无边框对话框
    this->setMouseTracking(true);// 追踪鼠标
    ui->widget_frame->setMouseTracking(true);
    ui->widget_back->setMouseTracking(true);
    ui->widget_container->setMouseTracking(true);
//    ui->widget_center->setMouseTracking(true);

    this->setFocusPolicy(Qt::ClickFocus);

//    ui->widget_frame->setContentsMargins(1,1,1,1);

    //安装事件监听器,让标题栏识别鼠标双击
    ui->lab_Title->installEventFilter(this);
//    this->installEventFilter(this);

    ui->widget_frame->setContentsMargins(0,0,0,0);
    ui->widget_title->hide();

    mRate = 9.0 / 16;
    mLimitRect = QApplication::desktop()->screenGeometry();

}

MyCustomerWidget::~MyCustomerWidget()
{
    delete ui;
}

void MyCustomerWidget::setActive()
{
    ui->widget_frame->setContentsMargins(3,3,3,3);
    this->setCursor(Qt::SizeAllCursor);
    this->setFocus();
}

//void MyCustomerWidget::addWidget(QWidget *widget)
//{
//    ui->horizontalLayout_containner->addWidget(widget);
//}

QWidget *MyCustomerWidget::getContainWidget()
{
    return ui->widget_container;
}

void MyCustomerWidget::enterEvent(QEvent *event)
{
//    qDebug()<<__FUNCTION__;

//    ui->widget_frame->setContentsMargins(3,3,3,3);
//    this->setCursor(Qt::SizeAllCursor);
//    this->setFocus();
}

void MyCustomerWidget::leaveEvent(QEvent *event)
{
//    qDebug()<<__FUNCTION__;
//    ui->widget_frame->setContentsMargins(0,0,0,0);
}

void MyCustomerWidget::focusInEvent(QFocusEvent *event)
{
//    qDebug()<<__FUNCTION__;
//    ui->widget_frame->setContentsMargins(1,1,1,1);
//    this->setCursor(Qt::ArrowCursor);
}

void MyCustomerWidget::focusOutEvent(QFocusEvent *event)
{
//    qDebug()<<__FUNCTION__;
    ui->widget_frame->setContentsMargins(0,0,0,0);

    this->setCursor(Qt::ArrowCursor);

}

void MyCustomerWidget::keyPressEvent(QKeyEvent *event)
{
    qDebug()<<event->key()<<event->modifiers();
    qDebug()<<Qt::Key_Alt<<Qt::Key_F4<<0x01000023;
    switch (event->key())
    {
    case Qt::Key_Escape:
        break;
    case Qt::Key_F4 :  //ALT + F4
    case Qt::Key_Alt:
//        if (event->modifiers() == Qt::AltModifier)
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

////////////改变窗体大小相关

bool MyCustomerWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->lab_Title)
    {
        if (event->type() == QEvent::MouseButtonDblClick) {
            this->on_btnMenu_Max_clicked();
            return true;
        }
    }
//    else if (obj == this)
//    {
//        qDebug()<<"111";
//        if(event->type() == QEvent::KeyPress)
//        {
//            qDebug()<<"222";
//            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
//            qDebug()<<keyEvent->key();
//            if(keyEvent->key()  == Qt::Key_Alt)
//            {
//                qDebug()<<"333";
//                return true;
//            }
//        }
//    }

    return QObject::eventFilter(obj, event);
}


void MyCustomerWidget::mouseReleaseEvent(QMouseEvent *event)
{
//    qDebug()<<__FUNCTION__;
    if(event->button() == Qt::LeftButton){
        isLeftPressDown = false;
        if(dir != NONE) {
            this->releaseMouse();
            this->setCursor(QCursor(Qt::ArrowCursor));
        }
        emit sig_WindowMoveFinished(this->geometry());
    }
}

void MyCustomerWidget::mousePressEvent(QMouseEvent *event)
{
//    qDebug()<<__FUNCTION__;
    ui->widget_frame->setContentsMargins(3,3,3,3);
    this->setCursor(Qt::SizeAllCursor);
    this->raise();

    switch(event->button()) {
    case Qt::LeftButton:
        if (isMax) break;
        isLeftPressDown = true;
        checkCursorDirect(event->globalPos());

        if(dir != NONE) {
            mIsResizeMode = true;
            this->mouseGrabber();
        } else {
            mIsResizeMode = false;
            dragPosition = event->globalPos() - this->frameGeometry().topLeft();
        }
        break;
    case Qt::RightButton:
//        this->close();
        emit sig_MouseRightBtnClick();
        break;
    default:
        QWidget::mousePressEvent(event);
    }

}

void MyCustomerWidget::mouseMoveEvent(QMouseEvent *event)
{
    QPoint gloPoint = event->globalPos();
//qDebug()<<__FUNCTION__<<gloPoint<<isLeftPressDown<<dir;
    int x = this->x();
    int y = this->y();
    int w = this->width();
    int h = this->height();

    if (isMax) return;
    if (!isLeftPressDown)
    {
        checkCursorDirect(gloPoint);
        return;
    }
//    if(!isLeftPressDown)
//    {
//        checkCursorDirect(gloPoint);
//    }
//    else
    {
        if(dir != NONE)
        {
            QPoint point = mapToParent(event->pos());
//qDebug()<<x<<y<<w<<h<<point<<mLimitRect;
            if (point.x() < mLimitRect.x())
                point.setX(mLimitRect.x());

            if (point.y() < mLimitRect.y())
                point.setY(mLimitRect.y());


            switch(dir) {
            case LEFT:

                if (point.x() > (this->x() + this->width() - this->minimumWidth()))
                    point.setX(this->x() + this->width() - this->minimumWidth());

                w = w + x - point.x();
                x = point.x();
                h = w * mRate;

//                h+=30;

                break;
            case RIGHT:

                if (point.x() > (mLimitRect.x()+mLimitRect.width()))
                    point.setX(mLimitRect.x()+mLimitRect.width());


                w = point.x() - x;

                h = w * mRate;

//                h+=30;

                break;
            case UP:

                if (point.y() > (this->y() + this->height() - this->minimumHeight()))
                    point.setY(this->y() + this->height() - this->minimumHeight());

                h = h + y - point.y();
                y = point.y();
                w = h / mRate;

//                h+=30;

                break;
            case DOWN:

                if (point.y() > (mLimitRect.y()+mLimitRect.height()))
                    point.setY(mLimitRect.y()+mLimitRect.height());

                h = point.y() - y;

                w = h / mRate;

//                h+=30;

                break;
            case LEFTTOP:
//                if(rb.x() - gloPoint.x() <= this->minimumWidth())
//                    rMove.setX(tl.x());
//                else
//                    rMove.setX(gloPoint.x());
//                if(rb.y() - gloPoint.y() <= this->minimumHeight())
//                    rMove.setY(tl.y());
//                else
//                    rMove.setY(gloPoint.y());


//                w = rMove.width();
//                if (w < this->minimumWidth()) w = this->minimumWidth();

//                h = w * mRate;
//                rMove.setWidth(w);
//                rMove.setHeight(h);


                break;
            case RIGHTTOP:
//                rMove.setWidth(gloPoint.x() - tl.x());
//                rMove.setY(gloPoint.y());

//                h = rMove.height();
//                if (h < this->minimumHeight()) h = this->minimumHeight();

//                w = h / mRate;
//                rMove.setWidth(w);
//                rMove.setHeight(h);

                break;
            case LEFTBOTTOM:
//                rMove.setX(gloPoint.x());
////                rMove.setHeight(gloPoint.y() - tl.y());
//                w = rMove.width();
//                if (w < this->minimumWidth()) w = this->minimumWidth();

//                h = w * mRate;
//                rMove.setWidth(w);
//                rMove.setHeight(h);
                break;
            case RIGHTBOTTOM:

//                w = gloPoint.x() - tl.x();
//                if (w < this->minimumWidth()) w = this->minimumWidth();

//                h = w * mRate;
//                rMove.setWidth(w);
//                rMove.setHeight(h);

                break;
            default:
                break;
            }
//            this->setGeometry(rMove);
//            emit sig_WindowMoved(rMove);

            this->setGeometry(QRect(x, y, w, h));
            emit sig_WindowMoved(QRect(x, y, w, h));
        }
        else
        {
//            checkCursorDirect(event->globalPos());

            if (dir == NONE && !isMax)
            {
                QPoint point = event->globalPos() - dragPosition;

                if (point.x() < mLimitRect.x())
                    point.setX(mLimitRect.x());

                if (point.x() > (mLimitRect.x()+mLimitRect.width()-this->width()))
                    point.setX(mLimitRect.x()+mLimitRect.width()-this->width());

                if (point.y() < mLimitRect.y())
                    point.setY(mLimitRect.y());

                if (point.y() > (mLimitRect.y()+mLimitRect.height()-this->height()))
                    point.setY(mLimitRect.y()+mLimitRect.height()-this->height());

                move(point);
                emit sig_WindowMoved(QRect(point,this->size()));
            }

            event->accept();
        }
    }
//    QWidget::mouseMoveEvent(event);、
    event->accept();
}

void MyCustomerWidget::checkCursorDirect(const QPoint &cursorGlobalPoint)
{
    // 获取窗体在屏幕上的位置区域，tl为topleft点，rb为rightbottom点
    QRect rect = this->rect();
    QPoint tl = mapToGlobal(rect.topLeft());
    QPoint rb = mapToGlobal(rect.bottomRight());

    int x = cursorGlobalPoint.x();
    int y = cursorGlobalPoint.y();

//    if(tl.x() + PADDING >= x && tl.x() <= x && tl.y() + PADDING >= y && tl.y() <= y) {
//        // 左上角
//        dir = LEFTTOP;
//        this->setCursor(QCursor(Qt::SizeFDiagCursor));  // 设置鼠标形状
//    } else if(x >= rb.x() - PADDING && x <= rb.x() && y >= rb.y() - PADDING && y <= rb.y()) {
//        // 右下角
//        dir = RIGHTBOTTOM;
//        this->setCursor(QCursor(Qt::SizeFDiagCursor));
//    } else if(x <= tl.x() + PADDING && x >= tl.x() && y >= rb.y() - PADDING && y <= rb.y()) {
//        //左下角
//        dir = LEFTBOTTOM;
//        this->setCursor(QCursor(Qt::SizeBDiagCursor));
//    } else if(x <= rb.x() && x >= rb.x() - PADDING && y >= tl.y() && y <= tl.y() + PADDING) {
//        // 右上角
//        dir = RIGHTTOP;
//        this->setCursor(QCursor(Qt::SizeBDiagCursor));
//    } else
    if(x <= tl.x() + PADDING && x >= tl.x()) {
        // 左边
        dir = LEFT;
        this->setCursor(QCursor(Qt::SizeHorCursor));
    } else if( x <= rb.x() && x >= rb.x() - PADDING) {
        // 右边
        dir = RIGHT;
        this->setCursor(QCursor(Qt::SizeHorCursor));
    }else if(y >= tl.y() && y <= tl.y() + PADDING){
        // 上边
        dir = UP;
        this->setCursor(QCursor(Qt::SizeVerCursor));
    } else if(y <= rb.y() && y >= rb.y() - PADDING) {
        // 下边
        dir = DOWN;
        this->setCursor(QCursor(Qt::SizeVerCursor));
    }else {
        // 默认
        dir = NONE;
        this->setCursor(QCursor(Qt::ArrowCursor));
    }
}


void MyCustomerWidget::on_btnMenu_Close_clicked()
{
    close();
}

void MyCustomerWidget::on_btnMenu_Max_clicked()
{
    if (isMax) {
        this->setGeometry(mLocation);
        ui->btnMenu_Max->setIcon(QIcon(":/res/img/showmaxsizebtn.png"));
//        ui->btnMenu_Max->setToolTip(QStringLiteral("最大化"));
        ui->widget_frame->setContentsMargins(1,1,1,1);
    } else {
        mLocation = this->geometry();
        this->setGeometry(qApp->desktop()->availableGeometry());
        ui->btnMenu_Max->setIcon(QIcon(":/res/img/shownormalbtn.png"));
//        ui->btnMenu_Max->setToolTip(QStringLiteral("还原"));
        ui->widget_frame->setContentsMargins(0,0,0,0);
    }
    isMax = !isMax;
}

void MyCustomerWidget::on_btnMenu_Min_clicked()
{
    this->showMinimized();
}

void MyCustomerWidget::setTitle(QString str)
{
    ui->lab_Title->setText(str);
    this->setWindowTitle(str);
}
