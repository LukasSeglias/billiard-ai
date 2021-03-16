// *****************************************************************************
//
//     Copyright (c) 2016, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVDEVICESERIALPORTECHOSINK_H__
#define __PVDEVICESERIALPORTECHOSINK_H__

#include "PvDeviceSerialPort.h"


class PV_SERIAL_API PvDeviceSerialPortEchoSink
{
public:

    PvDeviceSerialPortEchoSink();
    virtual ~PvDeviceSerialPortEchoSink();

    // Notifications
    virtual void OnTransmitted( const PvString &aDeviceID, PvDeviceSerial aPort, const uint8_t *aBytes, uint32_t aByteCount, uint64_t aTimestamp );
    virtual void OnRead( const PvString &aDeviceID, PvDeviceSerial aPort, const uint8_t *aBytes, uint32_t aByteCount, uint64_t aTimestamp );
    virtual void OnReadComplete( const PvString &aDeviceID, PvDeviceSerial aPort, uint64_t aTimestamp );

private:

};

#endif
