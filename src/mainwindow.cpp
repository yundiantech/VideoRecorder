
/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    int devNums = waveInGetNumDevs();

    for(int i=0;i<devNums;i++)
    {
        WAVEINCAPSW p;
        waveInGetDevCaps(i,&p,sizeof(WAVEINCAPS));
        ui->comboBox_audiodeviceList->addItem(QString::fromWCharArray(p.szPname));
    }

    m_screenRecorder = new ScreenRecorder;

    connect(ui->pushButton_start,SIGNAL(clicked()),this,SLOT(slotBtnClick()));
    connect(ui->pushButton_stop,SIGNAL(clicked()),this,SLOT(slotBtnClick()));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::slotBtnClick()
{
    if (QObject::sender() == ui->pushButton_start)
    {

        ErroCode code = m_screenRecorder->init(ui->comboBox_audiodeviceList->currentText());

        if (code == SUCCEED)
        {
            m_screenRecorder->startRecord();
            ui->pushButton_start->setEnabled(false);
            ui->pushButton_stop->setEnabled(true);
//            SDL_CreateThread(putAudioThread, "parse_thread", NULL);
        }
        else
        {
            QMessageBox::critical(NULL,"提示","出错了,初始化失败。");
        }
    }
    else if (QObject::sender() == ui->pushButton_stop)
    {
        m_screenRecorder->stopRecord();
        ui->pushButton_stop->setEnabled(false);
        ui->pushButton_start->setEnabled(true);
    }
}
