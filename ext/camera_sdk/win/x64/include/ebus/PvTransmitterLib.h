// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVTRANSMITTERLIB_H__
#define __PVTRANSMITTERLIB_H__

#if !defined( PV_TRANSMITTER_NO_DECLSPEC )
    #if defined( PV_TRANSMITTER_EXPORTS )

	    #if !defined( PT_LIB_STATIC ) && defined( WIN32 )
		    #define PV_TRANSMITTER_API __declspec( dllexport )
	    #else
		    #define PV_TRANSMITTER_API
	    #endif

    #else

        #if !defined( PT_LIB_STATIC ) && defined( WIN32 )
            #define PV_TRANSMITTER_API __declspec( dllimport )
        #else
            #define PV_TRANSMITTER_API
        #endif

        #define PT_LIB_NAME "PvTransmitter"
        #include "PvLinkLib.h"

    #endif
#endif

#if !defined( PV_TRANSMITTER_API )
    #define PV_TRANSMITTER_API
#endif

#include "PvResult.h"

#endif
