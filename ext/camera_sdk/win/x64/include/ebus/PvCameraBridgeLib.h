// *****************************************************************************
//
//     Copyright (c) 2016, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVCAMERABRIDGELIB_H__
#define __PVCAMERABRIDGELIB_H__

#if !defined( PV_CAMERABRIDGE_NO_DECLSPEC )
    #if defined( PV_CAMERABRIDGE_EXPORTS )

	    #if !defined( PT_LIB_STATIC ) && defined( WIN32 )
		    #define PV_CAMERABRIDGE_API __declspec( dllexport )
	    #else
		    #define PV_CAMERABRIDGE_API
	    #endif

    #else

        #if !defined( PT_LIB_STATIC ) && defined( WIN32 )
            #define PV_CAMERABRIDGE_API __declspec( dllimport )
        #else
            #define PV_CAMERABRIDGE_API
        #endif

        #define PT_LIB_NAME "PvCameraBridge"
        #include "PvLinkLib.h"

    #endif
#endif

#if !defined( PV_CAMERABRIDGE_API )
    #define PV_CAMERABRIDGE_API
#endif

#include "PvResult.h"

#endif
