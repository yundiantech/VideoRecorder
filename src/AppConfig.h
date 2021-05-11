/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <stdint.h>

#include <QFile>
#include <QString>
#include <QTranslator>
#include <QDateTime>
#include <QColor>
#include <QRect>
#include <QPixmap>

///用来和主程序通信的共享内存KEY
#define Memory_KEY_NAME_Main "QuickvideoWebClient_Eyecool_HUI_123"
#define CURRENT_TIME QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss]")

#ifdef QT_NO_KEYWORDS
#define foreach Q_FOREACH
#endif

class MainWindow;
class MediaManager;

#include <QSpacerItem>
#include <QLayout>
#include <QSizePolicy>
#include <QWidget>
#include <QObject>
#include <QSize>
#include <QTableWidget>
#include <QHeaderView>

#define OBJMAXSIZE 16777215
#define OBJMINSIZE 0

class AppConfig
{
public:
    AppConfig();

    static QString APPID;
    static int VERSION;
    static QString VERSION_NAME;

    /// SERVER URL
    static QString URL_ROOT;
    static QString URL_UPDATE; //更新URL

    static QString Memory_KEY_NAME;

    /// 本地全局变量
    static QString AppDataPath_Main; //程序数据主目录
    static QString AppDataPath_Data; //程序数据的data目录
    static QString AppDataPath_Tmp; //临时目录(程序退出时会清空此目录)
    static QString AppDataPath_TmpFile; //程序运行时 创建次文件，退出时删除此文件，用来判断程序是否正常退出
    static QString AppFilePath_Log; //日志目录
    static QString AppFilePath_LogFile; //日志文件
    static QString AppFilePath_EtcFile; //配置信息
    static QString AppFilePath_DBFile; //本地数据库
    static QString AppDataPath_Video;  //默认存放视频的目录

    static bool gIsDebugMode; //是否调试模式，调试模式下，日志输出会多一些（按ctrl+shirt+alt+D进入调试模式）
    static bool gVideoKeepAspectRatio; //视频是否按比例显示
    static bool gVideoHardDecoder; //硬解解码
    static QString gVideoFilePath; //打开视频文件的默认位置(视频播放模式)

    static QString gAudioInputDeviceName;  //麦克风设备名称
    static QRect gLocalCameraWidgetRect;   //本地摄像头位置
    static QString gLocalCameraDeviceName; //本地摄像头设备名
    static bool gEnableVirtualAudioCapture; //是否启用声卡录制
    static QSize gVideoFileSize;  //视频文件分辨率
    static int gVideoQuality;     //视频画质
    static QString gVideoDirPath; //存放视频路径
    static int gCaptureAreaRate;  //采集区域比例
    static int gPictureAreaRate;  //广告图片区域比例

    static MainWindow *gMainWindow;
    static MediaManager *gMediaManager;

    static void MakeDir(QString dirName);

    static void setURL_ROOT(const QString &rootUrl = "");
    static void InitAllDataPath(); //初始化所有数据保存的路径

    static void SetStyle(QWidget *wdiget, const QString &styleName);

    static QString bufferToString(QByteArray sendbuf);
    static QByteArray StringToBuffer(QString);
    static QString getFileMd5(QString filePath,qint64 size=-1);
    static QString getMd5(QString str);
    static QString bufferToFile(uint8_t *buffer, const int &len, const QString &suffix);

    ///配置文件
    static void loadConfigInfoFromFile();
    static void saveConfigInfoToFile();

    ///写日志
    static void WriteLog(QString str);
    static void InitLogFile();
    static QString getSizeInfo(qint64 size);

    static QImage ImagetoGray( QImage image); //生成灰度图
    static QString ImageToFile(const QImage &image);
    static QString getBase64Str(const QImage &image);
    static QString getBase64Str(QString filePath);
    static QString getBase64Str(const QByteArray &arrayBuffer);
    static QImage base64ToQImage(const QString &base64Str, const bool &isBmp = false);
    static QString base64ToImageFile(const QString &base64Str, const bool &isBmp = false);
    static QString bufferToFile(char *buffer, const int &len);
    static QByteArray fileToBuffer(const QString &filePath);

    static bool copyFile(const QString &srcFile, const QString & destFile);

    ///拷贝文件夹
    static bool copyDirectoryFiles(const QString &fromDir, const QString &toDir, bool coverFileIfExist);

    ///删除目录
    static bool removeDirectory(QString dirName);

    ///重启软件
    static bool restartSelf();

    ///休眠函数(毫秒)
    static void mSleep(int mSecond);

};

#endif // APPCONFIG_H
