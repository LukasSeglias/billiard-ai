// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVDEVICEGEV_H__
#define __PVDEVICEGEV_H__

#include "PvDevice.h"


class PV_DEVICE_API PvDeviceGEV : public PvDevice
{
public:

	PvDeviceGEV();
	virtual ~PvDeviceGEV();

	PvResult Connect( const PvDeviceInfo *aDeviceInfo );
	PvResult Connect( const PvDeviceInfo *aDeviceInfo, PvAccessType aAccessType );

	PvResult Connect( const PvString &aInfo );
    PvResult Connect( const PvString &aInfo, PvAccessType aAccessType );

    PvResult SetStreamDestination( const PvString &aIPAddress, uint16_t aDataPort, uint32_t aChannel = 0 ); 
    PvResult ResetStreamDestination( uint32_t aChannel = 0 );

    PvResult SetPacketSize( uint32_t aPacketSize, uint32_t aChannel = 0 );
    PvResult NegotiatePacketSize( uint32_t aChannel = 0, uint32_t aDefaultPacketSize = 0 );

	PvResult ReadRegister( int64_t aAddress, uint32_t &aValue );
	PvResult WriteRegister( int64_t aAddress, uint32_t aValue, bool aAcknowledge = true );

    PvAccessType GetAccessType() const;
    PvDeviceClass GetClass() const;
    PvString GetIPAddress() const;
    PvString GetMACAddress() const;

    static PvResult GetAccessType( const PvString &aDeviceIPAddress, PvAccessType &aAccessType );
    static PvResult SetIPConfiguration( 
        const PvString &aMACAddress, 
        const PvString &aIP, 
        const PvString &aSubnetMask = PvString( "255.255.255.0" ), 
        const PvString &aGateway = PvString( "0.0.0.0" ) );

protected:
    
private:

	 // Not implemented
	PvDeviceGEV( const PvDeviceGEV & );
	const PvDeviceGEV &operator=( const PvDeviceGEV & );

};

#endif
