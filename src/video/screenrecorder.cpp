
/**
 * Ò¶º£»Ô
 * QQÈº121376426
 * http://blog.yundiantech.com/
 */

#include "screenrecorder.h"

#include "AppConfig.h"

#include <QDateTime>
#include <QDebug>

ScreenRecorder::ScreenRecorder()
{
    m_videoThread = new GetVideoThread();
    m_audioThread = new GetVideoThread();

    m_saveVideoFileThread = new SaveVideoFileThread;

    m_videoThread->setSaveVideoFileThread(m_saveVideoFileThread);
    m_audioThread->setSaveVideoFileThread(m_saveVideoFileThread);

}


ScreenRecorder::~ScreenRecorder()
{
    delete m_videoThread;
    delete m_audioThread;

    if (m_saveVideoFileThread)
    {
        delete m_saveVideoFileThread;
    }

}

void ScreenRecorder::setFileName(QString filePath)
{
    if (m_saveVideoFileThread != NULL)
        m_saveVideoFileThread->setFileName(filePath);
}

ErroCode ScreenRecorder::init(QString videoDevName, bool useVideo, QString audioDevName, bool useAudio)
{
    m_useVideo = useVideo;
    m_useAudio = useAudio;

    if (useVideo)
    {
        ErroCode code = m_videoThread->init(videoDevName,useVideo,"",false);
        if (code != SUCCEED)
        {
            qDebug()<<"ÊÓÆµ³õÊ¼»¯Ê§°Ü";
            return code;
        }
    }

    if (useAudio)
    {
        ErroCode code = m_audioThread->init("",false,audioDevName,useAudio);
        if (code != SUCCEED)
        {
            qDebug()<<"ÒôÆµ³õÊ¼»¯Ê§°Ü";
            return code;
        }
    }


    if (m_saveVideoFileThread != NULL)
    {
        m_saveVideoFileThread->setContainsVideo(useVideo);
        m_saveVideoFileThread->setContainsAudio(useAudio);
    }

    return SUCCEED;
}

void ScreenRecorder::startRecord()
{
    if (m_saveVideoFileThread != NULL)
        m_saveVideoFileThread->startEncode();

    if (m_useVideo)
    {
        m_videoThread->startRecord();
    }

    if (m_useAudio)
    {
        m_audioThread->startRecord();
    }
}

void ScreenRecorder::pauseRecord()
{
    if (m_useVideo)
    {
        m_videoThread->pauseRecord();
    }

    if (m_useAudio)
    {
        m_audioThread->pauseRecord();
    }
}

void ScreenRecorder::restoreRecord()
{
    if (m_useVideo)
    {
        m_videoThread->restoreRecord();
    }

    if (m_useAudio)
    {
        m_audioThread->restoreRecord();
    }
}

void ScreenRecorder::stopRecord()
{    
    if (m_useVideo)
    {
        m_videoThread->stopRecord();
    }

    if (m_useVideo)
    {
        while(m_videoThread->isRunning())
        {
            AppConfig::mSleep(10);
        }
    }

    if (m_useAudio)
    {
        m_audioThread->stopRecord();
    }

    if (m_useAudio)
    {
        while(m_audioThread->isRunning())
        {
            AppConfig::mSleep(10);
        }
    }

    if (m_saveVideoFileThread != NULL)
        m_saveVideoFileThread->stopEncode();

}

void ScreenRecorder::setPicRange(int x,int y,int w,int h)
{
    m_videoThread->setPicRange(x,y,w,h);

    if (m_saveVideoFileThread != NULL)
        m_saveVideoFileThread->setWidth(w, h);

}

void ScreenRecorder::setVideoFrameRate(int value)
{
    if (m_saveVideoFileThread != NULL)
        m_saveVideoFileThread->setVideoFrameRate(value);
}

double ScreenRecorder::getVideoPts()
{
    if (m_saveVideoFileThread != NULL)
    {
        return m_saveVideoFileThread->getVideoPts();
    }
    else
    {
        return 0;
    }

}

double ScreenRecorder::getAudioPts()
{
    if (m_saveVideoFileThread != NULL)
    {
        return m_saveVideoFileThread->getAudioPts();
    }
    else
    {
        return 0;
    }
}
