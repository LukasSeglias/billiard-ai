// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVDEVICEFINDERWND_H__
#define __PVDEVICEFINDERWND_H__

#include "PvGUILib.h"
#include "PvWnd.h"
#include "PvDeviceInfo.h"


class DeviceFinderWnd;


class PV_GUI_API PvDeviceFinderWnd : public PvWnd
{
public:

	PvDeviceFinderWnd();
	virtual ~PvDeviceFinderWnd();

#ifdef WIN32
	const PvDeviceInfo *GetSelected() const;
#else
	const PvDeviceInfo *GetSelected();
#endif // WIN32
	virtual bool OnFound( const PvDeviceInfo *aDI );

    void GetGEVEnabled( bool &aSelectable, bool &aVisible ) const;
    void SetGEVEnabled( bool aSelectable, bool aVisible );

    void GetU3VEnabled( bool &aSelectable, bool &aVisible ) const;
    void SetU3VEnabled( bool aSelectable, bool aVisible );

    void GetPleoraProtocolEnabled( bool &aSelectable, bool &aVisible ) const;
    void SetPleoraProtocolEnabled( bool aSelectable, bool aVisible );

protected:

private:

    // Not implemented
	PvDeviceFinderWnd( const PvDeviceFinderWnd & );
	const PvDeviceFinderWnd &operator=( const PvDeviceFinderWnd & );

};

#endif
