#ifndef MUTEX_H
#define MUTEX_H


#if defined(WIN32)
    #include <WinSock2.h>
    #include <Windows.h>
//#elif defined(Q_OS_LINUX)
#else
    #include <pthread.h>
#endif

class Mutex
{
public:
    Mutex();
    ~Mutex();

    //ȷ��ӵ�л��������̶߳Ա�������Դ�Ķ��Է���
    int Lock() const;

    //�ͷŵ�ǰ�߳�ӵ�еĻ��������ʹ�����߳̿���ӵ�л�����󣬶Ա�������Դ���з���
    int Unlock() const;

private:

#if defined(WIN32)
     HANDLE m_mutex;
#else
    pthread_mutex_t mutex;
#endif

};

#endif // MUTEX_H
