// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVDEVICEINFOPLEORAPROTOCOL_H__
#define __PVDEVICEINFOPLEORAPROTOCOL_H__

#include "PvDeviceInfo.h"


class PV_SYSTEM_API PvDeviceInfoPleoraProtocol : public PvDeviceInfo
{
public:

    PvDeviceInfoPleoraProtocol();
	virtual ~PvDeviceInfoPleoraProtocol();

	PvDeviceInfoPleoraProtocol &operator=( const PvDeviceInfoPleoraProtocol & );

    PvString GetMACAddress() const;
    PvString GetIPAddress() const;
    PvString GetSubnetMask() const;
    PvString GetDefaultGateway() const;

    uint8_t GetDeviceID() const;
    uint8_t GetModuleID() const;
    uint8_t GetSubID() const;
    uint8_t GetVendorID() const;
    uint8_t GetSoftwareMajor() const;
    uint8_t GetSoftwareMinor() const;

protected:

#ifndef PV_GENERATING_DOXYGEN_DOC

	PvDeviceInfoPleoraProtocol( PvInterface *aInterface );

    void Init();

    void SetIPAddress( const std::string &aValue ) { *mIPAddress = aValue; }
    void SetMACAddress( const std::string &aValue ) { *mMACAddress = aValue; }
    void SetSubnetMask( const std::string &aValue ) { *mSubnetMask = aValue; }
    void SetDefaultGateway( const std::string &aValue ) { *mDefaultGateway = aValue; }

    void SetDeviceID( uint8_t aValue ) { mDeviceID = aValue; }
    void SetModuleID( uint8_t aValue ) { mModuleID = aValue; }
    void SetSubID( uint8_t aValue ) { mSubID = aValue; }
    void SetVendorID( uint8_t aValue ) { mVendorID = aValue; }
    void SetSoftwareMajor( uint8_t aValue ) { mSoftwareMajor = aValue; }
    void SetSoftwareMinor( uint8_t aValue ) { mSoftwareMinor = aValue; }

#endif // PV_GENERATING_DOXYGEN_DOC

private:

	 // Not implemented
    PvDeviceInfoPleoraProtocol( const PvDeviceInfoPleoraProtocol & );

    std::string *mIPAddress;
    std::string *mMACAddress;
    std::string *mSubnetMask;
    std::string *mDefaultGateway;

    uint8_t mDeviceID;
    uint8_t mModuleID;
    uint8_t mSubID;
    uint8_t mVendorID;
    uint8_t mSoftwareMajor;
    uint8_t mSoftwareMinor;

};

#endif
