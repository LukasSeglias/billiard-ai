// *****************************************************************************
//
//     Copyright (c) 2009, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVSERIALLIB_H__
#define __PVSERIALLIB_H__

#if !defined( PV_SERIAL_NO_DECLSPEC )
    #if defined( PV_SERIAL_EXPORTS )

	    #if !defined( PT_LIB_STATIC ) && defined( WIN32 )
		    #define PV_SERIAL_API __declspec( dllexport )
	    #else
		    #define PV_SERIAL_API
	    #endif

    #else

        #if !defined( PT_LIB_STATIC ) && defined( WIN32 )
            #define PV_SERIAL_API __declspec( dllimport )
        #else
            #define PV_SERIAL_API
        #endif

        #define PT_LIB_NAME "PvSerial"
        #include "PvLinkLib.h"

    #endif
#endif

#if !defined( PV_SERIAL_API )
    #define PV_SERIAL_API
#endif

#include "PvResult.h"

#endif
