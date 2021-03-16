// *****************************************************************************
//
//     Copyright (c) 2011, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "PvAppUtilsLib.h"
#include "PvDevice.h"
#include "PvStream.h"


namespace PvAppUtilsLib
{
    class AcquisitionStateManager;
}

typedef enum
{
    PvAcquisitionStateUnknown = -1,
    PvAcquisitionStateUnlocked = 0,
    PvAcquisitionStateLocked

} PvAcquisitionState;


class PV_APPUTILS_API PvAcquisitionStateEventSink
{
public:

	PvAcquisitionStateEventSink();
	virtual ~PvAcquisitionStateEventSink();

    virtual void OnAcquisitionStateChanged( PvDevice* aDevice, PvStream* aStream, uint32_t aSource, PvAcquisitionState aState );

};


class PV_APPUTILS_API PvAcquisitionStateManager
{
public:

    PvAcquisitionStateManager( PvDevice* aDevice, PvStream* aStream = 0, uint32_t aSource = 0 );
    ~PvAcquisitionStateManager();

    PvResult Start();
    PvResult Stop();

    PvAcquisitionState GetState() const;
    uint32_t GetSource() const;

    PvResult RegisterEventSink( PvAcquisitionStateEventSink* aEventSink );
    PvResult UnregisterEventSink( PvAcquisitionStateEventSink* aEventSink );

private:

    PvAppUtilsLib::AcquisitionStateManager *mThis;

	 // Not implemented
	PvAcquisitionStateManager( const PvAcquisitionStateManager & );
	const PvAcquisitionStateManager &operator=( const PvAcquisitionStateManager & );

};

