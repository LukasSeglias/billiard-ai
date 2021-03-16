// *****************************************************************************
//
//     Copyright (c) 2011, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVSERIALBRIDGEMANAGERWND_H__
#define __PVSERIALBRIDGEMANAGERWND_H__

#include "PvGUILib.h"
#include "PvWnd.h"

#include "PvSerialBridge.h"
#include "PvGenParameterArray.h"
#include "PvPropertyList.h"
#include "PvStringList.h"


class PV_GUI_API PvSerialBridgeManagerWnd : public PvWnd
{
public:

	PvSerialBridgeManagerWnd();
	virtual ~PvSerialBridgeManagerWnd();

    PvResult SetDevice( IPvDeviceAdapter *aDevice );

    PvResult Save( PvPropertyList &aPropertyList );
    PvResult Load( PvPropertyList &aPropertyList, PvStringList &aErrors );

    PvResult Recover();

protected:

private:

    // Not implemented
	PvSerialBridgeManagerWnd( const PvSerialBridgeManagerWnd & );
	const PvSerialBridgeManagerWnd &operator=( const PvSerialBridgeManagerWnd & );

};

#endif
