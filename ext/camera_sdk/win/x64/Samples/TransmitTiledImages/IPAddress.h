// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

///
/// \class CIPAddress
///
/// \brief Control overload to automatically parse the IP address
///
class IPAddress
    : public CIPAddressCtrl
{
public:
    ///
    /// \brief Set a IP address into the control
    ///
    /// \param aIPStr The String with the IP address
    ///
    void IPAddress::SetAsText( LPCTSTR lpszString )
    {
        int lIP[ 4 ];
        int lCount = swscanf_s( lpszString, _T( "%i.%i.%i.%i" ), lIP, lIP + 1, lIP + 2, lIP + 3 );
        if ( lCount == 4 )
        {
            BYTE lIPb[ 4 ];
            for ( int i = 0; i < 4; i++ )
            {
                ASSERT( lIP[ i ] >= 0 );
                ASSERT( lIP[ i ] <= 255 );

                lIPb[ i ] = static_cast<BYTE>( lIP[ i ] );
            }

            CIPAddressCtrl::SetAddress( lIPb[ 0 ], lIPb[ 1 ], lIPb[ 2 ], lIPb[ 3 ] );
        }
        else
        {
            CIPAddressCtrl::SetAddress( 0, 0, 0, 0 );
        }
    }

    void IPAddress::GetAsText( CString& lValue )
    {
        BYTE lIPb[ 4 ];

        CIPAddressCtrl::GetAddress( lIPb[ 0 ], lIPb[ 1 ], lIPb[ 2 ], lIPb[ 3 ] );
        lValue.Format( _T( "%i.%i.%i.%i" ), lIPb[ 0 ], lIPb[ 1 ], lIPb[ 2 ], lIPb[ 3 ] );
    }
};