// *****************************************************************************
//
//     Copyright (c) 2008, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVSYSTEM_H__
#define __PVSYSTEM_H__

#include "PvSystemEventSink.h"
#include "PvNetworkAdapter.h"
#include "PvUSBHostController.h"


namespace PvSystemLib
{
    class System;
};


class PV_SYSTEM_API PvSystem
{
public:

    PvSystem();
    virtual ~PvSystem();

    PvResult Find();
    PvResult FindDevice( const PvString &aDeviceToFind, const PvDeviceInfo **aDeviceInfo );

    void SetDetectionTimeout( uint32_t aTimeout );
    uint32_t GetDetectionTimeout() const;

    void SetSubnetBroadcastEnabled( bool aValue );
    bool GetSubnetBroadcastEnabled() const;

    uint32_t GetGEVSupportedVersion() const;
    uint32_t GetU3VSupportedVersion() const;

    PvResult RegisterEventSink( PvSystemEventSink *aEventSink );
    PvResult UnregisterEventSink( PvSystemEventSink *aEventSink );

    uint32_t GetInterfaceCount() const;
    const PvInterface *GetInterface( uint32_t aIndex ) const;

    uint32_t GetDeviceCount() const;
    const PvDeviceInfo *GetDeviceInfo( uint32_t aIndex ) const;

protected:

private:

	 // Not implemented
	PvSystem( const PvSystem & );
	const PvSystem &operator=( const PvSystem & );

    PvSystemLib::System *mThis;

};

#endif
