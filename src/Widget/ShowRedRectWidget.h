/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef SHOWREDRECTWIDGET_H
#define SHOWREDRECTWIDGET_H

#include <QWidget>

namespace Ui {
class ShowRedRectWidget;
}

class ShowRedRectWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ShowRedRectWidget(QWidget *parent = 0);
    ~ShowRedRectWidget();

private:
    Ui::ShowRedRectWidget *ui;
};

#endif // SHOWREDRECTWIDGET_H
