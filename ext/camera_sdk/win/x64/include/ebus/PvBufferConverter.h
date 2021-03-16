// *****************************************************************************
//
//     Copyright (c) 2008, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVBUFFERCONVERTER_H__
#define __PVBUFFERCONVERTER_H__

#include "PvBuffer.h"
#include "PvBufferConverterRGBFilter.h"


namespace PvBufferLib
{
    class BufferConverter;
};


typedef enum 
{
    PvBayerFilterSimple = 1,
    PvBayerFilter3X3 = 2

} PvBayerFilterType;


class PV_BUFFER_API PvBufferConverter
{
public:

    PvBufferConverter( int32_t aMaxNumberOfThreads = -1 );
    virtual ~PvBufferConverter();

    static bool IsConversionSupported( PvPixelType aSource, PvPixelType aDestination );

    PvResult Convert( const PvBuffer *aSource, PvBuffer *aDestination, bool aReallocIfNeeded = true, bool aFlipY = false );
    
    PvBayerFilterType GetBayerFilter() const;
    PvResult SetBayerFilter( PvBayerFilterType aFilter );

    PvResult ResetRGBFilter();
    PvResult SetRGBFilter( PvBufferConverterRGBFilter &aFilter );

    uint32_t GetConversionThreadsPriority() const;
    PvResult SetConversionThreadsPriority( uint32_t aPriority );

protected:

private:

    // Not implemented
	PvBufferConverter( const PvBufferConverter & );
	const PvBufferConverter &operator=( const PvBufferConverter & );

    PvBufferLib::BufferConverter *mThis;
};

#endif
