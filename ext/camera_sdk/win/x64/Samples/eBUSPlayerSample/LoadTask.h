// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __LOAD_TASK_H__
#define __LOAD_TASK_H__

#include "Task.h"
#include "Player.h"

#include <assert.h>

#include <PvConfigurationReader.h>


class LoadTask : public Task
{
#ifdef _AFXDLL
    DECLARE_DYNAMIC( LoadTask )
#endif // _AFXDLL

public:

    LoadTask( Player *aPlayer, PvConfigurationReader *aReader, PvStringList &aErrorList )
        : mPlayer( aPlayer )
        , mReader( aReader )
        , mErrorList( aErrorList )
    {
        assert( aPlayer != NULL );
        assert( mReader!= NULL );
    }

    virtual void Execute()
    {
        mResult = mPlayer->OpenConfigTaskHandler( GetProgress(), mReader, mErrorList );
    }

private:

    Player *mPlayer;
    PvConfigurationReader *mReader;
    PvStringList &mErrorList;

};


#endif // __LOAD_TASK_H__

