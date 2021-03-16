// *****************************************************************************
//
//     Copyright (c) 2008, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVPIPELINE_H__
#define __PVPIPELINE_H__

#include "PvStreamLib.h"
#include "PvStream.h"
#include "PvPipelineEventSink.h"


namespace PvStreamLib
{
    class Pipeline;

}


class PV_STREAM_API PvPipeline
{

public:

    PvPipeline( PvStream *aStream );
    virtual ~PvPipeline();

    uint32_t GetBufferSize() const;
    uint32_t GetBufferCount() const;
    uint32_t GetOutputQueueSize() const;
    bool GetHandleBufferTooSmall() const;

    bool IsStarted();

    void SetBufferSize( uint32_t aSize );
    PvResult SetBufferCount( uint32_t aBufferCount );
    void SetHandleBufferTooSmall( bool aValue );

    PvResult RetrieveNextBuffer(
        PvBuffer ** aBuffer,
        uint32_t aTimeout = 0xFFFFFFFF,
		PvResult * aOperationResult = NULL );

    PvResult ReleaseBuffer( PvBuffer * aBuffer );

    PvResult Start();
    PvResult Stop();
    PvResult Reset();

	// Notifications
    PvResult RegisterEventSink( PvPipelineEventSink *aEventSink );
    PvResult UnregisterEventSink( PvPipelineEventSink *aEventSink );

    uint32_t GetBufferHandlingThreadPriority() const;
    PvResult SetBufferHandlingThreadPriority( uint32_t aPriority );

protected:

private:

    PvStreamLib::Pipeline * mThis;

	 // Not implemented
	PvPipeline( const PvPipeline& );
	const PvPipeline &operator=( const PvPipeline & );

};

#endif
