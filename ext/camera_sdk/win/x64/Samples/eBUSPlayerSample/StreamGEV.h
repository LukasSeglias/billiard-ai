// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __STREAM_GEV_H__
#define __STREAM_GEV_H__

#include <Stream.h>


class StreamGEV : public Stream
{
#ifdef _AFXDLL
    DECLARE_DYNAMIC( StreamGEV )
#endif // _AFXDLL

public:

    StreamGEV( IMessageSink *aSink );
    ~StreamGEV();

    PvResult Open( Setup *aSetup, const PvDeviceInfo *aDeviceInfo, uint16_t aChannel );
    PvResult Open( Setup *aSetup, const std::string &aDeviceIP, const std::string &aLocalIP, uint16_t aChannel );

    uint16_t GetLocalPort();
    std::string GetDeviceIPAddress();
    std::string GetLocalIPAddress();

    // Stream
    std::string GetFindString() { return GetDeviceIPAddress(); }

protected:

private:

};


#endif // __STREAM_GEV_H__

