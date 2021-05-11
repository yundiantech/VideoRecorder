#ifndef LOGWRITER_H
#define LOGWRITER_H

#include <time.h>
#include <string.h>
#include <list>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string>

#include "Mutex/Cond.h"

#define LOGSTR_MAX_LENGTH 512

/**
 * @brief The LogWriter class
 * д��־�� ����ʱ����־��Ϣд���ļ�  ��������־�ļ�
 */

class LogWriter
{
public:
    struct LogInfoNode
    {
        int cameraId;
        uint64_t mCreateTime; //������ʱ��(�����жϹ��˶��)
        std::string logStr;

        LogInfoNode()
        {
            cameraId = 0;
    //        memset(time, 0x0, 32);
    //        memset(logStr, 0x0, LOGSTR_MAX_LENGTH);
        }

    };

    LogWriter();
    ~LogWriter();

    void writeLog(int cameraId, const std::string &str);

    void run();

private:
    char fileName[20];

    char *mTmpBuffer;

    void addLogNode(const LogInfoNode &node);

    std::list<LogInfoNode> mLogNodeList; //���ݶ���
    Cond *mCondition;

};

#endif // LOGWRITER_H
