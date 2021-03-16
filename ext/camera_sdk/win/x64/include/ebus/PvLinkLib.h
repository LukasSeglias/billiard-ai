// *****************************************************************************
//
//     Copyright (c) 2017, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#if defined( PV_DEBUG )
    #define _PT_DEBUG_
    #define PT_DEBUG_ENABLED
#endif

#ifdef PT_LIB_STATIC
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

#if !defined( PV_NO_IMPORT_LIBS )
    #pragma comment( lib, PT_LIB_NAME PT_SUFFIX_STATIC PT_SUFFIX_64 PT_SUFFIX_DEBUG ".lib" )
#endif

#undef PT_SUFFIX_STATIC
#undef PT_SUFFIX_DEBUG
#undef PT_SUFFIX_64
#undef PT_LIB_NAME
