// *****************************************************************************
//
// Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include <PvSampleUtils.h>
#include <PvBuffer.h>

#include "VideoSource.h"


///
/// \brief Constructor
///

VideoSource::VideoSource()
    : mSeed( 0 )
{
}


///
/// \brief Destructor
///

VideoSource::~VideoSource()
{
}


///
/// \brief Copies the pattern into a buffer
///

void VideoSource::CopyPattern( PvBuffer *aBuffer )
{
    PvImage *lImage = aBuffer->GetImage();
    uint32_t lWidth = lImage->GetWidth();
    uint32_t lHeight = lImage->GetHeight();

    unsigned char *lPtr = lImage->GetDataPointer();
    for ( uint32_t y = 0; y < lHeight; y++ )
    {
        unsigned char lValue = mSeed + y;
        for ( uint32_t x = 0; x < lWidth; x++ )
        {
            *( lPtr++ ) = lValue++;
        }
    }

    // Make sure we get a different pattern next time by
    mSeed++;
}




