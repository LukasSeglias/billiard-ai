// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVSTREAMU3V_H__
#define __PVSTREAMU3V_H__

#include "PvStream.h"


class PV_STREAM_API PvStreamU3V : public PvStream
{
public:
	
	PvStreamU3V();
	virtual ~PvStreamU3V();

    PvResult Open( const PvDeviceInfo *aDeviceInfo );
    PvResult Open( const PvDeviceInfo *aDeviceInfo, uint16_t aChannel, uint32_t aBuffersCapacity = 64,  
        uint32_t aSmallImageThreshold = 512, uint32_t aMaximumNumberOfPendingURB = 64 );

    PvResult Open( const PvString &aInfo );
    PvResult Open( const PvString &aInfo, uint16_t aChannel, uint32_t aBuffersCapacity = 64,  
        uint32_t aSmallImageThreshold = 512, uint32_t aMaximumNumberOfPendingURB = 64 );

    PvString GetGUID();

    virtual PvStreamType GetType() const;
private:

private:

	 // Not implemented
	PvStreamU3V( const PvStreamU3V & );
    const PvStreamU3V &operator=( const PvStreamU3V & );

};

#endif
