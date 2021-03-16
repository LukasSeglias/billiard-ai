// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PV_DEVICEINFO_H__
#define __PV_DEVICEINFO_H__

#include "PvSystemLib.h"
#include "PvSystemEnums.h"


class PvInterface;
class PvDevice;


class PV_SYSTEM_API PvDeviceInfo
{
public:

	virtual ~PvDeviceInfo();
    PvDeviceInfo *Copy() const;

    PvDeviceInfoType GetType() const;

    PvString GetVendorName() const;
    PvString GetModelName() const;
    PvString GetVersion() const;
    PvString GetManufacturerInfo() const;
    PvString GetSerialNumber() const;
    PvString GetUserDefinedName() const;
    
    PvString GetLicenseMessage() const;
    PvString GetDisplayID() const;
    PvString GetUniqueID() const;
    PvString GetConnectionID() const;
    
    const PvInterface *GetInterface() const;

    bool IsConfigurationValid() const;
    bool IsLicenseValid() const;

    PvDeviceClass GetClass() const;

protected:

#ifndef PV_GENERATING_DOXYGEN_DOC

	PvDeviceInfo( PvDeviceInfoType, PvInterface *aInterface );
	const PvDeviceInfo &operator=( const PvDeviceInfo &aFrom );

    void Init();

    void SetLicenseValid( bool aValue ) { mLicenseValid = aValue; }
    void SetClass( PvDeviceClass aValue ) { mClass = aValue; }
    void SetConfigurationValid( bool aValue ) { mConfigurationValid = aValue; }

    void SetVendorName( const std::string &aValue ) { *mVendorName = aValue; }
    void SetModelName( const std::string &aValue ) { *mModelName = aValue; }
    void SetVersion( const std::string &aValue ) { *mVersion = aValue; }
    void SetManufacturerInfo( const std::string &aValue ) { *mManufacturerInfo = aValue; }
    void SetSerialNumber( const std::string &aValue ) { *mSerialNumber = aValue; }
    void SetUserDefinedName( const std::string &aValue ) { *mUserDefinedName = aValue; }

    void SetConnectionID( const std::string &aValue ) { *mConnectionID = aValue; }
    void SetDisplayID( const std::string &aValue ) { *mDisplayID = aValue; }
    void SetUniqueID( const std::string &aValue ) { *mUniqueID = aValue; }
    void SetCompareID( const std::string &aValue ) { *mCompareID = aValue; }
    void SetLicenseMessage( const std::string &aValue ) { *mLicenseMessage = aValue; }

    std::string *GetCompareID() { return mCompareID; }

#endif // PV_GENERATING_DOXYGEN_DOC

private:

    bool mLicenseValid;
    bool mConfigurationValid;

    const PvInterface *mInterface;

    std::string *mVendorName;
    std::string *mModelName;
    std::string *mVersion;
    std::string *mManufacturerInfo;
    std::string *mSerialNumber;
    std::string *mUserDefinedName;
    
    std::string *mConnectionID;
    std::string *mDisplayID;
    std::string *mUniqueID;
    std::string *mCompareID;
    std::string *mLicenseMessage;

    PvDeviceInfoType mType;      
    PvDeviceClass mClass;

	 // Not implemented
    PvDeviceInfo();
    PvDeviceInfo( const PvDeviceInfo & );

};


#endif // __PV_DEVICEINFO_H__

