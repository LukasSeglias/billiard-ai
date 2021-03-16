// *****************************************************************************
//
//     Copyright (c) 2015, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVTAPGEOMETRY_H__
#define __PVTAPGEOMETRY_H__


typedef enum
{
    PvTapGeometryUnknown = -1,

    // Area scan
    PvTapGeometryAS_2X_1Y = 0,
    PvTapGeometryAS_2XE_1Y,
    PvTapGeometryAS_2XM_1Y,
    PvTapGeometryAS_1X_1Y2,
    PvTapGeometryAS_1X_2YE,
    PvTapGeometryAS_4X_1Y,
    PvTapGeometryAS_2X2_1Y,
    PvTapGeometryAS_2X2M_1Y,
    PvTapGeometryAS_1X2_2YE,
    PvTapGeometryAS_1X2_1Y2,
    PvTapGeometryAS_2X_2YE,
    PvTapGeometryAS_2X_1Y2,
    PvTapGeometryAS_2X2E_1Y,
    PvTapGeometryAS_2XE_2YE,
    PvTapGeometryAS_2XE_1Y2,
    PvTapGeometryAS_2XM_2YE,
    PvTapGeometryAS_2XM_1Y2,
    PvTapGeometryAS_8X_1Y,
    PvTapGeometryAS_4X2_1Y,
    PvTapGeometryAS_4X2E_1Y,
    PvTapGeometryAS_2X2E_2YE,

    // Linescan
    PvTapGeometryLS_2X,
    PvTapGeometryLS_2XE,
    PvTapGeometryLS_2XM,
    PvTapGeometryLS_4X,
    PvTapGeometryLS_2X2,
    PvTapGeometryLS_2X2E,
    PvTapGeometryLS_2X2M,
    PvTapGeometryLS_8X,
    PvTapGeometryLS_4X2,
    PvTapGeometryLS_4X2E,

    PvTapGeometryLast,

} PvTapGeometryEnum;


#endif
