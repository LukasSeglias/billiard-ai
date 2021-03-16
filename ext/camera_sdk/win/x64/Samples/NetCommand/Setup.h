// *****************************************************************************
//
//     Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "afxwin.h"

#include <PvConfigurationReader.h>
#include <PvConfigurationWriter.h>


class Setup
{
public:

    Setup();
    virtual ~Setup();

    void Reset();

    // Persistence
    void Save( PvConfigurationWriter &aWriter );
    void Load( PvConfigurationReader &aReader );
    bool IsTheSame( PvConfigurationReader &aReader );

    enum Role
    {
        RoleInvalid = -1,
        RoleCtrlData = 0,
        RoleCtrl,
        RoleData,
    };

    enum Destination
    {
        DestinationInvalid = -1,
        DestinationUnicastAuto = 0,
        DestinationUnicastOther = 1,
        DestinationMulticast = 2,
        DestinationUnicastSpecific = 3
    };

    Role GetRole() const { return mRole; }
    Destination GetDestination() const { return mDestination; }
    unsigned short GetUnicastSpecificPort() const { return mUnicastSpecificPort; }
    const CString &GetUnicastIP() const { return mUnicastIP; }
    unsigned short GetUnicastPort() const { return mUnicastPort; }
    const CString &GetMulticastIP() const { return mMulticastIP; }
    unsigned short GetMulticastPort() const { return mMulticastPort; }

    void SetRole( Role aRole ) { mRole = aRole; }
    void SetDestination( Destination aDestination ) { mDestination = aDestination; }
    void SetUnicastSpecificPort( unsigned short aPort ) { mUnicastSpecificPort = aPort; }
    void SetUnicastIP( const CString &aIP ) { mUnicastIP = aIP; }
    void SetUnicastPort( unsigned short aPort ) { mUnicastPort = aPort; }
    void SetMulticastIP( const CString &aMulticastIP ) { mMulticastIP = aMulticastIP; }
    void SetMulticastPort( unsigned short aMulticastPort ) { mMulticastPort = aMulticastPort; }

protected:
        
    Role mRole;
    Destination mDestination;
    unsigned short mUnicastSpecificPort;
    CString mUnicastIP;
    unsigned short mUnicastPort;
    CString mMulticastIP;
    unsigned short mMulticastPort;

    static void Save( PvConfigurationWriter &aWriter, Setup *aSetup );
    static void Load( PvConfigurationReader &aReader, Setup *aSetup );

    static void StrToRole( const CString &aStr, Role &aRole );
    static void StrToDestination( const CString &aStr, Destination &aDestination );
};


