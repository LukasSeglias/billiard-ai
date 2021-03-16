// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVBASELIB_H__
#define __PVBASELIB_H__

#if !defined( PV_BASE_NO_DECLSPEC )
    #if defined( PV_BASE_EXPORTS )

	    #if !defined( PT_LIB_STATIC ) && defined( WIN32 )
		    #define PV_BASE_API __declspec( dllexport )
	    #else
		    #define PV_BASE_API
	    #endif

    #else

        #if !defined( PT_LIB_STATIC ) && defined( WIN32 )
            #define PV_BASE_API __declspec( dllimport )
        #else
            #define PV_BASE_API
        #endif

        #define PT_LIB_NAME "PvBase"
        #include "PvLinkLib.h"

    #endif
#endif

#if !defined( PV_BASE_API )
    #define PV_BASE_API
#endif

#include "PvTypes.h"

PV_BASE_API int PvGetVersionMajor();
PV_BASE_API int PvGetVersionMinor();
PV_BASE_API int PvGetVersionSub();
PV_BASE_API int PvGetVersionBuild();

#ifdef PT_VLD
#include <vld.h>
#endif

#define PVDELETE(a) if ( a != NULL ) { delete a; a = NULL; }
#define PVDELETEARRAY( a ) if ( a != NULL ) { delete []a; a = NULL; }

#endif
