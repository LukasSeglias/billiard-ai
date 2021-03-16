// *****************************************************************************
//
// Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

#include <PvSampleTransmitterConfig.h>


// Default values
#define DEFAULT_CONNECTION_ID ( "" )


// Application config
class Configuration : public PvSampleTransmitterConfig
{
public:

    Configuration()
        : mConnectionID( DEFAULT_CONNECTION_ID )
    {
    }

    ~Configuration()
    {
    }

    const char *GetConnectionID() const { return mConnectionID.c_str(); }

    void ParseCommandLine( int aCount, const char **aArgs )
    {
        if ( ParseOptionFlag( aCount, aArgs, "--help" ) )
        {
            PrintHelp();
            exit( 0 );
        }

        PvSampleTransmitterConfig::ParseCommandLine( aCount, aArgs );

        ParseOption<string>( aCount, aArgs, "--connectionid", mConnectionID );
    }

    void PrintHelp()
    {
        cout << "This utility receives an image stream from a GigE Vision or USB3 Vision device, writes text on it and retransmits it using the GigEVision Streaming Protocol (GVSP)." << endl << endl << endl;

        PvSampleTransmitterConfig::PrintHelp();

        cout << "--connectionid=<IP address or GUID of the device from which to receive>" << endl;
        cout << "Default behavior opens the device finder window to allow the user to select a device." << endl << endl;
    }

private:

    string mConnectionID;

};


#endif // __CONFIGURATION_H__

