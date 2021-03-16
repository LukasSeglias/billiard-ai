// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef _GEV_DEVICE_INPUT_
#define _GEV_DEVICE_INPUT_

#include "afxmt.h"
#include <PvStreamGEV.h>
#include <PvDevice.h>
#include <PvPipeline.h>
#include "Setup.h"
#include "PvStreamFilter.h"

class GEVDeviceInput : public PvDeviceEventSink
{
public:
    GEVDeviceInput();
    virtual ~GEVDeviceInput();

    BOOL Init();
    void StartStreaming();
    void StopStreaming();
    void Connect( PvDeviceInfo *aDI );
    void Disconnect();

    enum Role
    {
        RoleInvalid = -1,
        RoleDeviceTransmitter = 0,
        RoleDeviceReceiver,
        RoleDeviceTransceiver,
        RoleDevicePeripheral,
        RoleSoftwareReceiver
    };

private:

    PvDevice mDevice;
    PvStreamGEV mStream;
    PvPipeline mPipeline;

    CMutex mStartStreamingMutex;
    CMutex mStartAcquisitionMutex;

    Role mRole;

    CString mDescription;
    CString mIPAddress;
    CString mMACAddress;
    CString mManufacturer;
    CString mModel;
    CString mName;
    Setup mSetup;
    PvStreamFilter *mPvStreamFilter;
};


#endif //_GEV_DEVICE_INPUT_