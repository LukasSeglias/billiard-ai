// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

//
// Shows how to use the PvDeviceSerialPort class to communicate with a Pleora GigE Vision or USB3 Vision device.
//

#include <PvSampleUtils.h>
#include <PvDevice.h>
#include <PvDeviceSerialPort.h>
#include <PvDeviceAdapter.h>


PV_INIT_SIGNAL_HANDLER();


// #define UART0
#define BULK0

#define SPEED ( "Baud9600" )
#define STOPBITS ( "One" )
#define PARITY ( "None" )

#define TEST_COUNT ( 16 )

//
// Shows how to use a PvDeviceSerialPort to write data to an video interface and continuously read it back
// (with the video interface serial port set in loop back mode).
//

bool TestSerialCommunications()
{
    PvResult lResult = PvResult::Code::INVALID_PARAMETER;

    PvString lConnectionID;
    if ( !PvSelectDevice( &lConnectionID ) )
    {
        cout << "No device selected." << endl;
        return false;
    }

    // Connect to the GEV or U3V Device
    cout << "Connecting to device" << endl;
    PvDevice *lDevice = PvDevice::CreateAndConnect( lConnectionID, &lResult );
    if ( !lResult.IsOK() )
    {
        cout << "Unable to connect to device" << endl;
        return false;
    }

    // Create PvDevice adapter
    PvDeviceAdapter *lDeviceAdapter = new PvDeviceAdapter( lDevice );

    // Get device parameters need to control streaming
    PvGenParameterArray *lParams = lDevice->GetParameters();

    // Configure serial port - this is done directly on the device GenICam interface, not 
    // on the serial port object! 
#ifdef UART0
    lParams->SetEnumValue( "Uart0BaudRate", SPEED );
    lParams->SetEnumValue( "Uart0NumOfStopBits", STOPBITS );
    lParams->SetEnumValue( "Uart0Parity", PARITY );

    // For this test to work without attached serial hardware we enable the port loop back
    lParams->SetBooleanValue( "Uart0Loopback", true );
#endif
#ifdef BULK0
    lParams->SetEnumValue( "BulkSelector", "Bulk0" );
    lParams->SetEnumValue( "BulkMode", "UART" );
    lParams->SetEnumValue( "BulkBaudRate", SPEED );
    lParams->SetEnumValue( "BulkNumOfStopBits", STOPBITS );
    lParams->SetEnumValue( "BulkParity", PARITY );

    // For this test to work without attached serial hardware we enable the port loop back
    lParams->SetBooleanValue( "BulkLoopback", true );
#endif // BULK0

    // Open serial port
    PvDeviceSerialPort lPort;
#ifdef UART0
    lResult = lPort.Open( lDeviceAdapter, PvDeviceSerial0 );
#endif // UART0
#ifdef BULK0
    lResult = lPort.Open( lDeviceAdapter, PvDeviceSerialBulk0 );
#endif // BULK0
    if ( !lResult.IsOK() )
    {
        cout << "Unable to open serial port on device: " << lResult.GetCodeString().GetAscii() << " " <<  lResult.GetDescription().GetAscii() << endl;
        return false;
    }
    cout << "Serial port opened" << endl;

	uint32_t lSize = 1;
    // Make sure the PvDeviceSerialPort receive queue is big enough to buffer all bytes 
    // Note that for every iteration in the test below, lSize is doubled so lSize << TEST_COUNT is the size (2x to give extra)
    lPort.SetRxBufferSize( ( lSize << TEST_COUNT ) * 2);

    uint8_t *lInBuffer = NULL;
    uint8_t *lOutBuffer = NULL;

    
    for ( int lCount = 0; lCount < TEST_COUNT; lCount++ )
    {
        // Allocate test buffers
        lInBuffer = new uint8_t[ lSize ];
        lOutBuffer = new uint8_t[ lSize ];

        // Fill input buffer with random data
        for ( uint32_t i = 0; i < lSize; i++ )
        {
            lInBuffer[ i ] = static_cast<uint8_t>( rand() % 256 );
        }

        // Send the buffer content on the serial port
        uint32_t lBytesWritten = 0;
        lResult = lPort.Write( lInBuffer, lSize, lBytesWritten );
        if ( !lResult.IsOK() )
        {
            // Unable to send data over serial port!
            cout << "Error sending data over the serial port: " << lResult.GetCodeString().GetAscii() << " " <<  lResult.GetDescription().GetAscii() << endl;
            break;
        }

        cout << "Sent " << lBytesWritten << " bytes through the serial port" << endl;

        //
        // Wait until we have all the bytes or we timeout. The Read method only times out
        // if no data is available when the function call occurs, otherwise it returns
        // all the currently available data. It is possible we have to call Read multiple
        // times to retrieve all the data if all the expected data hasn't been received yet.
        //
        // Your own code driving a serial protocol should check for a message being complete,
        // whether it is on some sort of EOF or length. You should pump out data until you
        // have what you are waiting for or reach some timeout.
        //
        uint32_t lTotalBytesRead = 0;
        while ( lTotalBytesRead < lSize )
        {
            uint32_t lBytesRead = 0;
            lResult = lPort.Read( lOutBuffer + lTotalBytesRead, lSize - lTotalBytesRead, lBytesRead, 500 );
            if ( lResult.GetCode() == PvResult::Code::TIMEOUT )
            {
                cout << "Timeout" << endl;
                break;
            }

            // Increments read head
            lTotalBytesRead += lBytesRead;
        }

        // Validate answer
        if ( lTotalBytesRead != lBytesWritten )
        {
            // Did not receive all expected bytes
            cout << "Only received " << lTotalBytesRead << " out of " << lBytesWritten << " bytes" << endl;
        }
        else
        {
            // Compare input and output buffers
            uint32_t lErrorCount = 0;
            for ( uint32_t i = 0; i < lBytesWritten; i++ )
            {
                if ( lInBuffer[ i ] != lOutBuffer[ i ] )
                {
                    lErrorCount++;
                }
            }

            // Display error count
            cout << "Error count: " << lErrorCount << endl;
        }

        delete []lInBuffer; 
        lInBuffer = NULL;
        
        delete []lOutBuffer; 
        lOutBuffer = NULL;

        // Grow test case
        lSize *= 2;

        cout << endl;
    }

    if ( lInBuffer != NULL )
    {
        delete []lInBuffer;
        lInBuffer = NULL;
    }

    if ( lOutBuffer != NULL )
    {
        delete []lOutBuffer;
        lOutBuffer = NULL;
    }

    // Close serial port
    lPort.Close();
    cout << "Serial port closed" << endl;

    // Delete device adapter (before freeing PvDevice!)
    delete lDeviceAdapter;
    lDeviceAdapter = NULL;

    // Release the device. Use PvDevice::Free as the device was allocated using PvDevice::CreateAndConnect.
    cout << "Disconnecting and freeing device" << endl;
    PvDevice::Free( lDevice );

    return true;
}


//
// Main function
//

int main()
{
    PV_SAMPLE_INIT();

    // PvDeviceSerialPort used to perform serial communication through a Pleora GigE Vision or USB3 Vision device
    cout << "DeviceSerialPort sample" << endl << endl;
    TestSerialCommunications();

    cout << endl << "<press a key to exit>" << endl;
    PvWaitForKeyPress();

    PV_SAMPLE_TERMINATE();

    return 0;
}

