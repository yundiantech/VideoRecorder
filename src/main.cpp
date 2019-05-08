
/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "mainwindow.h"
#include <QApplication>

#include <QTextCodec>

#undef main

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTextCodec *codec = QTextCodec::codecForName("GBK"); //获取系统编码
    QTextCodec::setCodecForLocale(codec);

    MainWindow w;

    //第二个参数为录制文件的路径
    if (argc >= 2)
    {
        QString str = QString(argv[1]);
        w.setSaveFile(str);
    }

    return a.exec();
}
