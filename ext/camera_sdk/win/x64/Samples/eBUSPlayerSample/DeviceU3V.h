// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __DEVICE_U3V_H__
#define __DEVICE_U3V_H__

#include <Device.h>


class DeviceU3V : public Device
{
#ifdef _AFXDLL
    DECLARE_DYNAMIC( DeviceU3V )
#endif // _AFXDLL

public:

    DeviceU3V( IMessageSink *aSink, LogBuffer *aLogBuffer );
    ~DeviceU3V();

    std::string GetGUID();

    virtual PvResult Connect( IProgress *aProgress, Setup *aSetup, const PvDeviceInfo *aDeviceInfo, uint16_t &aChannel, const PvString &aCommunicationParameters );
    PvResult SetStreamDestination( Setup *aSetup, Stream *aStream, uint16_t aChannel );

    std::string GetFindString() { return GetGUID(); } 

protected:

private:

};


#endif // __DEVICE_U3V_H__

