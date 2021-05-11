#include "ShareMemery.h"

ShareMemery::ShareMemery(QObject *parent) :
    QObject(parent)
{

}

ShareMemery::~ShareMemery()
{

}

void ShareMemery::setKey(const QString &key)
{
    mSharedMemory.setKey(key);
}

bool ShareMemery::sendMessage(const QString &message)
{
    bool isSucceed = false;

    if (!mSharedMemory.create(1024))
    {
        if (mSharedMemory.attach())
        {
            mSharedMemory.lock();
            char * to = static_cast<char*>(mSharedMemory.data());
            const char * from = message.toUtf8().data();
            ::memcpy(to, from, message.size());
            mSharedMemory.unlock();

            mSharedMemory.detach();
        }
    }

    return isSucceed;
}
