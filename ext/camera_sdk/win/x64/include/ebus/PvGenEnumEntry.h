// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVGENENUMENTRY_H__
#define __PVGENENUMENTRY_H__

#include "PvGenICamLib.h"
#include "PvGenTypes.h"


namespace PvGenICamLib
{
    class GenEnumEntryInternal;
}


class PvGenEnumEntry
{
public:

	PV_GENICAM_API PvResult GetValue( int64_t &aValue ) const;
	PV_GENICAM_API PvResult GetName( PvString &aName ) const;

	PV_GENICAM_API PvResult GetToolTip( PvString &aToolTip ) const;
	PV_GENICAM_API PvResult GetDescription( PvString &aDescription ) const;
	PV_GENICAM_API PvResult GetVisibility( PvGenVisibility &aVisibility ) const;
    PV_GENICAM_API PvResult GetDisplayName( PvString &aDisplayName ) const;
    PV_GENICAM_API PvResult GetNameSpace( PvGenNameSpace &aNameSpace ) const;

	PV_GENICAM_API PvResult IsVisible( PvGenVisibility aVisibility, bool &aVisible ) const;
	PV_GENICAM_API PvResult IsAvailable( bool &aAvailable ) const;

	PV_GENICAM_API bool IsVisible( PvGenVisibility aVisibility ) const;
	PV_GENICAM_API bool IsAvailable() const;

protected:

	PvGenEnumEntry();
	virtual ~PvGenEnumEntry();

    PvGenICamLib::GenEnumEntryInternal *mThis;

private:

};

#endif
