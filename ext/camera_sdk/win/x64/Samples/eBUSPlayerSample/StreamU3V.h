// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __STREAM_U3V_H__
#define __STREAM_U3V_H__

#include <Stream.h>


class StreamU3V : public Stream
{
#ifdef _AFXDLL
    DECLARE_DYNAMIC( StreamU3V )
#endif // _AFXDLL

public:

    StreamU3V( IMessageSink *aSink );
    ~StreamU3V();

    PvResult Open( Setup *aSetup, const PvDeviceInfo *aDeviceInfo, uint16_t aChannel );
    PvResult Recover();

    std::string GetGUID();

    // Stream
    std::string GetFindString() { return GetGUID(); }

protected:

private:

    std::string mDeviceGUID;
    uint16_t mChannel;
};


#endif // __STREAM_U3V_H__

