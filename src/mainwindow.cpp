
/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QUrl>
#include <QTimer>
#include <QProcess>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QDesktopServices>

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    av_register_all();
    avformat_network_init();
    avdevice_register_all();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        fprintf(stderr,"Could not initialize SDL - %s. \n", SDL_GetError());
        exit(1);
    }

    setWindowFlags(Qt::WindowStaysOnTopHint|Qt::FramelessWindowHint);  //使窗口的标题栏隐藏
    setAttribute(Qt::WA_TranslucentBackground, true);

#if (QT_VERSION <= QT_VERSION_CHECK(5,0,0))
    AppDataPath = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#else
    AppDataPath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
#endif

    QString dirName = AppDataPath + "\\ScreenRecorder\\etc";
    SettingFile = dirName + "\\set.conf";

    dirName.replace("/","\\");

    QDir dir;
    dir.mkpath(dirName);

    connect(ui->startButton,SIGNAL(clicked()),this,SLOT(slotBtnClicked()));
    connect(ui->pauseButton,SIGNAL(clicked()),this,SLOT(slotBtnClicked()));
    connect(ui->stopButton,SIGNAL(clicked()),this,SLOT(slotBtnClicked()));
    connect(ui->pushButton_playBack,SIGNAL(clicked()),this,SLOT(slotBtnClicked()));

    connect(ui->selectRectButton,SIGNAL(clicked()),this,SLOT(slotSelectRectBtnClick()));
    connect(ui->editRectButton,SIGNAL(clicked()),this,SLOT(slotEditRectBtnClick()));
    connect(ui->hideRectButton,SIGNAL(clicked()),this,SLOT(slotHideRectBtnClick()));

    m_screenRecorder = NULL;
    isLeftBtnPressed = false;
    m_recordeState = Stop;

    rect = QRect(0,0,0,0);

    selectRectWidget = new SelectRect(NULL,SelectRect::RecordGif);
    QDesktopWidget* desktopWidget = QApplication::desktop();
    deskRect = desktopWidget->screenGeometry();//获取可用桌面大小
    m_rate = deskRect.height() * 1.0 / deskRect.width();
    selectRectWidget->setRate(m_rate);

    connect(selectRectWidget,SIGNAL(finished(QRect)),this,SLOT(slotSelectRectFinished(QRect)));
    connect(selectRectWidget,SIGNAL(rectChanged(QRect)),this,SLOT(slotSelectRectFinished(QRect)));

    initDev();
    loadFile();

    connect(ui->toolButton_video,SIGNAL(clicked(bool)),this,SLOT(slotToolBtnToggled(bool)));
    connect(ui->toolButton_audio,SIGNAL(clicked(bool)),this,SLOT(slotToolBtnToggled(bool)));
    connect(ui->toolButton_file,SIGNAL(clicked(bool)),this,SLOT(slotToolBtnToggled(bool)));

    ///动画类 用来实现窗体从上方慢慢出现
    animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(1000);

    ui->toolButton_setting->setChecked(false);
    ui->widget_extern->hide();
    QTimer::singleShot(50,this,SLOT(showOut()));

    m_timer = new QTimer;
    connect(m_timer,SIGNAL(timeout()),this,SLOT(slotTimerTimeOut()));
    m_timer->setInterval(500);

    connect(ui->checkBox,SIGNAL(clicked(bool)),this,SLOT(slotCheckBoxClick(bool)));

    if (ui->toolButton_video->isChecked())
    {
        selectRectWidget->show();
        selectRectWidget->setPointHide();
        ui->hideRectButton->setText(QStringLiteral("隐藏"));
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    delete selectRectWidget;
}

void MainWindow::showOut()
{
    show();
    move(deskRect.width() / 2 - width() / 2,0-height());
    animation->setStartValue(QRect(deskRect.width() / 2 - width() / 2,0-height(),width(),height()));
    animation->setEndValue(QRect(deskRect.width() / 2 - width() / 2,0,width(),height()));
    animation->start();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_screenRecorder)
    {
        m_screenRecorder->stopRecord();
    }

    selectRectWidget->close();
}

void MainWindow::mousePressEvent(QMouseEvent * event)
{
    if (event->button() == Qt::LeftButton)
    {
        isLeftBtnPressed = true;
         dragPosition=event->globalPos()-frameGeometry().topLeft();
         event->accept();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent * event)
{  //实现鼠标移动窗口
    if (event->buttons() & Qt::LeftButton)
    {
        if (isLeftBtnPressed)
        {
            move(event->globalPos() - dragPosition);
            event->accept();
        }
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent * event)
{
    isLeftBtnPressed = false;
    event->accept();
}

void MainWindow::setSaveFile(QString fileName)
{

    QString str = fileName;

    saveFileName = str.replace("/","\\\\");

    ui->lineEdit_filename->setText(saveFileName);
    QString dirName = saveFileName.left(saveFileName.lastIndexOf("\\")-1);

    QDir dir;
    dir.mkpath(dirName);

    saveFile();
}

void MainWindow::loadFile()
{

    QFile file(SettingFile);
    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream fileOut(&file);
        fileOut.setCodec("GBK");  //unicode UTF-8  ANSI
        while (!fileOut.atEnd())
        {
            QString str = fileOut.readLine();
//            qDebug()<<str;

            str = str.remove(" ");

            if (str.isEmpty())
            {
                continue;
            }

            if (str.contains("area="))
            {
                str = str.remove("area=");

                QStringList strList = str.split(",");

                if (strList.size() == 4)
                {
                    rect = QRect(strList.at(0).toInt(),strList.at(1).toInt(),strList.at(2).toInt(),strList.at(3).toInt());
                }
            }
            else if (str.contains("mode="))
            {
                str = str.remove("mode=");
                if (str == "video")
                {
                    ui->toolButton_video->setChecked(true);
                }
                else
                {
                    ui->toolButton_audio->setChecked(true);
                    ui->startButton->setEnabled(true);
                }
            }
            else if (str.contains("framerate="))
            {
                str = str.remove("framerate=");

                int rate = str.toInt();

                if (rate > 0 && rate <= 25)
                {
                    ui->comboBox_framerate->setCurrentIndex(rate-1);
                }
            }
            else if (str.contains("filename="))
            {
                str = str.remove("filename=");
                str.replace("/","\\\\");
                saveFileName = str;
                ui->lineEdit_filename->setText(saveFileName);

                QString dirName = saveFileName.left(saveFileName.lastIndexOf("\\")-1);

                QDir dir;
                dir.mkpath(dirName);
            }
        }
        file.close();
    }
    else
    {
        ui->toolButton_video->setChecked(true);
    }

    if (rect.width() <= 0 || rect.height() <= 0 || !deskRect.contains(rect))
    {
        rect = deskRect;
    }

    selectRectWidget->setRect(rect);
    ui->startButton->setEnabled(true);
    selectRectWidget->setVisible(false);
    ui->hideRectButton->setText(QStringLiteral("显示"));
    ui->startButton->setEnabled(true);
}

void MainWindow::saveFile()
{

//    QDir dir(AppDataPath);
//    dir.mkdir("etc");

    QFile file(SettingFile);
    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream fileOut(&file);
        fileOut.setCodec("GBK");  //unicode UTF-8  ANSI

        if (!ui->comboBox_audio->currentText().isEmpty())
        {
            fileOut<<QString("audio=%1").arg(ui->comboBox_audio->currentText());
            fileOut<<"\n";
        }

        QString areaStr = QString("area=%1,%2,%3,%4").arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height());
        QString modeStr = "mode=audio";
        if (ui->toolButton_video->isChecked())
        {
            modeStr = "mode=video";
        }

        QString fileStr = QString("filename=%1").arg(saveFileName);
        QString rateStr = QString("framerate=%1").arg(ui->comboBox_framerate->currentText().toInt());

        fileOut<<areaStr;
        fileOut<<"\n";
        fileOut<<modeStr;
        fileOut<<"\n";
        fileOut<<rateStr;
        fileOut<<"\n";
        fileOut<<fileStr;
        fileOut<<"\n";

        file.close();
    }
}

void MainWindow::initDev()
{
    QString videoDevName;
    QString audioDevName;
    QFile devFile(SettingFile);
    if (devFile.open(QIODevice::ReadOnly))
    {
        QTextStream fileOut(&devFile);
        fileOut.setCodec("GBK");  //unicode UTF-8  ANSI

        while (!fileOut.atEnd())
        {
            QString str = fileOut.readLine();
            str = str.remove("\r");
            str = str.remove("\n");
            if (str.contains("video="))
            {
                videoDevName = str.remove("video=");
            }
            else if (str.contains("audio="))
            {
                audioDevName = str.remove("audio=");
            }
        }

        devFile.close();
    }

    /// 执行ffmpeg命令行 获取音视频设备
    /// 请将ffmpeg.exe和程序放到同一个目录下

    QString ffmpegPath = QCoreApplication::applicationDirPath() + "/ffmpeg.exe";
    ffmpegPath.replace("/","\\\\");
    ffmpegPath = QString("\"%1\" -list_devices true -f dshow -i dummy 2>a.txt \n").arg(ffmpegPath);

     QProcess p(0);
     p.start("cmd");
     p.waitForStarted();
     p.write(ffmpegPath.toLocal8Bit());
     p.closeWriteChannel();
     p.waitForFinished();


    QFile file("a.txt");
    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream fileOut(&file);
        fileOut.setCodec("UTF-8");  //unicode UTF-8  ANSI

        bool isVideoBegin = false;
        bool isAudioBegin = false;

        while (!fileOut.atEnd())
        {
            QString str = fileOut.readLine();

            if (str.contains("DirectShow video devices") && str.contains("[dshow @"))
            {
                isVideoBegin = true;
                isAudioBegin = false;
                continue;
            }

            if (str.contains("DirectShow audio devices") && str.contains("[dshow @"))
            {
                isAudioBegin = true;
                isVideoBegin = false;
                continue;
            }

            if (str.contains("[dshow @"))
            {
                int index = str.indexOf("\"");
                str = str.remove(0,index);
                str = str.remove("\"");
                str = str.remove("\n");
                str = str.remove("\r");

                if (isVideoBegin)
                {
//                    if ("screen-capture-recorder" != str)
//                    ui->comboBox_camera->addItem(str);
                }
                else if (isAudioBegin)
                {
                    if ("virtual-audio-capturer" != str)
                    ui->comboBox_audio->addItem(str);
                }
            }

        }

        file.close();
    }
    else
    {
        qDebug()<<"open a.txt failed!";
    }

    QFile::remove("a.txt");

    for (int i=0;i<ui->comboBox_audio->count();i++)
    {
        if (ui->comboBox_audio->itemText(i) == audioDevName)
        {
            ui->comboBox_audio->setCurrentIndex(i);
            break;
        }
    }

}

void MainWindow::slotBtnClicked()
{
    if (QObject::sender() == ui->startButton)
    {
        startRecord();
    }
    else if (QObject::sender() == ui->pauseButton)
    {
        pauseRecord();
    }
    else if (QObject::sender() == ui->stopButton)
    {
        stopRecord();
    }
    else if (QObject::sender() == ui->pushButton_playBack)
    {
        QString path = saveFileName;
        path.replace("\\\\","/");
        path="file:///" + path;
        qDebug()<<QUrl(path)<<path;
        QDesktopServices::openUrl(QUrl(path));
    }
}

void MainWindow::slotToolBtnToggled(bool isChecked)
{
    if (QObject::sender() == ui->toolButton_video)
    {
        if (isChecked)
        {
            ui->toolButton_audio->setChecked(!isChecked);
            selectRectWidget->show();
        }
        else
        {
            ui->toolButton_video->setChecked(!isChecked);
        }
    }
    else if (QObject::sender() == ui->toolButton_audio)
    {
        if (isChecked)
        {
            ui->toolButton_video->setChecked(!isChecked);
            selectRectWidget->hide();
        }
        else
        {
            ui->toolButton_audio->setChecked(!isChecked);
        }
    }
    else if (QObject::sender() == ui->toolButton_file)
    {
        QString s = QFileDialog::getSaveFileName(
                   this, QStringLiteral("选择保存文件的路径"),
                       saveFileName,//初始目录
                    QStringLiteral("视频文件 (*.mp4);;"));
         if (!s.isEmpty())
         {
             saveFileName = s.replace("/","\\\\");

             ui->lineEdit_filename->setText(saveFileName);

             saveFile();
         }
    }
}

void MainWindow::slotSelectRectFinished(QRect re)
{
    /// 1.传给ffmpeg编码的图像宽高必须是偶数。
    /// 2.图像裁剪的起始位置和结束位置也必须是偶数
    /// 而手动选择的区域很有可能会是奇数，因此需要处理一下 给他弄成偶数
    /// 处理的方法很简答：其实就是往前或者往后移一个像素
    /// 一个像素的大小肉眼基本也看不出来啥区别。

    int x = re.x();
    int y = re.y();
    int w = re.width();
    int h = re.height();

    if (x % 2 != 0)
    {
        x--;
        w++;
    }

    if (y % 2 != 0)
    {
        y--;
        h++;
    }

    if (w % 2 != 0)
    {
        w++;
    }

    if (h % 2 != 0)
    {
        h++;
    }

    rect = QRect(x,y,w,h);

    QString str = QStringLiteral("==当前区域==\n\n起点(%1,%2)\n\n大小(%3 x %4)")
            .arg(rect.left()).arg(rect.left()).arg(rect.width()).arg(rect.height());

    ui->showRectInfoLabel->setText(str);

    ui->startButton->setEnabled(true);
    ui->editRectButton->setEnabled(true);
    ui->hideRectButton->setEnabled(true);
    ui->hideRectButton->setText(QStringLiteral("隐藏"));

    saveFile();

}

bool MainWindow::startRecord()
{
    int ret = 0;
    QString msg = "ok";
    if (m_recordeState == Recording)
    {
        ret = -1;
        msg = "is already start!";
    }
    else if (m_recordeState == Pause)
    {
        ui->startButton->setEnabled(false);
        ui->pauseButton->setEnabled(true);
        ui->stopButton->setEnabled(true);

        m_screenRecorder->restoreRecord();

        m_recordeState = Recording;
        m_timer->start();
    }
    else if (m_recordeState == Stop)
    {

        if (rect.width() <= 0 || rect.height() <= 0 || !deskRect.contains(rect))
        {
            rect = deskRect;
        }

        if (saveFileName.remove(" ").isEmpty())
        {
            ret = -2;
            msg = "filepath not set";
            QMessageBox::critical(this, QStringLiteral("提示"), QStringLiteral("请先设置保存文件路径"));
        }
        else
        {
            saveFile();

            QString audioDevName = ui->comboBox_audio->currentText();

            if (audioDevName.isEmpty())
            {
                QMessageBox::critical(this, QStringLiteral("提示"), QStringLiteral("出错了,音频或视频设备未就绪，程序无法运行！"));

                ret = -3;
                msg = "audio device not set";
                goto end;
            }

            if (m_screenRecorder)
                delete m_screenRecorder;

            m_screenRecorder = new ScreenRecorder;
            m_screenRecorder->setFileName(saveFileName.toLocal8Bit().data());
            m_screenRecorder->setVideoFrameRate(ui->comboBox_framerate->currentText().toInt());

            if (ui->toolButton_video->isChecked())
            {
                if (m_screenRecorder->init("screen-capture-recorder",true,audioDevName,true) == SUCCEED)
                {
                    m_screenRecorder->setPicRange(rect.x(),rect.y(),rect.width(),rect.height());
                    m_screenRecorder->startRecord();
                }
                else
                {
                    QMessageBox::critical(this, QStringLiteral("提示"), QStringLiteral("出错了,初始化录屏设备失败！"));

                    ret = -4;
                    msg = "init screen device failed!";
                    goto end;

                }
            }
            else
            {
                if (m_screenRecorder->init("",false,audioDevName,true) == SUCCEED)
                {
//                    qDebug()<<rect;
                    m_screenRecorder->setPicRange(rect.x(),rect.y(),rect.width(),rect.height());
                    m_screenRecorder->startRecord();
                }
                else
                {
                    QMessageBox::critical(this, QStringLiteral("提示"), QStringLiteral("出错了,初始化音频设备失败！"));
                    ret = -5;
                    msg = "init audio device failed!";
                    goto end;
                }
            }

            ui->startButton->setEnabled(false);
            ui->pauseButton->setEnabled(true);
            ui->stopButton->setEnabled(true);

            ui->selectRectButton->setEnabled(false);
            ui->editRectButton->setEnabled(false);
            //ui->hideRectButton->setEnabled(false);

            ui->comboBox_audio->setEnabled(false);
        //    ui->comboBox_recordeMode->setEnabled(false);

            ui->toolButton_video->setEnabled(false);
            ui->toolButton_audio->setEnabled(false);

            m_recordeState = Recording;
            m_timer->start();
        }

        ui->pushButton_playBack->setEnabled(false);
    }

end:

//    m_erroMsg = QString("\"ret\":%1,\"msg\":\"%2\"").arg(ret).arg(msg);
    return ret == 0;
}

bool MainWindow::pauseRecord()
{
    int ret = 0;
    QString msg = "ok";
    if (m_recordeState == Recording)
    {
        m_timer->stop();

        ui->startButton->setEnabled(true);
        ui->pauseButton->setEnabled(false);

        ui->selectRectButton->setEnabled(false);
        ui->editRectButton->setEnabled(false);
        //ui->hideRectButton->setEnabled(false);

        m_screenRecorder->pauseRecord();

        m_recordeState = Pause;
    }
    else
    {
        ret = -6;
        msg = "is not started!";
    }

//    m_erroMsg = QString("\"ret\":%1,\"msg\":\"%2\"").arg(ret).arg(msg);
    return ret == 0;
}

bool MainWindow::stopRecord()
{
    int ret = 0;
    QString msg = "ok";
    if (m_recordeState != Stop)
    {
        m_timer->stop();
        m_recordeState = Stop;
        ui->label_time->setText("00:00:00");
        m_screenRecorder->stopRecord();

        ui->startButton->setEnabled(true);
        ui->pauseButton->setEnabled(false);
        ui->stopButton->setEnabled(false);

        ui->selectRectButton->setEnabled(true);
        ui->editRectButton->setEnabled(true);

        ui->comboBox_audio->setEnabled(true);
        ui->toolButton_video->setEnabled(true);
        ui->toolButton_audio->setEnabled(true);

        ui->pushButton_playBack->setEnabled(true);

    }
    else
    {
        ret = -7;
        msg = "not started!";
    }

//    m_erroMsg = QString("\"ret\":%1,\"msg\":\"%2\"").arg(ret).arg(msg);

    return ret == 0;
}

void MainWindow::slotSelectRectBtnClick()
{
    selectRectWidget->getReadyToSelect();
}

void MainWindow::slotEditRectBtnClick()
{
    selectRectWidget->showFullScreen();
    selectRectWidget->editRect();
}

void MainWindow::slotHideRectBtnClick()
{
    if (selectRectWidget->isVisible())
    {
       selectRectWidget->setVisible(false);
       ui->hideRectButton->setText(QStringLiteral("显示"));
    }
    else
    {
       selectRectWidget->setVisible(true);
       ui->hideRectButton->setText(QStringLiteral("隐藏"));
    }
}

void MainWindow::slotTimerTimeOut()
{
    qint64 audioPts = 0;
    if (m_screenRecorder)
    {
        audioPts = m_screenRecorder->getAudioPts();
    }

    QString hStr = QString("00%1").arg(audioPts/3600);
    QString mStr = QString("00%1").arg(audioPts%3600/60);
    QString sStr = QString("00%1").arg(audioPts%60);

    QString str = QString("%1:%2:%3").arg(hStr.right(2)).arg(mStr.right(2)).arg(sStr.right(2));
    ui->label_time->setText(str);

}

void MainWindow::slotCheckBoxClick(bool checked)
{
    if (checked)
    {
        selectRectWidget->setRate(m_rate);
    }
    else
    {
        selectRectWidget->setRate(-1);
    }
}

