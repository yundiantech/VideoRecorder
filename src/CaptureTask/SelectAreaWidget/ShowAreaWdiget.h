/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef ShowAreaWdiget_H
#define ShowAreaWdiget_H

#include <QWidget>
#include <QTimer>
#include <QDialog>

namespace Ui {
class ShowAreaWdiget;
}

//鼠标实现改变窗口大小
#define PADDING 6
enum Direction { UP=0, DOWN, LEFT, RIGHT, LEFTTOP, LEFTBOTTOM, RIGHTBOTTOM, RIGHTTOP, NONE };


class ShowAreaWdiget : public QWidget
{
    Q_OBJECT

public:
    explicit ShowAreaWdiget(QWidget *parent = 0);
    ~ShowAreaWdiget();

signals:
    void sig_WindowMoved(const QRect rect);
    void sig_DoubleClicked();

private:
    Ui::ShowAreaWdiget *ui;

    QTimer *mTimer;

    ///以下是改变窗体大小相关
    ////////
protected:
//    bool eventFilter(QObject *obj, QEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

private:
    bool isMax; //是否最大化
    QRect mLocation;

    bool mIsResizeMode;
    bool isLeftPressDown;  // 判断左键是否按下
    QPoint dragPosition;   // 窗口移动拖动时需要记住的点
    int dir;        // 窗口大小改变时，记录改变方向

    void checkCursorDirect(const QPoint &cursorGlobalPoint);

private slots:
    void slotTimerTimeOut();

};

#endif // ShowAreaWdiget_H
