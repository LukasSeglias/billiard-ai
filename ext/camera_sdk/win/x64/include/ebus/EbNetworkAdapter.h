// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __EBNETWORKADAPTER_H__
#define __EBNETWORKADAPTER_H__

#include "EbInstallerLib.h"
#include "EbDriver.h"
#include "PtTypes.h"


namespace EbInstallerLib
{
    class NetworkAdapter;
};

class EbInstaller;

class EB_INSTALLER_LIB_API EbNetworkAdapter
{
public:

    PtString GetName() const;
    PtString GetUniqueId() const;
    PtString GetMACAddress() const;

    const EbDriver* GetCurrentDriver() const;
    bool IsRebootNeeded() const;

    // Enabling eBUS Universal Pro
    PtResult SetDriverEnabled( bool aEnabled = true );
    PtResult GetDriverEnabled( bool& aEnabled ) const;  

    // Device driver configuration dialog ( From Device Manager )
    PtResult ShowDevicePropertiesDialog( HWND aParent );

    // IPv4 version properties configuration dialog ( From Local Connection Properties )
    PtResult ShowIPv4PropertiesDialog( HWND aParent );

    // IP Address Configuration
    PtResult SetDynamicIpAddress();
    PtResult SetStaticIpAddress( PtString aIpAddress, PtString aSubnetMask );
    PtResult SetStaticIpAddresses( PtString aIpAddress[], PtString aSubnetMask[], uint32_t aCount );
    PtResult GetIpAddresses( bool& aDynamic, PtString aIpAddress[], PtString aSubnetMask[], uint32_t& aCount );

    // Jumbo Packet
    PtResult IsJumboPacketAvailable( bool& aAvailable ) const;
    PtResult GetJumboPacketEnabled( bool& aEnabled ) const; 
    PtResult SetJumboPacketEnabled( bool aEnabled = true );
    PtResult GetJumboPacketValue( uint32_t& aValue );

    // Rx Buffers
    PtResult IsReceiveBuffersAvailable( bool& aAvailable ) const;
    PtResult GetReceiveBuffersValues( uint32_t& aDefault, uint32_t& aMin, uint32_t& aMax ) const; 
    PtResult SetReceiveBuffersValue( uint32_t aValue );
    PtResult GetReceiveBuffersValue( uint32_t& aValue );

    // Tx Buffers
    PtResult IsTransmitBuffersAvailable( bool& aAvailable ) const;
    PtResult GetTransmitBuffersValues( uint32_t& aDefault, uint32_t& aMin, uint32_t& aMax ) const; 
    PtResult SetTransmitBuffersValue( uint32_t aValue );
    PtResult GetTransmitBuffersValue( uint32_t& aValue );

protected:

#ifndef DOXYGEN
    EbNetworkAdapter();
    EbNetworkAdapter( EbInstallerLib::NetworkAdapter* aNetworkAdapter );
    virtual ~EbNetworkAdapter();

    EbInstallerLib::NetworkAdapter* mThis;
#endif // DOXYGEN

    friend class EbInstaller;

};

#endif
