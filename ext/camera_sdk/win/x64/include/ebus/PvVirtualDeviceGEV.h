// *****************************************************************************
//
//     Copyright (c) 2011, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVVIRTUALDEVICEGEV_H__
#define __PVVIRTUALDEVICEGEV_H__

#include "PvVirtualDeviceLib.h"


namespace PvVirtualDeviceLib
{
    class VirtualDeviceGEV;
};

class IPvTransmitterGEV;


class PV_VIRTUAL_DEVICE_API PvVirtualDeviceGEV
{
public:

    PvVirtualDeviceGEV();
    ~PvVirtualDeviceGEV();
    PvResult SetGEVSpecificationVersion( uint16_t aMajor, uint16_t aMinor );
    PvResult GetGEVSpecificationVersion( uint16_t& aMajor, uint16_t& aMinor );
    PvResult SetGVCPCapabilityPacketResendCommandSupported( bool aValue );
    bool GetGVCPCapabilityPacketResendCommandSupported();
    PvResult SetDeviceVersion( const PvString &aDeviceVersion );
    PvString GetDeviceVersion();
    PvResult SetSerialNumber( const PvString &aSerialNumber );
    PvString GetSerialNumber();
    PvResult SetManufacturerName( const PvString &aManufacturerName );
    PvString GetManufacturerName();
    PvResult SetModelName( const PvString &aModelName );
    PvString GetModelName();

    PvResult AddTransmitterGEV( IPvTransmitterGEV * aTransmitterGEV );

    PvResult StartListening( PvString aIPAddress );
    void StopListening();

    uint32_t GetDevicePortThreadPriority() const;
    PvResult SetDevicePortThreadPriority( uint32_t aPriority );

private:

    PvVirtualDeviceLib::VirtualDeviceGEV* mThis;

};

#endif
