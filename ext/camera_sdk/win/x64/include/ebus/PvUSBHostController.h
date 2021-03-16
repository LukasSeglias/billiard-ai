// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVUSBHOSTCONTROLLER_H__
#define __PVUSBHOSTCONTROLLER_H__

#include "PvInterface.h"
#include "PvDeviceInfoUSB.h"
#include "PvDeviceInfoU3V.h"


class PV_SYSTEM_API PvUSBHostController : public PvInterface
{
public:

    PvUSBHostController();
    virtual ~PvUSBHostController();

    uint32_t GetVendorID() const;
    uint32_t GetDeviceID() const;
    uint32_t GetSubsystemID() const;

    uint32_t GetRevision() const;

    PvUSBSpeed GetSpeed() const;

protected:

    PvUSBHostController( PvSystemLib::IFinderReporter *aFinderReporter );
	PvUSBHostController&operator=( const PvUSBHostController &aFrom );

    void Init();

    void SetVendorID( uint32_t aValue ) { mVendorID = aValue; }
    void SetDeviceID( uint32_t aValue ) { mDeviceID = aValue; }
    void SetSubsystemID( uint32_t aValue ) { mSubsystemID = aValue; }
    void SetRevision( uint32_t aValue ) { mRevision = aValue; }
    void SetSpeed( PvUSBSpeed aValue ) { mSpeed = aValue; }

private:

	 // Not implemented
	PvUSBHostController( const PvUSBHostController & );

    uint32_t mVendorID;
    uint32_t mDeviceID;
    uint32_t mSubsystemID;
    uint32_t mRevision;
    PvUSBSpeed mSpeed;

};

#endif
