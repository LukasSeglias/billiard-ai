// *****************************************************************************
//
//     Copyright (c) 2008, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVGENICAMLIB_H__
#define __PVGENICAMLIB_H__

#if !defined( PV_GENICAM_NO_DECLSPEC )
    #if defined( PV_GENICAM_EXPORTS )

	    #if !defined( PT_LIB_STATIC ) && defined( WIN32 )
		    #define PV_GENICAM_API __declspec( dllexport )
	    #else
		    #define PV_GENICAM_API
	    #endif

    #else

        #if !defined( PT_LIB_STATIC ) && defined( WIN32 )
            #define PV_GENICAM_API __declspec( dllimport )
        #else
            #define PV_GENICAM_API
        #endif

        #define PT_LIB_NAME "PvGenICam"
        #include "PvLinkLib.h"

    #endif
#endif

#if !defined( PV_GENICAM_API )
    #define PV_GENICAM_API
#endif

#include "PvResult.h"

#endif
