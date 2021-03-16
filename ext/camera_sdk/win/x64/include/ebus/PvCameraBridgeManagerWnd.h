// *****************************************************************************
//
//     Copyright (c) 2016, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVCAMERABRIDGEMANAGERWND_H__
#define __PVCAMERABRIDGEMANAGERWND_H__

#include "PvGUILib.h"
#include "PvWnd.h"
#include "PvCameraBridge.h"


class PV_GUI_API PvCameraBridgeManagerWnd : public PvWnd
{
public:

	PvCameraBridgeManagerWnd();
	virtual ~PvCameraBridgeManagerWnd();

    PvResult SetDevice( PvDevice *aDevice );
    PvResult SetStream( PvStream *aStream );

    PvResult Save( PvConfigurationWriter &aWriter );
    PvResult Load( PvConfigurationReader &aRead, PvStringList &aErrors );

    PvResult Recover();

    int GetBridgeCount() const;
    PvDeviceSerial GetBridgePort( int aIndex );
    PvCameraBridge *GetBridge( int aIndex );

    bool IsPCFVisible() const;
    void ShowPCF();
    void HidePCF();

protected:

private:

    // Not implemented
	PvCameraBridgeManagerWnd( const PvCameraBridgeManagerWnd & );
	const PvCameraBridgeManagerWnd &operator=( const PvCameraBridgeManagerWnd & );

};

#endif
