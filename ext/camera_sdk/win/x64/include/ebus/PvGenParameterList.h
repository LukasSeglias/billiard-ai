// *****************************************************************************
//
//     Copyright (c) 2009, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVGENPARAMETERLIST_H__
#define __PVGENPARAMETERLIST_H__

#include "PvGenICamLib.h"
#include "PvGenParameter.h"


class PvGenParameter;


namespace PvGenICamLib
{
    class GenParameterList;
}


class PV_GENICAM_API PvGenParameterList
{
public:

    PvGenParameterList();
    ~PvGenParameterList();

    void Clear();
    void Add( PvGenParameter *aParameter );

    uint32_t GetSize() const;
    PvGenParameter *GetItem( uint32_t aIndex );
	PvGenParameter *operator[]( uint32_t aIndex );
  
    PvGenParameter *GetFirst();
    PvGenParameter *GetNext();

protected:

private:

#ifndef PV_GENERATING_DOXYGEN_DOC

    PvGenICamLib::GenParameterList *mThis;

#endif // PV_GENERATING_DOXYGEN_DOC

    // Not implemented
	PvGenParameterList( const PvGenParameterList & );
	const PvGenParameterList &operator=( const PvGenParameterList & );

};

#endif
