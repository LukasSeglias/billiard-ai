// *****************************************************************************
//
//     Copyright (c) 2011, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include <PvSampleUtils.h>
#include <PvDevice.h>

#include "SerialCOMPortBridge.h"
#include "CameraLinkDLLBridge.h"


PV_INIT_SIGNAL_HANDLER();


//
// Selects and connects a device.
// Notes: User is responsible to delete PvDevice object when it is no longer needed.
//

PvDevice* ConnectDevice()
{
    PvResult lResult = PvResult::Code::INVALID_PARAMETER;

    // Select device
    PvString lConnectionID;
    if ( !PvSelectDevice( &lConnectionID ) )
    {
        cout << "No device selected." << endl;
        return false;
    }

    // Creates and connects the device controller based on the selected device. 
    cout << "Connecting to device" << endl;
    PvDevice *lDevice = PvDevice::CreateAndConnect( lConnectionID, &lResult );
    if ( ( lDevice == NULL ) || !lResult.IsOK() )
    {
        cout << "Unable to connect to device" << endl;
        return NULL;
    }

    return lDevice;
}


//
// Main function
//

int main( int argc, char* argv[] )
{
    PV_SAMPLE_INIT();

    cout << "PvSerialBridge sample - serial device control through a Pleora video interface" << endl << endl;

    // Select, connect a PvDevice
    PvDevice* lDevice = ConnectDevice();
    if ( lDevice == NULL )
    {
        return -1;
    }

    int lSelection = 0;
    while ( ( lSelection < 1 ) || ( lSelection > 2 ) )
    {
        cout << "1. Start COM bridge" << endl;
        cout << "2. Start CameraLink DLL bridge" << endl;
        cout << "?>";

        cin >> lSelection;
    }

    cout << "" << endl;

    switch ( lSelection )
    {
    case 1:
        SerialCOMPortBridge( lDevice );
        break;

    case 2:
        CameraLinkDLLBridge( lDevice );
        break;
    }

    // Device created with PvDevice::CreateAndConnect, release with PvDevice::Free
    PvDevice::Free( lDevice );

    PV_SAMPLE_TERMINATE();

    return 0;
}


