// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __DISCONNECT_TASK_H__
#define __DISCONNECT_TASK_H__

#include "Task.h"
#include "Player.h"

#include <assert.h>


class DisconnectTask : public Task
{
#ifdef _AFXDLL
    DECLARE_DYNAMIC( DisconnectTask )
#endif // _AFXDLL

public:

    DisconnectTask( Player *aPlayer )
        : mPlayer( aPlayer )
    {
        assert( mPlayer != NULL );
    }

    virtual void Execute()
    {
        mResult = mPlayer->DisconnectTaskHandler( GetProgress() );
    }

private:

    Player *mPlayer;

};


#endif // __DISCONNECT_TASK_H__

