// *****************************************************************************
//
//     Copyright (c) 2008, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVPERSISTENCELIB_H__
#define __PVPERSISTENCELIB_H__

#if !defined( PV_PERSISTENCE_NO_DECLSPEC )
    #if defined( PV_PERSISTENCE_EXPORTS )

	    #if !defined( PT_LIB_STATIC ) && defined( WIN32 )
		    #define PV_PERSISTENCE_API __declspec( dllexport )
	    #else
		    #define PV_PERSISTENCE_API
	    #endif

    #else

        #if !defined( PT_LIB_STATIC ) && defined( WIN32 )
            #define PV_PERSISTENCE_API __declspec( dllimport )
        #else
            #define PV_PERSISTENCE_API
        #endif

        #define PT_LIB_NAME "PvPersistence"
        #include "PvLinkLib.h"

    #endif
#endif

#if !defined( PV_PERSISTENCE_API )
    #define PV_PERSISTENCE_API
#endif

#include "PvResult.h"

#endif
