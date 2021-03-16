// *****************************************************************************
//
//     Copyright (c) 2008, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVBUFFERLIB_H__
#define __PVBUFFERLIB_H__

#if !defined( PV_BUFFER_NO_DECLSPEC )
    #if defined( PV_BUFFER_EXPORTS )

	    #if !defined( PT_LIB_STATIC ) && defined( WIN32 )
		    #define PV_BUFFER_API __declspec( dllexport )
	    #else
		    #define PV_BUFFER_API
	    #endif

    #else

        #if !defined( PT_LIB_STATIC ) && defined( WIN32 )
            #define PV_BUFFER_API __declspec( dllimport )
        #else
            #define PV_BUFFER_API
        #endif

        #define PT_LIB_NAME "PvBuffer"
        #include "PvLinkLib.h"

    #endif
#endif

#if !defined( PV_BUFFER_API )
    #define PV_BUFFER_API
#endif

#include "PvResult.h"

#endif
