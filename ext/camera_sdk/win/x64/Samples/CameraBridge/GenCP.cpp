// *****************************************************************************
//
//     Copyright (c) 2016, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "GenCP.h"

#include <PvSampleUtils.h>
#include <PvCameraBridge.h>

#include <assert.h>


#define IPENGINEPORT ( PvDeviceSerialBulk0 )


void GenCP( PvDevice *aDevice )
{
    PvCameraBridge lBridge( aDevice, IPENGINEPORT );

    // Retrieve avaialble XML IDs
    PvStringList lXMLIDs;
    PvResult lResult = lBridge.GetGenCPXMLIDs( lXMLIDs );
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
    cout << "Connecting GenCP camera..." << endl;
    lResult = lBridge.ConnectGenCP( lXMLID );
    if ( !lResult.IsOK() )
    {
        cout << "Unable to connect to GenCP camera: " << 
            lResult.GetCodeString().GetAscii() << " " << 
            lResult.GetDescription().GetAscii() << endl;
        return;
    }

    cout << "GenCP Camera successfully connected!" << endl;

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


