// *****************************************************************************
//
//     Copyright (c) 2016, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __SIMPLEIMAGINGLIB_H__
#define __SIMPLEIMAGINGLIB_H__

#if !defined( SIMPLEIMAGINGLIB_NO_DECLSPEC )
    #if defined( SIMPLEIMAGINGLIB_EXPORTS )

	    #if !defined( PT_LIB_STATIC ) && defined( WIN32 )
		    #define SIMPLEIMAGING_API __declspec( dllexport )
	    #else
		    #define SIMPLEIMAGING_API
	    #endif

    #else

        #if !defined( PT_LIB_STATIC ) && defined( WIN32 )
            #define SIMPLEIMAGING_API __declspec( dllimport )
        #else
            #define SIMPLEIMAGING_API
        #endif

        #define PT_LIB_NAME "SimpleImagingLib"
        #include "PvLinkLib.h"

    #endif
#endif

#if !defined( SIMPLEIMAGING_API )
    #define SIMPLEIMAGING_API
#endif

#endif
