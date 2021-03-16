// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __SETUP_H__
#define __SETUP_H__

#include "Persistent.h"


class Setup : public Persistent
{
#ifdef _AFXDLL
    DECLARE_DYNAMIC( Setup )
#endif // _AFXDLL

public:

    Setup();
    ~Setup();

    void Reset();

    // Persistent
    PvResult Save( PvConfigurationWriter *aWriter );
    PvResult Load( PvConfigurationReader *aReader );

    bool IsTheSame( PvConfigurationReader *aReader );

    static PvString IPFromBytes( uint8_t a0, uint8_t a1, uint8_t a2, uint8_t a3 );

    enum Role
    {
        RoleInvalid = -1,
        RoleCtrlData = 0,
        RoleCtrl = 1,
        RoleData = 2,
        RoleMonitor
    };

    enum Destination
    {
        DestinationInvalid = -1,
        DestinationUnicastAuto = 0,
        DestinationUnicastSpecific = 1,
        DestinationUnicastOther = 2,
        DestinationMulticast = 3
    };

    bool IsController() const { return ( mRole == RoleCtrl ) || ( mRole == RoleCtrlData ); }
    bool IsDataReceiver() const { return ( mRole == RoleData ) || ( mRole == RoleCtrlData ); }

    // Get accessors
    Role GetRole() const { return mRole; }
    Destination GetDestination() const { return mDestination; }
    unsigned short GetUnicastSpecificPort() const { return mUnicastSpecificPort; }
    PvString GetUnicastIP() const { return mUnicastIP.c_str(); }
    unsigned short GetUnicastPort() const { return mUnicastPort; }
    PvString GetMulticastIP() const { return mMulticastIP.c_str(); }
    unsigned short GetMulticastPort() const { return mMulticastPort; }
    unsigned short GetDefaultChannel() const { return mDefaultChannel; }

    // Set accessors
    void SetRole( Role aRole ) { mRole = aRole; }
    void SetDestination( Destination aDestination ) { mDestination = aDestination; }
    void SetUnicastSpecificPort( unsigned short aPort ) { mUnicastSpecificPort = aPort; }
    void SetUnicastIP( const PvString &aIP ) { mUnicastIP = aIP.GetAscii(); }
    void SetUnicastPort( unsigned short aPort ) { mUnicastPort = aPort; }
    void SetMulticastIP( const PvString &aMulticastIP ) { mMulticastIP = aMulticastIP.GetAscii(); }
    void SetMulticastPort( unsigned short aMulticastPort ) { mMulticastPort = aMulticastPort; }
    void SetDefaultChannel( unsigned short aDefaultChannel ) { mDefaultChannel = aDefaultChannel; }

    // Enabled state
    bool IsEnabled() const { return mEnabled; }
    void SetEnabled( bool aEnabled ) { mEnabled = aEnabled; }
    void UpdateEnabled( Role aRole, Destination &aDest );
    bool IsRoleCtrlDataEnabled() const { return mRoleCtrlDataEnabled; }
    bool IsRoleCtrlEnabled() const { return mRoleCtrlEnabled; }
    bool IsRoleDataEnabled() const { return mRoleDataEnabled; }
    bool IsRoleMonitorEnabled() const { return mRoleMonitorEnabled; }
    bool IsDestUnicastAutoEnabled() const { return mDestUnicastAutoEnabled; }
    bool IsDestUnicastSpecificEnabled() const { return mDestUnicastSpecificEnabled; }
    bool IsDestUnicastOtherEnabled() const { return mDestUnicastOtherEnabled; }
    bool IsDestMulticastEnabled() const { return mDestMulticastEnabled; }
    bool IsChannelEnabled() const { return mChannelEnabled; }

protected:

    static void StrToRole( const std::string &aStr, Role &aRole );
    static void StrToDestination( const std::string &aStr, Destination &aDestination );

private:

    Role mRole;
    Destination mDestination;
    unsigned short mUnicastSpecificPort;
    std::string mUnicastIP;
    unsigned short mUnicastPort;
    std::string mMulticastIP;
    unsigned short mMulticastPort;
    unsigned short mDefaultChannel;

    bool mEnabled;
    bool mRoleCtrlDataEnabled;
    bool mRoleCtrlEnabled;
    bool mRoleDataEnabled;
    bool mRoleMonitorEnabled;
    bool mDestUnicastAutoEnabled;
    bool mDestUnicastSpecificEnabled;
    bool mDestUnicastOtherEnabled;
    bool mDestMulticastEnabled;
    bool mChannelEnabled;
};


#endif // __SETUP_H__

