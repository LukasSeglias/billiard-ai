// *****************************************************************************
//
// Copyright (c) 2017, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include <PvSampleTransmitterConfig.h>

#include "DataGenerator.h"

#include <algorithm>


#define DEFAULT_WIDTH ( 1280 )
#define DEFAULT_HEIGHT ( 720 )
#define DEFAULT_CHUNKSIZE ( 128 )
#define DEFAULT_PATTERN ( TestPatternDiagonalMoving )


class Configuration : public PvSampleTransmitterConfig
{
public:

    Configuration()
        : mWidth( DEFAULT_WIDTH )
        , mHeight( DEFAULT_HEIGHT )
        , mChunkSize( DEFAULT_CHUNKSIZE )
        , mPattern( DEFAULT_PATTERN )
    {
    }

    uint32_t GetWidth() const { return mWidth; }
    uint32_t GetHeight() const { return mHeight; }
    uint32_t GetChunkSize() const { return mChunkSize; }
    TestPattern GetTestPattern() const { return mPattern; }

    void ParseCommandLine( int aCount, const char **aArgs )
    {
        // Call base class implementation
        PvSampleTransmitterConfig::ParseCommandLine( aCount, aArgs );

        ParseOption<uint32_t>( aCount, aArgs, "--width", mWidth );
        ParseOption<uint32_t>( aCount, aArgs, "--height", mHeight );
        ParseOption<uint32_t>( aCount, aArgs, "--chunk", mChunkSize );

        // Read pattern string
        std::string lPattern;
        ParseOption<string>( aCount, aArgs, "--pattern", lPattern );

        // Lowercase
        std::transform( lPattern.begin(), lPattern.end(), lPattern.begin(), ::tolower );

        // String to enum
        if ( lPattern == "diagonalmoving" )
        {
            mPattern = TestPatternDiagonalMoving;
        }
        else if ( lPattern == "diagonalstatic" )
        {
            mPattern = TestPatternDiagonalStatic;
        }
        else if ( lPattern == "horizontalmoving" )
        {
            mPattern = TestPatternHorizontalMoving;
        }
        else if ( lPattern == "horizontalstatic" )
        {
            mPattern = TestPatternHorizontalStatic;
        }
        else if ( lPattern == "verticalmoving" )
        {
            mPattern = TestPatternVerticalMoving;
        }
        else if ( lPattern == "verticalstatic" )
        {
            mPattern = TestPatternVerticalStatic;
        }
    }

    void PrintHelp()
    {
        // Call base class implementation
        PvSampleTransmitterConfig::PrintHelp();

        cout << "--width=<width in pixels>" << endl;
        cout << "Default: " << DEFAULT_WIDTH << endl << endl;

        cout << "--height=<height in pixels>" << endl;
        cout << "Default: " << DEFAULT_HEIGHT << endl << endl;

        cout << "--chunk=<chunk size in bytes>" << endl;
        cout << "Default: " << DEFAULT_CHUNKSIZE << endl << endl;

        cout << "--pattern=<diagonalmoving|diagonalstatic|horizontalmoving|horitzontalstatic|verticalmoving|verticalstatic>" << endl;
        cout << "Default: " << DEFAULT_PATTERN << endl << endl;
    }

protected:

private:

    uint32_t mWidth;
    uint32_t mHeight;

    uint32_t mChunkSize;

    TestPattern mPattern;

};

