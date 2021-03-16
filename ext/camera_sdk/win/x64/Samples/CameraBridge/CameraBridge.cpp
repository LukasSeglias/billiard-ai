// *****************************************************************************
//
//     Copyright (c) 2016, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include <PvSampleUtils.h>
#include <PvDevice.h>

#include "PleoraCameraFile.h"
#include "GenCP.h"

#ifdef WIN32
#include "CLProtocol.h"
#endif


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
        return NULL;
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

    cout << "PvCameraBridge sample - camera control through a Pleora video interface" << endl << endl;

    // Select, connect a PvDevice
    PvDevice* lDevice = ConnectDevice();
    if ( lDevice == NULL )
    {
        return -1;
    }

    int lSelection = 0;
    while ( ( lSelection < 1 ) || ( lSelection > 3 ) )
    {
        cout << endl << "==== [Camera Bridge method] ====" << endl;
        cout << "1. Start Pleora Camera File bridge" << endl;
        cout << "2. Start GenCP bridge" << endl;
#ifdef WIN32
        cout << "3. Start CLProtocol bridge" << endl;
#endif
        cout << "?>";

        cin >> lSelection;
    }

    cout << "" << endl;

    switch ( lSelection )
    {
    case 1:
        PleoraCameraFile( lDevice );
        break;

    case 2:
        GenCP( lDevice );
        break;

#ifdef WIN32
    case 3:
        CLProtocol( lDevice );
        break;
#endif
    }

    // Device created with PvDevice::CreateAndConnect, release with PvDevice::Free
    PvDevice::Free( lDevice );

    PV_SAMPLE_TERMINATE();

    return 0;
}


