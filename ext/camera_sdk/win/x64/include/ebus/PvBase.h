// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVBASE_H__
#define __PVBASE_H__

#include "PvBaseLib.h"
#include "PvResult.h"

#ifdef WIN32
PV_BASE_API PvResult PvBinariesLocation( PvString &aLocation );
#endif

PV_BASE_API void PvExtensionActivate( const PvString &aName );
PV_BASE_API void PvExtensionDisable( const PvString &aName );
PV_BASE_API bool PvExtensionQuery( const PvString &aName );

#endif
