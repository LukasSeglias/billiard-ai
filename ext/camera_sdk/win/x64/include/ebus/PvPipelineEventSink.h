// *****************************************************************************
//
//     Copyright (c) 2008, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVPIPELINEEVENTSINK_H__
#define __PVPIPELINEEVENTSINK_H__

#include "PvStreamLib.h"


class PvPipeline;
class PvBuffer;


class PV_STREAM_API PvPipelineEventSink
{
public:

    PvPipelineEventSink();
    virtual ~PvPipelineEventSink();

    // Notifications
    virtual void OnBufferCreated( PvPipeline *aPipeline, PvBuffer *aBuffer );
    virtual void OnBufferDeleted( PvPipeline *aPipeline, PvBuffer *aBuffer );
    virtual void OnStart( PvPipeline *aPipeline );
    virtual void OnStop( PvPipeline *aPipeline );
    virtual void OnReset( PvPipeline *aPipeline );
    virtual void OnBufferTooSmall( PvPipeline *aPipeline, bool *aReallocAll, bool *aResetStats );
    virtual void OnBufferReady( PvPipeline *aPipeline );

};

#endif
