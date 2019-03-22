#ifndef MYSCROLLAREA_H
#define MYSCROLLAREA_H

#include <QMenu>
#include <QWidget>

namespace Ui {
class MyScrollArea;
}

class MyScrollArea : public QWidget
{
    Q_OBJECT

public:

    enum BtnClickType
    {
        popOut = 0,
        popIn,
    };

    explicit MyScrollArea(QWidget *parent = 0);
    ~MyScrollArea();

    void setLoading(bool);

    QWidget * getVideoContainWidget();
    void setVideoWidth(int width,int height);

signals:
    void btnClick(MyScrollArea::BtnClickType);
    void resizeSignal(QSize size);  //回显视频的窗口改变了大小
protected:
    void resizeEvent(QResizeEvent* event);
    void enterEvent(QEvent*);
    void leaveEvent(QEvent*);
    void mousePressEvent(QMouseEvent*);
    void closeEvent(QCloseEvent *);

private:
    Ui::MyScrollArea *ui;

    QMenu*popMenu;
    QAction *popOutAction;

private slots:
    void slotBtnClick();
    void slotActionTriggered(bool);

};

#endif // MYSCROLLAREA_H
