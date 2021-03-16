// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVDEVICEENUMS_H__
#define __PVDEVICEENUMS_H__


typedef enum
{
    PvAccessUnknown = 0,
    PvAccessOpen,
    PvAccessControl,
    PvAccessExclusive,
    PvAccessReadOnly,

} PvAccessType;


typedef enum
{
    PvActionAckStatusOK = 0,
    PvActionAckStatusLate = 1,
    PvActionAckStatusOverflow = 2,
    PvActionAckStatusNoRefTime = 3,

} PvActionAckStatusEnum;


#endif
