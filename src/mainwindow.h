/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "audio/getaudiothread.h"

namespace Ui {
class MainWindow;
}

/**
 * @brief The MainWindow class 界面操作相关的主窗体类
 */

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    GetAudioThread *mAudioThread; //采集麦克风的线程

    bool startRecord();
    void stopRecord();

private slots:
    void slotBtnClicked();
};

#endif // MAINWINDOW_H
