// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __STREAM_H__
#define __STREAM_H__

#include <IMessageSink.h>
#include <DisplayThread.h>
#include <ImageSaving.h>
#include <BufferOptions.h>
#include <Setup.h>

#include <PvStream.h>
#include <PvPipeline.h>
#include <PvLogger.h>


class Stream : public Persistent, PvPipelineEventSink
#ifdef _AFXDLL
    , public CObject
#endif // _AFXDLL
{
#ifdef _AFXDLL
    DECLARE_DYNAMIC( Stream )
#endif // _AFXDLL

public:

    Stream( IMessageSink *aSink );
    virtual ~Stream();

    static Stream *Create( const PvDeviceInfo *aDeviceInfo, IMessageSink *aSink );

    virtual PvResult Open( Setup *aSetup, const PvDeviceInfo *aDeviceInfo, uint16_t aChannel ) = 0;
    PvResult Close();

    bool IsOpened();
    bool IsStarted();
    bool IsPipelineReallocated() const { return mPipelineReallocated; }

    PvGenParameterArray *GetParameters();

    void Start( DisplayThread *aThread, ImageSaving *aSaving, BufferOptions *aOptions, PvGenParameterArray *aDeviceParameters );
    void Stop();
    
    void Reset( uint32_t aPayloadSize );
	void ResetStatistics();

    PvResult Save( PvConfigurationWriter *aWriter );
    PvResult Load( PvConfigurationReader *aReader );

    PvStream *GetStream() { return mStream; }
    PvPipeline *GetPipeline() { return mPipeline; }

    virtual std::string GetFindString() = 0;
    uint16_t GetChannel();

protected:

    // PvPipelineEventSink
    void OnBufferCreated( PvPipeline *aPipeline, PvBuffer *aBuffer );
    void OnBufferDeleted( PvPipeline *aPipeline, PvBuffer *aBuffer );
    void OnStart( PvPipeline *aPipeline );
    void OnStop( PvPipeline *aPipeline );
    void OnReset( PvPipeline *aPipeline );
    void OnBufferTooSmall( PvPipeline *aPipeline, bool *aReallocAll, bool *aResetStats );

    void SetStream( PvStream *aStream ) { mStream = aStream; }
    void SetPipeline( PvPipeline *aPipeline );

    PvLogger mLogger;

private:

    PvStream *mStream;
    PvPipeline *mPipeline;

    DisplayThread *mDisplayThread;
    ImageSaving *mImageSaving;
    BufferOptions *mBufferOptions;

    IMessageSink *mMessageSink;

    bool mPipelineReallocated;

};


#endif // __STREAM_H__

