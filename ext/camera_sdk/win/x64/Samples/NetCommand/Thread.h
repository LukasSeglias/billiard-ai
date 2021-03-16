// *****************************************************************************
//
//     Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "PvBuffer.h"


class Thread
{
public:

    Thread();
    ~Thread();

    void Start();
    void Stop();

    void SetPriority( int aPriority );
    int GetPriority() const;

    bool IsDone();
    DWORD GetReturnValue();

protected:

    static unsigned long WINAPI Link( void *aParam );
    virtual DWORD Function() = 0;

    bool IsStopping() const;

private:

    HANDLE mHandle;
    DWORD mID;

    bool mStop;

    DWORD mReturnValue;
};

