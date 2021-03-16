// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVDISPLAYWND_H__
#define __PVDISPLAYWND_H__

#include "PvGUILib.h"
#include "PvWnd.h"
#include "PvBuffer.h"
#include "PvBufferConverter.h"


class DisplayWnd;

#ifndef WIN32
typedef enum
{
    PvRendererModeDefault = 0,
    PvRendererModeAccelerated = 1,
    PvRendererModeHardwareAccelerated = 2,
    PvRendererModeUndefined = 999,

} PvRendererMode;
#endif // !WIN32


class PV_GUI_API PvDisplayWnd : public PvWnd
{
public:

#ifdef WIN32
    PvDisplayWnd();
#else
    PvDisplayWnd( int32_t aMaxNumberOfThreads = -1 );
#endif // WIN32
	virtual ~PvDisplayWnd();

	PvResult Display( const PvBuffer &aBuffer, bool aVSync = false );
	PvResult SetBackgroundColor( uint8_t aR, uint8_t aG, uint8_t aB );

    PvBufferConverter &GetConverter();

#ifndef WIN32
    PvRendererMode GetRendererMode();
    PvResult SetRendererMode( PvRendererMode aMode );

    void Refresh();
    static PvRendererMode GetHighestRendererMode();
    static PvString GetRenderer();
    static PvString GetRendererVersion();
#endif // WIN32
	void Clear();


    void SetTextOverlay( const PvString &aText );
    void SetTextOverlayColor( uint8_t aR, uint8_t aG, uint8_t aB );
    PvResult SetTextOverlaySize( int32_t aSize );
    void SetTextOverlayOffsetX( uint32_t aX );
    void SetTextOverlayOffsetY( uint32_t aY );

    PvString GetTextOverlay() const;
    void GetTextOverlayColor( uint8_t &aR, uint8_t &aG, uint8_t &aB ) const;
    int32_t GetTextOverlaySize() const;
    int32_t GetTextOverlayOffsetX() const;
    int32_t GetTextOverlayOffsetY() const;

    PvBuffer &GetInternalBuffer();

    int GetHScrollPos() const;
    PvResult SetHScrollPos( int aPos );
    void GetHScrollRange( int &aMin, int &aMax ) const;
#ifdef WIN32
    PvWindowHandle GetHScrollHandle();
#endif
    
    int GetVScrollPos() const;
    PvResult SetVScrollPos( int aPos );
    void GetVScrollRange( int &aMin, int &aMax ) const;
#ifdef WIN32
    PvWindowHandle GetVScrollHandle();
#endif

    void Zoom1_1();
    void ZoomIn();
    void ZoomOut();
    void ZoomFit();

protected:

private:

    // Not implemented
	PvDisplayWnd( const PvDisplayWnd & );
	const PvDisplayWnd &operator=( const PvDisplayWnd & );

};

#endif
