// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __IPVDEVICEEVENTSINK_H__
#define __IPVDEVICEEVENTSINK_H__

#include "PvDeviceLib.h"
#include "PvStringList.h"


class IPvDeviceEventSink
{
public: 

    virtual void NotifyEvent( 
        uint16_t aEventID, uint16_t aChannel, uint64_t aBlockID, uint64_t aTimestamp, 
        const void *aData, uint32_t aDataLength ) = 0;

    virtual void NotifyInvalidatedGenParameter( const PvString &aParameterName ) = 0;

};

#endif
