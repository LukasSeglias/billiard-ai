// *****************************************************************************
//
//     Copyright (c) 2011, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVSTRINGLIST_H__
#define __PVSTRINGLIST_H__

#include "PvString.h"


namespace PvBaseLib
{
    class StringList;
}


class PV_BASE_API PvStringList
{
public:

    PvStringList();
    ~PvStringList();

    void Clear();
    void Add( const PvString &aString );

    uint32_t GetSize() const;
    PvString *GetItem( uint32_t aIndex );
    PvString *operator[]( uint32_t aIndex );
  
    PvString *GetFirst();
    PvString *GetNext();

protected:

private:

#ifndef PV_GENERATING_DOXYGEN_DOC

    PvBaseLib::StringList *mThis;

#endif // PV_GENERATING_DOXYGEN_DOC

    // Not implemented
	PvStringList( const PvStringList & );
	const PvStringList &operator=( const PvStringList & );

};

#endif
