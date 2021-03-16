// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVSYSTEMEVENTSINK_H__
#define __PVSYSTEMEVENTSINK_H__

#include "PvSystemLib.h"


class PvInterface;
class PvDeviceInfo;


class PV_SYSTEM_API PvSystemEventSink
{
public:

    PvSystemEventSink();
    virtual ~PvSystemEventSink();

    virtual void OnDeviceFound( 
        const PvInterface *aInterface, const PvDeviceInfo *aDeviceInfo, 
        bool &aIgnore );

};

#endif
