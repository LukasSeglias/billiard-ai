// *****************************************************************************
//
//     Copyright (c) 2009, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVDEINTERLACER_H__
#define __PVDEINTERLACER_H__

#include "PvBuffer.h"


namespace PvBufferLib
{
    class DeInterlacer;
};


class PV_BUFFER_API PvDeInterlacer
{

public:

    PvDeInterlacer();
    virtual ~PvDeInterlacer();

    PvResult Apply( const PvBuffer *aIn, PvBuffer *aOut );
    PvResult Apply( const PvBuffer *aInOdd, const PvBuffer *aInEven, PvBuffer *aOut );

    PvResult ApplyOdd( const PvBuffer *aIn, PvBuffer *aOut );
    PvResult ApplyEven( const PvBuffer *aIn, PvBuffer *aOut );

    PvResult ApplyDoubling( const PvBuffer *aIn, PvBuffer *aOut );
    PvResult ApplyBlending( const PvBuffer *aInOdd, const PvBuffer *aInEven, PvBuffer *aOut );

    bool GetFieldInversion() const;
    void SetFieldInversion( bool aInvert );

protected:

private:

    // Not implemented
	PvDeInterlacer( const PvDeInterlacer & );
	const PvDeInterlacer &operator=( const PvDeInterlacer & );

    PvBufferLib::DeInterlacer *mThis;
};

#endif
