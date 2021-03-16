// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

//
// This sample shows you how to control features programmatically.
//

#include <PvSampleUtils.h>
#include <PvDevice.h>
#include <PvDeviceGEV.h>
#include <PvDeviceU3V.h>
#include <PvGenParameterArray.h>
#include <PvGenParameter.h>
#include <PvStream.h>
#include <PvStreamGEV.h>
#include <PvStreamU3V.h>


PV_INIT_SIGNAL_HANDLER();


//
// Have the user select a device and connect the PvDevice object
// to the user's selection.
// Notes: User is responsible to delete PvDevice object when it is no longer needed.
//

PvDevice *Connect( const PvString &aConnectionID )
{
    PvResult lResult;

    // Connect to the GigE Vision or USB3 Vision device
    cout << "Connecting device" << endl;
    PvDevice *lDevice = PvDevice::CreateAndConnect( aConnectionID, &lResult );
    if ( !lResult.IsOK() )
    {
        cout << "Unable to connect to device" << endl;
        PvDevice::Free( lDevice );
        return NULL;
    }

    return lDevice;
}

//
// Dumps the full content of a PvGenParameterArray.
//

bool DumpGenParameterArray( PvGenParameterArray *aArray )
{
    // Getting array size
    uint32_t lParameterArrayCount = aArray->GetCount();
    cout << endl;
    cout << "Array has " << lParameterArrayCount << " parameters" << endl;

    // Traverse through Array and print out parameters available.
    for( uint32_t x = 0; x < lParameterArrayCount; x++ )
    {
        // Get a parameter
        PvGenParameter *lGenParameter = aArray->Get( x );

        // Don't show invisible parameters - display everything up to Guru.
        if ( !lGenParameter->IsVisible( PvGenVisibilityGuru ) )
        {
            continue;
        }

        // Get and print parameter's name.
        PvString lGenParameterName, lCategory;
        lGenParameter->GetCategory( lCategory );
        lGenParameter->GetName( lGenParameterName );
        cout << lCategory.GetAscii() << ":" << lGenParameterName.GetAscii() << ", ";

        // Parameter available?
        if ( !lGenParameter->IsAvailable() )
        {
            cout << "{Not Available}" << endl;
            continue;
        }

        // Parameter readable?
        if ( !lGenParameter->IsReadable() )
        {
            cout << "{Not readable}" << endl;
            continue;
        }
        
        // Get the parameter type
        PvGenType lType;
        lGenParameter->GetType( lType );
        switch ( lType )
        {
            case PvGenTypeInteger:
                {
                    int64_t lValue;             
                    static_cast<PvGenInteger *>( lGenParameter )->GetValue( lValue );
                    cout << "Integer: " << lValue;
                }
                break;
            
            case PvGenTypeEnum:
                {
                    PvString lValue;                
                    static_cast<PvGenEnum *>( lGenParameter )->GetValue( lValue );
                    cout << "Enum: " << lValue.GetAscii();
                }
                break;
            
            case PvGenTypeBoolean:
                {
                    bool lValue;                
                    static_cast<PvGenBoolean *>( lGenParameter )->GetValue( lValue );
                    if( lValue ) 
                    {
                        cout << "Boolean: TRUE";
                    }
                    else 
                    {
                        cout << "Boolean: FALSE";
                    }
                }
                break;

            case PvGenTypeString:
                {
                    PvString lValue;
                    static_cast<PvGenString *>( lGenParameter )->GetValue( lValue );
                    cout << "String: " << lValue.GetAscii();
                }
                break;

            case PvGenTypeCommand:
                cout << "Command";
                break;

            case PvGenTypeFloat:
                {
                    double lValue;              
                    static_cast<PvGenFloat *>( lGenParameter )->GetValue( lValue );
                    cout << "Float: " << lValue;
                }
                break;
                
            default:
                // Unexpected
                break;
        }
        cout << endl;
    }

    return true;
}


//
// Get Host's communication-related settings.
//

bool GetHostCommunicationRelatedSettings()
{
    // Communication link can be configured before we connect to the device.
    // No need to connect to the device.
    cout << "Using non-connected PvDevice" << endl;
    PvDeviceGEV lDeviceGEV;

    // Get the communication link parameters array
    cout << "Retrieving communication link parameters array" << endl;
    PvGenParameterArray* lComLink = lDeviceGEV.GetCommunicationParameters();

    // Dumping communication link parameters array content
    cout << "Dumping communication link parameters array content" << endl;
    DumpGenParameterArray( lComLink );

    lDeviceGEV.Disconnect();

    return true;
}


//
// Get the Device's settings
//

bool GetDeviceSettings( PvString &aConnectionID )
{
    // Select

    // Connect to the selected device.
    PvDevice* lDevice = Connect( aConnectionID );
    if ( lDevice == NULL )
    {
        return false;
    }
    
    // Get the device's parameters array. It is built from the 
    // GenICam XML file provided by the device itself.
    cout << "Retrieving device's parameters array" << endl;
    PvGenParameterArray* lParameters = lDevice->GetParameters();

    // Dumping device's parameters array content.
    cout << "Dumping device's parameters array content" << endl;
    DumpGenParameterArray( lParameters );

    // Get width parameter - mandatory GigE Vision parameter, it should be there.
    PvGenParameter *lParameter = lParameters->Get( "Width" );

    // Converter generic parameter to width using dynamic cast. If the
    // type is right, the conversion will work otherwise lWidth will be NULL.
    PvGenInteger *lWidthParameter = dynamic_cast<PvGenInteger *>( lParameter );

    if ( lWidthParameter == NULL )
    {
        cout << "Unable to get the width parameter." << endl;
    }

    // Read current width value.
    int64_t lOriginalWidth = 0;
    if ( !(lWidthParameter->GetValue( lOriginalWidth ).IsOK()) )
    {
        cout << "Error retrieving width from device" << endl;   
    }

    // Read max.
    int64_t lMaxWidth = 0;
    if ( !(lWidthParameter->GetMax( lMaxWidth ).IsOK()) )
    {
        cout << "Error retrieving width max from device" << endl;   
    }

    // Change width value.
    if ( !lWidthParameter->SetValue( lMaxWidth ).IsOK() )
    {
        cout << "Error changing width on device - the device is on Read Only Mode, please change to Exclusive to change value" << endl; 
    } 

    // Reset width to original value.
    if ( !lWidthParameter->SetValue( lOriginalWidth ).IsOK() )
    {
        cout << "Error changing width on device" << endl;   
    }

    // Disconnect the device.
    PvDevice::Free( lDevice );
    
    return true;
}


//
// Get Image stream controller settings.
//

bool GetImageStreamControllerSettings( const PvString &aConnectionID )
{
    PvString lDeviceID;
    PvResult lResult = PvResult::Code::INVALID_PARAMETER;

    // Creates stream object
    cout << "Opening stream" << endl;
    PvStream* lStream = PvStream::CreateAndOpen( aConnectionID, &lResult );
    if ( ( lStream == NULL ) || !lResult.IsOK() )
    {
        cout << "Error creating and opening stream" << endl;
        PvStream::Free( lStream );
        return false;
    }

    // Get the stream parameters. These are used to configure/control
    // some stream related parameters and timings and provides
    // access to statistics from this stream.
    cout << "Retrieving stream's parameters array" << endl;
    PvGenParameterArray* lParameters = lStream->GetParameters();

    // Dumping device's parameters array content.
    cout << "Dumping stream's parameters array content" << endl;
    DumpGenParameterArray( lParameters );

    // Close and free PvStream
    PvStream::Free( lStream );

    return true;
}

//
// Main function.
//

int main()
{
    PV_SAMPLE_INIT();

    cout << "Device selection" << endl << endl;
    PvString lConnectionID;
    if ( !PvSelectDevice( &lConnectionID ) )
    {
        return -1;
    }

    cout << "GenCamParameter sample" << endl << endl;
    // Communication link parameters display.
    cout << "1. Communication link parameters display" << endl << endl;
    GetHostCommunicationRelatedSettings();
    
    cout << endl;

    // Device parameters display.
    cout << "2. Device parameters display" << endl << endl;
    GetDeviceSettings( lConnectionID );

    cout << endl;

    // Image stream parameters display.
    cout << "3. Image stream parameters display" << endl << endl;
    GetImageStreamControllerSettings( lConnectionID );

    cout << endl;

    cout << "<press a key to exit>" << endl;
    PvWaitForKeyPress();

    PV_SAMPLE_TERMINATE();

    return 0;
}


