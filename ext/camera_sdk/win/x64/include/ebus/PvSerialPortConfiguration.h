// *****************************************************************************
//
//     Copyright (c) 2011, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVSERIALPORTCONFIGURATION_H__
#define __PVSERIALPORTCONFIGURATION_H__

#include "PvSerialLib.h"


typedef enum 
{
    PvParityInvalid = -1,
    PvParityNone = 0,
    PvParityEven = 1,
    PvParityOdd = 2

} PvParity;


class PV_SERIAL_API PvSerialPortConfiguration
{
public:

    PvSerialPortConfiguration();
    PvSerialPortConfiguration( uint32_t aBaudRate, PvParity aParity, uint32_t aByteSize, uint32_t aStopBits );
    ~PvSerialPortConfiguration();

    PvResult IsValid() const;
    void MakeInvalid();

    uint32_t mBaudRate;
    PvParity mParity;
    uint32_t mByteSize;
    uint32_t mStopBits;

private:

};

#endif
