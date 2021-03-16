// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __SAVE_TASK_H__
#define __SAVE_TASK_H__

#include "Task.h"
#include "Player.h"

#include <assert.h>

#include <PvConfigurationWriter.h>


class SaveTask : public Task
{
#ifdef _AFXDLL
    DECLARE_DYNAMIC( SaveTask )
#endif // _AFXDLL

public:

    SaveTask( Player *aPlayer, PvConfigurationWriter *aWriter, PvStringList &aErrorList )
        : mPlayer( aPlayer )
        , mWriter( aWriter )
        , mErrorList( aErrorList )
    {
        assert( mPlayer != NULL );
        assert( aWriter != NULL );
    }

    virtual void Execute()
    {
        mResult = mPlayer->SaveConfigTaskHandler( GetProgress(), mWriter, mErrorList, true );
    }

private:

    Player *mPlayer;
    PvConfigurationWriter *mWriter;
    PvStringList &mErrorList;

};


#endif // __SAVE_TASK_H__

