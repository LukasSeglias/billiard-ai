// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVDEVICEINFOUSB_H__
#define __PVDEVICEINFOUSB_H__

#include "PvDeviceInfo.h"


class PV_SYSTEM_API PvDeviceInfoUSB : public PvDeviceInfo
{
public:

    PvDeviceInfoUSB();
	virtual ~PvDeviceInfoUSB();

	PvDeviceInfoUSB &operator=( const PvDeviceInfoUSB & );

    PvUSBStatus GetStatus() const;

protected:

#ifndef PV_GENERATING_DOXYGEN_DOC

	PvDeviceInfoUSB( PvDeviceInfoType aType, PvInterface *aInterface );

    void SetStatus( PvUSBStatus aValue ) { mStatus = aValue; }

#endif // PV_GENERATING_DOXYGEN_DOC

private:

	 // Not implemented
    PvDeviceInfoUSB( const PvDeviceInfoUSB & );

    PvUSBStatus mStatus;

};

#endif
