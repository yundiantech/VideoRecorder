/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "SelectAreaWidget.h"
#include "ui_SelectAreaWidget.h"

#include <QPainter>
#include <QDateTime>
#include <QDebug>

SelectAreaWidget::SelectAreaWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SelectAreaWidget)
{
qDebug()<<__FUNCTION__<<"000";
    ui->setupUi(this);
qDebug()<<__FUNCTION__<<"111";
    setWindowFlags(Qt::WindowStaysOnTopHint|Qt::FramelessWindowHint|Qt::Tool|Qt::X11BypassWindowManagerHint); //使窗口置顶
//    setWindowFlags(Qt::FramelessWindowHint|Qt::Tool|Qt::X11BypassWindowManagerHint); //使窗口置顶
    setAttribute(Qt::WA_TranslucentBackground, true);

qDebug()<<__FUNCTION__<<"444";
    mIsLeftBtnPressed = false;
    mLastUpdateTime = 0;

    mIsMakingNewArea = false;
    mIsEditing = false;

    mColor = QColor(255, 0, 0);

    ui->widget->hide();

    connect(ui->widget, &ShowAreaWdiget::sig_WindowMoved, this, &SelectAreaWidget::slotWindowMoved);
    connect(ui->widget, &ShowAreaWdiget::sig_DoubleClicked, this, &SelectAreaWidget::slotWindowDoubleClicked);
qDebug()<<__FUNCTION__<<"555";
    setMouseTracking(true);
qDebug()<<__FUNCTION__<<"666";
    showFullScreen();
qDebug()<<__FUNCTION__<<"999";
}

SelectAreaWidget::~SelectAreaWidget()
{
    delete ui;
}

void SelectAreaWidget::setRect(const QRect &rect)
{
    mRect = rect;

    ui->widget->setGeometry(mRect);
//    ui->widget->show();

    update();
}

void SelectAreaWidget::setEditMode(const bool &value)
{
    mIsEditing = value;

    if (mIsEditing)
    {
        ui->widget->show();
    }
    else
    {
        ui->widget->hide();
    }

    update();
}

void SelectAreaWidget::getReadyToSelect()
{
    showFullScreen();
    setCursor(Qt::CrossCursor);
    mIsMakingNewArea = true;
    mRect = QRect(0, 0, 0, 0);
    setEditMode(true);
    ui->widget->hide();
//    update();
}

bool SelectAreaWidget::isRectAvailable()
{
    bool isAvailable = false;

    if (mRect.width() >= 100 && mRect.height() >= 100)
    {
       isAvailable = true;
    }

    return isAvailable;
}

void SelectAreaWidget::setColor(const QColor &color)
{
    mColor = color;
    update();
}

void SelectAreaWidget::setInTop(const bool &top)
{
    if (top)
    {
        setWindowFlags(Qt::WindowStaysOnTopHint|Qt::FramelessWindowHint|Qt::Tool|Qt::X11BypassWindowManagerHint); //使窗口置顶
    }
    else
    {
        setWindowFlags(Qt::FramelessWindowHint|Qt::Tool|Qt::X11BypassWindowManagerHint); //使窗口置顶
    }
    this->show();
}

void SelectAreaWidget::paintEvent(QPaintEvent *event)
{
//qDebug()<<__FUNCTION__<<"000";
    QPainter painter(this);

    if (mIsEditing)
    {
        int alpha = 2;

        painter.save();

        painter.setBrush(QBrush(QColor(0, 0, 0, alpha)));
        painter.drawRect(this->geometry());

        painter.restore();
    }

    //绘制选中区域透明背景
    if (mIsEditing)
    {
        painter.save();

        QColor m_qColor = QColor(0, 0, 0, 100);

        painter.setRenderHints(QPainter::Antialiasing,true);
        painter.setRenderHints(QPainter::SmoothPixmapTransform, true);
        QPainterPath path;
        path.addRect(mRect); //此区域内背景透明

        ///绘制阴影背景
        path.moveTo(0, 0);
        path.addRect(this->geometry());
        painter.setBrush(QBrush(m_qColor,Qt::SolidPattern));
        painter.setPen(m_qColor);
        painter.drawPath(path);

        painter.restore();
    }

    //绘制红色矩形框
    if (mIsMakingNewArea || !mIsEditing)
    {
        painter.save();

        QPen pen;
        pen.setColor(mColor);
        pen.setWidth(RECT_WIDTH);

        painter.setPen(pen);
        painter.drawRect(mRect);

        painter.restore();
    }

//    QStyleOption opt;
//    opt.init(this);
//    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
//qDebug()<<__FUNCTION__<<"999";
}

void SelectAreaWidget::mousePressEvent(QMouseEvent *event)
{
//qDebug()<<__FUNCTION__<<mIsMakingNewArea<<mRect;
    if (!mIsMakingNewArea) return;

//    bool mIsNeedMode = false; //是否为重新选一个区域

//    if (mRect.width() <= 0 || mRect.height() <= 0)
//    {
//        mIsNeedMode = true;
//    }

//    if (mIsNeedMode)
    if (event->button() == Qt::LeftButton)
    {
        mStartPoint = event->pos();
        mIsLeftBtnPressed = true;
    }
}

void SelectAreaWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
//    qDebug()<<__FUNCTION__;
    if (mIsEditing)
    {
        if (isRectAvailable())
        {
            setEditMode(false);
            emit sig_SelectFinished(mRect);
        }
    }
}

void SelectAreaWidget::mouseReleaseEvent(QMouseEvent *event)
{
    mIsLeftBtnPressed = false;

    if (mIsMakingNewArea)
    {
        if (isRectAvailable())
        {
            mIsMakingNewArea = false;
            setRect(mRect);
            ui->widget->show();
            setCursor(Qt::ArrowCursor);
        }
        else
        {
            mRect = QRect(0, 0, 0, 0);
            update();

            setCursor(Qt::CrossCursor);
        }
    }



//qDebug()<<__FUNCTION__;
}

void SelectAreaWidget::mouseMoveEvent(QMouseEvent *event)
{
//    if (!mIsEditAble) return;

    if (mIsLeftBtnPressed)
    {
        if ((QDateTime::currentMSecsSinceEpoch() - mLastUpdateTime) > 40)
        {
            mLastUpdateTime = QDateTime::currentMSecsSinceEpoch();

            int x = mStartPoint.x();
            int y = mStartPoint.y();
            int w = event->pos().x() - x;
            int h = event->pos().y() - y;

            if (w < 0)
            {
                w = abs(w);
                x = x - w;
            }

            if (h < 0)
            {
                h = abs(h);
                y = y - h;
            }

            mRect = QRect(x, y, w, h);

//    qDebug()<<mIsLeftBtnPressed<<mStartPoint<<event->pos()<<mRect;
            update();
        }
    }
//    else
//    {
//        if (mIsMakingNewArea)
//        {

//        }
//        else
//        {
//            if (mIsEditAble)
//            {
//                if (mRect.contains(event->pos()))
//                {
//                    setCursor(Qt::SizeAllCursor);
//                }
//                else
//                {
//                    setCursor(Qt::ArrowCursor);
//                }
//            }
//        }
//    }
}

void SelectAreaWidget::slotWindowMoved(const QRect rect)
{
    setRect(rect);
}

void SelectAreaWidget::slotWindowDoubleClicked()
{
    if (mIsEditing)
    {
        setEditMode(false);
        emit sig_SelectFinished(mRect);
    }
}
