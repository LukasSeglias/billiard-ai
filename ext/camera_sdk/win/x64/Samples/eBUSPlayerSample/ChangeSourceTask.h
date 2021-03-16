// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __CHANGE_SOURCE_TASK_H__
#define __CHANGE_SOURCE_TASK_H__

#include "Task.h"
#include "Player.h"

#include <assert.h>


class ChangeSourceTask : public Task
{
#ifdef _AFXDLL
    DECLARE_DYNAMIC( ChangeSourceTask )
#endif // _AFXDLL

public:

    ChangeSourceTask( Player *aPlayer, int64_t aNewSource )
        : mPlayer( aPlayer )
        , mNewSource( aNewSource )
    {
        assert( mPlayer != NULL );
    }

    virtual void Execute()
    {
        mResult = mPlayer->ChangeSourceTaskHandler( GetProgress(), mNewSource );
    }

private:

    Player *mPlayer;
    int64_t mNewSource;

};


#endif // __CHANGE_SOURCE_TASK_H__

