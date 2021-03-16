// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVSYSTEMENUMS_H__
#define __PVSYSTEMENUMS_H__

typedef enum
{
    PvDeviceClassUnknown = 0,
    PvDeviceClassTransmitter,
    PvDeviceClassReceiver,
    PvDeviceClassTransceiver,
    PvDeviceClassPeripheral,

} PvDeviceClass;


typedef enum
{
    PvUSBStatusNotInitialized = -1,
    PvUSBStatusConnected = 0,
    PvUSBStatusFailedEnumeration,
    PvUSBStatusGeneralFailure,
    PvUSBStatusCausedOvercurrent,
    PvUSBStatusNotEnoughPower,
    PvUSBStatusNotEnoughBandwidth,
    PvUSBStatusHubNestedTooDeeply,
    PvUSBStatusInLegacyHub,
    PvUSBStatusEnumerating,
    PvUSBStatusReset,

} PvUSBStatus;


typedef enum
{
    PvInterfaceTypeUnknown = -1,
    PvInterfaceTypeUSBHostController = 0,
    PvInterfaceTypeNetworkAdapter = 1

} PvInterfaceType;


typedef enum
{
    PvDeviceInfoTypeUnknown = -1,
    PvDeviceInfoTypeGEV = 0,
    PvDeviceInfoTypePleoraProtocol,
    PvDeviceInfoTypeUSB,
    PvDeviceInfoTypeU3V

} PvDeviceInfoType;


typedef enum
{
    PvUSBSpeedUnsupported = -1,
    PvUSBSpeedUnknown = 0,
    PvUSBSpeedLow,
    PvUSBSpeedFull,
    PvUSBSpeedHigh,
    PvUSBSpeedSuper

} PvUSBSpeed;


typedef enum
{
    PvDeviceTypeUnknown = -1,
    PvDeviceTypeGEV = 0,
    PvDeviceTypeU3V = 1

} PvDeviceType;


#endif
