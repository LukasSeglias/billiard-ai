// *****************************************************************************
//
//     Copyright (c) 2016, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

// The Pleora Camera Files are not available yet. Contact your Pleora representative if you would like to learn more about this feature.

#include "CLProtocol.h"

#include <PvSampleUtils.h>
#include <PvCameraBridge.h>


#define IPENGINEPORT ( PvDeviceSerialBulk0 )
#define PCFPATH ( "fullpathtoyourpcffile.yml" )


// Class used to log callbacks from PvCameraBridge
class MySink : public PvCameraBridgeEventSink
{
public:

    // The camera bridge is asking us to set an application specific parameter
    PvResult OnSetParameter( const PvString &aName, const PvString &aValue )
    {
        cout << "Application specific parameter set: " << 
            aName.GetAscii() << "=" << 
            aValue.GetAscii() << endl;

        return PvResult::Code::OK;
    }

    // Error notification
    void OnError( const PvString &aMessage )
    {
        cout << aMessage.GetAscii() << endl;
    }

};


void PleoraCameraFile( PvDevice *aDevice )
{
    PvCameraBridge lBridge( aDevice, IPENGINEPORT );

    // Create and register camera bridge event sink
    MySink lSink;
    lBridge.RegisterEventSink( &lSink );

    // Connect: load Pleora Camera File
    PvResult lResult = lBridge.ConnectPleoraCameraFile( PCFPATH );
    if ( !lResult.IsOK() )
    {
        cout << "Error loading Pleora Camera File: " << 
            lResult.GetCodeString().GetAscii() << " " << 
            lResult.GetDescription().GetAscii() << endl;

        // Output parser errors, if any
        PvStringList lErrors;
        lBridge.GetPleoraCameraFileParserErrors( lErrors );
        if ( lErrors.GetSize() > 0 )
        {
            cout << "Parser errors detected:" << endl;
            PvString *lError = lErrors.GetFirst();
            while ( lError != NULL )
            {
                cout << "    " << lError->GetAscii() << endl;
                lError = lErrors.GetNext();
            }
        }

        return;
    }
    
    // Dump the value of all camera parameters
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


