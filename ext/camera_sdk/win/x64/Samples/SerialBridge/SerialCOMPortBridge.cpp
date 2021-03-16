// *****************************************************************************
//
//     Copyright (c) 2011, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "SerialCOMPortBridge.h"

#include <PvSampleUtils.h>
#include <PvSerialBridge.h>
#include <PvDeviceAdapter.h>


#define BRIDGECOMPORT ( "COM10" )
#define BRIDGECOMPORT_BAUDRATE ( 9600 )
#define BRIDGECOMPORT_PARITY ( PvParityNone )
#define BRIDGECOMPORT_BYTESIZE ( 8 )
#define BRIDGECOMPORT_STOPBITS ( 1 )

#define IPENGINEPORT ( PvDeviceSerialBulk0 )


void SerialCOMPortBridge( PvDevice *aDevice )
{
    PvDeviceGEV* lDeviceGEV = dynamic_cast<PvDeviceGEV*>( aDevice );
    if ( lDeviceGEV != NULL )
    {
        // Using its GenICam interface, configure your device serial port
        // to match what is expected/required by the camera
        PvGenParameterArray *lDeviceParams = lDeviceGEV->GetParameters();
        // ...
    }

    // Prepare COM port configuration
    PvSerialPortConfiguration lConfig;
    lConfig.mBaudRate = BRIDGECOMPORT_BAUDRATE;
    lConfig.mParity = BRIDGECOMPORT_PARITY;
    lConfig.mByteSize = BRIDGECOMPORT_BYTESIZE;
    lConfig.mStopBits = BRIDGECOMPORT_STOPBITS;

    // Create a PvDevice adapter
    PvDeviceAdapter lAdapter( aDevice );

    // Start the bridge on COM10, UART0 on the device.
    PvSerialBridge lBridge;
    PvResult lResult = lBridge.Start( BRIDGECOMPORT, lConfig, &lAdapter, IPENGINEPORT );
    if ( !lResult.IsOK() )
    {
        cout << "Unable to start bridge: " << lResult.GetCodeString().GetAscii() << " " << lResult.GetDescription().GetAscii() << endl;
        return;
    }

    cout << "Serial COM port bridge started on " << BRIDGECOMPORT << endl;

    // Let the bridge run until a key is pressed, provides stats
    while ( !PvKbHit() )
    {
        cout << "Tx: " << lBridge.GetBytesSentToDevice() << " Rx: " << lBridge.GetBytesReceivedFromDevice() << "\r";

        ::Sleep( 50 );
    }

    cout << "" << endl;
}


