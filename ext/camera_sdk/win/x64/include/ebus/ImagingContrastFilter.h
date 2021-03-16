// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __IMAGINGCONTRASTFILTER_H__
#define __IMAGINGCONTRASTFILTER_H__

#include "ImagingFilter.h"


namespace SimpleImagingLib 
{


class SIMPLEIMAGING_API ImagingContrastFilter : public ImagingFilter
{
public:

	static const int32_t MAX_LEVEL;
	static const int32_t MIN_LEVEL;

	ImagingContrastFilter();
	~ImagingContrastFilter(void);

    void SetLevel( int32_t aLevel );

	int32_t GetLevel() const;

protected:

    void ProcessScanLine( uint8_t* aBuffer, uint32_t aImageWidth );

private:
	int32_t mLevel;
	float   mConstantLevel;
};


}

#endif
