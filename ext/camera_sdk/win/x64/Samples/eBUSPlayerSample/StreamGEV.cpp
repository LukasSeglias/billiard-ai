// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "eBUSPlayerShared.h"
#include "StreamGEV.h"

#include <PvStreamGEV.h>
#include <PvDeviceInfoGEV.h>
#include <PvNetworkAdapter.h>

#include <assert.h>
#include <sstream>


#define DEVICE_IP_ADDRESS ( "DeviceIPAddress" )


#ifdef _AFXDLL
    IMPLEMENT_DYNAMIC( StreamGEV, Stream )
#endif // _AFXDLL


///
/// \brief Constructor
///

StreamGEV::StreamGEV( IMessageSink *aSink )
    : Stream( aSink )
{
    SetStream( new PvStreamGEV );
    SetPipeline( new PvPipeline( GetStream() ) );
}


///
/// \brief Destructor
///

StreamGEV::~StreamGEV()
{

}


///
/// \brief Opens a PvStreamGEV from a device info
///

PvResult StreamGEV::Open( Setup *aSetup, const PvDeviceInfo *aDeviceInfo, uint16_t aChannel )
{
    PV_LOGINFO( mLogger, "Opening GigE Vision stream: " << aDeviceInfo->GetConnectionID().GetAscii() << " channel " << aChannel );

    const PvDeviceInfoGEV *lDeviceInfo = dynamic_cast<const PvDeviceInfoGEV *>( aDeviceInfo );
    if ( lDeviceInfo == NULL )
    {
        return PvResult::Code::INVALID_PARAMETER;
    }

    const PvNetworkAdapter *lInterface = dynamic_cast<const PvNetworkAdapter *>( aDeviceInfo->GetInterface() );
    if ( lInterface == NULL )
    {
        return PvResult::Code::INVALID_PARAMETER;
    }

    std::string lDeviceIP = lDeviceInfo->GetIPAddress().GetAscii();

    return Open( aSetup, lDeviceIP, "", aChannel );
}


///
/// \brief Opens a PvStreamGEV from its IP address
///

PvResult StreamGEV::Open( Setup *aSetup, const std::string &aDeviceIP, const std::string &aLocalIP, uint16_t aChannel )
{
    PvStreamGEV *lStream = dynamic_cast<PvStreamGEV *>( GetStream() );
    assert( lStream != NULL );

    if ( ( aSetup->GetRole() == Setup::RoleCtrlData ) ||
         ( aSetup->GetRole() == Setup::RoleData ) )
    {
        // Open stream
        if ( aSetup->GetDestination() == Setup::DestinationUnicastAuto )
        {
            PV_LOGINFO( mLogger, "Opening from device " << aDeviceIP.c_str() << " in DestinationUnicastAuto mode" );

            return lStream->Open(
                aDeviceIP.c_str(), 0, aChannel, aLocalIP.c_str() );
        }
        else if ( aSetup->GetDestination() == Setup::DestinationUnicastSpecific )
        {
            PV_LOGINFO( mLogger, "Opening from device " << aDeviceIP.c_str() << " in DestinationUnicastSpecific mode on local port " << 
                aSetup->GetUnicastSpecificPort() );

            return lStream->Open(
                aDeviceIP.c_str(), aSetup->GetUnicastSpecificPort(), aChannel, aLocalIP.c_str() );
        }
        else if ( aSetup->GetDestination() == Setup::DestinationMulticast )
        {
            PV_LOGINFO( mLogger, "Opening from device " << aDeviceIP.c_str() << " in DestinationMulticast mode on group " 
                << aSetup->GetMulticastIP().GetAscii() << ":" << aSetup->GetMulticastPort() );

            return lStream->Open( aDeviceIP.c_str(), aSetup->GetMulticastIP(),
                aSetup->GetMulticastPort(), aChannel, aLocalIP.c_str() );
        }
        else
        {
            assert( 0 );
        }
    }

    return PvResult::Code::OK;
}


///
/// \brief Returns the local port for an opened GEV stream object
///

uint16_t StreamGEV::GetLocalPort()
{
    if ( !IsOpened() )
    {
        return 0;
    }

    PvStreamGEV *lStream = dynamic_cast<PvStreamGEV *>( GetStream() );
    assert( lStream != NULL );

    return lStream->GetLocalPort();
}


///
/// \brief Returns the local IP address for an opened GEV stream object
///

std::string StreamGEV::GetLocalIPAddress()
{
    if ( !IsOpened() )
    {
        return "0.0.0.0";
    }

    PvStreamGEV *lStream = dynamic_cast<PvStreamGEV *>( GetStream() );
    assert( lStream != NULL );

    return lStream->GetLocalIPAddress().GetAscii();
}


///
/// \brief Returns the device IP address for an opened GEV stream object
///

std::string StreamGEV::GetDeviceIPAddress()
{
    if ( !IsOpened() )
    {
        return "0.0.0.0";
    }

    PvGenInteger *lDeviceIPAddress = GetParameters()->GetInteger( DEVICE_IP_ADDRESS );

    int64_t lValue = 0;
    lDeviceIPAddress->GetValue( lValue );

    std::stringstream lSS;
    lSS << ( ( lValue & 0xFF000000 ) >> 24 ) << ".";
    lSS << ( ( lValue & 0x00FF0000 ) >> 16 ) << ".";
    lSS << ( ( lValue & 0x0000FF00 ) >> 8 ) << ".";
    lSS << ( ( lValue & 0x000000FF ) );

    return lSS.str();
}

