// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "eBUSPlayerShared.h"
#include "Mutex.h"


#ifdef _AFXDLL
    IMPLEMENT_DYNAMIC( Mutex, CObject )
#endif // _AFXDLL


///
/// \brief Constructor
///

Mutex::Mutex()
{
#ifndef _AFXDLL
    pthread_mutex_init( &mMutex, NULL );
#endif
}


///
/// \brief Destructor
///

Mutex::~Mutex()
{
#ifndef _AFXDLL
    pthread_mutex_destroy( &mMutex );
#endif
}


///
/// \brief Locks the mutex
///

void Mutex::Lock()
{
#ifdef _AFXDLL
    mMutex.Lock();
#else
    pthread_mutex_lock( &mMutex );
#endif
}


///
/// \brief Unlocks the mutex
///

void Mutex::Unlock()
{
#ifdef _AFXDLL
    mMutex.Unlock();
#else
    pthread_mutex_unlock( &mMutex );
#endif
}


