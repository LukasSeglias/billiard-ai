// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVGENFLOAT_H__
#define __PVGENFLOAT_H__

#include "PvGenParameter.h"


class PvGenFloat : public PvGenParameter
{
public:

	PV_GENICAM_API PvResult SetValue( double aValue );
	PV_GENICAM_API PvResult GetValue( double &aValue ) const;
	
	PV_GENICAM_API PvResult GetMin( double &aMin ) const;
	PV_GENICAM_API PvResult GetMax( double &aMax ) const;

	PV_GENICAM_API PvResult GetRepresentation( PvGenRepresentation &aRepresentation ) const;

	PV_GENICAM_API PvResult GetUnit( PvString &aUnit ) const;

protected:

	PvGenFloat();
	virtual ~PvGenFloat();

private:

    // Not implemented
	PvGenFloat( const PvGenFloat & );
	const PvGenFloat &operator=( const PvGenFloat & );
};

#endif
