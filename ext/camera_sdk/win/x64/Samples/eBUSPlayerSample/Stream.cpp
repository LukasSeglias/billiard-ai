// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "eBUSPlayerShared.h"
#include "Stream.h"

#include "StreamU3V.h"
#include "StreamGEV.h"

#include <assert.h>


#define RESET ( "Reset" )
#define CHANNEL ( "Channel" )


#ifdef _AFXDLL
    IMPLEMENT_DYNAMIC( Stream, CObject )
#endif // _AFXDLL


///
/// \brief Constructor
///

Stream::Stream( IMessageSink *aSink )
    : mMessageSink( aSink )
    , mStream( NULL )
    , mPipeline( NULL )
    , mDisplayThread( NULL )
    , mImageSaving( NULL )
    , mBufferOptions( NULL )
    , mPipelineReallocated( false )
    , mLogger( "eBUSPlayer" )
{

}


///
/// \brief Destructor
///

Stream::~Stream()
{
    Close();

    assert( mDisplayThread == NULL );
    assert( mImageSaving == NULL );
    assert( mBufferOptions == NULL );

    assert( mPipeline != NULL );
    PVDELETE( mPipeline );

    assert( mStream != NULL );
    PVDELETE( mStream );
}


///
/// \brief Creates the right type of stream object based on aDeviceInfo
///

Stream *Stream::Create( const PvDeviceInfo *aDeviceInfo, IMessageSink *aSink )
{
    switch ( aDeviceInfo->GetType() )
    {
    case PvDeviceInfoTypeGEV:
        return new StreamGEV( aSink );

    case PvDeviceInfoTypeU3V:
        return new StreamU3V( aSink );
        
    default:
        break;
    }

    return NULL;
}


///
/// \brief Closes the stream
///

PvResult Stream::Close()
{
    Stop();

    return mStream->Close();
}


///
/// \brief Returns true if the stream is opened
///

bool Stream::IsOpened()
{
    return ( mStream != NULL ) && mStream->IsOpen();
}


///
/// \brief Returns true if the stream is started (pipeline and display thread running)
///

bool Stream::IsStarted()
{
    return ( mDisplayThread != NULL ) && mDisplayThread->IsRunning();
}


///
/// \brief Returns the stream parameters
///

PvGenParameterArray *Stream::GetParameters()
{
    if ( mStream != NULL )
    {
        return mStream->GetParameters();
    }

    return NULL;
}


///
/// \brief Starts the stream
///

void Stream::Start( DisplayThread *aThread, ImageSaving *aSaving, BufferOptions *aOptions, PvGenParameterArray *aDeviceParameters )
{
    PV_LOGINFO( mLogger, "Starting stream" );

    mDisplayThread = aThread;
    mImageSaving = aSaving;
    mBufferOptions = aOptions;

    mPipelineReallocated = false;

    if ( !mDisplayThread->IsRunning() )
    {
        // Start display thread
        mDisplayThread->Start( mPipeline, aDeviceParameters );
#ifdef WIN32
        mDisplayThread->SetPriority( THREAD_PRIORITY_ABOVE_NORMAL );
#elif LINUX
        mDisplayThread->SetPriority( 50 );
#endif
    }

    if ( !mPipeline->IsStarted() )
    {
        // Start pipeline thread
        mPipeline->Start();
    }
}


///
/// \brief Stops the stream
///

void Stream::Stop()
{
    PV_LOGINFO( mLogger, "Stopping stream" );

    if ( ( mDisplayThread != NULL ) && mDisplayThread->IsRunning() )
    {
        // Stop the display thread, but don't wait just yet
        mDisplayThread->Stop( false );

        // Stop the pipeline (if it was running)
        if ( mPipeline->IsStarted() )
        {
            mPipeline->Stop();
        }

        // Wait on display thread to be completed
        mDisplayThread->WaitComplete();
    }

    mDisplayThread = NULL;
    mImageSaving = NULL;
    mBufferOptions = NULL;
}


///
/// \brief Resets the pipeline, stats data receiver, etc.
///
 
void Stream::Reset( uint32_t aPayloadSize )
{
    if ( !IsOpened() )
    {
        return;
    }
	// Should clean up and reset the display thread *before* resetting pipeline because
	// display thread can be holding on to buffers that it will try to release later (after pipeline has reset)
	if (mDisplayThread != NULL)
	{
		mDisplayThread->ResetStatistics();
	}
	
    if ( aPayloadSize > 0 )
    {
        mPipeline->SetBufferSize( aPayloadSize );
    }

    mPipeline->Reset();
	ResetStatistics();
}


///
/// \brief Resets the streaming statistics
///

void Stream::ResetStatistics()
{
	if ( !IsOpened() )
	{
		return;
	}

    mPipelineReallocated = false;

    GetParameters()->GetCommand( RESET )->Execute();
    mImageSaving->ResetStats();
    mDisplayThread->ResetStatistics();
}


///
/// \brief Saves the stream configuration
///

PvResult Stream::Save( PvConfigurationWriter *aWriter )
{
    return aWriter->Store( mStream );
}


///
/// \brief Loads the stream configuration
///

PvResult Stream::Load( PvConfigurationReader *aReader )
{
    return aReader->Restore( 0, mStream );
}



///
/// \brief PvPipelineEventSink
///

void Stream::OnBufferCreated( PvPipeline *aPipeline, PvBuffer *aBuffer )
{
    
}


///
/// \brief PvPipelineEventSink
///

void Stream::OnBufferDeleted( PvPipeline *aPipeline, PvBuffer *aBuffer )
{

}


///
/// \brief PvPipelineEventSink
///

void Stream::OnStart( PvPipeline *aPipeline )
{

}


///
/// \brief PvPipelineEventSink
///

void Stream::OnStop( PvPipeline *aPipeline )
{

}


///
/// \brief PvPipelineEventSink
///

void Stream::OnReset( PvPipeline *aPipeline )
{

}


///
/// \brief PvPipelineEventSink
///

void Stream::OnBufferTooSmall( PvPipeline *aPipeline, bool *aReallocAll, bool *aResetStats )
{
    PV_LOGINFO( mLogger, "Buffer too small notification received from PvPipeline" );

    *aReallocAll = true;
    *aResetStats = true;

    // Save event for status warnings
    mPipelineReallocated = true;
}


///
/// \brief Returns the channel the stream receives data on
///

uint16_t Stream::GetChannel()
{
    if ( IsOpened() )
    {
        int64_t lValue = 0;
        if ( GetParameters()->GetIntegerValue( CHANNEL, lValue ).IsOK() )
        {
            return static_cast<uint16_t>( lValue );
        }
    }

    return 0;
}


///
/// \brief Sets the pipeline pointer
///

void Stream::SetPipeline( PvPipeline *aPipeline ) 
{ 
    mPipeline = aPipeline; 
    mPipeline->RegisterEventSink( this );
}

