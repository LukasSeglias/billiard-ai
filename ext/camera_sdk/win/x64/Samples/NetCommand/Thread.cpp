// *****************************************************************************
//
// Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// -----------------------------------------------------------------------------
//
// Generic thread class used by NetCommand.
// 
// *****************************************************************************

#include "stdafx.h"

#include "Thread.h"


// ==========================================================================
Thread::Thread()
    : mHandle( 0 )
    , mID( 0 )
    , mStop( false )
    , mReturnValue( 0 )
{
}

// ==========================================================================
Thread::~Thread()
{
    if ( mHandle != INVALID_HANDLE_VALUE )
    {
        Stop();
    }
}

// ==========================================================================
void Thread::Start()
{
    mHandle = ::CreateThread(
        NULL,               // Security attributes
        0,                  // Stack size, 0 is default
        Link,               // Start address
        this,               // Parameter
        0,                  // Creation flags
        &mID );             // Thread ID
}

// ==========================================================================
void Thread::Stop()
{
    ASSERT( mHandle != INVALID_HANDLE_VALUE );

    mStop = true;
    DWORD lRetVal = ::WaitForSingleObject( mHandle, INFINITE );
    ASSERT( lRetVal != WAIT_TIMEOUT  );

    ::CloseHandle( mHandle );
    mHandle = INVALID_HANDLE_VALUE;

    mID = 0;
}

// ==========================================================================
void Thread::SetPriority( int aPriority )
{
    ASSERT( mHandle != INVALID_HANDLE_VALUE );
    ::SetThreadPriority( mHandle, aPriority );
}

// ==========================================================================
int Thread::GetPriority() const
{
    ASSERT( mHandle != INVALID_HANDLE_VALUE );
    return ::GetThreadPriority( mHandle );
}

// ==========================================================================
bool Thread::IsStopping() const
{
    ASSERT( mHandle != INVALID_HANDLE_VALUE );
    return mStop;
}

// ==========================================================================
bool Thread::IsDone()
{
    if ( ( mHandle == INVALID_HANDLE_VALUE ) ||
         ( mID == 0 ) )
    {
        return true;
    }

    return ( ::WaitForSingleObject( mHandle, 0 ) == WAIT_OBJECT_0 );
}

// ==========================================================================
unsigned long WINAPI Thread::Link( void *aParam )
{
    Thread *lThis = reinterpret_cast<Thread *>( aParam );
    lThis->mReturnValue = lThis->Function();
    return lThis->mReturnValue;
}

// ==========================================================================
DWORD Thread::GetReturnValue()
{
    return mReturnValue;
}

