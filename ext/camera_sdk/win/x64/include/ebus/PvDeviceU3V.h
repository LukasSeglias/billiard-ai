// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVDEVICEU3V_H__
#define __PVDEVICEU3V_H__

#include "PvDevice.h"


class PV_DEVICE_API PvDeviceU3V : public PvDevice
{
public:

	PvDeviceU3V();
	virtual ~PvDeviceU3V();

	PvResult Connect( const PvString &aInfo );
	PvResult Connect( const PvDeviceInfo *aDeviceInfo );

    PvString GetGUID() const;

protected:
    
private:

	 // Not implemented
	PvDeviceU3V( const PvDeviceU3V & );
	const PvDeviceU3V &operator=( const PvDeviceU3V & );

};

#endif
