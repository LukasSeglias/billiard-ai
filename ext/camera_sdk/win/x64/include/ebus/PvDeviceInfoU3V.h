// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVDEVICEINFOU3V_H__
#define __PVDEVICEINFOU3V_H__

#include "PvDeviceInfoUSB.h"


class PV_SYSTEM_API PvDeviceInfoU3V : public PvDeviceInfoUSB
{
public:

    PvDeviceInfoU3V();
	virtual ~PvDeviceInfoU3V();

    PvDeviceInfoU3V &operator=( const PvDeviceInfoU3V & );

    uint32_t GetGenCPVersion() const;
    uint32_t GetU3VVersion() const;

    PvString GetDeviceGUID() const;
    PvString GetFamilyName() const;
    PvString GetU3VSerialNumber() const;
    PvString GetDevicePath() const;
    PvString GetDeviceKey() const;
    PvString GetSpeedMessage() const;
    PvString GetPowerMessage() const;

	bool IsLowSpeedSupported() const;
    bool IsFullSpeedSupported() const;
    bool IsHighSpeedSupported() const;
    bool IsSuperSpeedSupported() const;
    bool IsCurrentSpeedSupported() const;

    PvUSBSpeed GetSpeed() const;

    uint32_t GetMaxPower() const;
    uint32_t GetMaxPacketSize() const;

    bool IsPleoraDriverInstalled() const;
    bool IsInitializedCapabilities() const;

protected:

#ifndef PV_GENERATING_DOXYGEN_DOC

	PvDeviceInfoU3V( PvInterface *aInterface );

    void Init();

    void SetGenCPVersion( uint32_t aValue ) { mGenCPVersion = aValue; }
    void SetU3VVersion( uint32_t aValue ) { mU3VVersion = aValue; }

    void SetDeviceGUID( const std::string &aValue ) { *mDeviceGUID = aValue; }
    void SetFamilyName( const std::string &aValue ) { *mFamilyName = aValue; }
    void SetU3VSerialNumber( const std::string &aValue ) { *mU3VSerialNumber = aValue; }
    void SetDevicePath( const std::string &aValue ) { *mDevicePath = aValue; }
    void SetDeviceKey( const std::string &aValue ) { *mDeviceKey = aValue; }
    void SetSpeedMessage( const std::string &aValue ) { *mSpeedMessage = aValue; }
    void SetPowerMessage( const std::string &aValue ) { *mPowerMessage = aValue; }

    void SetLowSpeedSupported( bool aValue ) { mLowSpeedSupported = aValue; }
    void SetFullSpeedSupported( bool aValue ) { mFullSpeedSupported = aValue; }
    void SetHighSpeedSupported( bool aValue ) { mHighSpeedSupported = aValue; }
    void SetSuperSpeedSupported( bool aValue ) { mSuperSpeedSupported = aValue; }
    void SetCurrentSpeedSupported( bool aValue ) { mCurrentSpeedSupported = aValue; }

    void SetSpeed( PvUSBSpeed aValue ) { mSpeed = aValue; }

    void SetMaxPower( uint32_t aValue ) { mMaxPower = aValue; }
    void SetMaxPacketSize( uint32_t aValue ) { mMaxPacketSize = aValue; }

    void SetPleoraDriverInstalled( bool aValue ) { mPleoraDriverInstalled = aValue; }
    void SetInitializedCapabilities( bool aValue ) { mInitializedCapabilities = aValue; }

#endif // PV_GENERATING_DOXYGEN_DOC

private:

	 // Not implemented
    PvDeviceInfoU3V( const PvDeviceInfoU3V & );

    uint32_t mGenCPVersion;
    uint32_t mU3VVersion;

    std::string *mDeviceGUID;
    std::string *mFamilyName;
    std::string *mU3VSerialNumber;
    std::string *mDevicePath;
    std::string *mDeviceKey;
    std::string *mSpeedMessage;
    std::string *mPowerMessage;

    bool mLowSpeedSupported;
    bool mFullSpeedSupported;
    bool mHighSpeedSupported;
    bool mSuperSpeedSupported;
    bool mCurrentSpeedSupported;
    bool mInitializedCapabilities;

    PvUSBSpeed mSpeed;

    uint32_t mMaxPower;
    uint32_t mMaxPacketSize;

    bool mPleoraDriverInstalled;

};

#endif
