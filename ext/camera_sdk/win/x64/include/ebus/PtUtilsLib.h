// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PTUTILSLIB_H__
#define __PTUTILSLIB_H__

#ifndef PT_UTILS_LIB_API

	#ifdef PT_UTILSLIB_EXPORTS

		// Define the export symbol
		#if !defined( PT_LIB_STATIC ) && defined( WIN32 )
			#define PT_UTILS_LIB_API __declspec( dllexport )
		#else
			#define PT_UTILS_LIB_API
		#endif // PT_LIB_STATIC

	#else // PT_UTILSLIB_EXPORTS

		// Define the import symbol
		#if !defined(PT_LIB_STATIC) && defined(WIN32)
			#define PT_UTILS_LIB_API __declspec( dllimport )
		#else
			#define PT_UTILS_LIB_API
		#endif // PT_LIB_STATIC

        #if defined ( PT_DEBUG )
            #define _PT_DEBUG_
            #define PT_DEBUG_ENABLED
        #endif // PT_DEBUG

        // Define the suffix used for the static version of the library file
        #ifdef PT_LIB_STATIC
            #define PT_SUFFIX_STATIC "_s"
        #else // PT_LIB_STATIC
            #define PT_SUFFIX_STATIC
        #endif // PT_LIB_STATIC
        
        // Define the suffix used for the debug version of the library file
        #if defined( _PT_DEBUG_ ) && defined( PT_DEBUG_ENABLED )
            #define PT_SUFFIX_DEBUG "_Dbg"
        #else // defined( _PT_DEBUG_ ) && defined( PT_DEBUG_ENABLED )
            #define PT_SUFFIX_DEBUG
        #endif // defined( _PT_DEBUG_ ) && defined( PT_DEBUG_ENABLED )

        // Define the suffix used for the 64-bit version of the library file
        #if defined( _PT_64_ ) || defined( _WIN64 )
            #define PT_SUFFIX_64 "64"
        #else
            #define PT_SUFFIX_64
        #endif

        // Ask the compiler to link the required version of the library
        #pragma comment( lib, "PtUtilsLib" PT_SUFFIX_STATIC PT_SUFFIX_64 PT_SUFFIX_DEBUG ".lib" )

        // Undefined the symbols used to link the required library file
        #undef PT_SUFFIX_STATIC
        #undef PT_SUFFIX_DEBUG
        #undef PT_SUFFIX_64

	#endif // PT_UTILSLIB_EXPORTS

#endif // PT_UTILS_LIB_API

#endif
