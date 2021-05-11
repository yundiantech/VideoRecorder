/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef SELECTAREAWIDGET_H
#define SELECTAREAWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QPaintEvent>

#define RECT_WIDTH 2

namespace Ui {
class SelectAreaWidget;
}

class SelectAreaWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SelectAreaWidget(QWidget *parent = nullptr);
    ~SelectAreaWidget();

    void getReadyToSelect();
    void setRect(const QRect &rect);
    void setEditMode(const bool &value);

    bool isRectAvailable();

    void setInTop(const bool &top);

    void setColor(const QColor &color);

signals:
    void sig_SelectFinished(const QRect &rect);

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    Ui::SelectAreaWidget *ui;

    QColor mColor;

    bool mIsMakingNewArea; //正在选择新区域
    bool mIsEditing;

    QPoint mStartPoint; //记录鼠标左键开始按下时候的位置
    bool mIsLeftBtnPressed;
    QRect mRect;

    qint64 mLastUpdateTime; //用来显示刷新频率

private slots:
    void slotWindowMoved(const QRect rect);
    void slotWindowDoubleClicked();
};

#endif // SELECTAREAWIDGET_H
