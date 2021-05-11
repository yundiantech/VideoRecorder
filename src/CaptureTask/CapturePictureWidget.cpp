/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "CapturePictureWidget.h"
#include "ui_CapturePictureWidget.h"

#include <QScreen>

CapturePictureWidget::CapturePictureWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CapturePictureWidget)
{
    ui->setupUi(this);

    mId = 0;

}

CapturePictureWidget::~CapturePictureWidget()
{
    delete ui;
}

void CapturePictureWidget::setTask(const int &id, const QRect &capRect)
{
    mId      = id;
    mCapRect = capRect;
}

void CapturePictureWidget::setFilePath(const QString &filePath)
{
    mFilePath = filePath;

    doUpdate();
}

void CapturePictureWidget::doUpdate()
{
    QPixmap pixmap = QPixmap(mFilePath);

    ui->label->setPixmap(pixmap);
}

