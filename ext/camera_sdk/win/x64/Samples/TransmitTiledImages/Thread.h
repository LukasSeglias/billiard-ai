// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

///
/// \class Thread
///
/// \brief Basic thread class
///
class Thread
{
public:

    Thread();
    ~Thread();

    virtual bool Start();
    virtual bool Stop();

    void SetPriority( int aPriority );
    int GetPriority() const;

    bool IsDone();
    DWORD GetReturnValue();

    bool IsStarted();

protected:

    static unsigned long WINAPI Link( void *aParam );
    virtual DWORD Function() = 0;

    bool IsStopping() const;

    HANDLE mStartEvent;

private:

    HANDLE mHandle;

    DWORD mID;

    bool mStop;

    DWORD mReturnValue;
};

