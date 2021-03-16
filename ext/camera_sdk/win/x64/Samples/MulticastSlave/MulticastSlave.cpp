// *****************************************************************************
//
//     Copyright (c) 2008, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

//
// To receive images using PvPipeline
//

#include <PvSampleUtils.h>
#include <PvStream.h>
#include <PvDevice.h>
#include <PvPipeline.h>
#include <PvBuffer.h>
#include <PvStreamGEV.h>


PV_INIT_SIGNAL_HANDLER();


//
// PvPipeline event handler class. Used to trap buffer too small events
// and let the pipeline know we want all the buffers to be resized immediately.
//

class PipelineEventSink : public PvPipelineEventSink
{
protected:

    void OnBufferTooSmall( PvPipeline *aPipeline, bool *aReallocAll, bool *aResetStats )
    {
        *aReallocAll = true;
        *aResetStats = true;

        cout << "Buffers reallocated by the pipeline" << endl;
    }

};

//
// Starts a multicast slave.
//

bool StartSlave()
{
    // Let the user select the device to receive from.
    PvString lConnectionID;
    PvDeviceInfoType lType;
    if ( !PvSelectDevice( &lConnectionID, &lType ) )
    {
        return false;
    }

    // Multicast is only possible with GigE Vision devices
    if ( lType != PvDeviceInfoTypeGEV )
    {
        cout << "This sample only supports GigE Vision devices" << endl;
        return false;
    }

    // Create the PvStreamGEV object.
    PvStreamGEV lStreamGEV;

    // Create the PvPipeline object.
    PvPipeline lPipeline( &lStreamGEV );

    // Create a PvPipeline event sink (used to trap buffer too small events).
    PipelineEventSink lPipelineEventSink;
    lPipeline.RegisterEventSink( &lPipelineEventSink );

    // Open stream
    cout << "Opening stream" << endl;
    if ( !lStreamGEV.Open( lConnectionID, "239.192.1.1", 1042 ).IsOK() )
    {
        cout << "Error opening stream" << endl;
        return false;
    }

    // IMPORTANT: the pipeline needs to be "armed", or started before 
    // we instruct the device to send us images.
    cout << "Starting pipeline" << endl;
    lPipeline.SetBufferCount( 16 );
    lPipeline.Start();

    // Get stream parameters/stats.
    PvGenParameterArray *lStreamParams = lStreamGEV.GetParameters();
    PvGenInteger *lBlockCount = dynamic_cast<PvGenInteger *>( lStreamParams->Get( "BlockCount" ) );
    PvGenFloat *lFrameRate = dynamic_cast<PvGenFloat *>( lStreamParams->Get( "AcquisitionRate" ) );
    PvGenFloat *lBandwidth = dynamic_cast<PvGenFloat *>( lStreamParams->Get( "Bandwidth" ) );
    PvGenBoolean *lRequestMissingPackets = dynamic_cast<PvGenBoolean *>( lStreamParams->Get( "RequestMissingPackets" ) );

    // Disabling request missing packets.
    lRequestMissingPackets->SetValue( false );

    char lDoodle[] = "|\\-|-/";
    int lDoodleIndex = 0;
    int64_t lBlockCountVal = 0;
    double lFrameRateVal = 0.0;
    double lBandwidthVal = 0.0;

    // Acquire images until the user instructs us to stop.
    cout << endl;
    cout << "<press a key to stop receiving>" << endl;
    while ( !PvKbHit() )
    {
        // Retrieve next buffer     
        PvBuffer *lBuffer = NULL;
        PvResult  lOperationResult;
        PvResult lResult = lPipeline.RetrieveNextBuffer( &lBuffer, 1000, &lOperationResult );
        
        if ( lResult.IsOK() )
        {
            if (lOperationResult.IsOK())
            {
                //
                // We now have a valid buffer. This is where you would typically process the buffer.
                // -----------------------------------------------------------------------------------------
                // ...

                lBlockCount->GetValue( lBlockCountVal );
                lFrameRate->GetValue( lFrameRateVal );
                lBandwidth->GetValue( lBandwidthVal );
            
                // If the buffer contains an image, display width and height.
                uint32_t lWidth = 0, lHeight = 0;
                if ( lBuffer->GetPayloadType() == PvPayloadTypeImage )
                {
                    // Get image specific buffer interface.
                    PvImage *lImage = lBuffer->GetImage();

                    // Read width, height
                    lWidth = lImage->GetWidth();
                    lHeight = lImage->GetHeight();
                }
            
                cout << fixed << setprecision( 1 );
                cout << lDoodle[ lDoodleIndex ];
                cout << " BlockID: " << uppercase << hex << setfill('0') << setw(16) << lBuffer->GetBlockID() << " W: " << dec << lWidth << " H: " 
                     << lHeight << " " << lFrameRateVal << " FPS " << ( lBandwidthVal / 1000000.0 ) << " Mb/s\r";
            }

            // We have an image - do some processing (...) and VERY IMPORTANT,
            // release the buffer back to the pipeline.
            lPipeline.ReleaseBuffer( lBuffer );
        }
        else
        {
            // Timeout
            cout <<  lDoodle[ lDoodleIndex ] << " Timeout\r";
        }

        ++lDoodleIndex %= 6;
    }

    // We stop the pipeline - letting the object lapse out of 
    // scope would have had the destructor do the same, but we do it anyway.
    cout << "Stop pipeline" << endl;
    lPipeline.Stop();

    // Now close the stream. Also optional but nice to have.
    cout << "Closing stream" << endl;
    lStreamGEV.Close();

    // Unregistered pipeline event sink. Optional but nice to have.
    lPipeline.UnregisterEventSink( &lPipelineEventSink );

    return true;
}


//
// Main function
//

int main()
{
    PV_SAMPLE_INIT();

    cout << "MulticastSlave sample" << endl << endl;
    // Starts the multicast slave
    cout << "Starts the multicast slave" << endl << endl;
    StartSlave();

    cout << endl;
    cout << "<press a key to exit>" << endl;
    PvWaitForKeyPress();

    PV_SAMPLE_TERMINATE();

    return 0;
}

