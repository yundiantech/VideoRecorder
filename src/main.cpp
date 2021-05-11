/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "MainWindow.h"
#include <QApplication>

#include <QTextCodec>
#include <QMessageBox>
#include <QSharedMemory>
#include <QScreen>
#include <QDir>

#include "AppConfig.h"

#undef main
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QApplication::setQuitOnLastWindowClosed(false);

    a.setWindowIcon(QIcon(":/img/logo.png"));

    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);
//    QTextCodec::setCodecForCStrings(codec);
//    QTextCodec::setCodecForTr(codec);

    AppConfig::InitAllDataPath();

    QString videodir;

    for (int i=1;i<argc;i++)
    {
#if 1
        QTextCodec *gbk = QTextCodec::codecForName("GB18030");
        QString paramStr = gbk->toUnicode(argv[i]);
#else
//        QString paramStr = QString::fromLocal8Bit(argv[i]);
#endif
        if (paramStr.contains("videodir="))
        {
            videodir=paramStr.remove("videodir=");
        }

    }

    if (!videodir.isEmpty())
    {
        AppConfig::MakeDir(videodir);

        QDir dir(videodir);
        if (dir.exists())
        {
            AppConfig::gVideoDirPath = videodir;
        }
    }

    QSharedMemory sharedmem(AppConfig::Memory_KEY_NAME);
    if (!sharedmem.create(1024))
    {
        if (sharedmem.attach())
        {
            sharedmem.lock();
            char * to = static_cast<char*>(sharedmem.data());
            const char * from = "show out";
            ::memcpy(to, from, strlen(from));
            sharedmem.unlock();
        }
        else
        {
            QMessageBox::information(0, "", QStringLiteral("另一个程序已经在运行了"));
        }

        return 0;
    }

    MainWindow w;
    w.showFullScreen();

    return a.exec();
}
