// *****************************************************************************
//
// Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __VIDEO_SOURCE_H__
#define __VIDEO_SOURCE_H__

#include <PvSampleUtils.h>


struct CvCapture;
class PvBuffer;


// Video source
class VideoSource
{
public:

    VideoSource();
    ~VideoSource();

    void CopyPattern( PvBuffer *aBuffer );

private:

    unsigned char mSeed;
};


#endif // __VIDEO_SOURCE_H__

