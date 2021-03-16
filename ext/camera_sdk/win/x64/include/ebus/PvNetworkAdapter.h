// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVNETWORKADAPTER_H__
#define __PVNETWORKADAPTER_H__

#include "PvSystemLib.h"
#include "PvInterface.h"
#include "PvDeviceInfoGEV.h"
#include "PvDeviceInfoPleoraProtocol.h"

#include <vector>


#ifndef PV_GENERATING_DOXYGEN_DOC

// Provided for in-object browsing
namespace PvSystemLib
{
    struct IPConfig
    {
        std::string mIPAddress;
        std::string mSubnetMask;
    };

    typedef std::vector<IPConfig> IPConfigVector;
    typedef std::vector<std::string> GatewayVector;
}

#endif // PV_GENERATING_DOXYGEN_DOC


class PV_SYSTEM_API PvNetworkAdapter : public PvInterface
{
public:

    virtual ~PvNetworkAdapter();

    PvString GetMACAddress() const;
    PvString GetDescription() const;
    
    uint32_t GetIPAddressCount() const;
    PvString GetIPAddress( uint32_t aIndex ) const;
    PvString GetSubnetMask( uint32_t aIndex ) const;
    PvString GetDefaultGateway() const;

    bool IsPleoraDriverInstalled() const;

protected:

#ifndef PV_GENERATING_DOXYGEN_DOC

    PvNetworkAdapter( PvSystemLib::IFinderReporter *aFinderReporter );
	PvNetworkAdapter&operator=( const PvNetworkAdapter &aFrom );

    void Init();

    PvSystemLib::IPConfigVector *GetIPConfigs() { return mIPConfigs; }
    const PvSystemLib::IPConfigVector *GetIPConfigs() const { return mIPConfigs; } 

    void SetMAC( const std::string &aValue ) { *mMAC = aValue; }
    void SetDescription( const std::string &aValue ) { *mDescription = aValue; }
    void SetGateway( const std::string &aValue ) { *mGateway = aValue; }

    void SetDriverInstalled( bool aValue ) { mDriverInstalled = aValue; }

#endif // PV_GENERATING_DOXYGEN_DOC

private:

	 // Not implemented
    PvNetworkAdapter();
	PvNetworkAdapter( const PvNetworkAdapter & );

    std::string *mMAC;
    std::string *mDescription;
    std::string *mGateway;

    PvSystemLib::IPConfigVector *mIPConfigs;

    bool mDriverInstalled;

};

#endif
