// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVSAMPLETRANSMITTERCONFIG_H__
#define __PVSAMPLETRANSMITTERCONFIG_H__

#include "PvSampleUtils.h"
#include "PvSystem.h"
#include "PvNetworkAdapter.h"
#include "PvPixelType.h"

#include <map>


#define DEFAULT_NUMBER_OF_BUFFERS ( 48 )
#define DEFAULT_SOURCE_ADDRESS ( "" )
#define DEFAULT_DESTINATION_ADDRESS ( "239.192.1.1" )
#define DEFAULT_DESTINATION_PORT ( 1042 )
#define DEFAULT_SOURCE_PORT ( 0 )
#define DEFAULT_BUFFER_COUNT ( 4 )
#define DEFAULT_PACKET_SIZE ( 1440 )
#define DEFAULT_FPS ( 30 )
#define DEFAULT_SILENT ( false )


#ifndef PV_GENERATING_DOXYGEN_DOC


// Application config
class PvSampleTransmitterConfig
{
public:

    PvSampleTransmitterConfig()
    {
        SetDefaults();
    }

    ~PvSampleTransmitterConfig()
    {
    }

    const char *GetSourceAddress() const { return mSourceAddress.c_str(); }
    uint16_t GetSourcePort() const { return mSourcePort; }
    const char *GetDestinationAddress() const { return mDestinationAddress.c_str(); }
    uint16_t GetDestinationPort() const { return mDestinationPort; }
    float GetFPS() const { return mFPS; }
    bool GetSilent() const { return mSilent; }
    uint32_t GetPacketSize() const { return mPacketSize; }
    uint32_t GetBufferCount() const { return mBufferCount; }

    virtual void ParseCommandLine( int aCount, const char **aArgs )
    {
        if ( ParseOptionFlag( aCount, aArgs, "--help" ) )
        {
            PrintHelp();
            exit( 0 );
        }

        ParseOption<float>( aCount, aArgs, "--fps", mFPS );
        ParseOption<uint32_t>( aCount, aArgs, "--packetsize", mPacketSize );
        ParseOption<string>( aCount, aArgs, "--destinationaddress", mDestinationAddress );
        ParseOption<uint16_t>( aCount, aArgs, "--destinationport", mDestinationPort );
        ParseOption<string>( aCount, aArgs, "--sourceaddress", mSourceAddress );
        ParseOption<uint16_t>( aCount, aArgs, "--sourceport", mSourcePort );
        ParseOption<uint32_t>( aCount, aArgs, "--buffercount", mBufferCount );
        ParseOptionFlag( aCount, aArgs, "--silent", &mSilent );
        if ( mDestinationPort == 0 )
        {
            cout << "Please enter a destination port." << endl;
            cin >> mDestinationPort;
        }
    }

    virtual void PrintHelp()
    {
        cout << "Optional command line arguments:" << endl << endl;

        cout << "--help " << endl << "Print this help message." << endl << endl;

        cout << "--packetsize=<maximimum size of streaming packets>" << endl;
        cout << "Default: 1440 For best results, set \"Jumbo Frames\" property on your NIC and increase this value accordingly." << endl << endl;

        cout << "--destinationaddress=<destination address in the form 123.456.789.101>" << endl;
        cout << "Default: " << DEFAULT_DESTINATION_ADDRESS << endl << endl;

        cout << "--destinationport=<destination port>" << endl;
        cout << "Default: " << DEFAULT_DESTINATION_PORT << endl << endl;

        cout << "--sourceaddress=<source address in the form 123.456.789.101>" << endl;
        cout << "Default: first valid address encountered while enumerating interfaces" << endl << endl;

        cout << "--sourceport=<source port>" << endl;
        cout << "Default: " << DEFAULT_SOURCE_PORT << " - a port is automatically assigned when the socket is opened" << endl << endl;

        cout << "--buffercount=<number of transmit buffers to use>" << endl;
        cout << "Default: " << DEFAULT_BUFFER_COUNT << " - increase this number when sending smaller images at high frame rates." << endl << endl;

        cout << "--silent" << endl;
        cout << "Don't wait for a key press." << endl;
        cout << "By default, the system waits for a key press before it begins transmitting. " << endl << endl;

        cout << "--fps=<frame per second>" << endl;
        cout << "Default: " << DEFAULT_FPS << endl << endl;

    }

private:

    void SetDefaults()
    {
        // Only used to enumerate network interfaces, no need to call Find
        PvSystem lSystem;

        // Find default source address
        bool lFound = false;
        for ( uint32_t i = 0; i < lSystem.GetInterfaceCount() && !lFound; i++ )
        {
            const PvNetworkAdapter *lNetworkAdapter = dynamic_cast<const PvNetworkAdapter *>( lSystem.GetInterface( i ) );
            if ( lNetworkAdapter == NULL )
            {
                continue;
            }

            uint32_t lIPCount = lNetworkAdapter->GetIPAddressCount();
            for ( uint32_t j = 0; j < lIPCount; j++ )
            {
                PvString lIP = lNetworkAdapter->GetIPAddress( j );
                if ( strcmp( "0.0.0.0", lIP.GetAscii() ) != 0 )
                {
                    mSourceAddress = lIP.GetAscii();
                    lFound = true;
                }
            }
        }
        if ( !lFound )
        {
            cout << "No valid interfaces found." << endl;
            exit( 1 );
        }

        // Set static defaults
        mDestinationAddress = DEFAULT_DESTINATION_ADDRESS;
        mDestinationPort = DEFAULT_DESTINATION_PORT;
        mSourcePort = DEFAULT_SOURCE_PORT;
        mPacketSize = DEFAULT_PACKET_SIZE;
        mBufferCount = DEFAULT_BUFFER_COUNT;
        mFPS = DEFAULT_FPS;
        mSilent = DEFAULT_SILENT;
    }

    string mSourceAddress;
    uint16_t mSourcePort;

    string mDestinationAddress;
    uint16_t mDestinationPort;

    float mFPS;
    bool mSilent;

    uint32_t mPacketSize;
    uint32_t mBufferCount;

};


#endif // PV_GENERATING_DOXYGEN_DOC

#endif
