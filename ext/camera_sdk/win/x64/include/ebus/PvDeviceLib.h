// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVDEVICELIB_H__
#define __PVDEVICELIB_H__

#if !defined( PV_DEVICE_NO_DECLSPEC )
    #if defined( PV_DEVICE_EXPORTS )

	    #if !defined( PT_LIB_STATIC ) && defined( WIN32 )
		    #define PV_DEVICE_API __declspec( dllexport )
	    #else
		    #define PV_DEVICE_API
	    #endif

    #else

        #if !defined( PT_LIB_STATIC ) && defined( WIN32 )
            #define PV_DEVICE_API __declspec( dllimport )
        #else
            #define PV_DEVICE_API
        #endif

        #define PT_LIB_NAME "PvDevice"
        #include "PvLinkLib.h"

    #endif
#endif

#if !defined( PV_DEVICE_API )
    #define PV_DEVICE_API
#endif

#include "PvResult.h"

#endif
