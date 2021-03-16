// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __CONNECT_TASK_H__
#define __CONNECT_TASK_H__

#include "Task.h"
#include "Player.h"

#include <assert.h>


class ConnectTask : public Task
{
#ifdef _AFXDLL
    DECLARE_DYNAMIC( ConnectTask )
#endif // _AFXDLL

public:

    ConnectTask( Player *aPlayer, const PvDeviceInfo *aDI, PvConfigurationReader *aCR )
        : mPlayer( aPlayer )
        , mDI( aDI )
        , mCR( aCR )
    {
        assert( mPlayer != NULL );
        assert( ( mDI != NULL ) || ( mCR != NULL ) );
    }

    virtual void Execute()
    {
        mResult = mPlayer->ConnectTaskHandler( GetProgress(), mDI, mCR );
    }

private:

    Player *mPlayer;
    const PvDeviceInfo *mDI;
    PvConfigurationReader *mCR;

};


#endif // __CONNECT_TASK_H__

