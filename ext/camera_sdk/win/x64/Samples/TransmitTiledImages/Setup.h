// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "Constants.h"

#include <PvConfigurationWriter.h>
#include <PvConfigurationReader.h>

// Current possible role of a device
#define ROLE_INVALID          ( 0 )
#define ROLE_CTRLDATA         ( 1 )
#define ROLE_DATA             ( 2 )

// Current possible destination 
#define DESTINATION_INVALID             ( 0 )
#define DESTINATION_UNICAST_AUTO        ( 1 )
#define DESTINATION_UNICAST_SPECIFIC    ( 2 )
#define DESTINATION_MULTICAST           ( 3 )


///
/// \class Setup
///
/// \brief Communication setup data holder
///
class Setup
{
public:

    ///
    /// \brief Constructor
    ///
    Setup()
    {
        Reset();
    }

    ///
    /// \brief Destructor
    ///
    virtual ~Setup()
    {
    }

    ///
    /// \brief Reset all the internal information
    ///
    inline void Reset()
    {
        mRole = ROLE_INVALID;
        mDestination = DESTINATION_INVALID;
        mIPAddress = _T( "" );
        mPort = 0;
    }

    /// 
    /// \brief Set the role
    ///
    /// \param aRole New role
    ///
    inline void SetRole( int aRole ) 
    {
        mRole = aRole; 
    }

    /// 
    /// \brief Get the role
    ///
    /// \return the role
    ///
    inline int GetRole() 
    { 
        return mRole;
    }

    /// 
    /// \brief Set the destination
    ///
    /// \param aDestination New destination
    ///
    inline void SetDestination( int aDestination ) 
    {
        mDestination = aDestination;
    }

    /// 
    /// \brief Get the destination
    ///
    /// \return the destination
    ///
    inline int GetDestination() { return mDestination; }

    /// 
    /// \brief Set the IP address
    ///
    /// \param aIPAddress New ip address
    ///
    inline void SetIPAddress( CString& aIPAddress ) 
    { 
        mIPAddress = aIPAddress; 
    }

    /// 
    /// \brief Get the ip address
    ///
    /// \return the ip address
    ///
    inline CString& GetIPAddress() { return mIPAddress; }

    /// 
    /// \brief Set the udp port
    ///
    /// \param aPort New udp port
    ///
    inline void SetPort( unsigned int aPort ) 
    {
        mPort = aPort; 
    }

    /// 
    /// \brief Get the udp port
    ///
    /// \return the udp port
    ///
    inline unsigned int GetPort() 
    {
        return mPort; 
    }
    
    ///
    /// \brief Store the class into a persistence object
    ///
    /// \param aWriter The persistence object
    /// \param aRow The tiling index row
    /// \param aColumn The tiling index column
    ///
    /// \return Error State
    ///
    PvResult Store( PvConfigurationWriter& aWriter, int aRow, int aColumn );

    ///
    /// \brief Load the class from a persistence object
    ///
    /// \param aReader The persistence object
    /// \param aRow The tiling index row
    /// \param aColumn The tiling index column
    ///
    /// \return Error State
    ///
    PvResult Restore( PvConfigurationReader& aReader, int aRow, int aColumn );

private:
    // Role of the connection
    int mRole;

    // Destination 
    int mDestination;

    // Optional IP address
    CString mIPAddress;

    // Optional Port
    unsigned short mPort;
};

