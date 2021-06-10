#include "MoudleConfig.h"

#include <time.h>
#if defined(WIN32)
#include <WinSock2.h>
#include <Windows.h>
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

int MoudleConfig::VERSION = 1;
char MoudleConfig::VERSION_NAME[32] = "1.0.0";

MoudleConfig::MoudleConfig()
{

}

void MoudleConfig::mkdir(char *dirName)
{
#if defined(WIN32)
    ///如果目录不存在 则创建它
    if (access(dirName, 0)!=0)
    {
        _mkdir(dirName);
    }
#else
    ///如果目录不存在 则创建它
    if (access(dirName, R_OK)!=0)
    {
        char cmd[128];
        sprintf(cmd,"mkdir %s", dirName);
        system(cmd);
    }
#endif
}

void MoudleConfig::mkpath(char *path)
{
#if defined(WIN32)
        ///windows创建文件夹命令 路径得是反斜杠 因此这里需要替换一下
        char dirPath[128] = {0};
        strcpy(dirPath, path);

        MoudleConfig::replaceChar(dirPath, '/', '\\');

        ///如果目录不存在 则创建它
        if (access(dirPath, 0)!=0)
        {
    //        _mkdir(dirPath);
            char cmd[128];
            sprintf(cmd,"mkdir %s", dirPath);
            system(cmd);
        }

#else
    ///如果目录不存在 则创建它
    if (access(path,R_OK)!=0)
    {
        char cmd[128];
        sprintf(cmd,"mkdir %s -p",path);
        system(cmd);
    }
#endif
}

void MoudleConfig::removeDir(char *dirName)
{
    if (strlen(dirName) <= 0) return;

    if (access(dirName, 0) != 0 ) //文件夹不存在
    {
        return;
    }

#if defined(WIN32)

    ///删除本地文件
    char cmd[128];
    sprintf(cmd,"rd /s /q \"%s\"", dirName);
    system(cmd);

#else

    char cmd[128];
    sprintf(cmd,"rm -rf \"%s\"",dirName);
    system(cmd);

#endif
}

void MoudleConfig::removeFile(const char *filePath)
{
    if (filePath == NULL || strlen(filePath) <= 0) return;

#if defined(WIN32)

        ///删除本地文件
        remove(filePath);

#else
        ///删除本地文件
        char cmd[128] = {0};
        sprintf(cmd,"rm -rf \"%s\"",filePath);
        system(cmd);
#endif
}

void MoudleConfig::copyFile(const char *srcFile, const char *destFile)
{

#if defined(WIN32)
        CopyFileA(srcFile, destFile, FALSE);
#else

        ///将文件拷贝到远端服务器
        char copyfilecmd[512];
        sprintf(copyfilecmd,"cp \"%s\" \"%s\"", srcFile, destFile);
        system(copyfilecmd);

#endif
}


std::string MoudleConfig::stringFormat(const char * fmt, ...)
{
#if defined(WIN32)
    std::string _str;
    va_list marker = NULL;
    va_start(marker, fmt);

    size_t num_of_chars = _vscprintf(fmt, marker);
    _str.resize(num_of_chars);
    vsprintf_s((char *)_str.c_str(), num_of_chars + 1, fmt, marker);
    va_end(marker);

    return _str;
#else
    std::string strResult="";
    if (NULL != fmt)
    {
//            va_list marker = NULL;
        va_list marker;
        va_start(marker, fmt);                            //初始化变量参数
        size_t nLength = vprintf(fmt, marker) + 1;    //获取格式化字符串长度
        std::vector<char> vBuffer(nLength, '\0');        //创建用于存储格式化字符串的字符数组
        int nWritten = vsnprintf(&vBuffer[0], vBuffer.size(), fmt, marker);
        if (nWritten>0)
        {
            strResult = &vBuffer[0];
        }
        va_end(marker);                                    //重置变量参数
    }
    return strResult;
#endif
}

std::string MoudleConfig::stringReplaceAll(std::string &str, const std::string &old_value, const std::string &new_value)
{
    for(std::string::size_type pos(0); pos!=std::string::npos; pos+=new_value.length())
    {
        if((pos=str.find(old_value,pos))!=std::string::npos)
            str.replace(pos,old_value.length(),new_value);
        else
            break;
    }
    return   str;
}

void MoudleConfig::replaceChar(char *string, char oldChar, char newChar)
{
    int len = strlen(string);
    int i;
    for (i = 0; i < len; i++){
        if (string[i] == oldChar){
            string[i] = newChar;
        }
    }
}

std::string MoudleConfig::removeFirstAndLastSpace(std::string &s)
{
    if (s.empty())
    {
        return s;
    }
    s.erase(0,s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return s;
}

std::string MoudleConfig::getIpFromRtspUrl(std::string rtspUrl)
{
    std::string strIP;
    std::string strUrl = rtspUrl;
    if(strUrl.find("@") == std::string::npos)
    {
        long nPos1 = strUrl.find("//");
        long nPos2 = strUrl.rfind(":");
        if(nPos1 != std::string::npos && nPos2 != std::string::npos)
        {
            long nOffset = nPos2 - nPos1 - strlen("//");
            strIP = strUrl.substr(nPos1 + strlen("//"), nOffset);
        }
    }
    else
    {
        long nPos1 = strUrl.find("@");
        long nPos2 = strUrl.rfind(":");
        if(nPos1 != std::string::npos && nPos2 != std::string::npos)
        {
            long nOffset = nPos2 - nPos1 - strlen("@");
            strIP = strUrl.substr(nPos1 + strlen("@"), nOffset);

            int index = strIP.find("/");
            strIP = strIP.substr(0, index);
        }
    }
    return strIP;
}

void MoudleConfig::mSleep(int mSecond)
{
#if defined(WIN32)
    Sleep(mSecond);
#else
    usleep(mSecond * 1000);
#endif
}

int64_t MoudleConfig::getTimeStamp_MilliSecond()
{
    int64_t mSecond = 0; //当前毫秒数

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

    int64_t timeStamp = (int64_t)(time(NULL) * 1000) + mSecond;

    return timeStamp;

}
