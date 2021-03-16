// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVSTREAMEVENTSINK_H__
#define __PVSTREAMEVENTSINK_H__

#include "PvStreamLib.h"


class PvBuffer;


class PV_STREAM_API PvStreamEventSink
{
public:

    PvStreamEventSink();
    virtual ~PvStreamEventSink();

    virtual void OnBufferQueued( PvBuffer *aBuffer );
    virtual void OnBufferRetrieved( PvBuffer* aBuffer );

};

#endif
