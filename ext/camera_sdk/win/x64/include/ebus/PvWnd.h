// *****************************************************************************
//
//     Copyright (c) 2008, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVWND_H__
#define __PVWND_H__

#include "PvGUILib.h"


class Wnd;
#ifndef WIN32
    class QWidget;
#endif // WIN32


class PV_GUI_API PvWnd
{
public:

	void SetPosition( int32_t  aPosX, int32_t  aPosY, int32_t  aSizeX, int32_t  aSizeY );
	void GetPosition( int32_t &aPosX, int32_t &aPosY, int32_t &aSizeX, int32_t &aSizeY );

#ifdef WIN32
	PvResult ShowModal( PvWindowHandle aParentHwnd = 0 );
	PvResult ShowModeless( PvWindowHandle aParentHwnd = 0 );
	PvResult Create( PvWindowHandle aHwnd, uint32_t aID );
#else
    PvResult ShowModal();
    PvResult ShowModal( QWidget* aParentHwnd );

    PvResult ShowModeless();
    PvResult ShowModeless( QWidget* aParentHwnd );

    PvResult Create( QWidget* aHwnd );
#endif // WIN32

	PvString GetTitle() const;
	void SetTitle( const PvString &aTitle );
#ifndef WIN32
    QWidget* GetQWidget();
#endif

	PvResult Close();

#ifdef WIN32
	PvWindowHandle GetHandle();
    PvResult DoEvents();
#else
    static void DoEvents();
#endif // WIN32

protected:

    PvWnd();
	virtual ~PvWnd();

    Wnd *mThis;

private:

    // Not implemented
	PvWnd( const PvWnd & );
	const PvWnd &operator=( const PvWnd & );

};

#endif
