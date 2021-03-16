// *****************************************************************************
//
//     Copyright (c) 2009, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVDEVICESERIALPORT_H__
#define __PVDEVICESERIALPORT_H__

#include "PvSerialLib.h"
#include "PvDeviceSerialEnums.h"
#include "IPvDeviceAdapter.h"


class PvDeviceSerialPortEchoSink;
namespace PvSerialLib
{
    class DeviceSerialPort;
}


class PV_SERIAL_API PvDeviceSerialPort
{
public:

    PvDeviceSerialPort();
    virtual ~PvDeviceSerialPort();

    PvResult Open( IPvDeviceAdapter *aDevice, PvDeviceSerial aPort );
    PvResult Close();
    bool IsOpened();

    PvResult Write( const uint8_t *aBuffer, uint32_t aSize, uint32_t &aBytesWritten );
    PvResult GetWriteProgress( uint32_t &aWritten, uint32_t &aTotal );
    PvResult Read( uint8_t *aBuffer, uint32_t aBufferSize, uint32_t &aBytesRead, uint32_t aTimeout = 0 );
    PvResult NotifyReadComplete();

    PvResult FlushRxBuffer();
    PvResult GetRxBytesReady( uint32_t &aBytes ); 
    PvResult GetRxBufferSize( uint32_t &aSize );
    PvResult SetRxBufferSize( uint32_t aSize );

    static bool IsSupported( IPvDeviceAdapter *aDevice, PvDeviceSerial aPort );

    static PvResult RegisterEchoSink( PvDeviceSerialPortEchoSink *aSink );
    static PvResult UnregisterEchoSink( PvDeviceSerialPortEchoSink *aSink );

    uint64_t GetBytesSent() const;
    uint64_t GetBytesReceived() const;
    void ResetStatistics();

private:

    // Not implemented
	PvDeviceSerialPort( const PvDeviceSerialPort & );
	const PvDeviceSerialPort &operator=( const PvDeviceSerialPort & );

    PvSerialLib::DeviceSerialPort * mThis;
};


#endif // __PVDEVICESERIALPORT_H__

