// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"

#include "TransmitterThread.h"

#include "PvImage.h"

TransmitterThread::TransmitterThread()
    : mCurrentBuffersTable( NULL )
    , mDisplay( NULL )
    , mMode( TILING_MODE_RATIO )
    , mMainWnd( NULL )
{
}

TransmitterThread::~TransmitterThread()
{
    Stop();

    mTransmitter.Close();
}

bool TransmitterThread::Initialize( CWnd* aMainWnd, PvDisplayWnd* aDisplay, CurrentBuffersTable* aCurrentBuffersTable )
{
    bool lReturn;

    mCurrentBuffersTable = aCurrentBuffersTable;
    mDisplay = aDisplay;
    mMainWnd = aMainWnd;

    // Allocates the memory for the queue storage
    lReturn = mBufferFree.Initialize( TX_POOL_SIZE + 1 );

    if( lReturn )
    {
        // Load the buffer free list with the PvBuffer. This will 
        // allow us to manage the memory properly later on.
        for( int i = 0; i < TX_POOL_SIZE; i++ )
        {
            mBuffers[ i ].SetConverter( &mConverter );
            mBufferFree.Push( &mBuffers[ i ] );
        }
    }
    mBufferConvertedImage.SetConverter( &mConverter );

    return lReturn;
}

bool TransmitterThread::Configure( unsigned int aMaxInputWidth, unsigned int aMaxInputHeight, 
        TransmitterConfig& aConfig )
{
    bool lResult;
    PvResult lPvResult;

    // Reset the throttling of the output
    mThrottleOutput.Reset();
    mThrottleOutput.SetFPS( ( double ) aConfig.GetFrameRate() );

    // Check for valid tiling mode
    if( aConfig.GetTilingMode() < TILING_MODE_MIN && aConfig.GetTilingMode() >= TILING_MODE_MAX )
    {
        return false;
    }
    mMode = aConfig.GetTilingMode();
    
     // Allocate a RGB conversion buffer
    if( mBufferConvertedImage.GetWidth() != aMaxInputWidth 
        || mBufferConvertedImage.GetHeight() != aMaxInputHeight )
    {
        lResult = mBufferConvertedImage.Alloc( aMaxInputWidth, aMaxInputHeight );
        if( !lResult )
        {
            return false;
        }
    }

    // Open the transmitter connection
    if( aConfig.GetDestinationIPAddress() != mTransmitter.GetDestinationIPAddress().GetAscii()
        || aConfig.GetSourceIPAddress() != mTransmitter.GetSourceIPAddress().GetAscii()
        || mTransmitter.GetDestinationPort() != aConfig.GetDestinationPort() )
    {
        mTransmitter.Close();

        lPvResult = mTransmitter.Open( aConfig.GetDestinationIPAddress().GetBuffer(), aConfig.GetDestinationPort(), 
            aConfig.GetSourceIPAddress().GetBuffer() );

        if( !lPvResult.IsOK() )
        {
            return false;
        }
    }

    // Set the packet size
    lPvResult = mTransmitter.SetPacketSize( aConfig.GetPacketSize() );
    if( !lPvResult.IsOK() )
    {
        return false;
    }

    // Check what is the required size to be able to resize if needed
    if( mBuffers[ 0 ].GetWidth() != aConfig.GetWidth()
        || mBuffers[ 0 ].GetHeight() != aConfig.GetHeight() )
    {
        // After a stop or in the init mode, their is no buffer in the transmitter
        // we can resize the buffer without problems.
        for( int i = 0; i < TX_POOL_SIZE; i++ )
        {
            lResult = mBuffers[ i ].Alloc( aConfig.GetWidth(), aConfig.GetHeight() );
            if( !lResult )
            {
                return false;
            }
        }
    }

    // Blank all the image to not have artefact when image size change between test
    for( int i = 0; i < TX_POOL_SIZE; i++ )
    {
        mBuffers[ i ].SetZero();
    }

    // Reset the statistic
    mTransmitter.ResetStats();
    

    return true;
}

bool TransmitterThread::Start()
{
    return Thread::Start();
}
    
bool TransmitterThread::Stop()
{
    PvBuffer* lBuffer;
    ImageBuffer* lImageBuffer = NULL;

    // Kill the thread sending the data
    Thread::Stop();

    // Abort all the current transmission to be able to empty the memory
    mTransmitter.AbortQueuedBuffers();
    while( mTransmitter.RetrieveFreeBuffer( &lBuffer, 0 ).IsOK() )
    {
        if( lBuffer )
        {
            lImageBuffer = ImageBuffer::GetImageBufferFromPvBuffer( lBuffer );
            mBufferFree.Push( lImageBuffer );
        }
    }

    // Blank all the image to not have artefact when image size change between test
    for( int i = 0; i < TX_POOL_SIZE; i++ )
    {
        mBuffers[ i ].SetZero();
    }

    return true;
}

DWORD TransmitterThread::Function()
{
    ImageBuffer* lImageBuffer = NULL;
    PvBuffer* lBuffer = NULL;
    DWORD lWaitTime;

    mDisplayRate.Reset();

    mThrottleOutput.Prepare();

    while ( !IsStopping() )
    {
        mThrottleOutput.Start();

        // Get a free buffer
        lImageBuffer = mBufferFree.Pop();
        if( lImageBuffer )
        {
            // First we get a snapshot from the current buffer table
            mCurrentBuffersTable->LockSnapshot();

            // Convert the buffers into the tiling format
            Convert( lImageBuffer );

            // The snapshot is not useful anymore. 
            // Release it
            mCurrentBuffersTable->UnlockSnapshot();

            // Transmit the image over the network
            mTransmitter.QueueBuffer( lImageBuffer->GetPvBuffer() );

            // This pointer is not valid anymore because the buffer is into the transmitter
            lImageBuffer = NULL;
        }

        // Check if any buffer has been transmitted properly and can be retrieve
        // This is a while loop to ensure we can be late and recover if needed
        lBuffer = NULL;
        lWaitTime = mThrottleOutput.WaitTime();
        while( mTransmitter.RetrieveFreeBuffer( &lBuffer, lWaitTime ).GetCode() != PvResult::Code::TIMEOUT )
        {
            if( lBuffer )
            {
                lImageBuffer = ImageBuffer::GetImageBufferFromPvBuffer( lBuffer );

                // Here we will possibly make the display of a subset of the transmission rate
                // to avoid making more refresh than the monitor can support
                if( lBuffer->GetOperationResult().IsOK() 
                    && mDisplayRate.IsTimeToDisplay( DISPLAY_FPS ) )
                {
                    // Display the image to the monitor if this is the time
                    // NOTE: To simplify the sample, this is done by the transmitter thread. In some case, 
                    // more complex, the user may want to dispatch this task to another thread in parallel.
                    mDisplay->Display( *( lImageBuffer->GetPvBuffer() ), false );
                }

                // The buffer is not needed anymore, return it to the pool
                mBufferFree.Push( lImageBuffer );
                lBuffer = NULL;
            }
           
            // Get the next amount of ms too sleep
            lWaitTime = mThrottleOutput.WaitTime();
            if( lWaitTime == 0 ) 
            {
                break;
            }
        }

    }

    return 0;
}

void TransmitterThread::Convert( ImageBuffer* aBuffer )
{
    bool lResult;
    PvBuffer* lSrcBuffer;
    PvImage* lSrcImage;
    int lHeight;
    int lWidth;
    int lTileWidth;
    int lTileHeight;

    aBuffer->SetZero();

    lTileWidth = aBuffer->GetWidth() / MAX_TILES_ROW;
    lTileHeight = aBuffer->GetHeight() / MAX_TILES_COLUMN;

    // For each of the snapshot buffer, crop them into the 
    for( int i = 0; i < MAX_TILES_ROW; i++ )
    {
        for( int j = 0; j < MAX_TILES_COLUMN; j++ )
        {
            lSrcBuffer = mCurrentBuffersTable->GetBufferFromSnapshot( i, j );
            // In case of not pointer, the resulting tile is already blank from the initialization
            if( lSrcBuffer )
            {
                lSrcImage = lSrcBuffer->GetImage();
                if( lSrcImage )
                {
                    // Because of the possible case of receiver only mode, the mBufferConvertedImage
                    // may not be able to contain the whole image and require the re-allocation of the
                    // memory on the fly. In a real system, if the image size can be determined, this 
                    // re-allocation should be avoided.
                    if( mBufferConvertedImage.GetWidth() < ( int ) lSrcImage->GetWidth()
                        || mBufferConvertedImage.GetHeight() < ( int ) lSrcImage->GetHeight() ) 
                    {
                        lHeight = max( mBufferConvertedImage.GetWidth(), ( int ) lSrcImage->GetWidth() );
                        lWidth = max( mBufferConvertedImage.GetHeight(), ( int ) lSrcImage->GetHeight() );

                        lResult = mBufferConvertedImage.Alloc( lWidth, lHeight );
                        if( !lResult )
                        {
                            // This is a case we will not be able to recover. Better stop the system
                            // now and not corrupt it!
                            mMainWnd->PostMessage( WM_CONVERSIONFAIL, 0, 0 );
                            return;
                        }
                    }
 
                    if( ( int ) lSrcImage->GetWidth() != 0 && ( int ) lSrcImage->GetHeight() != 0 )
                    {
                        // When attaching the buffer, the buffer is automatically converted to a format compatible with CImage.
                        lResult = mBufferConvertedImage.Copy( lSrcBuffer );
                        if( lResult )
                        {
                            switch( mMode )
                            {
                            case TILING_MODE_RATIO:
                                // This will stretch the image preserving the ratio into the buffer directly
                                mBufferConvertedImage.FitToWithRatio( aBuffer, lTileWidth * j, lTileHeight * i, lTileWidth, lTileHeight );
                                break;
                            case TILING_MODE_STRETCH:
                                // This will stretch the image to the buffer directly
                                mBufferConvertedImage.StretchTo( aBuffer, lTileWidth * j, lTileHeight * i, lTileWidth, lTileHeight );
                                break;
                            case TILING_MODE_CROP:
                                // This will crop the image to the buffer directly
                                mBufferConvertedImage.CropTo( aBuffer, lTileWidth * j, lTileHeight * i, lTileWidth, lTileHeight );
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

void TransmitterThread::GetStats( TransmitterStats& aStats )
{
    aStats.BlocksTransmitted = ( unsigned int ) mTransmitter.GetBlocksTransmitted();
    aStats.InstantaneousTransmissionRate = ( float ) mTransmitter.GetInstantaneousTransmissionRate();
    aStats.InstantaneousPayloadThroughput = ( float ) ( mTransmitter.GetInstantaneousPayloadThroughput() / 1000000.0f );
    aStats.DisplayFrameRate = ( unsigned int ) mDisplayRate.GetAverage();
}

