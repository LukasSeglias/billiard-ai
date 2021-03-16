// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVDEVICEINFOGEV_H__
#define __PVDEVICEINFOGEV_H__

#include "PvDeviceInfo.h"


class PV_SYSTEM_API PvDeviceInfoGEV : public PvDeviceInfo
{
public:

    PvDeviceInfoGEV();
	virtual ~PvDeviceInfoGEV();

	PvDeviceInfoGEV &operator=( const PvDeviceInfoGEV &aFrom );

    PvString GetMACAddress() const;

    PvString GetIPAddress() const;
    PvString GetDefaultGateway() const;
    PvString GetSubnetMask() const;

    uint32_t GetGEVVersion() const;

protected:

#ifndef PV_GENERATING_DOXYGEN_DOC

	PvDeviceInfoGEV( PvInterface *aInterface );

    void Init();

    void SetIPAddress( const std::string &aValue ) { *mIPAddress = aValue; }
    void SetMACAddress( const std::string &aValue ) { *mMACAddress = aValue; }
    void SetDefaultGateway( const std::string &aValue ) { *mDefaultGateway = aValue; }
    void SetSubnetMask( const std::string &aValue ) { *mSubnetMask = aValue; }

    void SetGEVVersion( uint32_t aValue ) { mGEVVersion = aValue; }

#endif // PV_GENERATING_DOXYGEN_DOC

private:

	 // Not implemented
    PvDeviceInfoGEV( const PvDeviceInfoGEV & );

    std::string *mIPAddress;
    std::string *mMACAddress;
    std::string *mDefaultGateway;
    std::string *mSubnetMask;
    
    uint32_t mGEVVersion;

};

#endif
