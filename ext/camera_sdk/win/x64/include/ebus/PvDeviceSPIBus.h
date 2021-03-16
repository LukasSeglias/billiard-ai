
// *****************************************************************************
//
//     Copyright (c) 2009, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVDEVICESPIBUS_H__
#define __PVDEVICESPIBUS_H__

#include "PvDeviceSerialPort.h"


namespace PvSerialLib
{
    class DeviceSPIBus;
}; // namespace PvSerialLib


class PV_SERIAL_API PvDeviceSPIBus
{
public:

    PvDeviceSPIBus();
    ~PvDeviceSPIBus();

    PvResult Open( IPvDeviceAdapter *aDevice,
       PvDeviceSerial aPort = PvDeviceSerialBulk0 );

    PvResult Close();

    bool IsOpened();

    static bool IsSupported( IPvDeviceAdapter *aDevice,
       PvDeviceSerial aPort = PvDeviceSerialBulk0 );

    PvResult BurstRead(
       unsigned char *aBuffer,
       uint32_t aBufferSize,
       uint32_t &aBytesRead,
       uint32_t aTimeout = 0);

    PvResult BurstWriteAndRead(
       unsigned char *aWriteBuffer,
       uint32_t aWriteBufferSize,
       uint32_t aReadBufferSize = 0,
       bool aWriteRead = false,
       bool aSSNFlagOn = false );

private:

    PvSerialLib::DeviceSPIBus* mThis;

};

#endif
