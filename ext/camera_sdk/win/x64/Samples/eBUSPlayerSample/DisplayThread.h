// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __DISPLAY_THREAD_H__
#define __DISPLAY_THREAD_H__

#include "LogBuffer.h"

#include <PvDisplayThread.h>

class ImageSaving;
class ImageFiltering;


// Adapter used to allow interation with PvDisplayWnd (C++) or PvDisplayView (Cocoa/Objective-C)
#ifndef __MACH__
#include <PvDisplayWnd.h>
class IPvDisplayAdapter : public PvDisplayWnd
{
public:

    // Stub for OS X-only method
    virtual void ReleaseInternalBuffer() {}

};
#else
class IPvDisplayAdapter
{
public:

    IPvDisplayAdapter() {}
    virtual ~IPvDisplayAdapter() {}

    virtual PvBufferConverter &GetConverter() = 0;
    virtual PvResult Display( PvBuffer &aBuffer, bool aVsync ) = 0;
    virtual PvResult SetTextOverlay( const PvString &aText ) = 0;
    virtual PvBuffer &GetInternalBuffer() = 0;
    virtual void ReleaseInternalBuffer() {}

};
#endif


class DisplayThread : public PvDisplayThread, public IChanged
{
#ifdef _AFXDLL
    DECLARE_DYNAMIC( DisplayThread )
#endif // _AFXDLL

public:

    DisplayThread( 
        IPvDisplayAdapter *aDisplay, ImageFiltering *aImageFiltering,
        ImageSaving* aImageSaving, LogBuffer *aLogBuffer );
    ~DisplayThread();

    void WhiteBalance( PvBufferConverterRGBFilter *aFilterRGB );

#ifdef QT_VERSION
    PvRendererMode GetRendererMode() { return mDisplay->GetRendererMode(); }
    void SetRendererMode( PvRendererMode aMode ) { mDisplay->SetRendererMode( aMode ); }
#endif

    bool HasChanged();
    void ResetChanged();

protected:

    void Log( PvBuffer *aBuffer, uint64_t aLastBlockID );

    void OnBufferRetrieved( PvBuffer *aBuffer );
    void OnBufferDisplay( PvBuffer *aBuffer );
    void OnBufferDone( PvBuffer *aBuffer );
    void OnBufferLog( const PvString &aLog );
    void OnBufferTextOverlay( const PvString &aText );

private:

    LogBuffer *mLogBuffer;

    IPvDisplayAdapter *mDisplay;
    ImageFiltering *mImageFiltering;
    ImageSaving *mImageSaving;

    PvString mBaseline;

};

#endif // __DISPLAY_THREAD_H__

