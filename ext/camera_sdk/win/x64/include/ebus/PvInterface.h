// *****************************************************************************
//
//     Copyright (c) 2008, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVINTERFACE_H__
#define __PVINTERFACE_H__

#include "PvSystemLib.h"
#include "PvDeviceInfo.h"


namespace PvSystemLib
{
    class System;
    class NetworkAdapter;
    class USBHostController;

    class DeviceInfoVector;
    class IFinderReporter;

}


class PV_SYSTEM_API PvInterface
{
public:

    virtual ~PvInterface();

    PvInterfaceType GetType() const;

    PvString GetName() const;
    PvString GetDisplayID() const;
    PvString GetUniqueID() const;
    PvString GetCompareID() const;
    
    uint32_t GetDeviceCount() const;
    const PvDeviceInfo *GetDeviceInfo( uint32_t aIndex ) const;

    static bool Compare( const PvInterface *a1, const PvInterface *a2 );

protected:

#ifndef PV_GENERATING_DOXYGEN_DOC

    PvInterface( PvSystemLib::IFinderReporter *aFinderReporter, PvInterfaceType aType );
	PvInterface&operator=( const PvInterface &aFrom );

    void SetName( const std::string &aValue ) { *mName = aValue; }
    void SetDisplayID( const std::string &aValue ) { *mDisplayID = aValue; }
    void SetUniqueID( const std::string &aValue ) { *mUniqueID = aValue; }
    void SetCompareID( const std::string &aValue ) { *mCompareID = aValue; }

    void SetConfigurationValid( bool aValue ) { mConfigurationValid = aValue; }

    PvSystemLib::DeviceInfoVector *GetDevices() { return mDevices; }

#endif // PV_GENERATING_DOXYGEN_DOC

    friend class PvSystemLib::System;
    friend class PvSystemLib::NetworkAdapter;
    friend class PvSystemLib::USBHostController;

private:

	 // Not implemented
	PvInterface( const PvInterface & );

    PvInterfaceType mType;

    std::string *mName;
    std::string *mDisplayID;
    std::string *mUniqueID;
    std::string *mCompareID;

    bool mConfigurationValid;

    PvSystemLib::DeviceInfoVector *mDevices;

};

#endif
