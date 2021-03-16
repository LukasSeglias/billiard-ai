#pragma once

#include "ProgressDlg.h"
#include "Setup.h"

#include <PvDeviceInfoGEV.h>
#include <PvDeviceGEV.h>
#include <PvStreamGEV.h>


// =============================================================================
// Thread used to setup the system on a connect operation
//
class ConnectionThread : public Thread
{
public:

    ConnectionThread( Setup *aSetup, const PvDeviceInfoGEV *aDeviceInfo, PvDeviceGEV *aDevice, PvStreamGEV *aStream, CWnd* aParent );
    virtual ~ConnectionThread();
    
    PvResult Connect();

protected:

    DWORD Function();

private:

    Setup *mSetup;
    const PvDeviceInfoGEV *mDeviceInfo;
    PvDeviceGEV *mDevice;
    PvStreamGEV *mStream;
    ProgressDlg *mDlg;
    PvResult mResult;
};


