#include "myscrollarea.h"
#include "ui_myscrollarea.h"

#include <QDebug>
#include <QResizeEvent>

MyScrollArea::MyScrollArea(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MyScrollArea)
{
    ui->setupUi(this);

    popMenu = new QMenu;

    popOutAction = new QAction("窗口弹出",this);
    popOutAction->setCheckable(true);
    popMenu->addAction(popOutAction);
//    popMenu->addSeparator();       //添加分离器

    connect(popOutAction,SIGNAL(triggered(bool)),this,SLOT(slotActionTriggered(bool)));

    ui->widget_toolBtn->hide();
    ui->widget_video->setUpdatesEnabled(false);

    connect(ui->pushButton_popout,SIGNAL(clicked()),this,SLOT(slotBtnClick()));

}

MyScrollArea::~MyScrollArea()
{
    delete ui;
}

void MyScrollArea::setLoading(bool value)
{
    ui->widget_contain->setVisible(!value);
    ui->label_loading->setVisible(value);
}

void MyScrollArea::resizeEvent(QResizeEvent* event)
{
//    ui->widget_contain->resize(event->size());
//    ui->label_loading->setMinimumSize(event->size());

    qDebug()<<"void MyScrollArea::resizeEvent(QResizeEvent* event)"<<event->size();

    if (this->isVisible())
    {
        emit resizeSignal(event->size());
    }


}

void MyScrollArea::enterEvent(QEvent*)
{
//    if (this->parent() == NULL) return;
//    ui->widget_toolBtn->show();
}

void MyScrollArea::leaveEvent(QEvent*)
{
//    ui->widget_toolBtn->hide();
}

QWidget * MyScrollArea::getVideoContainWidget()
{
    return ui->widget_video;
}

void MyScrollArea::mousePressEvent(QMouseEvent*event)
{
    if (event->button() == Qt::RightButton)
    {
//        popMenu->exec(QCursor::pos());
    }
}

void MyScrollArea::closeEvent(QCloseEvent *event)
{
    popOutAction->setChecked(false);
    emit btnClick(popIn);

    event->ignore();

}

void MyScrollArea::setVideoWidth(int width,int height)
{
    ui->widget_contain->setMinimumSize(width,height);
    ui->widget_contain->setMaximumSize(width,height);

//    ui->widget_video->resize(width,height);

//    ui->widget_video->setMinimumSize(width,height);
//    ui->widget_video->setMaximumSize(width,height);
}


void MyScrollArea::slotBtnClick()
{
    if (QObject::sender() == ui->pushButton_popout)
    {
        ui->widget_toolBtn->hide();
        emit btnClick(popOut);
    }
}

void MyScrollArea::slotActionTriggered(bool isTriggered)
{
    if (QObject::sender() == popOutAction)
    {
        if (isTriggered)
        {
            emit btnClick(popOut);
        }
        else
        {
            emit btnClick(popIn);
        }
    }
}
