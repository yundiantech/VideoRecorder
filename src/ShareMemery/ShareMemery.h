#ifndef SHAREMEMERY_H
#define SHAREMEMERY_H

#include <QObject>
#include <QSharedMemory>

namespace Ui {
class ShareMemery;
}

class ShareMemery : public QObject
{
    Q_OBJECT

public:
    explicit ShareMemery(QObject *parent = nullptr);
    ~ShareMemery();

    void setKey(const QString &key);

    bool sendMessage(const QString &message);

private:
    QSharedMemory mSharedMemory;

};

#endif // SHAREMEMERY_H
