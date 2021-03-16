// *****************************************************************************
//
// Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

#include <PvSampleTransmitterConfig.h>


// Default values
#define DEFAULT_WIDTH ( 640 )
#define DEFAULT_HEIGHT ( 480 )
#define DEFAULT_NO_PATTERN ( false )
#define DEFAULT_FPS ( 30 )


// Application config
class Configuration : public PvSampleTransmitterConfig
{
public:

    Configuration()
        : mWidth( DEFAULT_WIDTH )
        , mHeight( DEFAULT_HEIGHT )
        , mNoPattern( DEFAULT_NO_PATTERN )
    {
    }

    ~Configuration()
    {
    }

    uint32_t GetWidth() const { return mWidth; }
    uint32_t GetHeight() const { return mHeight; }
    bool GetNoPattern() const { return mNoPattern; }

    void ParseCommandLine( int aCount, const char **aArgs )
    {
        if ( ParseOptionFlag( aCount, aArgs, "--help" ) )
        {
            PrintHelp();
            exit( 0 );
        }

        PvSampleTransmitterConfig::ParseCommandLine( aCount, aArgs );

        ParseOption<uint32_t>( aCount, aArgs, "--width", mWidth );
        ParseOption<uint32_t>( aCount, aArgs, "--height", mHeight );
        ParseOptionFlag( aCount, aArgs, "--nopattern", &mNoPattern );
    }

    void PrintHelp()
    {
        cout << "This utility transmits a test pattern to a given destination using the GigEVision Streaming Protocol (GVSP)." << endl << endl << endl;

        PvSampleTransmitterConfig::PrintHelp();

        cout << "--width=<width in pixels>" << endl;
        cout << "Default: " << DEFAULT_WIDTH << endl << endl;

        cout << "--height=<height in pixels>" << endl;
        cout << "Default: " << DEFAULT_HEIGHT << endl << endl;

        cout << "--fps=<frame per second>" << endl;
        cout << "Default: " << DEFAULT_FPS << endl << endl;

        cout << "--nopattern" << endl;
        cout << "Disables the test pattern." << endl;
        cout << "By default, each frame is populated with test data - this takes a little bit of CPU power so for pure benchmark purposes it may be advisable to disable this behaviour." << endl << endl;
    }

private:

    uint32_t mWidth;
    uint32_t mHeight;
    bool mNoPattern;

};


#endif // __CONFIGURATION_H__

