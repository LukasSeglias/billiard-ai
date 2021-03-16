// *****************************************************************************
//
//     Copyright (c) 2015, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __IPVTRANSMITTERGEV_H__
#define __IPVTRANSMITTERGEV_H__

#include "PvVirtualDeviceLib.h"


class PV_VIRTUAL_DEVICE_API IPvTransmitterGEV
{
public:

    IPvTransmitterGEV();
    virtual ~IPvTransmitterGEV();

    virtual bool IsOpen() const = 0;
    virtual bool IsTransmitting() const = 0;

    virtual bool GetExtendedIDs() const = 0;			
    virtual PvResult SetExtendedIDs( bool aExtendedID ) = 0;	
			
    virtual void QueuePacketResend( uint64_t aBlockID, uint32_t aFirstPacketID, uint32_t aLastPacketID ) = 0;

};

#endif
