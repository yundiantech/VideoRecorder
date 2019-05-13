#include "AppConfig.h"

#include <QTextStream>
#include <QProcess>
#include <QDesktopWidget>
#include <QDesktopServices>

#include <QByteArray>
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QTranslator>
#include <QDateTime>
#include <QApplication>
#include <QMessageBox>

#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>

#include <QJsonDocument>
#include <QJsonObject>

#include <QDebug>

#include <time.h>

#if defined(WIN32)
#include <windows.h>
#include <direct.h>
#include <io.h> //C (Windows)    access
#else
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>

void Sleep(long mSeconds)
{
    usleep(mSeconds * 1000);
}

#endif

int AppConfig::VERSION = 1;
QString AppConfig::VERSION_NAME = "2.0.0";

MainWindow *AppConfig::gMainWindow = NULL;
QRect AppConfig::gMainWindowRect;

QRect AppConfig::gScreenRect;

QString AppConfig::AppDataPath_Main;
QString AppConfig::AppDataPath_Data;
QString AppConfig::AppDataPath_Tmp;
QString AppConfig::AppDataPath_TmpFile;
QString AppConfig::AppFilePath_Video; //视频文件保存目录
QString AppConfig::AppFilePath_Log;
QString AppConfig::AppFilePath_LogFile;

QString AppConfig::AppFilePath_EtcFile;

AppConfig::AppConfig()
{

}

void AppConfig::MakeDir(QString dirName)
{

    QDir dir;
    dir.mkpath(dirName);

//    dirName.replace("/","\\");

//    QProcess p(0);
//    p.start("cmd", QStringList()<<"/c"<<"md "<<dirName);
//    p.waitForStarted();
//    p.waitForFinished();
}

void AppConfig::InitAllDataPath()
{
    QFileInfo fileInfo(QCoreApplication::applicationFilePath());
    AppDataPath_Main = fileInfo.absoluteDir().path();

//#if (QT_VERSION <= QT_VERSION_CHECK(5,0,0))
//    AppDataPath_Main = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
//#else
//    AppDataPath_Main = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
//#endif

    AppDataPath_Data = AppDataPath_Main + "/data";

    AppFilePath_Video = AppDataPath_Data + "/movie";
    MakeDir(AppFilePath_Video);

    QString dirName = AppDataPath_Data + "/etc";
    MakeDir(dirName);

    AppFilePath_EtcFile = dirName + "/main.conf";

    AppDataPath_Tmp = AppDataPath_Data + "/tmp";
    AppFilePath_Log = AppDataPath_Data + "/log";

    AppDataPath_TmpFile = AppDataPath_Tmp + "/tmp.txt";

    MakeDir(AppDataPath_Data);
    MakeDir(AppFilePath_Log);
    MakeDir(AppDataPath_Tmp);
qDebug()<<AppDataPath_Main;
    InitLogFile();
}

QString AppConfig::bufferToString(QByteArray sendbuf)
{
    QString tmp;
    for (int k = 0; k < sendbuf.size(); k++)
    {
        QString str = QString("%1").arg(sendbuf[k] & 0xff, 2, 16, QLatin1Char('0'));
//        tmp += str + " ";
        tmp += str;
    }
    tmp = tmp.toUpper();
    return tmp;
}

QByteArray AppConfig::StringToBuffer(QString str)
{
    QString text = str.remove(" ");
    QByteArray sendbuf;
    while(!text.isEmpty())
    {
        QString str = text.left(2);

        if (str.length() == 1)
        {
            str = "0" + str;
        }

        text.remove(0,2);
        int x = str.left(1).toInt(0,16) << 4;
        x += str.right(1).toInt(0,16);
        QByteArray buf;
        buf.resize(1);
        buf[0] = x;
        sendbuf.append(buf);
    }

    return sendbuf;
}

QString AppConfig::getFileMd5(QString filePath,qint64 size)
{
    QFile localFile(filePath);

    if (!localFile.open(QFile::ReadOnly))
    {
//        qDebug() << "file open error.";
        return "";
    }

    QCryptographicHash ch(QCryptographicHash::Md5);

    quint64 totalBytes = 0;
    quint64 bytesWritten = 0;
    quint64 bytesToWrite = 0;
    quint64 loadSize = 1024 * 4;
    QByteArray buf;

    totalBytes = localFile.size();

    if (size > 0)
    {
        bytesToWrite = size;
    }
    else
    {
        bytesToWrite = totalBytes;
    }

    if (bytesToWrite > totalBytes)
    {
        bytesToWrite = totalBytes;
    }

    while (1)
    {
        if(bytesToWrite > 0)
        {
            buf = localFile.read(qMin(bytesToWrite, loadSize));
            ch.addData(buf);
            bytesWritten += buf.length();
            bytesToWrite -= buf.length();
            buf.resize(0);
        }
        else
        {
            break;
        }

        if (bytesToWrite <= 0)
        {
            break;
        }

//        if(bytesWritten == totalBytes)
//        {
//            break;
//        }
    }

    localFile.close();
    QByteArray md5 = ch.result();
    return AppConfig::bufferToString(md5).toLower();
}

void AppConfig::InitLogFile()
{
//    QString logFilePath = AppFilePath_Log + "\\log.txt";
//    gLogFile.setFileName(logFilePath);


    QDir dir(AppFilePath_Log);

    QFileInfoList fileInfoList = dir.entryInfoList();
    foreach(QFileInfo fileInfo, fileInfoList)
    {
        if(fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;

        if(fileInfo.isFile())
        {
            qint64 t1 = fileInfo.created().toMSecsSinceEpoch();
            qint64 t2 = QDateTime::currentMSecsSinceEpoch();

            qint64 t = (t2 - t1) / 1000; //文件创建到现在的时间（单位：秒）

            if (t >= (24*3600*3)) //删除3天前的日志文件
//            if (t >= (60*20))
            {
                QFile::remove(fileInfo.absoluteFilePath());
            }
        }
    }

    AppFilePath_LogFile = AppFilePath_Log + QString("/log_%1.txt").arg(QDate::currentDate().toString("yyyy-MM-dd"));
    WriteLog("\r\n=======================================\r\n=======================================\r\n[App Start...]\r\n\r\n");

}

void AppConfig::WriteLog(QString str)
{
    qDebug()<<__FUNCTION__<<str;

    QFile file(AppFilePath_LogFile);
    if (file.open(QIODevice::ReadWrite))
    {
        file.seek(file.size());
        QString tmpStr = QString("[%1] %2 \r\n")
                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                .arg(str);

        QTextStream fileOut(&file);
        fileOut.setCodec("UTF-8");  //unicode UTF-8  ANSI
        fileOut<<tmpStr;
        file.close();
    }
}

QString AppConfig::getSizeInfo(qint64 size)
{
    int pee = 1024;

    char ch[10]={0};
    if (size > (pee*pee*pee)) //大于1G
    {
        sprintf(ch,"%dGB",(int)(size*1.0/pee/pee/pee+0.5));
    }
    else if (size > (pee*pee)) //大于1M
    {
        sprintf(ch,"%dMB",size/pee/pee);
    }
    else if (size > pee) //大于1K
    {
        sprintf(ch,"%dKB",size/pee);
    }
    else //小于1KB
    {
        sprintf(ch,"%dB",size);
    }

    QString str = QString(ch);

    return str;
}

QImage AppConfig::ImagetoGray( QImage image)
{
    int height = image.height();
    int width = image.width();
    QImage ret(width, height, QImage::Format_Indexed8);
    ret.setColorCount(256);
    for(int i = 0; i < 256; i++)
    {
        ret.setColor(i, qRgb(i, i, i));
    }
    switch(image.format())
    {
    case QImage::Format_Indexed8:
        for(int i = 0; i < height; i ++)
        {
            const uchar *pSrc = (uchar *)image.constScanLine(i);
            uchar *pDest = (uchar *)ret.scanLine(i);
            memcpy(pDest, pSrc, width);
        }
        break;
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
        for(int i = 0; i < height; i ++)
        {
            const QRgb *pSrc = (QRgb *)image.constScanLine(i);
            uchar *pDest = (uchar *)ret.scanLine(i);

            for( int j = 0; j < width; j ++)
            {
                 pDest[j] = qGray(pSrc[j]);
            }
        }
        break;
    }
    return ret;
}

#if defined(Q_OS_WIN32)
    #include <Windows.h>
    #include <shlobj.h>
    #include <ShellAPI.h>

    bool AppConfig::restartSelf()
    {
        SHELLEXECUTEINFO sei;
        TCHAR szModule [MAX_PATH],szComspec[MAX_PATH],szParams [MAX_PATH];

        // 获得文件名.
        if((GetModuleFileName(0,szModule,MAX_PATH)!=0) &&
            (GetShortPathName(szModule,szModule,MAX_PATH)!=0) &&
            (GetEnvironmentVariable(L"COMSPEC",szComspec,MAX_PATH)!=0))
        {
    //        QString dirPath = QCoreApplication::applicationDirPath();
            QString dirPath = QCoreApplication::applicationFilePath();
            dirPath.replace("/","\\");
            dirPath = "\"" + dirPath + "\"";
            // 设置命令参数.
            lstrcpy(szParams, L"/c ");
            lstrcat(szParams, (WCHAR*)dirPath.utf16());
            lstrcat(szParams, L" > nul");

    //        lstrcpy(szParams, L"/c del ");
    //        lstrcat(szParams, szModule);
    //        lstrcat(szParams, L" > nul");

            // 设置结构成员.
            sei.cbSize = sizeof(sei);
            sei.hwnd = 0;
            sei.lpVerb = L"Open";
            sei.lpFile = szComspec;
            sei.lpParameters = szParams;
            sei.lpDirectory = 0;
            sei.nShow = SW_HIDE;
            sei.fMask = SEE_MASK_NOCLOSEPROCESS;

            // 执行shell命令.
            if(ShellExecuteEx(&sei))
            {
                // 设置命令行进程的执行级别为空闲执行,使本程序有足够的时间从内存中退出.
                SetPriorityClass(sei.hProcess,IDLE_PRIORITY_CLASS);
                SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS);
                SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_TIME_CRITICAL);

               // 通知Windows资源浏览器,本程序文件已经被删除.
                SHChangeNotify(SHCNE_DELETE,SHCNF_PATH,(WCHAR*)dirPath.utf16(),0);
                return TRUE;
            }
        }
        return FALSE;
    }
#endif

void AppConfig::mSleep(int mSecond)
{
#if defined(WIN32)
    Sleep(mSecond);
#else
    usleep(mSecond * 1000);
#endif
}

int64_t AppConfig::getTimeStamp_MilliSecond()
{

    int mSecond = 0; //当前毫秒数

#if defined(WIN32)

    SYSTEMTIME sys;
    GetLocalTime( &sys );

    mSecond = sys.wMilliseconds;

#else

    struct timeval    tv;
    struct timezone tz;

    struct tm         *p;

    gettimeofday(&tv, &tz);
    p = localtime(&tv.tv_sec);

    mSecond = tv.tv_usec / 1000;


#endif

    int64_t timeStamp = time(NULL) * 1000 + mSecond;

    return timeStamp;

}
