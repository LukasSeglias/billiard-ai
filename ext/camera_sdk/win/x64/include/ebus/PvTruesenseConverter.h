// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVTRUESENSECONVERTER_H__
#define __PVTRUESENSECONVERTER_H__

#include "PvBuffer.h"


namespace PvBufferLib
{
    class TruesenseConverter;
};

struct YotsubaParam_t;


class PV_BUFFER_API PvTruesenseConverter
{
public:

    PvTruesenseConverter();
    virtual ~PvTruesenseConverter();

    static bool IsConversionSupported( PvPixelType aSource, PvPixelType aDestination );

    PvResult Convert( const PvImage *aSource, PvImage *aDestination, bool aReallocIfNeeded = true );

    uint32_t GetDarkfloor() const;
    float GetRedGain() const;
    float GetGreenGain() const;
    float GetBlueGain() const;
    float GetPanGain() const;
    float GetGlobalGain() const;
    float GetSharpenParam() const;
    float GetMaxSharpen() const;
    float GetHighLumaNoise() const;
    float GetLowLumaNoise() const;

    PvResult SetDarkfloor( uint32_t aValue );
    PvResult SetRedGain( float aValue );
    PvResult SetGreenGain( float aValue );
    PvResult SetBlueGain( float aValue );
    PvResult SetPanGain( float aValue );
    PvResult SetGlobalGain( float aValue );
    PvResult SetSharpenParam( float aValue );
    PvResult SetMaxSharpen( float aValue );
    PvResult SetHighLumaNoise( float aValue );
    PvResult SetLowLumaNoise( float aValue );

protected:

private:

    // Not implemented
    PvTruesenseConverter( const PvTruesenseConverter & );
    const PvTruesenseConverter &operator=( const PvTruesenseConverter & );

    PvBufferLib::TruesenseConverter *mThis;
    YotsubaParam_t *mParameters;

};

#endif
