#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <stdint.h>

#include <QFile>
#include <QString>
#include <QTranslator>

class MainWindow;

class AppConfig
{
public:
    AppConfig();

    static int VERSION;
    static QString VERSION_NAME;

    /// 本地全局变量
    static QString AppDataPath_Main; //程序数据主目录
    static QString AppDataPath_Data; //程序数据的data目录
    static QString AppDataPath_Tmp; //临时目录(程序退出时会清空此目录)
    static QString AppDataPath_TmpFile; //程序运行时 创建次文件，退出时删除此文件，用来判断程序是否正常退出
    static QString AppFilePath_Log; //日志目录
    static QString AppFilePath_Video; //视频文件保存目录
    static QString AppFilePath_LogFile; //日志文件
    static QString AppFilePath_EtcFile; //配置信息

    static MainWindow *gMainWindow;
    static QRect gMainWindowRect; //主窗口的位置 - 用于标记在非全屏模式下的弹窗大小
    static QRect gScreenRect;

    static void MakeDir(QString dirName);
    static void InitAllDataPath(); //初始化所有数据保存的路径

    static QString bufferToString(QByteArray sendbuf);
    static QByteArray StringToBuffer(QString);
    static QString getFileMd5(QString filePath,qint64 size=-1);

    static QString getMACAdress(); //获取mac地址 用于信令的count
    static QString getIpAdress();

    ///写日志
//    static QFile gLogFile;
    static void WriteLog(QString str);
    static void InitLogFile();
    static QString getSizeInfo(qint64 size);

    static QImage ImagetoGray( QImage image); //生成灰度图

    static void mSleep(int mSecond);

    static int64_t getTimeStamp_MilliSecond(); //获取时间戳（毫秒）

    ///删除目录
    static bool removeDirectory(QString dirName);

    ///重启软件
    static bool restartSelf();

};

#endif // APPCONFIG_H
