// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

//
// Shows how to use PvConfigurationWriter to store a configuration of the system and PvConfigurationReader to retrieve it.
//

#include <PvSampleUtils.h>
#include <PvDevice.h>
#include <PvDeviceGEV.h>
#include <PvDeviceU3V.h>
#include <PvStream.h>
#include <PvStreamGEV.h>
#include <PvStreamU3V.h>
#include <PvConfigurationWriter.h>
#include <PvConfigurationReader.h>


PV_INIT_SIGNAL_HANDLER();


#define FILE_NAME ( "PersistenceTest.pvxml" )

#define DEVICE_CONFIGURATION_TAG ( "DeviceConfiguration" )
#define STREAM_CONFIGURAITON_TAG ( "StreamConfiguration" )
#define STRING_INFORMATION_TAG ( "StringInformation" )

#define TEST_STRING ( "This is a test string" )


//
//  Store device and stream configuration.
//  Also store a string information.
//

bool StoreConfiguration( const PvString &aConnectionID )
{
    PvResult lResult;
    PvConfigurationWriter lWriter;

    // Connect to the GigE Vision or USB3 Vision device
    cout << "Connecting to device" << endl;
    PvDevice *lDevice = PvDevice::CreateAndConnect( aConnectionID, &lResult );
    if ( !lResult.IsOK() )
    {
        cout << "Unable to connect to device" << endl;
        PvDevice::Free( lDevice );
        return false;
    }

    // Store  with a PvDevice.
    cout << "Store device configuration" << endl;
    lWriter.Store( lDevice, DEVICE_CONFIGURATION_TAG );

    // Create and open PvStream
    cout << "Store stream configuration" << endl;
    PvStream *lStream = PvStream::CreateAndOpen( aConnectionID, &lResult );
    if ( !lResult.IsOK() )
    {
        cout << "Unable to open stream object from device" << endl;
        lDevice->Disconnect();
        PvDevice::Free( lDevice );
        return false;
    }

    // Store with a PvStream
    lWriter.Store( lStream, STREAM_CONFIGURAITON_TAG );

    // Store with a simple string.
    cout << "Store string information" << endl;
    lWriter.Store( TEST_STRING, STRING_INFORMATION_TAG );

    // Insert the path of your file here.
    cout << "Store string information" << endl;
    lWriter.Save( FILE_NAME );

    PvStream::Free( lStream );
    PvDevice::Free( lDevice );

    return true;
}


//
// Restore device configuration and verify that the device is connected. 
//

bool RestoreDevice()
{
    PvConfigurationReader lReader;
    
    // Load all the information into a reader.
    cout << "Load information and configuration" << endl;
    lReader.Load( FILE_NAME );

    PvDeviceGEV lDeviceGEV;
    PvDeviceU3V lDeviceU3V;
    PvDevice *lDevice = NULL;

    cout << "Restore configuration for a device with the configuration name" << endl;

    // Attempt restoring as a GEV device
    PvResult lResult = lReader.Restore( DEVICE_CONFIGURATION_TAG, &lDeviceGEV );
    if ( lResult.IsOK() )
    {
        // Success, keep PvDevice pointer to GEV device
        lDevice = &lDeviceGEV;
    }
    else
    {
        // Attempt restoring as a U3V device
        lResult = lReader.Restore( DEVICE_CONFIGURATION_TAG, &lDeviceU3V );
        if ( lResult.IsOK() )
        {
            // Success, keep PvDevice pointer to U3V device
            lDevice = &lDeviceU3V;
        }
        else
        {
            return false;
        }
    }

    cout << "Verify operation success" << endl;
    if ( !lDevice->IsConnected() )
    {
        return false;
    }

    lDevice->Disconnect();

    return true;
}


//
// Restore stream configuration and verify that the stream is open.
//

bool RestoreStream()
{
    PvConfigurationReader lReader;
    
    // Load all the information into a reader.
    cout << "Load information and configuration" << endl;
    lReader.Load( FILE_NAME );
    
    PvStreamGEV lStreamGEV;
    PvStreamU3V lStreamU3V;
    PvStream *lStream = NULL;

    cout << "Restore configuration for a stream with the configuration name" << endl;

    // Attempt restoring as a GEV Stream
    PvResult lResult = lReader.Restore( STREAM_CONFIGURAITON_TAG, &lStreamGEV );
    if ( lResult.IsOK() )
    {
        // Success, keep PvStream pointer to GEV Stream
        lStream = &lStreamGEV;
    }
    else
    {
        // Attempt restoring as a U3V Stream
        lResult = lReader.Restore( STREAM_CONFIGURAITON_TAG, &lStreamU3V );
        if ( lResult.IsOK() )
        {
            // Success, keep PvStream pointer to U3V Stream
            lStream = &lStreamU3V;
        }
        else
        {
            return false;
        }
    }

    cout << "Verify operation success" << endl;
    if ( !lStream->IsOpen() )
    {
        return false;
    }

    lStream->Close();

    return true;
}


//
// Restore string information and verify that the string is the same.
//

bool RestoreString()
{
    PvConfigurationReader lReader;
    
    // Load all the information into a reader.
    cout << "Load information and configuration" << endl;
    lReader.Load( FILE_NAME );

    // Restore the stream information.
    cout << "Restore information for a string with the information name" << endl;
    PvString lString;
    PvResult lResult = lReader.Restore( STRING_INFORMATION_TAG, lString );
    if ( !lResult.IsOK() )
    {
        return false;
    }

    cout << "Verify operation success" << endl;
    if ( lString != TEST_STRING )
    {
        return false;
    }

    return true;
}


//
// Main function.
//

int main()
{
    PV_SAMPLE_INIT();

    // Select device
    PvString lConnectionID;
    if ( !PvSelectDevice( &lConnectionID ) )
    {
        cout << "No device selected." << endl;
        return 0;
    }

    // Create the Buffer and fill it.
    cout << "ConfigurationReader sample" << endl << endl;
    cout << "1. Store the configuration" << endl << endl;
    if ( !StoreConfiguration( lConnectionID ) )
    {
        cout << "Cannot store the configuration correctly";
        return 0;
    }

    PvSleepMs( 1000 );

    cout << endl;
    cout << "2. Restore device configuration" << endl << endl;
    if ( !RestoreDevice() )
    {
        cout << "Cannot restore the configuration correctly";
        return 0;
    }

    PvSleepMs( 1000 );

    cout << endl;
    cout << "3. Restore stream configuration" << endl << endl;
    if ( !RestoreStream() )
    {
        cout << "Cannot restore the configuration correctly";
        return 0;
    }

    PvSleepMs( 1000 );

    cout << endl;
    cout << "4. Restore string information" << endl << endl;
    if ( !RestoreString() )
    {
        cout << "Cannot restore the configuration correctly";
        return 0;
    }

    cout << endl;
    cout << "<press a key to exit>" << endl;
    PvWaitForKeyPress();

    PV_SAMPLE_TERMINATE();

    return 0;
}

