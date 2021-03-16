// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __MUTEX_H__
#define __MUTEX_H__

#ifndef _AFXDLL
#include <pthread.h>
#endif


class Mutex
{
#ifdef _AFXDLL
    DECLARE_DYNAMIC( Mutex )
#endif // _AFXDLL

public:

    Mutex();
    ~Mutex();

    void Lock();
    void Unlock();

protected:

private:

#ifdef _AFXDLL
    CMutex mMutex;
#else
    pthread_mutex_t  mMutex;
#endif

};

class MutexHolder
{
public:
    MutexHolder( Mutex *mutex )
    {
        mMutex = mutex;
        mMutex->Lock();
    }
    ~MutexHolder()
    {
        mMutex->Unlock();
    }
private:
    Mutex *mMutex;
};

#endif // __MUTEX_H__

