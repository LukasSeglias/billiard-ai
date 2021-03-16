// *****************************************************************************
//
//     Copyright (c) 2015, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __IPVFILTER_H__
#define __IPVFILTER_H__

#include "PvBuffer.h"


class PV_BUFFER_API IPvFilter
{
public:

    IPvFilter();
    virtual ~IPvFilter();

    virtual PvResult Execute( const PvBuffer *aIn, PvBuffer *aOut ) = 0;
    
    virtual PvResult SetThreadCount( uint32_t aCount ) = 0;
    virtual uint32_t GetThreadCount() const = 0;

protected:

private:

};

#endif
