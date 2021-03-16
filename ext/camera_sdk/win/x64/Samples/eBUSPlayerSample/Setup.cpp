// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "eBUSPlayerShared.h"
#include "Setup.h"

#include <sstream>
#include <assert.h>


#define SETUPDLG_VERSION ( "1.0.0.0" )

#define TAG_VERSION ( "setupversion" )
#define TAG_ROLE ( "setuprole" )
#define TAG_DESTINATION ( "setupdestination" )
#define TAG_UNICASTSPECIFICPORT ( "setupunicastspecificport" )
#define TAG_DEFAULTCHANNEL ( "setupdefaultchannel" )
#define TAG_UNICASTIP ( "setupunicastip" )
#define TAG_UNICASTPORT ( "setupunicastport" )
#define TAG_MULTICASTIP ( "setupmulticastip" )

#define TAG_MULTICASTPORT ( "defaultchannel" )

#define VAL_ROLECTRLDATA ( "ctrldata" )
#define VAL_ROLECTRL ( "ctrl" )
#define VAL_ROLEDATA ( "data" )
#define VAL_ROLEMONITOR ( "monitor" )
#define VAL_DESTINATIONUNICASTSPECIFIC ( "unicastspecific" )
#define VAL_DESTINATIONUNICASTAUTO ( "unicastauto" )
#define VAL_DESTINATIONUNICASTOTHER ( "unicastother" )
#define VAL_DESTINATIONMULTICAST ( "multicast" )


#define VAL_ROLECTRLDATA ( "ctrldata" )
#define VAL_ROLECTRL ( "ctrl" )
#define VAL_ROLEDATA ( "data" )
#define VAL_ROLEMONITOR ( "monitor" )
#define VAL_DESTINATIONUNICASTSPECIFIC ( "unicastspecific" )
#define VAL_DESTINATIONUNICASTAUTO ( "unicastauto" )
#define VAL_DESTINATIONUNICASTOTHER ( "unicastother" )
#define VAL_DESTINATIONMULTICAST ( "multicast" )


#ifdef _AFXDLL
    IMPLEMENT_DYNAMIC( Setup, CObject )
#endif // _AFXDLL


///
/// \brief
///

Setup::Setup()
    : mEnabled( true )
{
    Reset();
}


///
/// \brief Destructor
///

Setup::~Setup()
{

}


///
/// \brief Resets the object configuration
///

void Setup::Reset()
{
    mRole = RoleCtrlData;
    mDestination = DestinationUnicastAuto;
    mUnicastSpecificPort = 0;
    mUnicastIP = "0.0.0.0";
    mUnicastPort = 0;
    mMulticastIP = "239.192.1.1";
    mMulticastPort = 1042;
    mDefaultChannel = 0;
}


///
/// \brief Persistence save
///

PvResult Setup::Save( PvConfigurationWriter *aWriter )
{
    std::string lStr;
    std::stringstream lSS;

    // Save a version string, just in case we need it in the future
    aWriter->Store( SETUPDLG_VERSION, TAG_VERSION );

    // Role mRole;
    lStr = VAL_ROLECTRLDATA;
    switch ( mRole )
    {
    case RoleCtrlData:
        lStr = VAL_ROLECTRLDATA;
        break;

    case RoleCtrl:
        lStr = VAL_ROLECTRL;
        break;

    case RoleData:
        lStr = VAL_ROLEDATA;
        break;

    case RoleMonitor:
        lStr = VAL_ROLEMONITOR;
        break;

    default:
        assert( 0 );
    }
    aWriter->Store( lStr.c_str(), TAG_ROLE );

    // Destination mDestination;
    lStr = VAL_DESTINATIONUNICASTAUTO;
    switch ( mDestination )
    {
    case DestinationUnicastAuto:
        lStr = VAL_DESTINATIONUNICASTAUTO;
        break;

    case DestinationUnicastSpecific:
        lStr = VAL_DESTINATIONUNICASTSPECIFIC;
        break;

    case DestinationUnicastOther:
        lStr = VAL_DESTINATIONUNICASTOTHER;
        break;

    case DestinationMulticast:
        lStr = VAL_DESTINATIONMULTICAST;
        break;

    default:
        assert( 0 );
    }
    aWriter->Store( lStr.c_str(), TAG_DESTINATION );

    // unsigned short mUnicastSpecificPort;
    lSS << mUnicastSpecificPort;
    aWriter->Store( lSS.str().c_str(), TAG_UNICASTSPECIFICPORT );

    // unsigned short mDefaultChannel;
    lSS.str( "" );
    lSS << mDefaultChannel;
    aWriter->Store( lSS.str().c_str(), TAG_DEFAULTCHANNEL );

    // std::string mUnicastIP;
    aWriter->Store( mUnicastIP.c_str(), TAG_UNICASTIP );

    // unsigned short mUnicastPort;
    lSS.str( "" );
    lSS << mUnicastPort;
    aWriter->Store( lSS.str().c_str(), TAG_UNICASTPORT );

    // std::string mMulticastIP;
    aWriter->Store( mMulticastIP.c_str(), TAG_MULTICASTIP );

    // unsigned short mMulticastPort;
    lSS.str( "" );
    lSS << mMulticastPort;
    aWriter->Store( lSS.str().c_str(), TAG_MULTICASTPORT );

    return PvResult::Code::OK;
}


///
/// \brief Persistence save
///

PvResult Setup::Load( PvConfigurationReader *aReader )
{
    PvResult lResult;
    PvString lPvStr;

    // Always load from a blank setup!
    Reset();

    // Role mRole;
    lResult = aReader->Restore( PvString( TAG_ROLE ), lPvStr );
    if ( lResult.IsOK() )
    {
        StrToRole( lPvStr.GetAscii(), mRole );
    }

    // Destination mDestination;
    lResult = aReader->Restore( PvString( TAG_DESTINATION ), lPvStr );
    if ( lResult.IsOK() )
    {
        StrToDestination( lPvStr.GetAscii(), mDestination );
    }

    // unsigned short mUnicastSpecificPort;
    lResult = aReader->Restore( PvString( TAG_UNICASTSPECIFICPORT ), lPvStr );
    if ( lResult.IsOK() )
    {
        std::stringstream lSS( lPvStr.GetAscii() );
        lSS >> mUnicastSpecificPort;
    }

    // unsigned short mDefaultChannel;
    lResult = aReader->Restore( PvString( TAG_DEFAULTCHANNEL ), lPvStr );
    if ( lResult.IsOK() )
    {
        std::stringstream lSS( lPvStr.GetAscii() );
        lSS >> mDefaultChannel;
    }

    // CString mUnicastIP;
    lResult = aReader->Restore( PvString( TAG_UNICASTIP ), lPvStr );
    if ( lResult.IsOK() )
    {
        mUnicastIP = lPvStr.GetAscii();
    }

    // unsigned short mUnicastPort;
    lResult = aReader->Restore( PvString( TAG_UNICASTPORT ), lPvStr );
    if ( lResult.IsOK() )
    {
        std::stringstream lSS( lPvStr.GetAscii() );
        lSS >> mUnicastPort;
    }

    // CString mMulticastIP;
    lResult = aReader->Restore( PvString( TAG_MULTICASTIP ), lPvStr );
    if ( lResult.IsOK() )
    {
        mMulticastIP = lPvStr.GetAscii();
    }

    // unsigned short mMulticastPort;
    lResult = aReader->Restore( PvString( TAG_MULTICASTPORT ), lPvStr );
    if ( lResult.IsOK() )
    {
        std::stringstream lSS( lPvStr.GetAscii() );
        lSS >> mMulticastPort;
    }

    return PvResult::Code::OK;
}


///
/// \brief Returns true if the configuration in the reader is the same as the object
///

bool Setup::IsTheSame( PvConfigurationReader *aReader )
{
    // Load a local setup
    Setup lSetup;
    lSetup.Load( aReader );

    // Start with the assumption that they are the same
    bool lSame = true;

    // Try to invalidate assumption with destination, role
    lSame &= lSetup.mDestination == mDestination;
    lSame &= lSetup.mRole == mRole;
    lSame &= lSetup.mDefaultChannel == mDefaultChannel;

    // Only if destination is unicast specific, compare destination
    if ( lSetup.mDestination == DestinationUnicastSpecific )
    {
        lSame &= lSetup.mUnicastSpecificPort == mUnicastSpecificPort;
    }

    // Only if destination is unicast other, compare destination
    if ( lSetup.mDestination == DestinationUnicastOther )
    {
        lSame &= lSetup.mUnicastIP == mUnicastIP;
        lSame &= lSetup.mUnicastPort == mUnicastPort;
    }

    // Only if destination is multicast, compare destination
    if ( lSetup.mDestination == DestinationMulticast )
    {
        lSame &= lSetup.mMulticastIP == mMulticastIP;
        lSame &= lSetup.mMulticastPort == mMulticastPort;
    }

    // Return conclusion!
    return lSame;
}


///
/// \brief Converts from a role to a string
///

void Setup::StrToRole( const std::string &aStr, Role &aRole )
{
    if ( aStr == VAL_ROLECTRLDATA )
    {
        aRole = RoleCtrlData;
    }
    else if ( aStr == VAL_ROLEDATA )
    {
        aRole = RoleData;
    }
    else if ( aStr == VAL_ROLECTRL )
    {
        aRole = RoleCtrl;
    }
    else if ( aStr == VAL_ROLEMONITOR )
    {
        aRole = RoleMonitor;
    }
}


///
/// \brief Converts to a destination type from a string
///

void Setup::StrToDestination( const std::string &aStr, Destination &aDestination )
{
    if ( aStr == VAL_DESTINATIONUNICASTAUTO )
    {
        aDestination = DestinationUnicastAuto;
    }
    else if ( aStr == VAL_DESTINATIONUNICASTSPECIFIC )
    {
        aDestination = DestinationUnicastSpecific;
    }
    else if ( aStr == VAL_DESTINATIONUNICASTOTHER )
    {
        aDestination = DestinationUnicastOther;
    }
    else if ( aStr == VAL_DESTINATIONMULTICAST )
    {
        aDestination = DestinationMulticast;
    }
}


///
/// \brief Updates the enabled state of each and every model item based on status
///

void Setup::UpdateEnabled( Role aRole, Destination &aDest )
{
    if ( !mEnabled )
    {
        // Master disable
        mRoleCtrlDataEnabled = false;
        mRoleCtrlEnabled = false;
        mRoleDataEnabled = false;
        mRoleMonitorEnabled = false;
        mDestUnicastAutoEnabled = false;
        mDestUnicastSpecificEnabled = false;
        mDestUnicastOtherEnabled = false;
        mDestMulticastEnabled = false;
        mChannelEnabled = false;
        return;
    }

    mRoleCtrlDataEnabled = true;
    mRoleCtrlEnabled = true;
    mRoleDataEnabled = true;
    
    bool lCtrlData = ( aRole == RoleCtrlData );
    bool lCtrl = ( aRole == RoleCtrl );
    bool lData = ( aRole == RoleData );

    // Stream radio buttons
    mDestUnicastAutoEnabled = ( lCtrlData || lData ) && mEnabled;
    mDestUnicastSpecificEnabled = ( lCtrlData || lData ) && mEnabled;
    mDestUnicastOtherEnabled = lCtrl && mEnabled;
    mDestMulticastEnabled = ( lCtrl || lData || lCtrlData ) && mEnabled;
    mChannelEnabled = ( lData || lCtrlData ) && mEnabled;

    // Make sure we check the first enabled option (if needed)
    bool lValid = false;
    lValid |= ( aDest == DestinationUnicastAuto ) && mDestUnicastAutoEnabled;
    lValid |= ( aDest == DestinationUnicastSpecific ) && mDestUnicastSpecificEnabled;
    lValid |= ( aDest == DestinationUnicastOther ) && mDestUnicastOtherEnabled;
    lValid |= ( aDest == DestinationMulticast ) && mDestMulticastEnabled;
    if ( !lValid )
    {
        if ( mDestUnicastAutoEnabled )
        {
            aDest = DestinationUnicastAuto;
        }
        else if ( mDestUnicastSpecificEnabled )
        {
            aDest = DestinationUnicastSpecific;
        }
        else if ( mDestUnicastOtherEnabled )
        {
            aDest = DestinationUnicastOther;
        }
        else if ( mDestMulticastEnabled )
        {
            aDest = DestinationMulticast;
        }
    }
}


///
/// \brief Returns an IP address string from 4 bytes
///

PvString Setup::IPFromBytes( uint8_t a0, uint8_t a1, uint8_t a2, uint8_t a3 )
{
    std::stringstream lSS;
    lSS << static_cast<int>( a0 ) << ".";
    lSS << static_cast<int>( a1 ) << ".";
    lSS << static_cast<int>( a2 ) << ".";
    lSS << static_cast<int>( a3 ) ;

    return lSS.str().c_str();
}


