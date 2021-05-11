/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef SELECTRUNNINGPROGRAMEDIALOG_H
#define SELECTRUNNINGPROGRAMEDIALOG_H

#include <QDialog>

#include "ShowProgramPictureWidget.h"

#include "Widget/CustomWidget/flowlayout.h"

namespace Ui {
class SelectRunningProgramDialog;
}

class SelectRunningProgramDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectRunningProgramDialog(QWidget *parent = nullptr);
    ~SelectRunningProgramDialog();

    void addIgnorHandle(HWND handle);
    void init();

    HWND getSelectedHandle();
    QRect getProgramRect();

private:
    Ui::SelectRunningProgramDialog *ui;

    ShowProgramPictureWidget *mSelectedWidget;

    FlowLayout *mFlowLayout;
    QList<ShowProgramPictureWidget*> mShowProgramPictureWidgetList;

    QList<HWND> mIgnoreHandleList; //忽略列表

private slots:
    void slotItemClicked(HWND handle);

};

#endif // SELECTRUNNINGPROGRAMEDIALOG_H
