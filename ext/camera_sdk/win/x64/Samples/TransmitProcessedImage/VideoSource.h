// *****************************************************************************
//
// Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __VIDEO_SOURCE_H__
#define __VIDEO_SOURCE_H__

#ifdef WIN32 

#pragma comment( lib, "gdiplus" )

#include <PvSampleUtils.h>
#include <PvDevice.h>
#include <PvStream.h>
#include <PvPipeline.h>
#include <PvBufferConverter.h>
#include <PvStreamGEV.h>

#include <map>

#include <windows.h>
#include <gdiplus.h>

typedef map<uint32_t, Gdiplus::Bitmap *> gBMap;
class PvTransmitterGEV;


// Video source
class VideoSource
{
public:

    VideoSource( const char *aConnectionID );
    ~VideoSource();

    uint32_t GetWidth();
    uint32_t GetHeight();
    PvPixelType GetPixelFormat();

    bool Connect();
    bool StartAcquisition();
    bool StopAcquisition();
    bool Disconnect();

    bool FillBuffer( PvBuffer *aBuffer, PvTransmitterGEV *aTransmitter );

private:

    Gdiplus::Bitmap *GetImageForBuffer( PvBuffer *aBuffer );
    void Draw( PvTransmitterGEV *aTransmitter, Gdiplus::Bitmap *aImage );

    PvString mConnectionID;

    PvDevice *mDevice;
    PvStream *mStream;
    PvPipeline* mPipeline;

    PvBufferConverter mConverter;

    gBMap mMap;
    ULONG_PTR mGdiplusToken;
};

#endif // WIN32

#endif // __VIDEO_SOURCE_H__

