// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVGENBOOLEAN_H__
#define __PVGENBOOLEAN_H__

#include "PvGenParameter.h"


class PvGenBoolean : public PvGenParameter
{
public:

	PV_GENICAM_API PvResult SetValue( bool aValue );
	PV_GENICAM_API PvResult GetValue( bool &aValue ) const;

protected:

	PvGenBoolean();
	virtual ~PvGenBoolean();

private:

    // Not implemented
	PvGenBoolean( const PvGenBoolean & );
	const PvGenBoolean &operator=( const PvGenBoolean & );
};

#endif
