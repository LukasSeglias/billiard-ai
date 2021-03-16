// *****************************************************************************
//
//     Copyright (c) 2014, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVLICENSELIB_H__
#define __PVLICENSELIB_H__

#if !defined( PV_LICENSE_NO_DECLSPEC )
    #if defined( PV_LICENSE_EXPORTS )

	    #if !defined( PT_LIB_STATIC ) && defined( WIN32 )
		    #define PV_LICENSE_API __declspec( dllexport )
	    #else
		    #define PV_LICENSE_API
	    #endif

    #else

        #if !defined( PT_LIB_STATIC ) && defined( WIN32 )
            #define PV_LICENSE_API __declspec( dllimport )
        #else
            #define PV_LICENSE_API
        #endif

        #define PT_LIB_NAME "PvLicense"
        #include "PvLinkLib.h"

    #endif
#endif

#if !defined( PV_LICENSE_API )
    #define PV_LICENSE_API
#endif

#include "PvResult.h"

#endif
