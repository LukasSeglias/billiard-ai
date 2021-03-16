// *****************************************************************************
//
//     Copyright (c) 2008, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVPIXELTYPE_H__
#define __PVPIXELTYPE_H__

#include "PvBufferLib.h"


//
// Color
//

#define PVPIXELMONO                  ( 0x01000000 )
#define PVPIXELRGB                   ( 0x02000000 ) // Pre GEV 1.1, kept for bw compatibility
#define PVPIXELCOLOR                 ( 0x02000000 ) // GEV 1.1
#define PVPIXELCUSTOM                ( 0x80000000 )
#define PVPIXELCOLORMASK             ( 0xFF000000 )

//
// Effective number of bits per pixel (including padding)
//

#define PVPIXEL1BIT                  ( 0x00010000 )
#define PVPIXEL2BIT                  ( 0x00020000 )
#define PVPIXEL4BIT                  ( 0x00040000 )
#define PVPIXEL8BIT                  ( 0x00080000 )
#define PVPIXEL10BIT                 ( 0x000A0000 )
#define PVPIXEL12BIT                 ( 0x000C0000 )
#define PVPIXEL16BIT                 ( 0x00100000 )
#define PVPIXEL20BIT                 ( 0x00140000 )
#define PVPIXEL24BIT                 ( 0x00180000 )
#define PVPIXEL30BIT                 ( 0x001E0000 )
#define PVPIXEL32BIT                 ( 0x00200000 )
#define PVPIXEL36BIT                 ( 0x00240000 )
#define PVPIXEL40BIT                 ( 0x00280000 )
#define PVPIXEL48BIT                 ( 0x00300000 )
#define PVPIXEL64BIT                 ( 0x00400000 )
#define PVBITSPERPIXELMASK           ( 0x00FF0000 )

//
// Pixel type ID
//

#define PVPIXELIDMASK                ( 0x0000FFFF )


typedef enum 
{

    PvPixelUndefined =                  ( 0 ),
    PvPixelMono8 =                      ( PVPIXELMONO  | PVPIXEL8BIT   | 0x0001 ),
    PvPixelMono8s =                     ( PVPIXELMONO  | PVPIXEL8BIT   | 0x0002 ),
    PvPixelMono10 =                     ( PVPIXELMONO  | PVPIXEL16BIT  | 0x0003 ),
    PvPixelMono10Packed =               ( PVPIXELMONO  | PVPIXEL12BIT  | 0x0004 ),
    PvPixelMono12 =                     ( PVPIXELMONO  | PVPIXEL16BIT  | 0x0005 ),
    PvPixelMono12Packed =               ( PVPIXELMONO  | PVPIXEL12BIT  | 0x0006 ),
    PvPixelMono16 =                     ( PVPIXELMONO  | PVPIXEL16BIT  | 0x0007 ),
    PvPixelBayerGR8 =                   ( PVPIXELMONO  | PVPIXEL8BIT   | 0x0008 ),
    PvPixelBayerRG8 =                   ( PVPIXELMONO  | PVPIXEL8BIT   | 0x0009 ),
    PvPixelBayerGB8 =                   ( PVPIXELMONO  | PVPIXEL8BIT   | 0x000A ),
    PvPixelBayerBG8 =                   ( PVPIXELMONO  | PVPIXEL8BIT   | 0x000B ),
    PvPixelBayerGR10 =                  ( PVPIXELMONO  | PVPIXEL16BIT  | 0x000C ),
    PvPixelBayerRG10 =                  ( PVPIXELMONO  | PVPIXEL16BIT  | 0x000D ),
    PvPixelBayerGB10 =                  ( PVPIXELMONO  | PVPIXEL16BIT  | 0x000E ),
    PvPixelBayerBG10 =                  ( PVPIXELMONO  | PVPIXEL16BIT  | 0x000F ),
    PvPixelBayerGR12 =                  ( PVPIXELMONO  | PVPIXEL16BIT  | 0x0010 ),
    PvPixelBayerRG12 =                  ( PVPIXELMONO  | PVPIXEL16BIT  | 0x0011 ),
    PvPixelBayerGB12 =                  ( PVPIXELMONO  | PVPIXEL16BIT  | 0x0012 ),
    PvPixelBayerBG12 =                  ( PVPIXELMONO  | PVPIXEL16BIT  | 0x0013 ),
    PvPixelRGB8 =                       ( PVPIXELCOLOR | PVPIXEL24BIT  | 0x0014 ), 
    PvPixelBGR8 =                       ( PVPIXELCOLOR | PVPIXEL24BIT  | 0x0015 ), 
    PvPixelRGBa8 =                      ( PVPIXELCOLOR | PVPIXEL32BIT  | 0x0016 ), 
    PvPixelBGRa8 =                      ( PVPIXELCOLOR | PVPIXEL32BIT  | 0x0017 ), 
    PvPixelRGB10 =                      ( PVPIXELCOLOR | PVPIXEL48BIT  | 0x0018 ), 
    PvPixelBGR10 =                      ( PVPIXELCOLOR | PVPIXEL48BIT  | 0x0019 ), 
    PvPixelRGB12 =                      ( PVPIXELCOLOR | PVPIXEL48BIT  | 0x001A ), 
    PvPixelBGR12 =                      ( PVPIXELCOLOR | PVPIXEL48BIT  | 0x001B ), 
    PvPixelRGB10V1Packed =              ( PVPIXELCOLOR | PVPIXEL32BIT  | 0x001C ), 
    PvPixelRGB10p32 =                   ( PVPIXELCOLOR | PVPIXEL32BIT  | 0x001D ), 
    PvPixelYUV411_8_UYYVYY =            ( PVPIXELCOLOR | PVPIXEL12BIT  | 0x001E ), 
    PvPixelYUV422_8_UYVY =              ( PVPIXELCOLOR | PVPIXEL16BIT  | 0x001F ), 
    PvPixelYUV8_UYV =                   ( PVPIXELCOLOR | PVPIXEL24BIT  | 0x0020 ), 
    PvPixelRGB8_Planar =                ( PVPIXELCOLOR | PVPIXEL24BIT  | 0x0021 ), 
    PvPixelRGB10_Planar =               ( PVPIXELCOLOR | PVPIXEL48BIT  | 0x0022 ), 
    PvPixelRGB12_Planar =               ( PVPIXELCOLOR | PVPIXEL48BIT  | 0x0023 ), 
    PvPixelRGB16_Planar =               ( PVPIXELCOLOR | PVPIXEL48BIT  | 0x0024 ), 
    PvPixelMono14 =                     ( PVPIXELMONO  | PVPIXEL16BIT  | 0x0025 ),
    PvPixelBayerGR10Packed =            ( PVPIXELMONO  | PVPIXEL12BIT  | 0x0026 ), 
    PvPixelBayerRG10Packed =            ( PVPIXELMONO  | PVPIXEL12BIT  | 0x0027 ), 
    PvPixelBayerGB10Packed =            ( PVPIXELMONO  | PVPIXEL12BIT  | 0x0028 ), 
    PvPixelBayerBG10Packed =            ( PVPIXELMONO  | PVPIXEL12BIT  | 0x0029 ), 
    PvPixelBayerGR12Packed =            ( PVPIXELMONO  | PVPIXEL12BIT  | 0x002A ), 
    PvPixelBayerRG12Packed =            ( PVPIXELMONO  | PVPIXEL12BIT  | 0x002B ), 
    PvPixelBayerGB12Packed =            ( PVPIXELMONO  | PVPIXEL12BIT  | 0x002C ), 
    PvPixelBayerBG12Packed =            ( PVPIXELMONO  | PVPIXEL12BIT  | 0x002D ), 
    PvPixelBayerGR16 =                  ( PVPIXELMONO  | PVPIXEL16BIT  | 0x002E ), 
    PvPixelBayerRG16 =                  ( PVPIXELMONO  | PVPIXEL16BIT  | 0x002F ), 
    PvPixelBayerGB16 =                  ( PVPIXELMONO  | PVPIXEL16BIT  | 0x0030 ), 
    PvPixelBayerBG16 =                  ( PVPIXELMONO  | PVPIXEL16BIT  | 0x0031 ), 
    PvPixelYUV422_8 =                   ( PVPIXELCOLOR | PVPIXEL16BIT  | 0x0032 ), 
    PvPixelRGB16 =                      ( PVPIXELCOLOR | PVPIXEL48BIT  | 0x0033 ), 
    PvPixelRGB12V1Packed =              ( PVPIXELCOLOR | PVPIXEL36BIT  | 0x0034 ), 
    PvPixelRGB565p =                    ( PVPIXELCOLOR | PVPIXEL16BIT  | 0x0035 ), 
    PvPixelBGR565p =                    ( PVPIXELCOLOR | PVPIXEL16BIT  | 0x0036 ), 
    PvPixelMono1p =                     ( PVPIXELMONO  | PVPIXEL1BIT   | 0x0037 ), 
    PvPixelMono2p =                     ( PVPIXELMONO  | PVPIXEL2BIT   | 0x0038 ), 
    PvPixelMono4p =                     ( PVPIXELMONO  | PVPIXEL4BIT   | 0x0039 ), 
    PvPixelYCbCr8_CbYCr =               ( PVPIXELCOLOR | PVPIXEL24BIT  | 0x003A ), 
    PvPixelYCbCr422_8 =                 ( PVPIXELCOLOR | PVPIXEL16BIT  | 0x003B ), 
    PvPixelYCbCr411_8_CbYYCrYY =        ( PVPIXELCOLOR | PVPIXEL12BIT  | 0x003C ), 
    PvPixelYCbCr601_8_CbYCr =           ( PVPIXELCOLOR | PVPIXEL24BIT  | 0x003D ), 
    PvPixelYCbCr601_422_8 =             ( PVPIXELCOLOR | PVPIXEL16BIT  | 0x003E ), 
    PvPixelYCbCr601_411_8_CbYYCrYY =    ( PVPIXELCOLOR | PVPIXEL12BIT  | 0x003F ), 
    PvPixelYCbCr709_8_CbYCr =           ( PVPIXELCOLOR | PVPIXEL24BIT  | 0x0040 ), 
    PvPixelYCbCr709_422_8 =             ( PVPIXELCOLOR | PVPIXEL16BIT  | 0x0041 ), 
    PvPixelYCbCr709_411_8_CbYYCrYY =    ( PVPIXELCOLOR | PVPIXEL12BIT  | 0x0042 ), 
    PvPixelYCbCr422_8_CbYCrY =          ( PVPIXELCOLOR | PVPIXEL16BIT  | 0x0043 ), 
    PvPixelYCbCr601_422_8_CbYCrY =      ( PVPIXELCOLOR | PVPIXEL16BIT  | 0x0044 ), 
    PvPixelYCbCr709_422_8_CbYCrY =      ( PVPIXELCOLOR | PVPIXEL16BIT  | 0x0045 ), 
    PvPixelMono10p =                    ( PVPIXELMONO  | PVPIXEL10BIT  | 0x0046 ), 
    PvPixelMono12p =                    ( PVPIXELMONO  | PVPIXEL12BIT  | 0x0047 ), 

    PvPixelBGR10p =                     ( PVPIXELCOLOR | PVPIXEL30BIT  | 0x0048 ),
    PvPixelBGR12p =                     ( PVPIXELCOLOR | PVPIXEL36BIT  | 0x0049 ),
    PvPixelBGR14 =                      ( PVPIXELCOLOR | PVPIXEL48BIT  | 0x004A ),
    PvPixelBGR16 =                      ( PVPIXELCOLOR | PVPIXEL48BIT  | 0x004B ),
    PvPixelBGRa10 =                     ( PVPIXELCOLOR | PVPIXEL64BIT  | 0x004C ),
    PvPixelBGRa10p =                    ( PVPIXELCOLOR | PVPIXEL40BIT  | 0x004D ),
    PvPixelBGRa12 =                     ( PVPIXELCOLOR | PVPIXEL64BIT  | 0x004E ),
    PvPixelBGRa12p =                    ( PVPIXELCOLOR | PVPIXEL48BIT  | 0x004F ),
    PvPixelBGRa14 =                     ( PVPIXELCOLOR | PVPIXEL64BIT  | 0x0050 ),
    PvPixelBGRa16 =                     ( PVPIXELCOLOR | PVPIXEL64BIT  | 0x0051 ),
    PvPixelRGB10p =                     ( PVPIXELCOLOR | PVPIXEL30BIT  | 0x0052 ),
    
    PvPixelBayerBG10p =                 ( PVPIXELMONO  | PVPIXEL10BIT  | 0x0052 ), 
    PvPixelBayerBG12p =                 ( PVPIXELMONO  | PVPIXEL12BIT  | 0x0053 ), 
    PvPixelBayerGB10p =                 ( PVPIXELMONO  | PVPIXEL10BIT  | 0x0054 ), 
    PvPixelBayerGB12p =                 ( PVPIXELMONO  | PVPIXEL12BIT  | 0x0055 ), 
    PvPixelBayerGR10p =                 ( PVPIXELMONO  | PVPIXEL10BIT  | 0x0056 ), 
    PvPixelBayerGR12p =                 ( PVPIXELMONO  | PVPIXEL12BIT  | 0x0057 ), 
    PvPixelBayerRG10p =                 ( PVPIXELMONO  | PVPIXEL10BIT  | 0x0058 ), 
    PvPixelBayerRG12p =                 ( PVPIXELMONO  | PVPIXEL12BIT  | 0x0059 ), 

    PvPixelYCbCr411_8 =                 ( PVPIXELCOLOR | PVPIXEL12BIT  | 0x005A ),
    PvPixelYCbCr8 =                     ( PVPIXELCOLOR | PVPIXEL24BIT  | 0x005B ),

    PvPixelRGB12p =                     ( PVPIXELCOLOR | PVPIXEL36BIT  | 0x005C ),
    PvPixelRGB14 =                      ( PVPIXELCOLOR | PVPIXEL48BIT  | 0x005D ),
    PvPixelRGBa10 =                     ( PVPIXELCOLOR | PVPIXEL64BIT  | 0x005E ),
    PvPixelRGBa10p =                    ( PVPIXELCOLOR | PVPIXEL40BIT  | 0x005F ),
    PvPixelRGBa12 =                     ( PVPIXELCOLOR | PVPIXEL64BIT  | 0x0060 ),
    PvPixelRGBa12p =                    ( PVPIXELCOLOR | PVPIXEL48BIT  | 0x0061 ),
    PvPixelRGBa14 =                     ( PVPIXELCOLOR | PVPIXEL64BIT  | 0x0062 ),
    PvPixelRGBa16 =                     ( PVPIXELCOLOR | PVPIXEL64BIT  | 0x0063 ),

    PvPixelYCbCr422_10 =                ( PVPIXELCOLOR | PVPIXEL32BIT  | 0x0065 ),
    PvPixelYCbCr422_12 =                ( PVPIXELCOLOR | PVPIXEL32BIT  | 0x0066 ),

    PvPixelSCF1WBWG8 =                  ( PVPIXELMONO | PVPIXEL8BIT  | 0x0067 ),
    PvPixelSCF1WBWG10 =                 ( PVPIXELMONO | PVPIXEL16BIT | 0x0068 ),
    PvPixelSCF1WBWG10p =                ( PVPIXELMONO | PVPIXEL10BIT | 0x0069 ),
    PvPixelSCF1WBWG12 =                 ( PVPIXELMONO | PVPIXEL16BIT | 0x006A ),
    PvPixelSCF1WBWG12p =                ( PVPIXELMONO | PVPIXEL12BIT | 0x006B ),
    PvPixelSCF1WBWG14 =                 ( PVPIXELMONO | PVPIXEL16BIT | 0x006C ),
    PvPixelSCF1WBWG16 =                 ( PVPIXELMONO | PVPIXEL16BIT | 0x006D ),
    PvPixelSCF1WGWB8 =                  ( PVPIXELMONO | PVPIXEL8BIT  | 0x006E ),
    PvPixelSCF1WGWB10 =                 ( PVPIXELMONO | PVPIXEL16BIT | 0x006F ),
    PvPixelSCF1WGWB10p =                ( PVPIXELMONO | PVPIXEL10BIT | 0x0070 ),
    PvPixelSCF1WGWB12 =                 ( PVPIXELMONO | PVPIXEL16BIT | 0x0071 ),
    PvPixelSCF1WGWB12p =                ( PVPIXELMONO | PVPIXEL12BIT | 0x0072 ),
    PvPixelSCF1WGWB14 =                 ( PVPIXELMONO | PVPIXEL16BIT | 0x0073 ),
    PvPixelSCF1WGWB16 =                 ( PVPIXELMONO | PVPIXEL16BIT | 0x0074 ),
    PvPixelSCF1WGWR8 =                  ( PVPIXELMONO | PVPIXEL8BIT  | 0x0075 ),
    PvPixelSCF1WGWR10 =                 ( PVPIXELMONO | PVPIXEL16BIT | 0x0076 ),
    PvPixelSCF1WGWR10p =                ( PVPIXELMONO | PVPIXEL10BIT | 0x0077 ),
    PvPixelSCF1WGWR12 =                 ( PVPIXELMONO | PVPIXEL16BIT | 0x0078 ),
    PvPixelSCF1WGWR12p =                ( PVPIXELMONO | PVPIXEL12BIT | 0x0079 ),
    PvPixelSCF1WGWR14 =                 ( PVPIXELMONO | PVPIXEL16BIT | 0x007A ),
    PvPixelSCF1WGWR16 =                 ( PVPIXELMONO | PVPIXEL16BIT | 0x007B ),
    PvPixelSCF1WRWG8 =                  ( PVPIXELMONO | PVPIXEL8BIT  | 0x007C ),
    PvPixelSCF1WRWG10 =                 ( PVPIXELMONO | PVPIXEL16BIT | 0x007D ),
    PvPixelSCF1WRWG10p =                ( PVPIXELMONO | PVPIXEL10BIT | 0x007E ),
    PvPixelSCF1WRWG12 =                 ( PVPIXELMONO | PVPIXEL16BIT | 0x007F ),
    PvPixelSCF1WRWG12p =                ( PVPIXELMONO | PVPIXEL12BIT | 0x0080 ),
    PvPixelSCF1WRWG14 =                 ( PVPIXELMONO | PVPIXEL16BIT | 0x0081 ),
    PvPixelSCF1WRWG16 =                 ( PVPIXELMONO | PVPIXEL16BIT | 0x0082 ),

    PvPixelYCbCr10_CbYCr =              ( PVPIXELCOLOR | PVPIXEL48BIT | 0x0083 ) ,
    PvPixelYCbCr10p_CbYCr =             ( PVPIXELCOLOR | PVPIXEL30BIT | 0x0084 ),
    PvPixelYCbCr12_CbYCr =              ( PVPIXELCOLOR | PVPIXEL48BIT | 0x0085 ),
    PvPixelYCbCr12p_CbYCr =             ( PVPIXELCOLOR | PVPIXEL36BIT | 0x0086 ),
    PvPixelYCbCr422_10p =               ( PVPIXELCOLOR | PVPIXEL20BIT | 0x0087 ),
    PvPixelYCbCr422_12p =               ( PVPIXELCOLOR | PVPIXEL24BIT | 0x0088 ),
    PvPixelYCbCr601_10_CbYCr =          ( PVPIXELCOLOR | PVPIXEL48BIT | 0x0089 ),
    PvPixelYCbCr601_10p_CbYCr =         ( PVPIXELCOLOR | PVPIXEL30BIT | 0x008A ),
    PvPixelYCbCr601_12_CbYCr =          ( PVPIXELCOLOR | PVPIXEL48BIT | 0x008B ),
    PvPixelYCbCr601_12p_CbYCr =         ( PVPIXELCOLOR | PVPIXEL36BIT | 0x008C ),
    PvPixelYCbCr601_422_10 =            ( PVPIXELCOLOR | PVPIXEL32BIT | 0x008D ),
    PvPixelYCbCr601_422_10p =           ( PVPIXELCOLOR | PVPIXEL20BIT | 0x008E ),
    PvPixelYCbCr601_422_12 =            ( PVPIXELCOLOR | PVPIXEL32BIT | 0x008F ),
    PvPixelYCbCr601_422_12p =           ( PVPIXELCOLOR | PVPIXEL24BIT | 0x0090 ),
    PvPixelYCbCr709_10_CbYCr =          ( PVPIXELCOLOR | PVPIXEL48BIT | 0x0091 ),
    PvPixelYCbCr709_10p_CbYCr =         ( PVPIXELCOLOR | PVPIXEL30BIT | 0x0092 ),
    PvPixelYCbCr709_12_CbYCr =          ( PVPIXELCOLOR | PVPIXEL48BIT | 0x0093 ),
    PvPixelYCbCr709_12p_CbYCr =         ( PVPIXELCOLOR | PVPIXEL36BIT | 0x0094 ),
    PvPixelYCbCr709_422_10 =            ( PVPIXELCOLOR | PVPIXEL32BIT | 0x0095 ),
    PvPixelYCbCr709_422_10p =           ( PVPIXELCOLOR | PVPIXEL20BIT | 0x0096 ),
    PvPixelYCbCr709_422_12 =            ( PVPIXELCOLOR | PVPIXEL32BIT | 0x0097 ),
    PvPixelYCbCr709_422_12p =           ( PVPIXELCOLOR | PVPIXEL24BIT | 0x0098 ),
    PvPixelYCbCr422_10_CbYCrY =         ( PVPIXELCOLOR | PVPIXEL32BIT | 0x0099 ),
    PvPixelYCbCr422_10p_CbYCrY =        ( PVPIXELCOLOR | PVPIXEL20BIT | 0x009A ),
    PvPixelYCbCr422_12_CbYCrY =         ( PVPIXELCOLOR | PVPIXEL32BIT | 0x009B ),
    PvPixelYCbCr422_12p_CbYCrY =        ( PVPIXELCOLOR | PVPIXEL24BIT | 0x009C ),
    PvPixelYCbCr601_422_10_CbYCrY =     ( PVPIXELCOLOR | PVPIXEL32BIT | 0x009D ),
    PvPixelYCbCr601_422_10p_CbYCrY =    ( PVPIXELCOLOR | PVPIXEL20BIT | 0x009E ),
    PvPixelYCbCr601_422_12_CbYCrY =     ( PVPIXELCOLOR | PVPIXEL32BIT | 0x009F ),
    PvPixelYCbCr601_422_12p_CbYCrY =    ( PVPIXELCOLOR | PVPIXEL24BIT | 0x00A0 ),
    PvPixelYCbCr709_422_10_CbYCrY =     ( PVPIXELCOLOR | PVPIXEL32BIT | 0x00A1 ),
    PvPixelYCbCr709_422_10p_CbYCrY =    ( PVPIXELCOLOR | PVPIXEL20BIT | 0x00A2 ),
    PvPixelYCbCr709_422_12_CbYCrY =     ( PVPIXELCOLOR | PVPIXEL32BIT | 0x00A3 ),
    PvPixelYCbCr709_422_12p_CbYCrY =    ( PVPIXELCOLOR | PVPIXEL24BIT | 0x00A4 ),

} PvPixelType;

// Mapping to Windows pixel types (MFC, .NET, DirectX, Windows Bitmap, etc.)
#define PV_PIXEL_WIN_RGB32 ( PvPixelBGRa8 )
#define PV_PIXEL_WIN_RGB24 ( PvPixelBGR8 )
#define PV_PIXEL_WIN_RGB16 ( PvPixelRGB565p )

// Mapping to Qt pixel types
#define PV_PIXEL_QT_RGB32 ( PvPixelBGRa8 )
#define PV_PIXEL_QT_RGB888 ( PvPixelBGR8 )
#define PV_PIXEL_QT_RGB565 ( PvPixelRGB565p )

// Mapping to OpenGL
#define PV_PIXEL_OPENGL_RGB32 ( PvPixelRGBa8 )
#define PV_PIXEL_OPENGL_RGB24 ( PvPixelRGB8 )
#define PV_PIXEL_OPENGL_BGR32 ( PvPixelBGRa8 )
#define PV_PIXEL_OPENGL_BGR24 ( PvPixelBGR8 )

// Pre GEV 2.0 pixel types
#ifndef PV_NO_GEV1X_PIXEL_TYPES
    #define PvPixelMono8Signed ( PvPixelMono8s )
    #define PvPixelRGB8Packed ( PvPixelRGB8 )
    #define PvPixelBGR8Packed ( PvPixelBGR8 )
    #define PvPixelRGBA8Packed ( PvPixelRGBa8 )
    #define PvPixelBGRA8Packed ( PvPixelBGRa8 )
    #define PvPixelRGB10Packed ( PvPixelRGB10 )
    #define PvPixelBGR10Packed ( PvPixelBGR10 )
    #define PvPixelRGB12Packed ( PvPixelRGB12 )
    #define PvPixelBGR12Packed ( PvPixelBGR12 )
    #define PvPixelRGB16Packed ( PvPixelRGB16 )
    #define PvPixelBGR10V1Packed ( PvPixelRGB10V1Packed )
    #define PvPixelBGR10V2Packed ( PvPixelRGB10p32 )
    #define PvPixelYUV411Packed ( PvPixelYUV411_8_UYYVYY )
    #define PvPixelYUV422Packed ( PvPixelYUV422_8_UYVY )
    #define PvPixelYUV422YUYVPacked ( PvPixelYUV422_8 )
    #define PvPixelYUV444Packed ( PvPixelYUV8_UYV )
    #define PvPixelRGB8Planar ( PvPixelRGB8_Planar )
    #define PvPixelRGB10Planar ( PvPixelRGB10_Planar )
    #define PvPixelRGB12Planar ( PvPixelRGB12_Planar )
    #define PvPixelRGB16Planar ( PvPixelRGB16_Planar )
#endif // PV_NO_GEV1X_PIXEL_TYPES
    
// Deprecated pixel types, for backward compatibility
#ifndef PV_NO_DEPRECATED_PIXEL_TYPES
    #define PvPixelWinRGB16 ( PvPixelRGB565p )
    #define PvPixelWinRGB32 ( PvPixelBGRa8 )
    #define PvPixelWinRGB24 ( PvPixelBGR8 )
    #define PvPixelWinBGR32 ( PvPixelRGBa8 )
    #define PvPixelWinBGR24 ( PvPixelRGB8 )
#endif // PV_NO_DEPRECATED_PIXEL_TYPES

PV_BUFFER_API uint32_t PvGetPixelBitCount( PvPixelType aType );

#endif
