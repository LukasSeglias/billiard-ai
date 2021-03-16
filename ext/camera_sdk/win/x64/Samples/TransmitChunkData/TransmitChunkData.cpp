// *****************************************************************************
//
// Copyright (c) 2017, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include <PvSampleUtils.h>
#include <PvTransmitterGEV.h>
#include <PvVirtualDeviceGEV.h>
#include <PvFPSStabilizer.h>

#include "Configuration.h"


PV_INIT_SIGNAL_HANDLER();


int main( int aCount, const char ** aArgs )
{
    // Creates default configuration, parse command line parameters
    Configuration lConfig;
    lConfig.ParseCommandLine( aCount, aArgs );

    // Create, initialize data generator
    DataGenerator lDG;
    lDG.AllocImages( PvPixelMono8, lConfig.GetWidth(), lConfig.GetHeight() );
    lDG.AllocChunkData( lConfig.GetChunkSize() );
    lDG.SetTestPattern( lConfig.GetTestPattern() );

    // Create transmitter, set packet size
    PvTransmitterGEV lTransmitter;
    lTransmitter.SetPacketSize( lConfig.GetPacketSize() );
    lTransmitter.SetSentBuffersTimeout( 1000 );

    // Open transmitter
    PvResult lResult = lTransmitter.Open( 
        lConfig.GetDestinationAddress(), lConfig.GetDestinationPort(), 
        lConfig.GetSourceAddress(), lConfig.GetSourcePort(), 
        true, true, 64, true, 0, 0 );
    if ( !lResult.IsOK() )
    {
        cout << "Failed to open a connection to the transmitter." << endl;
        return 1;
    }

    // Set maximum throughput (just to even out traffic, as we control throughput at the source)
    if ( static_cast<uint32_t>( lConfig.GetFPS() ) != 0 )
    {
        // Multiply image size (in bits) by FPS
        float lMax = static_cast<float>( lConfig.GetWidth() * lConfig.GetHeight() ) * ( PvGetPixelBitCount( PvPixelMono8 ) );
        lMax *= lConfig.GetFPS();

        // Since we control the throughput at the source, make sure maximum throughput is slightly
        // higher than what we need. We want to even out packet traffic, not slow down source frame rate
        lMax *= 1.1f;

        // Set max throughput
        lTransmitter.SetMaxPayloadThroughput( lMax );
    }

    cout << "Transmission stream opened:" << endl;
    cout << "Source: " << lTransmitter.GetSourceIPAddress().GetAscii() << " port " << lTransmitter.GetSourcePort() << endl;
    cout << "Destination: " << lConfig.GetDestinationAddress() << " port " << lConfig.GetDestinationPort() << endl; 

    // Create virtual device
    PvVirtualDeviceGEV lDevice;
    lDevice.SetGEVSpecificationVersion( 2, 0 );
    lDevice.SetGVCPCapabilityPacketResendCommandSupported( false );
    lDevice.SetSerialNumber( "00000001" );
    lDevice.SetDeviceVersion( "1.0" );
    lDevice.SetModelName( "eBUS SDK Transmit Chunk Data sample" );
    lDevice.SetManufacturerName( "Pleora Technologies Inc" );
    lDevice.AddTransmitterGEV( &lTransmitter );
    lDevice.StartListening( lConfig.GetSourceAddress() );

    if ( !lConfig.GetSilent() )
    {
        cout << "Press any key to begin transmitting.\r";
        PvWaitForKeyPress();
    }

    cout << "Press any key to stop transmitting." << endl;

    char lDoodle[] = "|\\-|-/";
    int lDoodleIndex = 0;

    // Reset transmitter stats
    lTransmitter.ResetStats();

    // Used to transmit at a steady frame rate
    PvFPSStabilizer lStabilizer;

    // Acquisition/transmission loop
    while ( !PvKbHit() )
    {
        // Queue buffers
        if ( ( static_cast<uint32_t>( lConfig.GetFPS() ) == 0 ) || 
             lStabilizer.IsTimeToDisplay( static_cast<uint32_t>( lConfig.GetFPS() ) ) )
        {
            // Are there buffers available for transmission?
            PvBuffer *lImage = NULL;
            PvBuffer *lChunkData = NULL;
            if ( lDG.GetNext( &lImage, &lChunkData ) )
            {
                // Queue the buffers for transmission
                lTransmitter.QueueBuffer( lImage );
                lTransmitter.QueueBuffer( lChunkData );
            }
        }

        // Retrieve transmitted buffers
        PvBuffer *lBuffer = NULL;
        while ( lTransmitter.RetrieveFreeBuffer( &lBuffer, 0 ).IsOK() )
        {
            // Queue buffers back in available buffer list
            lDG.Release( lBuffer );

            // Buffer transmission complete, display stats
            cout << fixed << setprecision( 1 );
            cout << lDoodle[ lDoodleIndex ] << " ";
            cout << "Transmitted " << ( lTransmitter.GetBlocksTransmitted() / 2 ) << " images ";
            cout << ( lTransmitter.GetInstantaneousTransmissionRate() / 2 ) << " FPS ";
            cout << "at " << lTransmitter.GetInstantaneousPayloadThroughput() / ( 1024.0f * 1024.0f ) << " Mb/s  \r";
            ++lDoodleIndex %= 6;
        }
    }

    // Close transmitter
    lTransmitter.Close();

    // Stop virtual device
    lDevice.StopListening();

    return 0;
}


