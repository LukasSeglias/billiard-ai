// *****************************************************************************
//
//     Copyright (c) 2015, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVTAPFILTER_H__
#define __PVTAPFILTER_H__

#include "IPvFilter.h"
#include "PvTapGeometry.h"


class TapFilter;


class PV_BUFFER_API PvTapFilter : public IPvFilter
{
public:

    PvTapFilter();
    virtual ~PvTapFilter();

    PvResult Execute( const PvBuffer *aIn, PvBuffer *aOut );

    PvResult SetThreadCount( uint32_t aCount );
    uint32_t GetThreadCount() const;

    PvResult SetGeometry( PvTapGeometryEnum aValue );
    PvTapGeometryEnum GetGeometry() const;

    static uint32_t GetSupportedGeometryCount();
    static PvTapGeometryEnum GetSupportedGeometryValue( uint32_t aIndex );
    static const PvString &GetSupportedGeometryName( uint32_t aIndex );
    static PvPixelType GetOutputPixelTypeFor( const PvBuffer *aBuffer );
    static uint32_t GetTapCountFor( PvTapGeometryEnum aValue );
    static uint32_t GetTapCountFor( const PvString &aValue );

protected:

private:

    // Not implemented
    PvTapFilter( const PvTapFilter & );
    const PvTapFilter &operator=( const PvTapFilter & );

    TapFilter * mThis;

};

#endif
