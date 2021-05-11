/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "ShowRedRectWidget.h"
#include "ui_ShowRedRectWidget.h"

ShowRedRectWidget::ShowRedRectWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShowRedRectWidget)
{
    ui->setupUi(this);

    this->setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::WindowStaysOnTopHint|Qt::FramelessWindowHint|Qt::Tool|Qt::X11BypassWindowManagerHint); //使窗口置顶
//    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint); //使窗口标题栏隐藏

}

ShowRedRectWidget::~ShowRedRectWidget()
{
    delete ui;
}

