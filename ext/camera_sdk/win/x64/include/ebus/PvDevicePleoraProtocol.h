// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVDEVICEPLEORAPROTOCOL_H__
#define __PVDEVICEPLEORAPROTOCOL_H__

#include "PvDevice.h"


class PV_DEVICE_API PvDevicePleoraProtocol
{
public:

    static PvResult SetIPConfiguration( 
        const PvString &aMACAddress, 
        const PvString &aIP, 
        const PvString &aSubnetMask = PvString( "255.255.255.0" ), 
        const PvString &aGateway = PvString( "0.0.0.0" ) );

protected:

private:

    // Not implemented
    PvDevicePleoraProtocol();
    PvDevicePleoraProtocol( const PvDevicePleoraProtocol & );
    const PvDevicePleoraProtocol &operator=( const PvDevicePleoraProtocol & );

};

#endif
