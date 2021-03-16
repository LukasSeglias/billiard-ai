// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

//
// Multicast master
//

#include <PvSampleUtils.h>
#include <PvStream.h>
#include <PvDevice.h>
#include <PvPipeline.h>
#include <PvBuffer.h>
#include <PvDeviceU3V.h>


PV_INIT_SIGNAL_HANDLER();


//
// Shows how to control a multicast master without receiving data
//

bool StartMaster()
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

    // Connect to the GigE Vision device
    PvDeviceGEV lDevice;
    if ( !lDevice.Connect( lConnectionID, PvAccessControl ).IsOK() )
    {
        cout << "Error connecting device" << endl;
        return false;
    }

    cout << "Setting stream destination to 239.192.1.1 port 1042" << endl;
    PvResult lResult = lDevice.SetStreamDestination( "239.192.1.1", 1042 );

    // Get device parameters need to control streaming
    PvGenParameterArray *lDeviceParams = lDevice.GetParameters();
    PvGenCommand *lStart = dynamic_cast<PvGenCommand *>( lDeviceParams->Get( "AcquisitionStart" ) );
    PvGenCommand *lStop = dynamic_cast<PvGenCommand *>( lDeviceParams->Get( "AcquisitionStop" ) );
    PvGenInteger *lGevSCPSPacketSize = dynamic_cast<PvGenInteger *>( lDeviceParams->Get( "GevSCPSPacketSize" ) );

    // Auto packet size negotiation does not happen with multicasting, here
    // we set it to small packets in order to be on the safe side and make sure all
    // multicast receiver can be reached.
    lGevSCPSPacketSize->SetValue( 1440 );

    // Enables stream before sending the AcquisitionStart command.
    cout << "Enable streaming on the controller." << endl;
    lDevice.StreamEnable();

    // Tell the device to start sending images to the multicast group
    cout << "Sending StartAcquisition command to device" << endl;
    lStart->Execute();

	// Acquire images until the user instructs us to stop
    PvFlushKeyboard();
	cout << endl;
	cout << "<press a key to stop streaming>" << endl;
  
    PvWaitForKeyPress();
    cout << endl;

    // Tell the device to stop sending images
    cout << "Sending AcquisitionStop command to the device" << endl;
    lStop->Execute();

    // Disable stream after sending the AcquisitionStop command.
    cout << "Disable streaming on the controller." << endl;
    lDevice.StreamDisable();

    // Disconnect the device. Optional, still nice to have
    cout << "Disconnecting device" << endl;
    lDevice.Disconnect();

    return true;
}


//
// Main function
//

int main()
{
    PV_SAMPLE_INIT();

    cout << "MulticastMaster sample" << endl << endl;
    // Connect to device and start streaming as a multicast master
    cout << "Connect to device and start streaming as a multicast master" << endl << endl;
    StartMaster();

    cout << endl;
    cout << "<press a key to exit>" << endl;
    PvWaitForKeyPress();

    PV_SAMPLE_TERMINATE();

    return 0;
}

