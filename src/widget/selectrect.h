
/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef SELECTRECT_H
#define SELECTRECT_H

#include <QLabel>
#include <QMouseEvent>
#include "pushpoint.h"

/*选取截图范围*/

class SelectRect : public QLabel
{
    Q_OBJECT
    
public:
    enum Mode //模式
    {
        ScreenShot = 0, //截图模式
        RecordGif = 1 //录屏模式
    };
    explicit SelectRect(QWidget *parent = 0,Mode m = ScreenShot);
    ~SelectRect();

    void setPointHide();
    void setMode(Mode m){runningMode = m;}
    void getReadyToSelect();
    void editRect();
    void setRate(float value){rate=value;}
    void setRect(QRect re){rect = re;updateRect(); emit finished(rect);}
protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
signals:
    void rectChanged(QRect); //范围发生改变(提供给截图使用)
    void releaseWithoutMove();//按下鼠标没移动就放开了(提供给截图使用:选中自动选择的区域)
    void finished(QRect);    //区域选取完毕(提供给录屏使用)
private:
    Mode runningMode;

    QRect rect;
    QRect rectangle;
    float rate;

    QPoint dragPosition;
    QPoint zeroPoint;

    bool FINISHED;
    bool DRAG;
    bool RELEASE;

    QLabel *m_showBorderLabel;

    PushPoint *locPoint[8]; //改变边框用的八个点
    void initLocationPoint();
    void layoutLocPoint(); //布局显示范围的8个点
    void updateRect();//更新显示矩形区域
private slots:
    void slotPointMoved(QPoint); //点被移动了
};

#endif // SELECTRECT_H
