// *****************************************************************************
//
//     Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include <PvDisplayThread.h>
#include <PvDisplayWnd.h>


class ThreadDisplay : public PvDisplayThread
{
public:

    ThreadDisplay();
    ~ThreadDisplay();

    void SetDisplay( PvDisplayWnd *aDisplay ) { mDisplay = aDisplay; }

protected:

    void OnBufferRetrieved( PvBuffer *aBuffer );
    void OnBufferDisplay( PvBuffer *aBuffer );
    void OnBufferDone( PvBuffer *aBuffer );

private:
    
    PvDisplayWnd *mDisplay;

};

