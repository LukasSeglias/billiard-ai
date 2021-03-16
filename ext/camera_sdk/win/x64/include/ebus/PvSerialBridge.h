// *****************************************************************************
//
//     Copyright (c) 2011, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVSERIALBRIDGE_H__
#define __PVSERIALBRIDGE_H__

#include "PvSerialLib.h"
#include "PvDeviceSerialPort.h"
#include "PvSerialPortConfiguration.h"


namespace PvSerialLib
{
    class Bridge;
}


class PV_SERIAL_API PvSerialBridge
{
public:

    PvSerialBridge();
    ~PvSerialBridge();

    // Start a serial COM port bridge
    PvResult Start( const PvString &aSerialPort, PvSerialPortConfiguration aSerialPortConfiguration, 
        IPvDeviceAdapter *aDevice, PvDeviceSerial aDevicePort );

    // Start, stops a Camera Link DLL bridge
    PvResult Start( const PvString &aName, IPvDeviceAdapter *aDevice, PvDeviceSerial aDevicePort );
    PvResult Stop();

    // Stats
    uint64_t GetBytesSentToDevice() const;
    uint64_t GetBytesReceivedFromDevice() const;
    void ResetStatistics();

    // Device serial port in use
    PvDeviceSerial GetDevicePort() const;

    // Retrieve supported/selected device serial port baud rates
    uint32_t GetSupportedBaudRateCount() const;
    uint32_t GetSupportedBaudRate( uint32_t aIndex ) const;
    uint32_t GetBaudRate() const;

    // Gets, sets the device serial hard-coded port baud rate (if cannot be read/written)
    uint32_t GetHardCodedBaudRate() const;
    PvResult SetHardCodedBaudRate( uint32_t aBaudRate );

    // Serial COM port bridge configuration
    PvString GetSerialPort() const;
    PvSerialPortConfiguration GetSerialPortConfiguration() const;
    PvResult SetSerialPortConfiguration( PvSerialPortConfiguration aSerialPortConfiguration );

    // Camera Link DLL bridge configuration
    PvString GetName() const;

    // Closes and re-opens the device serial port
    PvResult Recover();

private:

    // Not implemented
	PvSerialBridge( const PvSerialBridge & );
	const PvSerialBridge &operator=( const PvSerialBridge & );

    PvSerialLib::Bridge * mThis;
};

#endif
