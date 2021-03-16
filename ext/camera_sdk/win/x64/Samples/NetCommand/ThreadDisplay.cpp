// *****************************************************************************
//
//     Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"

#include "ThreadDisplay.h"


///
/// \brief Constructor
///

ThreadDisplay::ThreadDisplay()
    : mDisplay( NULL )
{
}


///
/// \brief Destructor
///

ThreadDisplay::~ThreadDisplay()
{
}


///
/// \brief Called by base class whenever a buffer is retrieved from the acquisition pipeline
///

void ThreadDisplay::OnBufferRetrieved( PvBuffer *aBuffer )
{

}


///
/// \brief Called by base class whenever a buffer needs to be displayed
///

void ThreadDisplay::OnBufferDisplay( PvBuffer *aBuffer )
{
    if ( mDisplay != NULL )
    {
        mDisplay->Display( *aBuffer );
    }
}


///
/// \brief Called by base class whenever a buffer processing has been completed, just before it is returned to the acquisition pipeline.
///

void ThreadDisplay::OnBufferDone( PvBuffer *aBuffer )
{

}

