// *****************************************************************************
//
//     Copyright (c) 2008, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVSTREAMLIB_H__
#define __PVSTREAMLIB_H__

#if !defined( PV_STREAM_NO_DECLSPEC )
    #if defined( PV_STREAM_EXPORTS )

	    #if !defined( PT_LIB_STATIC ) && defined( WIN32 )
		    #define PV_STREAM_API __declspec( dllexport )
	    #else
		    #define PV_STREAM_API
	    #endif

    #else

        #if !defined( PT_LIB_STATIC ) && defined( WIN32 )
            #define PV_STREAM_API __declspec( dllimport )
        #else
            #define PV_STREAM_API
        #endif

        #define PT_LIB_NAME "PvStream"
        #include "PvLinkLib.h"

    #endif
#endif

#if !defined( PV_STREAM_API )
    #define PV_STREAM_API
#endif

#include "PvResult.h"

#endif
