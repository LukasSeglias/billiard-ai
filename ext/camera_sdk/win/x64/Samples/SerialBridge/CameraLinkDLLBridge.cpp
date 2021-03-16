// *****************************************************************************
//
//     Copyright (c) 2011, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "CameraLinkDLLBridge.h"

#include <PvSampleUtils.h>
#include <PvSerialBridge.h>
#include <PvDeviceAdapter.h>


#define CLDLL_PORTNAME ( "COM1" )

#define IPENGINEPORT ( PvDeviceSerialBulk0 )


void CameraLinkDLLBridge( PvDevice *aDevice )
{
    PvDeviceGEV* lDeviceGEV = dynamic_cast<PvDeviceGEV*>( aDevice );
    if ( lDeviceGEV != NULL )
    {
        // Using its GenICam interface, configure your device serial port
        // to match what is expected/required by the camera
        PvGenParameterArray *lDeviceParams = lDeviceGEV->GetParameters();
        // ...
    }

    // Create PvDevice adpater, used to interface the serial communication library
    PvDeviceAdapter lAdapter( aDevice );

    // Start the Camera Link DLL bridge.
    PvSerialBridge lBridge;
    PvResult lResult = lBridge.Start( CLDLL_PORTNAME, &lAdapter, IPENGINEPORT );
    if ( !lResult.IsOK() )
    {
        cout << "Unable to start bridge: " << lResult.GetCodeString().GetAscii() << " " << lResult.GetDescription().GetAscii() << endl; 

        return;
    }

    cout << "Camera Link DLL bridge started" << endl;

    // Let the bridge run until a key is pressed, provides stats
    while ( !PvKbHit() )
    {
        cout << "Tx: " << lBridge.GetBytesSentToDevice() << " Rx: " << lBridge.GetBytesReceivedFromDevice() << "\r";
        ::Sleep( 50 );
    }

    cout << endl;
}


