// *****************************************************************************
//
// Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

///
/// \file This file contains a command line sample to show how to use 
/// the ebInstaller library.
///

#include <Windows.h>

#include <EbInstaller.h>

#include <string>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>


using namespace std;

// Error values returned by function from this sample
// Negative are errors
// 0 is success
// Positive are warnings / information
typedef enum
{
    EB_APP_ERROR = -3,
    EB_APP_INIT_FAILURE = -2,
    EB_APP_INVALID_ARGUMENT = -1,
    EB_APP_SUCCESS = 0,
    EB_APP_REBOOT_NEEDED = 1,
    EB_APP_REBOOT_AND_RECALL = 2 
} EB_APP_STATUS;

// List of function in this sample
int main( int aCount, const char** aArgs );
static EB_APP_STATUS PrintHelp();
static EB_APP_STATUS ListDrivers();
static EB_APP_STATUS Install( const char* aDriver );
static EB_APP_STATUS Uninstall( const char* aDriver );
static EB_APP_STATUS Update( const char* aDriver );
static EB_APP_STATUS ListNetworkAdapters();
static EB_APP_STATUS DriverEnable( const char* aMAC, bool aEnable );
static EB_APP_STATUS JumboPacketEnable( const char* aMAC, bool aEnable );
static EB_APP_STATUS ConfigureIP( const char* aArg );
static EB_APP_STATUS SetReceveiveAndTransmitBuffersToMax( const char* aMAC );

// Helper functions
static EbNetworkAdapter* FindNetworkAdapterByMac( EbInstaller& aInstaller, const char* aMAC );

///
/// \brief Entry point of the sample.
///
/// Parses the command line and calls another function for execution.
/// This sample cannot be run in WOW64 mode. It must run natively on the OS.
/// This sample should be run as administrator or you should use a manifest to 
/// increase the user's privilege level.
///
/// \param aCount The number of elements on the command line.
/// \param aArgs The list of arguments on the command line.
///
/// \return See EB_APP_STATUS.
///
int main( int aCount, const char** aArgs )
{
    char lTemp[ 128 ];

    // Parse the command line and call the proper function to perform the action.
    if( aCount == 2 )
    {
        // Read the argument
        stringstream lStringStream( aArgs[ 1 ] );
        lStringStream.getline( &lTemp[0], sizeof( lTemp ), '=' );
        string lArgument( lTemp );
        transform( lArgument.begin(), lArgument.end(), lArgument.begin(), ::tolower );

        if ( lArgument.compare( "--help" ) == 0 )
        {  
            return PrintHelp();
        }
        else if ( lArgument.compare( "--listdrivers" ) == 0 )
        {  
            return ListDrivers();
        }
        else if ( lArgument.compare( "--install" ) == 0 )
        {  
            lStringStream.getline( &lTemp[0], sizeof( lTemp ), ' ' );
            return Install( lTemp );
        }
        else if ( lArgument.compare( "--uninstall" ) == 0 )
        {  
            lStringStream.getline( &lTemp[0], sizeof( lTemp ), ' ' );
            return Uninstall( lTemp );
        }
        else if ( lArgument.compare( "--update" ) == 0 )
        {  
            lStringStream.getline( &lTemp[0], sizeof( lTemp ), ' ' );
            return Update( lTemp );
        }
        else if ( lArgument.compare( "--listnetworkadapters" ) == 0 )
        {  
            return ListNetworkAdapters();
        }
        else if ( lArgument.compare( "--driverenable" ) == 0 )
        {  
            lStringStream.getline( &lTemp[0], sizeof( lTemp ), ' ' );
            return DriverEnable( lTemp, true );
        }
        else if ( lArgument.compare( "--driverdisable" ) == 0 )
        {  
            lStringStream.getline( &lTemp[0], sizeof( lTemp ), ' ' );
            return DriverEnable( lTemp, false );
        }
        else if ( lArgument.compare( "--jumbopacketenable" ) == 0 )
        {  
            lStringStream.getline( &lTemp[0], sizeof( lTemp ), ' ' );
            return JumboPacketEnable( lTemp, true );
        }
        else if ( lArgument.compare( "--jumbopacketdisable" ) == 0 )
        {  
            lStringStream.getline( &lTemp[0], sizeof( lTemp ), ' ' );
            return JumboPacketEnable( lTemp, false );
        }
        else if ( lArgument.compare( "--ip" ) == 0 )
        {  
            lStringStream.getline( &lTemp[0], sizeof( lTemp ), ' ' );
            return ConfigureIP( lTemp );
        }
        else if ( lArgument.compare( "--setreceiveandtransmitbuffers" ) == 0 )
        {  
            lStringStream.getline( &lTemp[0], sizeof( lTemp ), ' ' );
            return SetReceveiveAndTransmitBuffersToMax( lTemp );
        }
        

    }

    // This is an unknown argument
    cerr << endl;
    cerr << "Unrecognized argument. Refer to the Help for more details." << endl;
    PrintHelp();
    return EB_APP_INVALID_ARGUMENT;
}

///
/// \brief Prints the help.
///
/// \return See EB_APP_STATUS.
///
EB_APP_STATUS PrintHelp()
{
    cout << endl;
    cout << endl;
    cout << "ebInstaller library test sample." << endl;
    cout << endl;
    cout << "Usage:" << endl;
    cout << "  --help Print this help." << endl;
    cout << "  --listdrivers Print the list of drivers." << endl;
    cout << "  --install=<driver> Install the driver <driver>." << endl;
    cout << "  --uninstall=<driver> Uninstall the driver <driver>." << endl;
    cout << "  --update=<driver> Update the driver <driver>." << endl;
    cout << "  --listnetworkadapters Print the list of network adapters, including the network adapter properties." << endl;
    cout << "  --driverenable=<network adapter MAC address> Enable the driver that is installed on the network adapter <network adapter MAC address>." << endl;
    cout << "  --driverdisable=<network adapter MAC address> Disable the driver that is installed on the network adapter <network adapter MAC address>." << endl;
    cout << "  --jumbopacketenable=<network adapter MAC address> Enable jumbo frames (packets) on the network adapter <network adapter MAC address>." << endl;
    cout << "  --jumbopacketdisable=<network adapter MAC address> Disable jumbo frames (packets) on the network adapter <network adapter MAC address>." << endl;
    cout << "  --ip=<network adapter MAC address> Assign an IP address to the network adapter <network adapter MAC address> using dynamic addressing." << endl;
    cout << "  --ip=<network adapter MAC address>,<IP address>,<subnet mask> Assign a static IP address <IP address> and subnet mask <subnet mask> to the network adapter <network adapter MAC address>." << endl;
    cout << "  --setreceiveandtransmitbuffers=<network adapter MAC address> Set the receive and transmit buffers to the maximum values for the network adapter <network adapter MAC address>." << endl;
    cout << endl;

    return EB_APP_SUCCESS;
}

///
/// \brief Lists all of the network adapters in the system.
///
/// \return See EB_APP_STATUS.
///
EB_APP_STATUS ListNetworkAdapters()
{
    EbInstaller lInstaller;

    // Initialize installer
    PtResult lResult = lInstaller.Initialize();
    if ( !lResult.IsOK() )
    {
        cerr << "Unable to initialize installer: " << lResult.GetCodeString().GetAscii() << ", " << lResult.GetDescription().GetAscii() << endl;

        // Failure to initialize installer, not much we can do...
        return EB_APP_INIT_FAILURE;
    }

    // Now and display the list of adapters
    for ( uint32_t i = 0; i < lInstaller.GetNetworkAdapterCount(); i++ )
    {
        bool lEnabled = false;
        bool lAvailable = false;
        uint32_t lValue = 0;
        bool lDynamic = false;
        uint32_t lCount = 0;

        EbNetworkAdapter* lAdapter = lInstaller.GetNetworkAdapter( i );

        cout << "    Name: " << lAdapter->GetName().GetAscii() << endl;
        cout << i << " - MAC address: " << lAdapter->GetMACAddress().GetAscii() << endl;

        // Driver enabled
        lAdapter->GetDriverEnabled( lEnabled );
        cout << "    Driver enabled: " << ( lEnabled ? "true" : "false" ) << endl;

        // IP configuration

        lCount = 0;
        lAdapter->GetIpAddresses( lDynamic, NULL, NULL, lCount );
        if( lDynamic )
        {
            cout << "    IP configuration: dynamic" << endl;
        }
        else
        {
            PtString* lIPAddress = NULL;
            PtString* lSubnetMask = NULL;

            // Now require the system to the Ip addresses list
            lIPAddress = new PtString[ lCount ];
            lSubnetMask = new PtString[ lCount ];
            lAdapter->GetIpAddresses( lDynamic, lIPAddress, lSubnetMask, lCount );

            cout << "    IP configuration: static" << endl;
            for( uint32_t i = 0; i < lCount; i++ )
            {
                cout << "                        " << lIPAddress[ i ].GetAscii() << " | " << lSubnetMask[ i ].GetAscii() << endl;
            }

            delete [] lIPAddress;
            lIPAddress = NULL;
            delete [] lSubnetMask;
             lSubnetMask = NULL;
        }

        // Jumbo Packet
        lAdapter->IsJumboPacketAvailable( lAvailable );
        if( lAvailable )
        {
            lAdapter->GetJumboPacketEnabled( lEnabled );
            cout << "    Jumbo frames enabled: " << ( lEnabled ? "true" : "false" ) << endl;
        }
        else
        {
            cout << "    Jumbo frames enabled: Not available" << endl;
        }

        // Receive Buffers
        lAdapter->IsReceiveBuffersAvailable( lAvailable );
        if( lAvailable )
        {
            lAdapter->GetReceiveBuffersValue( lValue );
            cout << "    Receive buffers: " << lValue << endl;
        }
        else
        {
            cout << "    Receive buffers: Not available" << endl;
        }

        // Transmit Buffers
        lAdapter->IsTransmitBuffersAvailable( lAvailable );
        if( lAvailable )
        {
            lAdapter->GetTransmitBuffersValue( lValue );
            cout << "    Transmit buffers: " << lValue << endl;
        }
        else
        {
            cout << "    Transmit buffers: Not available" << endl;
        }

        cout << endl;
    }

    return EB_APP_SUCCESS;
}

///
/// \brief Lists the available drivers.
///
/// \return See EB_APP_STATUS.
///
EB_APP_STATUS ListDrivers()
{
    EbInstaller lInstaller;

    // Initialize installer
    PtResult lResult = lInstaller.Initialize();
    if ( !lResult.IsOK() )
    {
        cerr << "Unable to initialize installer: " << lResult.GetCodeString().GetAscii() << ", " << lResult.GetDescription().GetAscii() << endl;

        // Failure to initialize installer, not much we can do...
        return EB_APP_INIT_FAILURE;
    }

    // Now and display the list of adapters
    for ( uint32_t i = 0; i < lInstaller.GetDriverCount(); i++ )
    {
        bool lInstalled = false;
        const EbDriver* lDriver = lInstaller.GetDriver( i );

        lInstaller.IsInstalled( lDriver, lInstalled );

        cout << i << " - Name: " << lDriver->GetName().GetAscii() << endl;
        cout << "    Display name: " << lDriver->GetDisplayName().GetAscii() << endl;
        cout << "    Installed: " << ( lInstalled ? "true" : "false" ) << endl;
    }
    cout << endl;

    return EB_APP_SUCCESS;
}

///
/// \brief Installs the driver.
///
/// \param aDriver The driver to be installed.
///
/// \return See EB_APP_STATUS.
///
EB_APP_STATUS Install( const char* aDriver )
{
    EbInstaller lInstaller;

    if( aDriver == NULL )
    {
        cerr << "aDriver cannot be NULL." << endl;
        return EB_APP_INVALID_ARGUMENT;
    }

    // Initialize installer
    PtResult lResult = lInstaller.Initialize();
    if ( !lResult.IsOK() )
    {
        cerr << "Unable to initialize installer: " << lResult.GetCodeString().GetAscii() << ", " << lResult.GetDescription().GetAscii() << endl;

        // Failure to initialize installer, not much we can do...
        return EB_APP_INIT_FAILURE;
    }

    // Now and display the list of drivers
    const EbDriver* lDriver = NULL;
    string lDriverName( aDriver );
    transform( lDriverName.begin(), lDriverName.end(), lDriverName.begin(), ::tolower );
    for ( uint32_t i = 0; i < lInstaller.GetDriverCount(); i++ )
    {
        lDriver = lInstaller.GetDriver( i );
        string lCurrentDriverName( lDriver->GetName().GetAscii() );
        transform( lCurrentDriverName.begin(), lCurrentDriverName.end(), lCurrentDriverName.begin(), ::tolower );
        if( lDriverName.compare( lCurrentDriverName ) == 0 )
        {
            break;
        }
        lDriver = NULL;
    }

    // Not found
    if( lDriver == NULL )
    {
        cerr << "Unknown driver." << endl;
        return EB_APP_INVALID_ARGUMENT;
    }

    lResult = lInstaller.Install( lDriver );
    if ( lResult == PtResult::Code::REBOOT_AND_RECALL )
    {
        // Success: reboot and call the driver installation tool (or this sample) to complete installation
        cout << "The installation process will resume after you reboot your computer. Please reboot your computer and re-try your action to complete the installation." << endl;
        return EB_APP_REBOOT_AND_RECALL;
    }
    else if ( lResult == PtResult::Code::REBOOT_NEEDED )
    {
        // Success: reboot to complete
        cout << "Please reboot your computer before using this driver."<< endl;
        return EB_APP_REBOOT_NEEDED;
    }
    else
    {
        // Installation failed
        cerr << "Installation failed: " << lResult.GetCodeString().GetAscii() << " " << lResult.GetDescription().GetAscii() << endl;
        cerr << "Please reboot your computer to ensure that the network adapter is restored to a valid state." << endl;
        return EB_APP_ERROR;
    }

    cout << "Installation complete." << endl;
    return EB_APP_SUCCESS;
}

///
/// \brief Uninstalls the driver.
///
/// \param aDriver The driver to be uninstalled.
///
/// \return See EB_APP_STATUS.
///
EB_APP_STATUS Uninstall( const char* aDriver )
{
    EbInstaller lInstaller;

    if( aDriver == NULL )
    {
        cerr << "aDriver cannot be NULL." << endl;
        return EB_APP_INVALID_ARGUMENT;
    }

    // Initialize installer
    PtResult lResult = lInstaller.Initialize();
    if ( !lResult.IsOK() )
    {
        cerr << "Unable to initialize installer: " << lResult.GetCodeString().GetAscii() << ", " << lResult.GetDescription().GetAscii() << endl;

        // Failure to initialize installer, not much we can do...
        return EB_APP_INIT_FAILURE;
    }

    // Now and display the list of drivers
    const EbDriver* lDriver = NULL;
    string lDriverName( aDriver );
    transform( lDriverName.begin(), lDriverName.end(), lDriverName.begin(), ::tolower );
    for ( uint32_t i = 0; i < lInstaller.GetDriverCount(); i++ )
    {
        lDriver = lInstaller.GetDriver( i );
        string lCurrentDriverName( lDriver->GetName().GetAscii() );
        transform( lCurrentDriverName.begin(), lCurrentDriverName.end(), lCurrentDriverName.begin(), ::tolower );
        if( lDriverName.compare( lCurrentDriverName ) == 0 )
        {
            break;
        }
        lDriver = NULL;
    }

    // Not found
    if( lDriver == NULL )
    {
        cerr << "Unknown driver." << endl;
        return EB_APP_INVALID_ARGUMENT;
    }

    lResult = lInstaller.Uninstall( lDriver );
    if ( lResult == PtResult::Code::REBOOT_AND_RECALL )
    {
        // Success: reboot and call the driver installation tool (or this sample) to complete installation
        cout << "The uninstall process will resume after you reboot your computer. Please reboot your computer and re-try your action to complete the installation." << endl;
        return EB_APP_REBOOT_AND_RECALL;
    }
    else if ( lResult == PtResult::Code::REBOOT_NEEDED )
    {
        // Success: reboot to complete
        cout << "Please reboot your computer before using this driver."<< endl;
        return EB_APP_REBOOT_NEEDED;
    }
    else
    {
        // Installation failed
        cerr << "Uninstall failed: " << lResult.GetCodeString().GetAscii() << " " << lResult.GetDescription().GetAscii() << endl;
        cerr << "Please reboot your computer to ensure that the network adapter is restored to a valid state." << endl;
        return EB_APP_ERROR;
    }

    cout << "Uninstall complete." << endl;
    return EB_APP_SUCCESS;
}

///
/// \brief Updates the driver.
///
/// \param aDriver The driver to be updated.
///
/// \return See EB_APP_STATUS.
///
EB_APP_STATUS Update( const char* aDriver )
{
    EbInstaller lInstaller;

    if( aDriver == NULL )
    {
        cerr << "aDriver cannot be NULL." << endl;
        return EB_APP_INVALID_ARGUMENT;
    }

    // Initialize installer
    PtResult lResult = lInstaller.Initialize();
    if ( !lResult.IsOK() )
    {
        cerr << "Unable to initialize installer: " << lResult.GetCodeString().GetAscii() << ", " << lResult.GetDescription().GetAscii() << endl;

        // Failure to initialize installer, not much we can do...
        return EB_APP_INIT_FAILURE;
    }

    // Now and display the list of drivers
    const EbDriver* lDriver = NULL;
    string lDriverName( aDriver );
    transform( lDriverName.begin(), lDriverName.end(), lDriverName.begin(), ::tolower );
    for ( uint32_t i = 0; i < lInstaller.GetDriverCount(); i++ )
    {
        lDriver = lInstaller.GetDriver( i );
        string lCurrentDriverName( lDriver->GetName().GetAscii() );
        transform( lCurrentDriverName.begin(), lCurrentDriverName.end(), lCurrentDriverName.begin(), ::tolower );
        if( lDriverName.compare( lCurrentDriverName ) == 0 )
        {
            break;
        }
        lDriver = NULL;
    }

    // Not found
    if( lDriver == NULL )
    {
        cerr << "Unknown driver." << endl;
        return EB_APP_INVALID_ARGUMENT;
    }

    lResult = lInstaller.Update( lDriver );
    if ( lResult == PtResult::Code::REBOOT_AND_RECALL )
    {
        // Success: reboot and call the driver installation tool (or this sample) to complete installation
        cout << "The update process will resume after you reboot your computer. Please reboot your computer and re-try your action to complete the installation." << endl;
        return EB_APP_REBOOT_AND_RECALL;
    }
    else if ( lResult == PtResult::Code::REBOOT_NEEDED )
    {
        // Success: reboot to complete
        cout << "Please reboot your computer before using this driver."<< endl;
        return EB_APP_REBOOT_NEEDED;
    }
    else
    {
        // Installation failed
        cerr << "Update failed: " << lResult.GetCodeString().GetAscii() << " " << lResult.GetDescription().GetAscii() << endl;
        cerr << "Please reboot your computer to ensure that the network adapter is restored to a valid state." << endl;
        return EB_APP_ERROR;
    }

    cout << "Update complete." << endl;
    return EB_APP_SUCCESS;
}

///
/// \brief Enables or disables the driver.
///
/// Only available for network adapters.
///
/// \param aMAC The MAC address of the network adapter on which to enable or disable the driver.
/// \param aEnable true to enable, false to disable.
///
/// \return See EB_APP_STATUS.
///
EB_APP_STATUS DriverEnable( const char* aMAC, bool aEnable )
{
    EbInstaller lInstaller;
    EbNetworkAdapter* lAdapter = NULL;

    if( aMAC == NULL )
    {
        cerr << "aMAC cannot be NULL." << endl;
        return EB_APP_INVALID_ARGUMENT;
    }

    // Initialize installer
    PtResult lResult = lInstaller.Initialize();
    if ( !lResult.IsOK() )
    {
        cerr << "Unable to initialize installer: " << lResult.GetCodeString().GetAscii() << ", " << lResult.GetDescription().GetAscii() << endl;

        // Failure to initialize installer, not much we can do...
        return EB_APP_INIT_FAILURE;
    }
    
    // Find the adapter
    lAdapter = FindNetworkAdapterByMac( lInstaller, aMAC );
    if( lAdapter == NULL )
    {
        cerr << "Unknown MAC address." << endl;
        return EB_APP_INVALID_ARGUMENT;
    }
    
    lResult = lAdapter->SetDriverEnabled( aEnable );
    if ( lResult == PtResult::Code::REBOOT_AND_RECALL )
    {
        cout << "The enable/disable process will resume after you reboot your computer. Please reboot your computer and re-try your action to complete the installation." << endl;
        return EB_APP_REBOOT_AND_RECALL;
    }
    else if ( lResult == PtResult::Code::REBOOT_NEEDED )
    {
        cout << "Please reboot your computer before using this driver."<< endl;
        return EB_APP_REBOOT_NEEDED;
    }
    else
    {
        cerr << "Enable/disable failed: " << lResult.GetCodeString().GetAscii() << " " << lResult.GetDescription().GetAscii() << endl;
        cerr << "Please reboot your computer to ensure that the network adapter is restored to a valid state." << endl;
        return EB_APP_ERROR;
    }

    cout << "Enable/disable complete." << endl;
    return EB_APP_SUCCESS;
}

///
/// \brief Enables or disables jumbo frames (packets) for the network adapter.
///
/// Only available for network adapters and 
/// operating systems that support jumbo frames.
///
/// \param aMAC The MAC address of the network adapter on which to enable or disable jumbo frames.
/// \param aEnable true to enable, false to disable.
///
/// \return See EB_APP_STATUS.
///
EB_APP_STATUS JumboPacketEnable( const char* aMAC, bool aEnable )
{
    EbInstaller lInstaller;
    EbNetworkAdapter* lAdapter = NULL;
    bool lAvailable = false;

    if( aMAC == NULL )
    {
        cerr << "aMAC cannot be NULL." << endl;
        return EB_APP_INVALID_ARGUMENT;
    }

    // Initialize installer
    PtResult lResult = lInstaller.Initialize();
    if ( !lResult.IsOK() )
    {
        cerr << "Unable to initialize installer: " << lResult.GetCodeString().GetAscii() << ", " << lResult.GetDescription().GetAscii() << endl;

        // Failure to initialize installer, not much we can do...
        return EB_APP_INIT_FAILURE;
    }
    
    // Find the adapter
    lAdapter = FindNetworkAdapterByMac( lInstaller, aMAC );
    if( lAdapter == NULL )
    {
        cerr << "Unknown MAC address." << endl;
        return EB_APP_INVALID_ARGUMENT;
    }

    lAdapter->IsJumboPacketAvailable( lAvailable );
    if( !lAvailable )
    {
        cout << "Jumbo frames are not available for this network adapter." << endl;
        return EB_APP_SUCCESS;
    }

    lResult = lAdapter->SetJumboPacketEnabled( aEnable );
    if ( lResult == PtResult::Code::REBOOT_NEEDED )
    {
        cout << "Please reboot your computer before using this driver."<< endl;
        return EB_APP_REBOOT_NEEDED;
    }
    else
    {
        cerr << "Setting jumbo frames failed: " << lResult.GetCodeString().GetAscii() << " " << lResult.GetDescription().GetAscii() << endl;
        cerr << "Please reboot your computer to ensure that the network adapter is restored to a valid state." << endl;
        return EB_APP_ERROR;
    }

    cout << "Jumbo frames are now configured." << endl;
    return EB_APP_SUCCESS;
}

///
/// \brief Configures the Internet protocol.
///
/// If an IP address and subnet mask are not provided, the network adapter uses dynamic addressing.
/// If it is provided, the static IP address and subnet mask are used.
///
/// \param aArgs "MAC[,IpAddress,SubnetMask]" Configuration from the command line.
///
/// \return See EB_APP_STATUS.
///
EB_APP_STATUS ConfigureIP( const char* aArgs )
{
    EbInstaller lInstaller;
    EbNetworkAdapter* lAdapter = NULL;
    stringstream lParser( aArgs );
    char lTemp[128];
    string lMac;
    string lIp;
    string lSubnetmask;
    bool lStatic = false;

    // Parse the inputs argument
    if( lParser.getline( &lTemp[ 0 ], sizeof( lTemp ), ',' ) )
    {
        lMac = lTemp;
    }
    else
    {
        cerr << "MAC address cannot be NULL." << endl;
        return EB_APP_INVALID_ARGUMENT;
    }
    if( lParser.getline( &lTemp[ 0 ], sizeof( lTemp ), ',' ) )
    {
        lIp = lTemp;
        if( lParser.getline( &lTemp[ 0 ], sizeof( lTemp ), ',' ) )
        {
            lSubnetmask = lTemp;
            lStatic = true;
        }
        else
        {
            cerr << "Unable to read the subnet mask." << endl;
            return EB_APP_INVALID_ARGUMENT;
        }
    }

    // Initialize installer
    PtResult lResult = lInstaller.Initialize();
    if ( !lResult.IsOK() )
    {
        cerr << "Unable to initialize installer: " << lResult.GetCodeString().GetAscii() << ", " << lResult.GetDescription().GetAscii() << endl;

        // Failure to initialize installer, not much we can do...
        return EB_APP_INIT_FAILURE;
    }
    
    // Find the adapter
    lAdapter = FindNetworkAdapterByMac( lInstaller, lMac.c_str() );
    if( lAdapter == NULL )
    {
        cerr << "Unknown MAC address." << endl;
        return EB_APP_INVALID_ARGUMENT;
    }

    // Set the configuration
    if( lStatic )
    {
        lResult = lAdapter->SetStaticIpAddress( lIp.c_str(), lSubnetmask.c_str() );
    }
    else
    {
        lResult = lAdapter->SetDynamicIpAddress();
    }
    if ( lResult == PtResult::Code::REBOOT_NEEDED )
    {
        cout << "Please reboot your computer before using this driver."<< endl;
        return EB_APP_REBOOT_NEEDED;
    }
    else
    {
        cerr << "Setting IP configuration failed: " << lResult.GetCodeString().GetAscii() << " " << lResult.GetDescription().GetAscii() << endl;
        cerr << "Please reboot your computer to ensure that the network adapter is restored to a valid state." << endl;
        return EB_APP_ERROR;
    }

    cout << "Setting IP configuration complete." << endl;
    return EB_APP_SUCCESS;
}

///
/// \brief Sets the number of receive and transmit buffers to the maximum values.
///
/// \param aMAC The MAC address of the network adapter for which to configure receive and transmit buffers.
///
/// \return See EB_APP_STATUS.
///
EB_APP_STATUS SetReceveiveAndTransmitBuffersToMax( const char* aMAC )
{
    EbInstaller lInstaller;
    EbNetworkAdapter* lAdapter = NULL;
    bool lAvailable = false;
    uint32_t lDefault;
    uint32_t lMin;
    uint32_t lMax;

    if( aMAC == NULL )
    {
        cerr << "aMAC cannot be NULL." << endl;
        return EB_APP_INVALID_ARGUMENT;
    }

    // Initialize installer
    PtResult lResult = lInstaller.Initialize();
    if ( !lResult.IsOK() )
    {
        cerr << "Unable to initialize installer: " << lResult.GetCodeString().GetAscii() << ", " << lResult.GetDescription().GetAscii() << endl;

        // Failure to initialize installer, not much we can do...
        return EB_APP_INIT_FAILURE;
    }
    
    // Find the adapter
    lAdapter = FindNetworkAdapterByMac( lInstaller, aMAC );
    if( lAdapter == NULL )
    {
        cerr << "Unknown MAC address." << endl;
        return EB_APP_INVALID_ARGUMENT;
    }

    // Set transmit buffers
    lAdapter->IsTransmitBuffersAvailable( lAvailable );
    if( lAvailable )
    {
        lResult = lAdapter->GetTransmitBuffersValues( lDefault, lMin, lMax );
        if( lResult.IsOK() )
        {
            cout << "Setting the number of transmit buffers to " << lMax << "." << endl;
            lResult = lAdapter->SetTransmitBuffersValue( lMax );
            if( !lResult.IsOK() )
            {
                cout << "Unable to set the maximum number of transmit buffers on this network adapter."<< endl;
            }            
        }
        else
        {
            cout << "Unable to retrieve the maximum number of transmit buffers on this network adapter."<< endl;
        }
    }
    else
    {
        cout << "You cannot set transmit buffers on this network adapter."<< endl;
    }

    // Set receive buffers
    lAdapter->IsReceiveBuffersAvailable( lAvailable );
    if( lAvailable )
    {
        lResult = lAdapter->GetReceiveBuffersValues( lDefault, lMin, lMax );
        if( lResult.IsOK() )
        {
            cout << "Setting the number of receive buffers to " << lMax << "." << endl;
            lResult = lAdapter->SetReceiveBuffersValue( lMax );
            if( !lResult.IsOK() )
            {
                cout << "Unable to set the maximum number of receive buffers on this network adapter."<< endl;
            }            
        }
        else
        {
            cout << "Unable to retrieve the maximum number of receive buffers on this network adapter."<< endl;
        }
    }
    else
    {
        cout << "You cannot set receive buffers on this network adapter."<< endl;
    }

    cout << "Setting transmit and receive buffers to maximum complete." << endl;
    return EB_APP_SUCCESS;
}

///
/// \brief Helper function that finds the network adapter based on MAC address.
///
/// \param aInstaller Initialized instance of the EbInstaller.
/// \param aMAC The MAC address of the network adapter.
///
/// \return Pointer to the network adapter or NULL if not found.
///
EbNetworkAdapter* FindNetworkAdapterByMac( EbInstaller& aInstaller, const char* aMAC )
{
    EbNetworkAdapter* lAdapter = NULL;
    uint32_t lCount;
    string lMAC( aMAC );
    
    transform( lMAC.begin(), lMAC.end(), lMAC.begin(), ::tolower );
    lCount = aInstaller.GetNetworkAdapterCount();

    for ( uint32_t i = 0; i < lCount; i++ )
    {
        string lCurrentMAC( aInstaller.GetNetworkAdapter( i )->GetMACAddress().GetAscii() );
        transform( lCurrentMAC.begin(), lCurrentMAC.end(), lCurrentMAC.begin(), ::tolower );

        if( lMAC.compare( lCurrentMAC ) == 0 )
        {
            lAdapter = aInstaller.GetNetworkAdapter( i );
            break;
        }
    }

    return lAdapter;
}