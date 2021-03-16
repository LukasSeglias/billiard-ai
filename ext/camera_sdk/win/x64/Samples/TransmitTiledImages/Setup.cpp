// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"

#include <PvPropertyList.h>

#include "Setup.h"

PvResult Setup::Store( PvConfigurationWriter& aWriter, int aRow, int aColumn )
{
    PvPropertyList lList;
    PvProperty lProperty;
    CString lTemp;
    int64_t lValue64;

    lValue64 = mRole;
    lProperty.SetName( _T( "Role" ) );
    lProperty.SetValue( lValue64 );
    lList.Add( lProperty );

    lValue64 = mDestination;
    lProperty.SetName( _T( "Destination" ) );
    lProperty.SetValue( lValue64 );
    lList.Add( lProperty );

    lList.Add( PvProperty( _T( "IPAddress" ), mIPAddress.GetBuffer() ) );

    lValue64 = mPort;
    lProperty.SetName( _T( "Port" ) );
    lProperty.SetValue( lValue64 );
    lList.Add( lProperty );

    lTemp.Format( _T( "Setup( %d, %d )" ), aRow, aColumn );

    return aWriter.Store( &lList, lTemp.GetBuffer() );
}

PvResult Setup::Restore( PvConfigurationReader& aReader, int aRow, int aColumn )
{
    PvPropertyList lList;
    PvProperty* lProperty;
    PvResult lResult;
    int64_t lValue64;
    CString lName;
    CString lTemp;

    Reset();

    lTemp.Format( _T( "Setup( %d, %d )" ), aRow, aColumn );

    lResult = aReader.Restore( lTemp.GetBuffer(), &lList );
    if( !lResult.IsOK() )
    {
        return lResult;
    }

    lProperty = lList.GetFirst();
    while( lProperty )
    {
        lName = lProperty->GetName().GetUnicode();
        if( lName == _T( "Role" ) )
        {
            lProperty->GetValue( lValue64 );
            mRole = ( int ) lValue64;
        }
        else if( lName == _T( "Destination" ) )
        {
            lProperty->GetValue( lValue64 );
            mDestination = ( int ) lValue64;
        }
        else if( lName == _T( "IPAddress" ) )
        {
            mIPAddress = lProperty->GetValue().GetUnicode();
        }
        else if( lName == _T( "Port" ) )
        {
            lProperty->GetValue( lValue64 );
            mPort = ( unsigned short ) lValue64;
        }
        lProperty = lList.GetNext();
    } 

    return PvResult::Code::OK;
}