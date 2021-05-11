/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef CAPTUREPICTUREWIDGET_H
#define CAPTUREPICTUREWIDGET_H

#include <QWidget>

#include "widget/customWidget/MyCustomerWidget.h"

namespace Ui {
class CapturePictureWidget;
}

class CapturePictureWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CapturePictureWidget(QWidget *parent = nullptr);
    ~CapturePictureWidget();

    void setTask(const int &id, const QRect &capRect);
    int getId(){return mId;}

    QRect getCaptureRect(){return mCapRect;}

    void setFilePath(const QString &filePath);
    QString getFilePath(){return mFilePath;}

private:
    Ui::CapturePictureWidget *ui;

    int mId;

    QString mFilePath;

    QRect mCapRect; //采集的区域

    void doUpdate();

};

#endif // CAPTUREPICTUREWIDGET_H
