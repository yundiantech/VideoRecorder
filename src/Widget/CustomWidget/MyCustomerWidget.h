/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef MYCUSTOMERWIDGET_H
#define MYCUSTOMERWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QDesktopWidget>

//鼠标实现改变窗口大小
#define PADDING 6

namespace Ui {
class MyCustomerWidget;
}

class MyCustomerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MyCustomerWidget(QWidget *parent = 0);
    ~MyCustomerWidget();

//    void addWidget(QWidget *widget);
    QWidget *getContainWidget();
    void setActive();

    void setRate(float value){mRate = value;}
    void setLimitRect(QRect rect){mLimitRect = rect;}

signals:
    void sig_WindowMoved(QRect rect);
    void sig_WindowMoveFinished(QRect rect);
    void sig_MouseRightBtnClick();
//    void sig_WindowResized(QRect rect);

protected:
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);

    void keyPressEvent(QKeyEvent *event);

private:
    Ui::MyCustomerWidget *ui;

///以下是改变窗体大小相关  不知道为何 做成继承的方式 会莫名奔溃 只能在每个窗体里面加上这个了
    ////////
protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);

private:
    bool isMax; //是否最大化
    QRect mLocation;

    bool mIsResizeMode;
    bool isLeftPressDown;  // 判断左键是否按下
    QPoint dragPosition;   // 窗口移动拖动时需要记住的点
    int dir;        // 窗口大小改变时，记录改变方向

    float mRate;   //窗口比例
    QRect mLimitRect; //限制区域

    void setTitle(QString str);
    void checkCursorDirect(const QPoint &cursorGlobalPoint);

private slots:
    void on_btnMenu_Close_clicked();
    void on_btnMenu_Max_clicked();
    void on_btnMenu_Min_clicked();

};

#endif // MYCUSTOMERDIALOG_H
