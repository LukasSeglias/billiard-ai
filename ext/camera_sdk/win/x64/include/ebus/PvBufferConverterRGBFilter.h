// *****************************************************************************
//
//     Copyright (c) 2008, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVBUFFERCONVERTERRGBFILTER_H__
#define __PVBUFFERCONVERTERRGBFILTER_H__

#include "PvBuffer.h"


namespace PvBufferLib
{
    class BufferConverterRGBFilter;
};


class PvBufferConverter;


class PV_BUFFER_API PvBufferConverterRGBFilter
{

public:

    PvBufferConverterRGBFilter();
    virtual ~PvBufferConverterRGBFilter();

    double GetGainR() const;
    double GetGainG() const;
    double GetGainB() const;

    void SetGainR( double aValue );
    void SetGainG( double aValue );
    void SetGainB( double aValue );

    int32_t GetOffsetR() const;
    int32_t GetOffsetG() const;
    int32_t GetOffsetB() const;
    
    void SetOffsetR( int32_t aValue );
    void SetOffsetG( int32_t aValue );
    void SetOffsetB( int32_t aValue );

    PvResult WhiteBalance( PvBuffer *aBuffer );
    void Reset();

protected:

private:

    friend class PvBufferConverter;

    // Not implemented
	PvBufferConverterRGBFilter( const PvBufferConverterRGBFilter & );
	const PvBufferConverterRGBFilter &operator=( const PvBufferConverterRGBFilter & );

    PvBufferLib::BufferConverterRGBFilter *mThis;
};

#endif
