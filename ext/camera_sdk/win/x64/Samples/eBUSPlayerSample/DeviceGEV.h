// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __DEVICE_GEV_H__
#define __DEVICE_GEV_H__

#include <Device.h>


class DeviceGEV : public Device
{
#ifdef _AFXDLL
    DECLARE_DYNAMIC( DeviceGEV )
#endif // _AFXDLL

public:

    DeviceGEV( IMessageSink *aSink, LogBuffer *aLogBuffer );
    ~DeviceGEV();

    PvResult SetStreamDestination( Setup *aSetup, Stream *aStream, uint16_t aChannel );
    PvResult ResetStreamDestination( uint16_t aChannel );

    bool IsUnconditionalStreamingEnabled();

    std::string GetLocalIPAddress();
    std::string GetIPAddress();

    PvResult Connect( IProgress *aProgress, Setup *aSetup, const PvDeviceInfo *aDeviceInfo, uint16_t &aChannel, const PvString &aCommunicationParameters );

    std::string GetFindString() { return GetIPAddress(); }

protected:

    PvResult SetStreamDestination( const std::string &aIpAddress, uint16_t aPort, uint16_t aChannel );
    PvResult NegotiatePacketSize( IProgress *aProgress );

private:

};


#endif // __DEVICE_GEV_H__

