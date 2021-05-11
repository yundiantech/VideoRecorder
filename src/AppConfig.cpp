/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "AppConfig.h"

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

#include <QUuid>
#include <QDebug>
#include <QUrl>

#if defined(WIN32)
#include <WinSock2.h>
#include <Windows.h>
#include <direct.h>
#include <io.h> //C (Windows)    access
#else
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>

void Sleep(long mSeconds);
//{
//    usleep(mSeconds * 1000);
//}

#endif

#include "Base64/Base64.h"

QString AppConfig::APPID = "{b89bb174-d00e-413e-aa6b-dfece6d39348}";
int AppConfig::VERSION = 1;
QString AppConfig::VERSION_NAME = "3.0.0";

QString AppConfig::Memory_KEY_NAME = "VideoRecorder_HUI_123";

bool AppConfig::gIsDebugMode = true;
bool AppConfig::gVideoKeepAspectRatio = true; //视频是否按比例显示
bool AppConfig::gVideoHardDecoder = false; //硬解解码

QString AppConfig::gAudioInputDeviceName;  //麦克风设备名称
QRect AppConfig::gLocalCameraWidgetRect   = QRect(100, 100, 352, 288);   //本地摄像头位置
QString AppConfig::gLocalCameraDeviceName; //本地摄像头设备名

bool AppConfig::gEnableVirtualAudioCapture = true; //是否启用声卡录制
QSize AppConfig::gVideoFileSize = QSize(0, 0); //视频文件分辨率
int AppConfig::gVideoQuality = 5;     //视频画质
QString AppConfig::gVideoDirPath; //存放视频路径
QString AppConfig::gVideoFilePath; //打开视频文件的默认位置(视频播放模式)
int AppConfig::gCaptureAreaRate = 70; //采集区域比例
int AppConfig::gPictureAreaRate = 60; //广告图片区域比例

/// SERVER URL
QString AppConfig::URL_ROOT;
QString AppConfig::URL_UPDATE; //更新URL

QString AppConfig::AppDataPath_Main;
QString AppConfig::AppDataPath_Data;
QString AppConfig::AppDataPath_Tmp;
QString AppConfig::AppDataPath_TmpFile;
QString AppConfig::AppFilePath_Log;
QString AppConfig::AppFilePath_LogFile;
QString AppConfig::AppFilePath_EtcFile;
QString AppConfig::AppFilePath_DBFile;
QString AppConfig::AppDataPath_Video;  //默认存放视频的目录

MainWindow *AppConfig::gMainWindow = nullptr;
MediaManager *AppConfig::gMediaManager = nullptr;

AppConfig::AppConfig()
{

}

void AppConfig::setURL_ROOT(const QString &rootUrl)
{
    AppConfig::URL_ROOT = rootUrl;

    if (AppConfig::URL_ROOT.isEmpty())
    {
        AppConfig::URL_ROOT = "https://videorecorder.yundiantech.com";
    }

    AppConfig::URL_UPDATE = AppConfig::URL_ROOT + "/api/update";

    QUrl url(AppConfig::URL_ROOT);
//qDebug()<<__FUNCTION__<<url.path()<<url.host();

}

void AppConfig::MakeDir(QString dirName)
{
    QDir dir;
    dir.mkpath(dirName);
}

void AppConfig::InitAllDataPath()
{

#if defined(WIN32)
    ///windows数据存储在C盘的数据目录下
    QFileInfo fileInfo(QCoreApplication::applicationFilePath());
    QString exeFileName = fileInfo.baseName(); //当前程序名字

    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
//    QString dataPath = QCoreApplication::applicationDirPath();

    if (dataPath.right(exeFileName.length()) == exeFileName)
    {
        dataPath.remove(dataPath.length() - exeFileName.length(), exeFileName.length());
    }

    if (!dataPath.endsWith("/"))
    {
        dataPath += "/";
    }

    dataPath += "AirClassRoom";

    if (dataPath.right(exeFileName.length()) != exeFileName)
    {
        if (!dataPath.endsWith("/"))
        {
            dataPath += "/";
        }

        dataPath += exeFileName;
    }

#else
    ///Linux则放在程序所在目录下的data目录下
    QFileInfo fileInfo(QCoreApplication::applicationFilePath());

    QString dataPath = fileInfo.absoluteDir().path();

    if (!dataPath.endsWith("/"))
    {
        dataPath += "/";
    }

#endif

    AppDataPath_Main = dataPath;

    AppDataPath_Data = AppDataPath_Main + "/data";

    QString dirName = AppDataPath_Data + "/etc";
    MakeDir(dirName);

    AppFilePath_EtcFile = dirName + "/main.conf";

    AppDataPath_Video  = AppDataPath_Data + "/Video";
    AppFilePath_DBFile = AppDataPath_Data + "/data.db";
    AppDataPath_Tmp = AppDataPath_Data + "/tmp";
    AppFilePath_Log = AppDataPath_Data + "/log";

    AppDataPath_TmpFile = AppDataPath_Tmp + "/tmp.txt";

    MakeDir(AppDataPath_Data);
    MakeDir(AppFilePath_Log);
    MakeDir(AppDataPath_Tmp);
    MakeDir(AppDataPath_Video);

    InitLogFile();

    if (AppConfig::gVideoDirPath.isEmpty())
    {
        AppConfig::gVideoDirPath = AppDataPath_Video;
    }
    else
    {
        MakeDir(AppConfig::gVideoDirPath);

        QDir dir(AppConfig::gVideoDirPath);
        if (!dir.exists(AppConfig::gVideoDirPath))
        {
            AppConfig::gVideoDirPath = AppDataPath_Video;
        }
    }

    AppConfig::loadConfigInfoFromFile();

    setURL_ROOT(AppConfig::URL_ROOT);

qDebug()<<__FUNCTION__<<dataPath;


}

void AppConfig::SetStyle(QWidget *wdiget, const QString &styleName)
{
    QFile file(QString(":/qss/%1.qss").arg(styleName));
    file.open(QFile::ReadOnly);
    QString qss = QLatin1String(file.readAll());
    wdiget->setStyleSheet(qss);
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

QString AppConfig::getMd5(QString str)
{
    QByteArray bytePwd = str.toUtf8();
    QByteArray bytePwdMd5 = QCryptographicHash::hash(bytePwd, QCryptographicHash::Md5);
    QString strPwdMd5 = bytePwdMd5.toHex();

    return strPwdMd5;
}

QString AppConfig::bufferToFile(uint8_t *buffer, const int &len, const QString &suffix)
{
    QString UUIDStr = QUuid::createUuid().toString();
    UUIDStr.remove("{");
    UUIDStr.remove("}");
    UUIDStr.remove("-");

    QString tmpFilePath = QString("%1/%2%3")
                        .arg(AppConfig::AppDataPath_Tmp)
                        .arg(UUIDStr)
                        .arg(suffix);

    QFile file(tmpFilePath);
    if (file.open( QIODevice::WriteOnly))
    {
        file.write((char*)buffer, len);
        file.close();
    }

    return tmpFilePath;
}

void AppConfig::loadConfigInfoFromFile()
{

    QFile file(AppConfig::AppFilePath_EtcFile);

    if (file.open(QIODevice::ReadOnly))
    {
        QString text(file.readAll());

        if (!text.isEmpty())
        {
            QJsonParseError json_error;
            QJsonDocument jsonDoc(QJsonDocument::fromJson(text.toUtf8(), &json_error));

            if(json_error.error == QJsonParseError::NoError)
            {
                QJsonObject object = jsonDoc.object();

                ///获取音视频设备
                QJsonValue deviceValue = object.value("device");

                if (deviceValue.isObject())
                {
                    QJsonObject deviceObject = deviceValue.toObject();

                    AppConfig::gAudioInputDeviceName = deviceObject.value("audioindevice").toString();
                    AppConfig::gLocalCameraDeviceName = deviceObject.value("cameradevice").toString();


                    QJsonObject cameraRectObject = deviceObject.value("camerarect").toObject();

                    int x = cameraRectObject.value("x").toInt();
                    int y = cameraRectObject.value("y").toInt();
                    int w = cameraRectObject.value("w").toInt();
                    int h = cameraRectObject.value("h").toInt();

                    if (x < 0)  x=0;
                    if (y < 0)  y=0;
                    if (w <= 0) w=352;
                    if (h <= 0) y=288;

                    AppConfig::gLocalCameraWidgetRect = QRect(x, y, w, h);

                }

                ///读取录屏相关配置
                QJsonValue recorderValue = object.value("recorder");
                if (recorderValue.isObject())
                {
                    QJsonObject recorderObject = recorderValue.toObject();

                    QJsonValue EnableVirtualAudioCaptureValue = recorderObject.value("enablevirtualaudio");
                    QJsonValue VideoQualityValue = recorderObject.value("videoquality");

//                    if (EnableVirtualAudioCaptureValue.isDouble())
//                    {
//                        AppConfig::gEnableVirtualAudioCapture = EnableVirtualAudioCaptureValue.toInt();
//                    }

                    if (VideoQualityValue.isDouble())
                    {
                        AppConfig::gVideoQuality = VideoQualityValue.toInt();
                    }

                    AppConfig::gVideoDirPath = recorderObject.value("videodirpath").toString();

                    QJsonObject videosizeObject = recorderObject.value("videosize").toObject();
                    int x = videosizeObject.value("x").toInt();
                    int y = videosizeObject.value("y").toInt();
                    int w = videosizeObject.value("w").toInt();
                    int h = videosizeObject.value("h").toInt();
//                    if (x < 0)  x=0;
//                    if (y < 0)  y=0;
//                    if (w <= 0) w=352;
//                    if (h <= 0) y=288;
                    AppConfig::gVideoFileSize = QSize(w, h);
                }

                ///读取采集相关配置
                QJsonValue captureValue = object.value("capture");
                if (captureValue.isObject())
                {
                    QJsonObject captureObject = captureValue.toObject();

                    QJsonValue penwidthValue = captureObject.value("penwidth");
                    QJsonValue pencoloridValue = captureObject.value("pencolorid");
                    QJsonValue capturearearateValue = captureObject.value("capturearearate");
                    QJsonValue picturearearateValue = captureObject.value("picturearearate");

                    if (capturearearateValue.isDouble())
                    {
                        AppConfig::gCaptureAreaRate = capturearearateValue.toInt();
                    }

                    if (picturearearateValue.isDouble())
                    {
                        AppConfig::gPictureAreaRate = picturearearateValue.toInt();
                    }
                }
            }
        }

        file.close();
    }
}

void AppConfig::saveConfigInfoToFile()
{
    QFile file(AppConfig::AppFilePath_EtcFile);
    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream fileOut(&file);
        fileOut.setCodec("UTF-8");  //unicode UTF-8  ANSI

        QJsonObject dataObject;
        dataObject.insert("appid", AppConfig::APPID);
        dataObject.insert("urlRoot", AppConfig::URL_ROOT);

        ///记录音视频设备
        {
            QJsonObject deviceObject;
            deviceObject.insert("audioindevice", AppConfig::gAudioInputDeviceName);
            deviceObject.insert("cameradevice", AppConfig::gLocalCameraDeviceName);

            {
                QJsonObject cameraRectObject;
                cameraRectObject.insert("x", AppConfig::gLocalCameraWidgetRect.x());
                cameraRectObject.insert("y", AppConfig::gLocalCameraWidgetRect.y());
                cameraRectObject.insert("w", AppConfig::gLocalCameraWidgetRect.width());
                cameraRectObject.insert("h", AppConfig::gLocalCameraWidgetRect.height());

                deviceObject.insert("camerarect", cameraRectObject);
            }

            dataObject.insert("device", deviceObject);
        }

        ///记录录屏相关配置
        {
            QJsonObject object;
            object.insert("enablevirtualaudio", AppConfig::gEnableVirtualAudioCapture);
            object.insert("videoquality", AppConfig::gVideoQuality);
            object.insert("videodirpath", AppConfig::gVideoDirPath);

            {
                QJsonObject videoSizeObject;
//                videoSizeObject.insert("x", AppConfig::gVideoFileSize.x());
//                videoSizeObject.insert("y", AppConfig::gVideoFileSize.y());
                videoSizeObject.insert("w", AppConfig::gVideoFileSize.width());
                videoSizeObject.insert("h", AppConfig::gVideoFileSize.height());

                object.insert("videosize", videoSizeObject);
            }

            dataObject.insert("recorder", object);
        }

        ///记录采集相关配置
        {
            QJsonObject object;
            object.insert("capturearearate", AppConfig::gCaptureAreaRate);
            object.insert("picturearearate", AppConfig::gPictureAreaRate);
            dataObject.insert("capture", object);
        }

        QJsonDocument json;
//        QJsonObject object;
//        object.insert("config", dataObject);

        //最外层是大括号所以是object
        json.setObject(dataObject);

        QString jsonStr = json.toJson(QJsonDocument::Compact);

        fileOut<<jsonStr;

//        std::string str = base64::base64_encode((unsigned char*)jsonStr.toUtf8().data(),jsonStr.toUtf8().size());
//        fileOut<<QString::fromStdString(str);

        file.close();
    }
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

#if 0
bool AppConfig::WriteLog(QString str)
{
    bool IsFileOpened = gLogFile.isOpen();

    if (!IsFileOpened)
    {
        IsFileOpened = gLogFile.open(QIODevice::ReadWrite);
        gLogFile.seek(gLogFile.size());
    }

    if (IsFileOpened)
    {
        QString tmpStr = QString("[%1] %2 \r\n")
                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                .arg(str);

        QTextStream fileOut(&gLogFile);
        fileOut.setCodec("UTF-8");  //unicode UTF-8  ANSI
        fileOut<<tmpStr;
    }

    return IsFileOpened;
}
#else
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
#endif

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

QString AppConfig::ImageToFile(const QImage &image)
{
    QString UUIDStr = QUuid::createUuid().toString();
    UUIDStr.remove("{");
    UUIDStr.remove("}");
    UUIDStr.remove("-");

    QString tmpFilePath = QString("%1/%2.bmp")
                        .arg(AppConfig::AppDataPath_Tmp)
                        .arg(UUIDStr);

    image.save(tmpFilePath);

    return tmpFilePath;
}

QString AppConfig::getBase64Str(const QImage &image)
{
    QString imageBase64Str;

    QString UUIDStr = QUuid::createUuid().toString();
    UUIDStr.remove("{");
    UUIDStr.remove("}");
    UUIDStr.remove("-");

    QString tmpFilePath = QString("%1/%2.jpg")
                        .arg(AppConfig::AppDataPath_Tmp)
                        .arg(UUIDStr);

    image.save(tmpFilePath);

    QFile file(tmpFilePath);
    if (file.open( QIODevice::ReadOnly))
    {
        QByteArray arrayBuffer = file.readAll();
        imageBase64Str = QString::fromStdString(Base64Encode((unsigned char*)arrayBuffer.data(), arrayBuffer.length()));

        file.close();
    }

    file.remove();

//qDebug()<<__FUNCTION__<<image<<tmpFilePath<<QFile::exists(tmpFilePath)<<imageBase64Str;
    return imageBase64Str;
}

QString AppConfig::getBase64Str(QString filePath)
{
    QString imageBase64Str;

    QFile file(filePath);
    if (file.open( QIODevice::ReadOnly))
    {
        QByteArray arrayBuffer = file.readAll();
        imageBase64Str = QString::fromStdString(Base64Encode((unsigned char*)arrayBuffer.data(), arrayBuffer.length()));
    }
//qDebug()<<__FUNCTION__<<tmpFilePath<<QFile::exists(tmpFilePath)<<imageBase64Str;
    return imageBase64Str;
}

QString AppConfig::getBase64Str(const QByteArray &arrayBuffer)
{
    QString base64Str;

    base64Str = QString::fromStdString(Base64Encode((unsigned char*)arrayBuffer.data(), arrayBuffer.length()));

    return base64Str;
}

QImage AppConfig::base64ToQImage(const QString &base64Str, const bool &isBmp)
{
    QString UUIDStr = QUuid::createUuid().toString();
    UUIDStr.remove("{");
    UUIDStr.remove("}");
    UUIDStr.remove("-");

    QString tmpFilePath = QString("%1/%2.jpg")
                        .arg(AppConfig::AppDataPath_Tmp)
                        .arg(UUIDStr);

    if (isBmp)
    {
        tmpFilePath = QString("%1/%2.bmp")
                                .arg(AppConfig::AppDataPath_Tmp)
                                .arg(UUIDStr);
    }

    std::string outStr = Base64Decode(base64Str.toStdString());

    QFile file(tmpFilePath);
    if (file.open( QIODevice::WriteOnly))
    {
        file.write(outStr.c_str(), outStr.size());
        file.close();
    }

    QImage image = QImage(tmpFilePath, "auto");

    return image;
}

QString AppConfig::base64ToImageFile(const QString &base64Str, const bool &isBmp)
{
    QString UUIDStr = QUuid::createUuid().toString();
    UUIDStr.remove("{");
    UUIDStr.remove("}");
    UUIDStr.remove("-");

    QString tmpFilePath = QString("%1/%2.jpg")
                        .arg(AppConfig::AppDataPath_Tmp)
                        .arg(UUIDStr);

    if (isBmp)
    {
        tmpFilePath = QString("%1/%2.bmp")
                                .arg(AppConfig::AppDataPath_Tmp)
                                .arg(UUIDStr);
    }

    std::string outStr = Base64Decode(base64Str.toStdString());

    QFile file(tmpFilePath);
    if (file.open( QIODevice::WriteOnly))
    {
        file.write(outStr.c_str(), outStr.size());
        file.close();
    }

    return tmpFilePath;
}

QString AppConfig::bufferToFile(char *buffer, const int &len)
{
    QString UUIDStr = QUuid::createUuid().toString();
    UUIDStr.remove("{");
    UUIDStr.remove("}");
    UUIDStr.remove("-");

    QString tmpFilePath = QString("%1/%2.bin")
                        .arg(AppConfig::AppDataPath_Tmp)
                        .arg(UUIDStr);

    QFile file(tmpFilePath);
    if (file.open( QIODevice::WriteOnly))
    {
        file.write(buffer, len);
        file.close();
    }

    return tmpFilePath;
}

QByteArray AppConfig::fileToBuffer(const QString &filePath)
{
    QByteArray buffer;

    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly))
    {
        buffer = file.readAll();
        file.close();
    }

    return buffer;
}

bool AppConfig::copyFile(const QString &srcFile, const QString & destFile)
{
    bool isSucceed = false;

    QFileInfo fileInfo(destFile);

    for (int i=0;i<5;i++)
    {
        QDir targetDir(fileInfo.absoluteDir());
        if(!targetDir.exists())
        {    /**< 如果目标目录不存在，则进行创建 */
            if(!targetDir.mkpath(targetDir.absolutePath()))
            {
                AppConfig::mSleep(100);
                continue;
            }
        }

        QFile::remove(destFile);
        isSucceed = QFile::copy(srcFile, destFile);

        if (isSucceed) break;

        AppConfig::mSleep(100);
    }

    return isSucceed;
}

//拷贝文件夹：
bool AppConfig::copyDirectoryFiles(const QString &fromDir, const QString &toDir, bool coverFileIfExist)
{
    QDir sourceDir(fromDir);
    QDir targetDir(toDir);
    if(!targetDir.exists()){    /**< 如果目标目录不存在，则进行创建 */
        if(!targetDir.mkdir(targetDir.absolutePath()))
            return false;
    }

    QFileInfoList fileInfoList = sourceDir.entryInfoList();
    foreach(QFileInfo fileInfo, fileInfoList){
        if(fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;

        if(fileInfo.isDir()){    /**< 当为目录时，递归的进行copy */
            if(!copyDirectoryFiles(fileInfo.filePath(),
                targetDir.filePath(fileInfo.fileName()),
                coverFileIfExist))
                return false;
        }
        else{            /**< 当允许覆盖操作时，将旧文件进行删除操作 */
            if(coverFileIfExist && targetDir.exists(fileInfo.fileName())){
                targetDir.remove(fileInfo.fileName());
            }

            /// 进行文件copy
            if(!QFile::copy(fileInfo.filePath(),
                targetDir.filePath(fileInfo.fileName()))){
                    return false;
            }
        }
    }
    return true;
}

bool AppConfig::removeDirectory(QString dirName)
{
  QDir dir(dirName);
  QString tmpdir = "";

  if (!QFile(dirName).exists()) return false;

  if(!dir.exists()){
    return false;
  }

  QFileInfoList fileInfoList = dir.entryInfoList();
  foreach(QFileInfo fileInfo, fileInfoList){
    if(fileInfo.fileName() == "." || fileInfo.fileName() == "..")
      continue;

    if(fileInfo.isDir()){
      tmpdir = dirName + ("/") + fileInfo.fileName();
      removeDirectory(tmpdir);
      dir.rmdir(fileInfo.fileName()); /**< 移除子目录 */
    }
    else if(fileInfo.isFile()){
      QFile tmpFile(fileInfo.fileName());
      dir.remove(tmpFile.fileName()); /**< 删除临时文件 */
    }
    else{
      ;
    }
  }

  dir.cdUp();            /**< 返回上级目录，因为只有返回上级目录，才可以删除这个目录 */
  if(dir.exists(dirName)){
    if(!dir.rmdir(dirName))
      return false;
  }
  return true;
}

#if defined(Q_OS_WIN32)
    #include <ShlObj.h>
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
#elif defined(Q_OS_MAC)
    bool Appconfig::restartSelf()
    {
        QString bashFilePath = QString("%1/run.sh").arg(Appconfig::AppDataPath_Data);
        QFile file(bashFilePath);
        if (file.open(QIODevice::WriteOnly))
        {
            QTextStream fileOut(&file);
            fileOut.setCodec("UTF-8");  //unicode UTF-8  ANSI


            QString dirPath = QApplication::applicationDirPath();
            QString filePath = QString("%1/%2").arg(dirPath).arg(Appconfig::AppExeName);
            QString runAppCmd = QString("open -a \""+filePath+"\" \n");

            fileOut<<QString("ping -c 4 -t 2 baidu.com \n"); //延时2秒
            fileOut<<runAppCmd; //启动程序

            file.close();
        }

    qDebug()<<bashFilePath;
        QProcess p(0);
        p.start("bash");
        p.waitForStarted();
        p.write(QString("chmod a+x \""+bashFilePath+"\" \n").toUtf8());
        p.write(QString("\""+bashFilePath+"\" &\n").toUtf8()); //后台运行
//        p.write(QString("open -a \""+bashFilePath+"\" \n").toUtf8());
        p.closeWriteChannel();
        p.waitForFinished();

        return true;
    }
#endif

void AppConfig::mSleep(int mSecond)
{
    Sleep(mSecond);
}
