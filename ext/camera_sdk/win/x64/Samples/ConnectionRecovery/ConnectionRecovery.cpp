// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

//
// To receives images using a PvPipeline, full recovery management.
//

#include <PvSampleUtils.h>
#include <PvDevice.h>
#include <PvPipeline.h>
#include <PvBuffer.h>
#include <PvStream.h>
#include <PvStreamGEV.h>
#include <PvStreamU3V.h>
#include <PvDeviceInfoGEV.h>
#include <PvDeviceInfoU3V.h>

#include <list>


PV_INIT_SIGNAL_HANDLER();


//
// Application class.
//

class ConnectionRecoveryApp : protected PvDeviceEventSink
{
public:

    ConnectionRecoveryApp();
    ~ConnectionRecoveryApp();

    bool Run();

protected:

    bool SelectDevice();
    bool ConnectDevice();
    void DisconnectDevice();
    bool OpenStream();
    void CloseStream();
    bool StartAcquisition();
    bool StopAcquisition();
    void ApplicationLoop();
    void TearDown( bool aStopAcquisition );

    // Inherited from PvDeviceEventSink.
    void OnLinkDisconnected( PvDevice* aDevice );

private:

    PvString mConnectionID;

    PvDevice* mDevice;
    PvStream* mStream;
    PvPipeline* mPipeline;

    bool mConnectionLost;

};


//
// Constructor.
//

ConnectionRecoveryApp::ConnectionRecoveryApp()
    : mDevice( NULL )
    , mStream( NULL )
    , mPipeline( NULL )
    , mConnectionLost( false )
{
}


//
// Destructor.
//

ConnectionRecoveryApp::~ConnectionRecoveryApp()
{   
}


//
// Main, only public function. Runs the application
//

bool ConnectionRecoveryApp::Run()
{
    // Select device
    if ( !SelectDevice() )
    {
        return false;
    }

    // Acquisition loop. Will break when user hits a key
    ApplicationLoop();

    // Closes, disconnects, etc.
    TearDown( true );

    return true;
}


//
// Selects a device to work with
//

bool ConnectionRecoveryApp::SelectDevice()
{
    std::cout << "--> SelectDevice" << std::endl;

    // Select the device
    if ( !PvSelectDevice( &mConnectionID ) )
    {
        std::cout << "No device selected." << std::endl;
        return false;
    }

    // IMPORTANT: 
    //
    // Here we assume that the device will come back with the same IP 
    // address either through DHCP, static IP configuration or simple network
    // unplug/replug LLA. If the device can come back with a different IP address
    // the MAC address of the device should be used as connection ID. This will
    // not be as efficient but will allow reconnecting the device even if it comes
    // back with a different IP address. 
    //
    // This does not apply to USB3 Vision devices who should always stick to the same device GUID.
    //

    return true;
}


//
// Selects, connects a device.
//

bool ConnectionRecoveryApp::ConnectDevice()
{
    std::cout << "--> ConnectDevice " <<  mConnectionID.GetAscii() << std::endl;

    // Connect to the selected Device
    PvResult lResult = PvResult::Code::INVALID_PARAMETER;
    mDevice = PvDevice::CreateAndConnect( mConnectionID, &lResult );
    if ( !lResult.IsOK() )
    {
        return false;
    }

    // Register this class as an event sink for PvDevice call-backs
    mDevice->RegisterEventSink( this );

    // Clear connection lost flag as we are now connected to the device
    mConnectionLost = false;

    return true;
}


//
// Opens stream, pipeline, allocates buffers
//

bool ConnectionRecoveryApp::OpenStream()
{
    std::cout << "--> OpenStream" << std::endl;

    // Creates and open the stream object based on the selected device.
    PvResult lResult = PvResult::Code::INVALID_PARAMETER;
    mStream = PvStream::CreateAndOpen( mConnectionID, &lResult );
    if ( !lResult.IsOK() )
    {
        std::cout << "Unable to open the stream" << std::endl;
        return false;
    }
    
    mPipeline = new PvPipeline( mStream );

    // Reading payload size from device
    int64_t lSize = mDevice->GetPayloadSize();

    // Create, init the PvPipeline object
    mPipeline->SetBufferSize( static_cast<uint32_t>( lSize ) );
    mPipeline->SetBufferCount( 16 );

    // The pipeline needs to be "armed", or started before  we instruct the device to send us images
    lResult = mPipeline->Start();
    if ( !lResult.IsOK() )
    {
        std::cout << "Unable to start pipeline" << std::endl;
        return false;
    }

    // Only for GigE Vision, if supported
    PvGenBoolean *lRequestMissingPackets = dynamic_cast<PvGenBoolean *>( mStream->GetParameters()->GetBoolean( "RequestMissingPackets" ) );
    if ( ( lRequestMissingPackets != NULL ) && lRequestMissingPackets->IsAvailable() )
    {
        // Disabling request missing packets.
        lRequestMissingPackets->SetValue( false );
    }

    return true;
}


//
// Closes the stream, pipeline
//

void ConnectionRecoveryApp::CloseStream()
{
    std::cout << "--> CloseStream" << std::endl;

    if ( mPipeline != NULL )
    {
        if (  mPipeline->IsStarted() )
        {
            if ( !mPipeline->Stop().IsOK() )
            {
                std::cout << "Unable to stop the pipeline." << std::endl;
            }
        }

        delete mPipeline;
        mPipeline = NULL;
    }

    if ( mStream != NULL )
    {
        if ( mStream->IsOpen() )
        {
            if ( !mStream->Close().IsOK() )
            {
                std::cout << "Unable to stop the stream." << std::endl;
            }
        }

        PvStream::Free( mStream );
        mStream = NULL;
    }
}


//
// Starts image acquisition
//

bool ConnectionRecoveryApp::StartAcquisition()
{
    std::cout << "--> StartAcquisition" << std::endl;

    // Flush packet queue to make sure there is no left over from previous disconnect event
    PvStreamGEV* lStreamGEV = dynamic_cast<PvStreamGEV*>( mStream );
    if ( lStreamGEV != NULL )
    {
        lStreamGEV->FlushPacketQueue(); 
    }

    // Set streaming destination (only GigE Vision devces)
    PvDeviceGEV* lDeviceGEV = dynamic_cast<PvDeviceGEV*>( mDevice );
    if ( lDeviceGEV != NULL )
    {
        // If using a GigE Vision, it is same to assume the stream object is GigE Vision as well
        PvStreamGEV* lStreamGEV = static_cast<PvStreamGEV*>( mStream );

        // Have to set the Device IP destination to the Stream
        PvResult lResult = lDeviceGEV->SetStreamDestination( lStreamGEV->GetLocalIPAddress(), lStreamGEV->GetLocalPort() );
        if ( !lResult.IsOK() )
        {
            std::cout << "Setting stream destination failed" << std::endl;
            return false;
        }
    }   

    // Enables stream before sending the AcquisitionStart command.
    mDevice->StreamEnable();

    // The pipeline is already "armed", we just have to tell the device to start sending us images
    PvResult lResult = mDevice->GetParameters()->ExecuteCommand( "AcquisitionStart" );
    if ( !lResult.IsOK() )
    {
        std::cout << "Unable to start acquisition" << std::endl;
        return false;
    }

    return true;
}


//
// Stops acquisition
//

bool ConnectionRecoveryApp::StopAcquisition()
{
    std::cout << "--> StopAcquisition" << std::endl;

    // Tell the device to stop sending images.
    mDevice->GetParameters()->ExecuteCommand( "AcquisitionStop" );

    // Disable stream after sending the AcquisitionStop command.
    mDevice->StreamDisable();

    PvDeviceGEV* lDeviceGEV = dynamic_cast<PvDeviceGEV*>( mDevice );
    if ( lDeviceGEV != NULL )
    {
        // Reset streaming destination (optional...)
        lDeviceGEV->ResetStreamDestination();
    }

    return true;
}


//
// Acquisition loop
//

void ConnectionRecoveryApp::ApplicationLoop()
{
    std::cout << "--> ApplicationLoop" << std::endl;

    char  lDoodle[] = "|\\-|-/";
    int lDoodleIndex = 0;
    bool lFirstTimeout = true;

    int64_t lImageCountVal = 0;
    double lFrameRateVal = 0.0;
    double lBandwidthVal = 0.0;

    // Acquire images until the user instructs us to stop.
    while ( !PvKbHit() )
    {
        // If connection flag is up, teardown device/stream
        if ( mConnectionLost && ( mDevice != NULL ) )
        {
            // Device lost: no need to stop acquisition
            TearDown( false );
        }

        // If the device is not connected, attempt reconnection
        if ( mDevice == NULL )
        {
            if ( ConnectDevice() )
            {
                // Device is connected, open the stream
                if ( OpenStream() )
                {
                    // Device is connected, stream is opened: start acquisition
                    if ( !StartAcquisition() )
                    {
                        TearDown( false );
                    }
                }
                else
                {
                    TearDown( false );
                }
            }
        }

        // If still no device, no need to continue the loop
        if ( mDevice == NULL )
        {
            continue;
        }

        if ( ( mStream != NULL ) && mStream->IsOpen() && 
             ( mPipeline != NULL ) && mPipeline->IsStarted() )
        {
            // Retrieve next buffer     
            PvBuffer *lBuffer = NULL;
            PvResult  lOperationResult;
            PvResult lResult = mPipeline->RetrieveNextBuffer( &lBuffer, 1000, &lOperationResult );
            
            if ( lResult.IsOK() )
            {
                if (lOperationResult.IsOK())
                {
                    //
                    // We now have a valid buffer. This is where you would typically process the buffer.
                    // -----------------------------------------------------------------------------------------
                    // ...

                    mStream->GetParameters()->GetIntegerValue( "BlockCount", lImageCountVal );
                    mStream->GetParameters()->GetFloatValue( "AcquisitionRate", lFrameRateVal );
                    mStream->GetParameters()->GetFloatValue( "Bandwidth", lBandwidthVal );
                
                    // If the buffer contains an image, display width and height.
                    uint32_t lWidth = 0, lHeight = 0;
                    if ( lBuffer->GetPayloadType() == PvPayloadTypeImage )
                    {
                        // Get image specific buffer interface.
                        PvImage *lImage = lBuffer->GetImage();

                        // Read width, height.
                        lWidth = lImage->GetWidth();
                        lHeight = lImage->GetHeight();
                    }
                    
                    std::cout << fixed << setprecision( 1 );
                    std::cout << lDoodle[ lDoodleIndex ];
                    std::cout << " BlockID: " << uppercase << hex << setfill('0') << setw(16) << lBuffer->GetBlockID() << " W: " << dec << lWidth << " H: " 
                         << lHeight << " " << lFrameRateVal << " FPS " << ( lBandwidthVal / 1000000.0 ) << " Mb/s  \r";

                    lFirstTimeout = true;
                }
                // We have an image - do some processing (...) and VERY IMPORTANT,
                // release the buffer back to the pipeline.
                mPipeline->ReleaseBuffer( lBuffer );
            }
            else
            {
                // Timeout
                
                if ( lFirstTimeout )
                {
                    std::cout << "" << std::endl;
                    lFirstTimeout = false;
                }

                std::cout << "Image timeout " << lDoodle[ lDoodleIndex ] << std::endl;
            }

            ++lDoodleIndex %= 6;
        }
        else
        {
            // No stream/pipeline, must be in recovery. Wait a bit...
            PvSleepMs( 100 );
        }
    }

    PvGetChar(); // Flush key buffer for next stop.
    std::cout << "" << std::endl;
}


//
// \brief Disconnects the device
//

void ConnectionRecoveryApp::DisconnectDevice()
{
    std::cout << "--> DisconnectDevice" << std::endl;

    if ( mDevice != NULL )
    {
        if ( mDevice->IsConnected() )
        {
            // Unregister event sink (call-backs).
            mDevice->UnregisterEventSink( this );
        }

        PvDevice::Free( mDevice );
        mDevice = NULL;
    }
}


//
// Tear down: closes, disconnects, etc.
//

void ConnectionRecoveryApp::TearDown( bool aStopAcquisition )
{
    std::cout << "--> TearDown" << std::endl;

    if ( aStopAcquisition )
    {
        StopAcquisition();
    }

    CloseStream();
    DisconnectDevice();
}


//
// PvDeviceEventSink callback
//
// Notification that the device just got disconnected.
//

void ConnectionRecoveryApp::OnLinkDisconnected( PvDevice *aDevice )
{
    std::cout << "=====> PvDeviceEventSink::OnLinkDisconnected callback" << std::endl;

    // Just set flag indicating we lost the device. The main loop will tear down the 
    // device/stream and attempt reconnection. 
    mConnectionLost = true;

    // IMPORTANT:
    // The PvDevice MUST NOT be explicitly disconnected from this callback. 
    // Here we just raise a flag that we lost the device and let the main loop
    // of the application (from the main application thread) perform the
    // disconnect.
    //
}


//
// Main function
//

int main()
{
    PV_SAMPLE_INIT();

    // Receives images using a PvPipeline, full recovery management.
    std::cout << "***********************************************************" << std::endl;
    std::cout << "ConnectionRecovery sample- image acquisition from a device." << std::endl;
    std::cout << "*---------------------------------------------------------*" << std::endl;
    std::cout << "* It is recommended to use a persistent, fixed IP address *" << std::endl;
    std::cout << "* or GUID on a device when relying on automatic recovery. *" << std::endl;
    std::cout << "***********************************************************" << std::endl;
    std::cout << "<press a key to terminate the application>" << std::endl;

    ConnectionRecoveryApp lApplication; 
    bool lRetVal = lApplication.Run();

    std::cout << std::endl;
    std::cout << "<press a key to exit>" << std::endl;
    PvWaitForKeyPress();

    PV_SAMPLE_TERMINATE();

    return ( lRetVal ) ? 0 : -1;
}


