// *****************************************************************************
//
//     Copyright (c) 2009, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVDEVICEI2CBUS_H__
#define __PVDEVICEI2CBUS_H__

#include "PvDeviceSerialPort.h"


namespace PvSerialLib
{
    class DeviceI2CBus;
}; // namespace PvSerialLib


class PV_SERIAL_API PvDeviceI2CBus
{
public:

    PvDeviceI2CBus();
    ~PvDeviceI2CBus();

    PvResult Open( IPvDeviceAdapter *aDevice,
       PvDeviceSerial aPort = PvDeviceSerialBulk0 );

    PvResult Close();

    bool IsOpened();

    static bool IsSupported( IPvDeviceAdapter *aDevice,
       PvDeviceSerial aPort = PvDeviceSerialBulk0 );

    PvResult BurstWrite(
       unsigned char aSlaveAddress,
       const unsigned char *aBuffer,
       uint32_t aBufferSize,
       bool aFastMode = true );

    PvResult IndirectBurstWrite(
       unsigned char aSlaveAddress,
       unsigned char aOffset,
       const unsigned char *aBuffer,
       uint32_t aBufferSize,
       bool aFastMode = true );

    PvResult BurstRead(
       unsigned char aSlaveAddress,
       unsigned char *aBuffer,
       uint32_t aBufferSize,
       uint32_t &aBytesRead,
       bool aFastMode = true );

    PvResult IndirectBurstRead(
       unsigned char aSlaveAddress,
       unsigned char aOffset,
       unsigned char *aBuffer,
       uint32_t aBufferSize,
       uint32_t &aBytesRead,
       bool aFastMode = true,
       bool aUseCombinedFormat = true );

    PvResult MasterTransmitter(
       uint8_t aSlaveAddress,
       const uint8_t *aBuffer,
       uint32_t aBufferSize,
       bool aFastMode = true,
       bool aGenerateStopCondition = true );

    PvResult MasterReceiverAfterFirstByte(
       uint8_t aSlaveAddress,
       uint8_t *aBuffer,
       uint32_t aBufferSize,
       uint32_t &aBytesRead,
       bool aFastMode = true,
       bool aGenerateStopCondition = true );

private:

    PvSerialLib::DeviceI2CBus* mThis;

};

#endif
