// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __IMAGINGFILTERRGB_H__
#define __IMAGINGFILTERRGB_H__

#include "SimpleImagingLib.h"
#include "ImagingFilter.h"


namespace SimpleImagingLib 
{


class SIMPLEIMAGING_API ImagingFilterRGB : public ImagingFilter
{
public:

	ImagingFilterRGB();

	int8_t GetGainX() const;

    void SetGainR( int8_t aValue );
	int8_t GetGainR() const;

    void SetGainG( int8_t aValue );
	int8_t GetGainG() const;

    void SetGainB( int8_t aValue );
	int8_t GetGainB() const;

protected:

    void ProcessScanLine( uint8_t* aBuffer, uint32_t aImageWidth );

private:

    XBGR mGains;
	
};


}

#endif
