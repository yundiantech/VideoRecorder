/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef DRAGABLEDIALOG_H
#define DRAGABLEDIALOG_H

#include <QWidget>
#include <QTimer>
#include <QDialog>

namespace Ui {
class DragAbleDialog;
}

class DragAbleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DragAbleDialog(QWidget *parent = 0);
    ~DragAbleDialog();

    QWidget *getContainWidget();

    void setTitle(QString str);

    void doShowMaxSize();
    void doShowFullScreen();
    void doShowNormal();

private:
    Ui::DragAbleDialog *ui;

    QTimer *mTimer;

    ///以下是改变窗体大小相关
    ////////
protected:
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

#endif // DRAGABLEDIALOG_H
