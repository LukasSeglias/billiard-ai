// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __IMAGINGFILTER_H__
#define __IMAGINGFILTER_H__

#include "SimpleImagingLib.h"
#include "ImagingBuffer.h"

#include <utility>
#include <algorithm>


namespace SimpleImagingLib 
{


#pragma pack( push, 1 )
struct XBGR
{
	uint8_t B;
	uint8_t G;
	uint8_t R;
	uint8_t X;

};
#pragma pack( pop )

class SIMPLEIMAGING_API ImagingFilter
{
public:

	ImagingFilter()
	{
	}

	virtual ~ImagingFilter()
	{
	}

	void Apply( ImagingBuffer* aImage )
	{
		Apply( aImage->GetTopPtr(), aImage->GetWidth(), aImage->GetHeight(), aImage->GetStride() );
	}

	void Apply( uint8_t* aBuffer, uint32_t aImageWidth, uint32_t aImageHeight, int32_t aStride )
	{
		uint8_t* lLine = aBuffer;
		for( uint32_t y = 0; y < aImageHeight; y++ )
		{
			
			ProcessScanLine( lLine, aImageWidth );
			lLine += aStride;
		}
	}

protected:

	virtual void ProcessScanLine( uint8_t* aBuffer, uint32_t aWidth ){}


};

template <typename T> T 
    Clip( const T& ioValue, const T& aLower, const T& aUpper ) 
{
	using namespace std;
	return std::max( aLower, min( ioValue, aUpper ) );
}


}

#endif
