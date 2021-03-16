// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __TASK_H__
#define __TASK_H__

#include <IProgress.h>
#include <PvResult.h>

class IProgress;


class Task
{
#ifdef _AFXDLL
    DECLARE_DYNAMIC( Task )
#endif // _AFXDLL

public:

    Task()
        : mProgress( NULL )
    {
    }

    virtual void Execute() = 0;

    void SetProgress( IProgress *aProgress ) { mProgress = aProgress; }
    IProgress *GetProgress() { return mProgress; }

    PvResult GetResult() const { return mResult; }

protected:

    PvResult mResult;

private:

    IProgress *mProgress;

};


#endif // __TASK_H__

