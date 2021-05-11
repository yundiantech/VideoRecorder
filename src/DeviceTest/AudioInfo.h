/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef AUDIOINFO_H
#define AUDIOINFO_H

#include <QIODevice>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QAudioInput>

class AudioInfo : public QIODevice
{
    Q_OBJECT

public:
    AudioInfo(const QAudioFormat &format, QObject *parent);
    ~AudioInfo();

    void start();
    void stop();

    qreal level() const { return m_level; }

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

private:
    const QAudioFormat m_format;
    quint32 m_maxAmplitude;
    qreal m_level; // 0.0 <= m_level <= 1.0

signals:
    void update(QByteArray);
};


#endif // AUDIOINFO_H
