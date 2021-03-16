// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVSTRING_H__
#define __PVSTRING_H__

#include "PvBaseLib.h"

#include <string>


namespace PtUtilsLib
{
    class String;
}


class PV_BASE_API PvString
{
public:

    PvString();
    PvString( const PvString & aValue );
    PvString( const char * aValue );
    PvString( const wchar_t * aValue );

    virtual ~PvString();

    const PvString &operator = ( const PvString & aValue );
    const PvString &operator += ( const PvString & aValue );

    bool operator == ( const char *aValue ) const;
    bool operator != ( const char *aValue ) const;

	bool operator == ( const wchar_t *aValue ) const;
    bool operator != ( const wchar_t *aValue ) const;

    bool operator == ( const PvString & aValue ) const;
    bool operator != ( const PvString & aValue ) const;

	operator const char *() const;
    operator const wchar_t *() const;

    const char *GetAscii() const;
    const wchar_t *GetUnicode() const;

    unsigned int GetLength() const;

private:

	mutable PtUtilsLib::String *mThis;

    const std::string *mAscii;
    const std::basic_string<wchar_t> *mUnicode;

};

#endif
