// *****************************************************************************
//
//     Copyright (c) 2011, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVPROPERTYLIST_H__
#define __PVPROPERTYLIST_H__

#include "PvPersistenceLib.h"
#include "PvProperty.h"


namespace PvPersistenceLib
{
    class PropertyList;

} // namespace PvPersistenceLib


class PV_PERSISTENCE_API PvPropertyList
{
public:

    PvPropertyList();
    ~PvPropertyList();

    void Clear();
    void Add( const PvProperty &aString );

    uint32_t GetSize() const;
    PvProperty *GetItem( uint32_t aIndex );
    PvProperty *operator[]( uint32_t aIndex );
  
    PvProperty *GetFirst();
    PvProperty *GetNext();

    PvProperty *GetProperty( const PvString &aName );

protected:

private:

#ifndef PV_GENERATING_DOXYGEN_DOC

    PvPersistenceLib::PropertyList *mThis;

#endif // PV_GENERATING_DOXYGEN_DOC

    // Not implemented
	PvPropertyList( const PvPropertyList & );
	const PvPropertyList &operator=( const PvPropertyList & );
};

#endif
