/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef SHOWPROGRAMPICTUREWIDGET_H
#define SHOWPROGRAMPICTUREWIDGET_H

#include <QWidget>
#include <QMouseEvent>

namespace Ui {
class ShowProgramPictureWidget;
}

class ShowProgramPictureWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ShowProgramPictureWidget(QWidget *parent = nullptr);
    ~ShowProgramPictureWidget();

    void setHandle(HWND handle);
    HWND getHandle(){return mHWnd;}

    QRect getRect(){return mRect;}

    void setTitle(const QString &title);

signals:
    void sig_Clicked(HWND handle);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    Ui::ShowProgramPictureWidget *ui;

    HWND mHWnd;
    QRect mRect;

};

#endif // SHOWPROGRAMPICTUREWIDGET_H
