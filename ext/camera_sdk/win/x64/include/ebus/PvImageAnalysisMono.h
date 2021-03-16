// *****************************************************************************
//
//     Copyright (c) 2015, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVIMAGEANALYSISMONO_H__
#define __PVIMAGEANALYSISMONO_H__

#include "PvAppUtilsLib.h"
#include "PvBuffer.h"
#include "PvConfigurationReader.h"
#include "PvConfigurationWriter.h"


namespace PvAppUtilsLib
{
    class ImageAnalysisMono;
}


class PV_APPUTILS_API PvImageAnalysisMono
{
public:

    PvImageAnalysisMono();
	~PvImageAnalysisMono();

    PvResult Process( PvBuffer *aBuffer );

    PvResult GetHistogram( uint32_t *aHistogram, int aBufferSize, int &aHistogramSize, uint32_t &aMaxValue );
    double GetAvg() const;
    double GetStdDev() const;

    void GetROI( int &aX, int &aY, int &aWidth, int &aHeight );
    void SetROI( int aX, int aY, int aWidth, int aHeight );

    void Reset();

    PvResult Load( PvConfigurationReader *aReader );
    PvResult Save( PvConfigurationWriter *aWriter );

private:

    PvAppUtilsLib::ImageAnalysisMono *mThis;

	 // Not implemented
	PvImageAnalysisMono( const PvImageAnalysisMono & );
	const PvImageAnalysisMono &operator=( const PvImageAnalysisMono & );

};

#endif
