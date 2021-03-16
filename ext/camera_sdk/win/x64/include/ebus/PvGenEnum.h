// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVGENENUM_H__
#define __PVGENENUM_H__

#include "PvGenParameter.h"
#include "PvGenEnumEntry.h"


class PvGenEnum : public PvGenParameter
{
public:

	PV_GENICAM_API PvResult SetValue( int64_t aValue );
	PV_GENICAM_API PvResult SetValue( const PvString &aValue );
	PV_GENICAM_API PvResult GetValue( PvString &aValue ) const;
	PV_GENICAM_API PvResult GetValue( int64_t &aValue ) const;

	PV_GENICAM_API PvResult GetEntriesCount( int64_t &aCount ) const;
	PV_GENICAM_API PvResult GetEntryByName( const PvString &aEntryName, const PvGenEnumEntry **aEntry ) const;
	PV_GENICAM_API PvResult GetEntryByIndex( int64_t aIndex, const PvGenEnumEntry **aEntry ) const;
	PV_GENICAM_API PvResult GetEntryByValue( int64_t aValue, const PvGenEnumEntry **aEntry ) const;

protected:

	PvGenEnum();
	virtual ~PvGenEnum();

private:

    // Not implemented
	PvGenEnum( const PvGenEnum & );
	const PvGenEnum &operator=( const PvGenEnum & );

};

#endif
