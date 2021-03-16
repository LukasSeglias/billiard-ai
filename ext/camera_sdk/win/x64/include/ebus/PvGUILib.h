// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVGUILIB_H__
#define __PVGUILIB_H__

#if !defined( PV_GUI_DOTNET )
	#if defined( PV_GUI_EXPORTS )

		  #if !defined( PT_LIB_STATIC ) && defined( WIN32 )
				#define PV_GUI_API __declspec( dllexport )
		  #else
				#define PV_GUI_API
		  #endif

	#else

		#if !defined( PT_LIB_STATIC ) && defined( WIN32 )
			#define PV_GUI_API __declspec( dllimport )
		#else
			#define PV_GUI_API
		#endif

		#if defined ( PV_DEBUG )
			#define _PT_DEBUG_
			#define PT_DEBUG_ENABLED
		#endif

		#if defined( PT_LIB_STATIC )
			#define PT_SUFFIX_STATIC "_s"
		#else
			#define PT_SUFFIX_STATIC
		#endif

		#if defined( _PT_DEBUG_ ) && defined( PT_DEBUG_ENABLED )
			#define PT_SUFFIX_DEBUG "_Dbg"
		#else
			#define PT_SUFFIX_DEBUG
		#endif

        #if defined( _PT_64_ ) || defined( _WIN64 )
			#define PT_SUFFIX_64 "64"
		#else
			#define PT_SUFFIX_64
		#endif

        #if ( _MSC_VER >= 1910 )
            // VC 15.0 (aka 2017)
            #define PT_SUFFIX_COMPILER "_VC15"
        #elif( _MSC_VER >= 1900 )
            // VC 14.0 (aka 2015)
            #define PT_SUFFIX_COMPILER "_VC14"
        #elif( _MSC_VER >= 1800 )
            // VC 12.0 (aka 2013)
            #define PT_SUFFIX_COMPILER "_VC12"
        #elif( _MSC_VER >= 1700 )
			// VC 11.0 (aka 2012)
			#define PT_SUFFIX_COMPILER "_VC11"
		#elif( _MSC_VER >= 1600 )
			// VC 10.0 (aka 2010)
			#define PT_SUFFIX_COMPILER "_VC10"
		#else
			#if defined( WIN32 )
				#pragma message ( "Warning: Your compiler is not officially supported by the eBUS SDK. Currently supported compiler versions on Windows include Visual C++ 10 2010 to Visual C++ 15 2017." )
			#endif
			#define PT_SUFFIX_COMPILER
		#endif

		#pragma comment( lib, "PvGUI" PT_SUFFIX_64 PT_SUFFIX_STATIC PT_SUFFIX_COMPILER PT_SUFFIX_DEBUG ".lib" )

		#undef PT_SUFFIX_STATIC
		#undef PT_SUFFIX_DEBUG
		#undef PT_SUFFIX_64

	#endif

	#include "PvTypes.h"
	#include "PvString.h"
	#include "PvResult.h"

    #if defined( WIN32 )

        #include <Windows.h>
        typedef HWND PvWindowHandle;

    #endif
#endif

#include "PvResult.h"

#endif
