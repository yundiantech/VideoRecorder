/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef DRAGABLEWIDGET_H
#define DRAGABLEWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QWidget>

namespace Ui {
class DragAbleWidget;
}

class DragAbleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DragAbleWidget(QWidget *parent = 0);
    ~DragAbleWidget();

    QWidget *getContainWidget();

    void setTitle(QString str);

    void doShowMaxSize();
    void doShowFullScreen();
    void doShowNormal();

private:
    Ui::DragAbleWidget *ui;

    QTimer *mTimer;

    ///以下是改变窗体大小相关
    ////////
protected:
//    bool eventFilter(QObject *obj, QEvent *event);
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

    void checkCursorDirect(const QPoint &cursorGlobalPoint);

    void showBorderRadius(bool isShow);
    void doChangeFullScreen();
    void doChangeMaxSize();

private slots:
    void slotTimerTimeOut();

};

#endif // DRAGABLEWIDGET_H
