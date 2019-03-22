/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    av_register_all();
    avformat_network_init();
    avdevice_register_all();

    mAudioThread = NULL;

    connect(ui->pushButton_start,SIGNAL(clicked()),this,SLOT(slotBtnClicked()));
    connect(ui->pushButton_stop,SIGNAL(clicked()),this,SLOT(slotBtnClicked()));

    ui->pushButton_start->setEnabled(true);
    ui->pushButton_stop->setEnabled(false);

}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::startRecord()
{
    bool isSucceed = false;
do
{
    if (mAudioThread != NULL)
        delete mAudioThread;

    mAudioThread = new GetAudioThread;

    /// 使用ffmepg命令行获取录音设备 然后拷贝进来：
    /// ffmpeg -list_devices true -f dshow -i dummy  2>E:/out.txt
    /// 打开E:\out.txt即可看到设备
    if (mAudioThread->init("插孔麦克风 (Realtek Audio)") == SUCCEED)
    {
        mAudioThread->startRecord();
    }
    else
    {
        QMessageBox::critical(NULL,"提示","出错了,初始化录屏设备失败！");

        break;
    }

    ui->pushButton_start->setEnabled(false);
    ui->pushButton_stop->setEnabled(true);

    isSucceed = true;

    ui->pushButton_start->setEnabled(false);
    ui->pushButton_stop->setEnabled(true);

}while(0);

    return isSucceed;
}

void MainWindow::stopRecord()
{
    if (mAudioThread != NULL)
        mAudioThread->stopRecord();
    ui->pushButton_start->setEnabled(true);
    ui->pushButton_stop->setEnabled(false);
}

void MainWindow::slotBtnClicked()
{
    if (QObject::sender() == ui->pushButton_start)
    {
        startRecord();
    }
    else if (QObject::sender() == ui->pushButton_stop)
    {
        stopRecord();
    }
}
