// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVSYSTEMLIB_H__
#define __PVSYSTEMLIB_H__

#if !defined( PV_SYSTEM_NO_DECLSPEC )
    #if defined( PV_SYSTEM_EXPORTS )

	    #if !defined( PT_LIB_STATIC ) && defined( WIN32 )
		    #define PV_SYSTEM_API __declspec( dllexport )
	    #else
		    #define PV_SYSTEM_API
	    #endif

    #else

        #if !defined( PT_LIB_STATIC ) && defined( WIN32 )
            #define PV_SYSTEM_API __declspec( dllimport )
        #else
            #define PV_SYSTEM_API
        #endif

        #define PT_LIB_NAME "PvSystem"
        #include "PvLinkLib.h"

    #endif
#endif

#if !defined( PV_SYSTEM_API )
    #define PV_SYSTEM_API
#endif

#include "PvResult.h"

#endif
