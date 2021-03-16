// *****************************************************************************
//
// Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// -----------------------------------------------------------------------------
//
// Class containing a project entity setup, detached from UI.
// 
// *****************************************************************************

#include "stdafx.h"
#include "Setup.h"


#define SETUP_VERSION ( _T( "1.0.0.0" ) )

#define TAG_VERSION ( _T( "setupversion" ) )
#define TAG_ROLE ( _T( "setuprole" ) )
#define TAG_DESTINATION ( _T( "setupdestination" ) )
#define TAG_UNICASTSPECIFICPORT ( _T( "setupunicastspecificport" ) )
#define TAG_UNICASTIP ( _T( "setupunicastip" ) )
#define TAG_UNICASTPORT ( _T( "setupunicastport" ) )
#define TAG_MULTICASTIP ( _T( "setupmulticastip" ) )
#define TAG_MULTICASTPORT ( _T( "setupmulticastport" ) )

#define VAL_ROLECTRLDATA ( _T( "ctrldata" ) )
#define VAL_ROLECTRL ( _T( "ctrl" ) )
#define VAL_ROLEDATA ( _T( "data" ) )
#define VAL_DESTINATIONUNICASTSPECIFIC ( _T( "unicastspecific" ) )
#define VAL_DESTINATIONUNICASTAUTO ( _T( "unicastauto" ) )
#define VAL_DESTINATIONUNICASTOTHER ( _T( "unicastother" ) )
#define VAL_DESTINATIONMULTICAST ( _T( "multicast" ) )


// ==========================================================================
Setup::Setup()
{
    Reset();
}

// ==========================================================================
Setup::~Setup()
{
}

// ==========================================================================
void Setup::Reset()
{
    mRole = RoleCtrlData;
    mDestination = DestinationUnicastAuto;
    mUnicastSpecificPort = 0;
    mUnicastIP = _T( "0.0.0.0" );
    mUnicastPort = 0;
    mMulticastIP = _T( "239.192.1.1" );
    mMulticastPort = 1042;
}

// ==========================================================================
void Setup::Save( PvConfigurationWriter &aWriter )
{
    Save( aWriter, this );
}

// ==========================================================================
void Setup::Save( PvConfigurationWriter &aWriter, Setup *aSetup )
{
    CString lStr;

    // Save a version string, just in case we need it in the future
    aWriter.Store( SETUP_VERSION, TAG_VERSION );

    // Role mRole;
    lStr = VAL_ROLECTRLDATA;
    switch ( aSetup->mRole )
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

        default:
            ASSERT( 0 );
    }
    aWriter.Store( (LPCTSTR)lStr, TAG_ROLE );

    // Destination mDestination;
    lStr = VAL_DESTINATIONUNICASTAUTO;
    switch ( aSetup->mDestination )
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
            ASSERT( 0 );
    }
    aWriter.Store( (LPCTSTR)lStr, TAG_DESTINATION );

    // unsigned short mUnicastSpecificPort;
    lStr.Format( _T( "%i" ), aSetup->mUnicastSpecificPort );
    aWriter.Store( (LPCTSTR)lStr, TAG_UNICASTSPECIFICPORT );

    // CString mUnicastIP;
    aWriter.Store( (LPCTSTR)aSetup->mUnicastIP, TAG_UNICASTIP );

    // unsigned short mUnicastPort;
    lStr.Format( _T( "%i" ), aSetup->mUnicastPort );
    aWriter.Store( (LPCTSTR)lStr, TAG_UNICASTPORT );

    // CString mMulticastIP;
    aWriter.Store( (LPCTSTR)aSetup->mMulticastIP, TAG_MULTICASTIP );

    // unsigned short mMulticastPort;
    lStr.Format( _T( "%i" ), aSetup->mMulticastPort );
    aWriter.Store( (LPCTSTR)lStr, TAG_MULTICASTPORT );
}

// ==========================================================================
void Setup::Load( PvConfigurationReader &aReader )
{
    Load( aReader, this );
}

// ==========================================================================
void Setup::Load( PvConfigurationReader &aReader, Setup *aSetup )
{
    PvResult lResult;
    PvString lPvStr;

    // Always load from a blank setup!
    aSetup->Reset();

    // Role mRole;
    lResult = aReader.Restore( PvString( TAG_ROLE ), lPvStr );
    if ( lResult.IsOK() )
    {
        StrToRole( lPvStr.GetUnicode(), aSetup->mRole );
    }

    // Destination mDestination;
    lResult = aReader.Restore( PvString( TAG_DESTINATION ), lPvStr );
    if ( lResult.IsOK() )
    {
        StrToDestination( lPvStr.GetUnicode(), aSetup->mDestination );
    }

    // unsigned short mUnicastSpecificPort;
    lResult = aReader.Restore( PvString( TAG_UNICASTSPECIFICPORT ), lPvStr );
    if ( lResult.IsOK() )
    {
        int lPort;
        swscanf_s( lPvStr.GetUnicode(), _T( "%i" ), &lPort );
        aSetup->mUnicastSpecificPort = static_cast<unsigned short>( lPort );
    }

    // CString mUnicastIP;
    lResult = aReader.Restore( PvString( TAG_UNICASTIP ), lPvStr );
    if ( lResult.IsOK() )
    {
        aSetup->mUnicastIP = lPvStr.GetUnicode();
    }

    // unsigned short mUnicastPort;
    lResult = aReader.Restore( PvString( TAG_UNICASTPORT ), lPvStr );
    if ( lResult.IsOK() )
    {
        int lPort;
        swscanf_s( lPvStr.GetUnicode(), _T( "%i" ), &lPort );
        aSetup->mUnicastPort = static_cast<unsigned short>( lPort );
    }

    // CString mMulticastIP;
    lResult = aReader.Restore( PvString( TAG_MULTICASTIP ), lPvStr );
    if ( lResult.IsOK() )
    {
        aSetup->mMulticastIP = lPvStr.GetUnicode();
    }

    // unsigned short mMulticastPort;
    lResult = aReader.Restore( PvString( TAG_MULTICASTPORT ), lPvStr );
    if ( lResult.IsOK() )
    {
        int lPort;
        swscanf_s( lPvStr.GetUnicode(), _T( "%i" ), &lPort );
        aSetup->mMulticastPort = static_cast<unsigned short>( lPort );
    }
}


// =============================================================================
bool Setup::IsTheSame( PvConfigurationReader &aReader )
{
    // Load a local setup
    Setup lSetup;
    Load( aReader, &lSetup );

    // Start with the assumption that they are the same
    bool lSame = true;

    // Try to invalidate assumption with destination, role
    lSame &= lSetup.mDestination == mDestination;
    lSame &= lSetup.mRole == mRole;

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

// ==========================================================================
void Setup::StrToRole( const CString &aStr, Role &aRole )
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
}

// ==========================================================================
void Setup::StrToDestination( const CString &aStr, Destination &aDestination )
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

