// *****************************************************************************
//
//     Copyright (c) 2016, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "CLProtocol.h"

#include <PvSampleUtils.h>
#include <PvCameraBridge.h>

#include <assert.h>


#define IPENGINEPORT ( PvDeviceSerialBulk0 )

#ifdef WIN32
void CLProtocol( PvDevice *aDevice )
{
    PvCameraBridge lBridge( aDevice, IPENGINEPORT );

    // Retrieve all CLProtocol options
    PvStringList lOptions;
    PvCameraBridge::GetCLProtocolTemplates( lOptions );
    if ( lOptions.GetSize() <= 0 )
    {
        // If no options are available, it means no CLProtocol DLL was available.
        // Either none are present on the system or they don't have their paths 
        // registered in the GENICAM_CLPROTOCOL environment variable.
        cout << "No options. No CLPRotocol DLL available." << endl;
        return;
    }

    // Select template
    cout << endl << "====[ Options ]====" << endl;
    for ( uint32_t i = 0; i < lOptions.GetSize(); i++ )
    {
        cout << i << ": " << lOptions[ i ]->GetAscii() << endl;
    }
    cout << "?>";
    int lOptionSelection;
    cin >> lOptionSelection;
    PvString lOption = *lOptions[ lOptionSelection ];

    // Retrieve camera ID and available XML IDs
    PvString lCameraID;
    PvStringList lXMLIDs;
    PvResult lResult = lBridge.ProbeCLProtocol( lOption, lCameraID, lXMLIDs );
    if ( !lResult.IsOK() )
    {
        cout << "Cannot retrieve XML IDs: " << 
            lResult.GetCodeString().GetAscii() << " " << 
            lResult.GetDescription().GetAscii() << endl;
        return;
    }

    // Select XML
    cout << endl << "==== [XML IDs] ====" << endl;
    for ( uint32_t i = 0; i < lXMLIDs.GetSize(); i++ )
    {
        cout << i << ": " << lXMLIDs[ i ]->GetAscii();
    }
    cout << "?>";
    int lXMLIDSelection;
    cin >> lXMLIDSelection;
    PvString lXMLID = *lXMLIDs[ lXMLIDSelection ];

    // Connect the camera
    cout << "Connecting CLProtocol camera..." << endl;
    lResult = lBridge.ConnectCLProtocol( lOption, lXMLID );
    if ( !lResult.IsOK() )
    {
        cout << "Unable to connect to CLProtocol camera: " << 
            lResult.GetCodeString().GetAscii() << " " << 
            lResult.GetDescription().GetAscii() << endl;
        return;
    }

    cout << "CLProtocol Camera successfully connected!" << endl;

    // Dump the value of all camera parameters
    cout << endl << "==== [Parameters] ====" << endl;
    PvGenParameterArray *lParameters = lBridge.GetParameters();
    uint32_t lCount = lParameters->GetCount();
    for ( uint32_t i = 0; i < lCount; i++ )
    {
        PvGenParameter *lParam = lParameters->Get( i );
		if ( lParam->IsReadable() )
		{
			PvString lValue, lName, lCategory;
			lParam->GetName( lName );
			lParam->ToString( lValue );
         
			cout << lName.GetAscii() << ": " << lValue.GetAscii() << endl;
		}
    }

    cout << endl;
}
#endif

