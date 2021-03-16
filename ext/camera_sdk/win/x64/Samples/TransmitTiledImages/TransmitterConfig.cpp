// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"

#include "TransmitterConfig.h"

PvResult TransmitterConfig::Store( PvConfigurationWriter& aWriter )
{
    PvPropertyList lList;
    PvProperty lProperty;
    CString lTemp;
    int64_t lValue64;

    lValue64 = mWidth;
    lProperty.SetName( _T( "Width" ) );
    lProperty.SetValue( lValue64);
    lList.Add( lProperty );

    lValue64 = mHeight;
    lProperty.SetName( _T( "Height" ) );
    lProperty.SetValue( lValue64);
    lList.Add( lProperty );

    lValue64 = mMode;
    lProperty.SetName( _T( "Mode" ) );
    lProperty.SetValue( lValue64);
    lList.Add( lProperty );

    lValue64 = mFps;
    lProperty.SetName( _T( "Fps" ) );
    lProperty.SetValue( lValue64);
    lList.Add( lProperty );

    lValue64 = mPacketSize;
    lProperty.SetName( _T( "PacketSize" ) );
    lProperty.SetValue( lValue64);
    lList.Add( lProperty );

    lList.Add( PvProperty( _T( "DestinationIPAddress" ), mDestinationIPAddress.GetBuffer() ) );

    lValue64 = mDestinationPort;
    lProperty.SetName( _T( "DestinationPort" ) );
    lProperty.SetValue( lValue64);
    lList.Add( lProperty );

    lList.Add( PvProperty( _T( "SourceIPAddress" ), mSourceIPAddress.GetBuffer() ) );

    lTemp = _T( "Transmitter Config" );
    return aWriter.Store( &lList, lTemp.GetBuffer() );

}

PvResult TransmitterConfig::Restore( PvConfigurationReader& aReader )
{
    PvPropertyList lList;
    PvProperty* lProperty;
    PvResult lResult;
    int64_t lValue64;
    CString lName;
    CString lTemp;

    Reset();

    lTemp = _T( "Transmitter Config" );
    lResult = aReader.Restore( lTemp.GetBuffer(), &lList );
    if( !lResult.IsOK() )
    {
        return lResult;
    }

    lProperty = lList.GetFirst();
    while( lProperty )
    {
        lName = lProperty->GetName().GetUnicode();
        if( lName == _T( "Height" ) )
        {
            lProperty->GetValue( lValue64 );
            mHeight = ( int ) lValue64;
        }
        else if( lName == _T( "Width" ) )
        {
            lProperty->GetValue( lValue64 );
            mWidth = ( int ) lValue64;
        }
        if( lName == _T( "Mode" ) )
        {
            lProperty->GetValue( lValue64 );
            mMode = ( TilingMode ) lValue64;
        }
        else if( lName == _T( "Fps" ) )
        {
            lProperty->GetValue( lValue64 );
            mFps = ( int ) lValue64;
        }
        if( lName == _T( "PacketSize" ) )
        {
            lProperty->GetValue( lValue64 );
            mPacketSize = ( int ) lValue64;
        }
        else if( lName == _T( "DestinationIPAddress" ) )
        {
            mDestinationIPAddress = lProperty->GetValue().GetUnicode();
        }
        else if( lName == _T( "DestinationPort" ) )
        {
            lProperty->GetValue( lValue64 );
            mDestinationPort = ( unsigned short ) lValue64;
        }
        else if( lName == _T( "SourceIPAddress" ) )
        {
            mSourceIPAddress = lProperty->GetValue().GetUnicode();
        }
        lProperty = lList.GetNext();
    } 

    return PvResult::Code::OK;
}