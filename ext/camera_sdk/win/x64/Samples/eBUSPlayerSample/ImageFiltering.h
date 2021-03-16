// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __IMAGE_FILTERING_H__
#define __IMAGE_FILTERING_H__

#include "Persistent.h"

#include <PvBufferConverter.h>
#include <PvBufferConverterRGBFilter.h>

#include <PvRangeFilter.h>
#include <PvTapFilter.h>


class DisplayThread;


class ImageFiltering : public Persistent
{
#ifdef _AFXDLL
    DECLARE_DYNAMIC( ImageFiltering )
#endif // _AFXDLL

public:

    ImageFiltering();
    ~ImageFiltering();

    // Persistent
    PvResult Save( PvConfigurationWriter *aWriter );
    PvResult Load( PvConfigurationReader *aReader );

    void ResetRGBFilter();
    void WhiteBalance( DisplayThread *aDisplayThread );
    void ConfigureConverter( PvBufferConverter &aConverter );

    bool GetRGBFilterEnabled() const { return mRGBFilterEnabled; }
    PvBufferConverterRGBFilter &GetRGBFilter() { return mRGBFilter; }
    PvBayerFilterType GetBayerFilter() const { return mBayerFilter; }
    PvTapGeometryEnum GetTapGeometry() const { return mTapGeometry; }

    void SetRGBFilterEnabled( bool aValue ) { mRGBFilterEnabled = aValue; }
    void SetBayerFilter( PvBayerFilterType aValue ) { mBayerFilter = aValue; }
    void SetTapGeometry( PvTapGeometryEnum aValue ) { mTapGeometry = aValue; }

    PvRangeFilter *GetRangeFilter() { return &mRangeFilter; }

protected:

private:

    bool mRGBFilterEnabled;
    PvBufferConverterRGBFilter mRGBFilter;
    PvBayerFilterType mBayerFilter;
    PvRangeFilter mRangeFilter;
    PvTapGeometryEnum mTapGeometry;

};


#endif // __IMAGE_FILTERING_H__

