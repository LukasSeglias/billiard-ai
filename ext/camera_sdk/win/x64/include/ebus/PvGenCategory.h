// *****************************************************************************
//
//     Copyright (c) 2011, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVGENCATEGORY_H__
#define __PVGENCATEGORY_H__

#include "PvGenICamLib.h"
#include "PvGenTypes.h"
#include "PvGenApi.h"


namespace PvGenICamLib
{
    class GenParameterArray;
    class GenParameterInternal;
    class GenParameterArrayManager;

}; // namespace PvGenICamLib


class PvGenCategory
{
public:

	PV_GENICAM_API PvResult GetName( PvString &aName ) const;
	PV_GENICAM_API PvResult GetToolTip( PvString &aToolTip ) const;
	PV_GENICAM_API PvResult GetDescription( PvString &aDescription ) const;
    PV_GENICAM_API PvResult GetDisplayName( PvString &aDisplayName ) const;
    PV_GENICAM_API PvResult GetNameSpace( PvGenNameSpace &aNameSpace ) const;

	PV_GENICAM_API PvResult GetVisibility( PvGenVisibility &aVisibility ) const;
	PV_GENICAM_API PvResult IsVisible( PvGenVisibility aCurrentVisibility, bool &aVisible ) const;
	PV_GENICAM_API bool IsVisible( PvGenVisibility aCurrentVisibility ) const;

    PV_GENICAM_API PvResult IsImplemented( bool &aImplemented ) const;
	PV_GENICAM_API bool IsImplemented() const;

	PV_GENICAM_API PV_GENAPI_NS::INode *GetNode();

protected:

	PvGenCategory();
	virtual ~PvGenCategory();

#ifndef PV_GENERATING_DOXYGEN_DOC

    PvGenICamLib::GenParameterInternal *mThis;

    friend class PvGenICamLib::GenParameterArray;
    friend class PvGenICamLib::GenParameterArrayManager;

#endif // PV_GENERATING_DOXYGEN_DOC 

private:

    // Not implemented
	PvGenCategory( const PvGenCategory & );
	const PvGenCategory &operator=( const PvGenCategory & );
};

#endif
