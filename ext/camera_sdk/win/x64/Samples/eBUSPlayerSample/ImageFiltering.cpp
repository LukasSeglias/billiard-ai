// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "eBUSPlayerShared.h"
#include "ImageFiltering.h"
#include "DisplayThread.h"

#include <assert.h>
#include <sstream>


#define FILTERINGDLG_VERSION ( "1.0.0.0" )

#define TAG_VERSION ( "filteringdlgversion" )
#define TAG_FILTERRGBENABLED ( "filterrgbenabled" )
#define TAG_REDGAIN ( "redgain" )
#define TAG_GREENGAIN ( "greengain" )
#define TAG_BLUEGAIN ( "bluegain" )
#define TAG_REDOFFSET ( "redoffset" )
#define TAG_GREENOFFSET ( "greenoffset" )
#define TAG_BLUEOFFSET ( "blueoffset" )
#define TAG_BAYERFILTERING ( "bayerfiltering" )
#define TAG_TAPGEOMETRY ( "tapgeometry" )

#define VAL_TRUE ( "true" )
#define VAL_FALSE ( "false" )
#define VAL_BAYERSIMPLE ( "simple" )
#define VAL_BAYER3X3 ( "3x3" )


#ifdef _AFXDLL
    IMPLEMENT_DYNAMIC( ImageFiltering, CObject )
#endif // _AFXDLL


///
/// \brief Constructor
///

ImageFiltering::ImageFiltering()
    : mBayerFilter( PvBayerFilterSimple )
    , mRGBFilterEnabled( false )
    , mTapGeometry( PvTapGeometryUnknown )
{

}


///
/// \brief Constructor
///

ImageFiltering::~ImageFiltering()
{

}


///
/// \brief Persistence save
///

PvResult ImageFiltering::Save( PvConfigurationWriter *aWriter )
{
    std::stringstream lSS;
    std::string lStr;

    // Save a version string, just in case we need it in the future
    aWriter->Store( FILTERINGDLG_VERSION, TAG_VERSION );

    // RGB filter enabled
    aWriter->Store( mRGBFilterEnabled ? VAL_TRUE : VAL_FALSE, TAG_FILTERRGBENABLED );

    // Gain R
    lSS.str( "" );
    lSS << mRGBFilter.GetGainR();
    aWriter->Store( lSS.str().c_str(), TAG_REDGAIN );

    // Gain G
    lSS.str( "" );
    lSS << mRGBFilter.GetGainG();
    aWriter->Store( lSS.str().c_str(), TAG_GREENGAIN );

    // Gain B
    lSS.str( "" );
    lSS << mRGBFilter.GetGainB();
    aWriter->Store( lSS.str().c_str(), TAG_BLUEGAIN );

    // Offset R
    lSS.str( "" );
    lSS << mRGBFilter.GetOffsetR();
    aWriter->Store( lSS.str().c_str(), TAG_REDOFFSET );

    // Offset G
    lSS.str( "" );
    lSS << mRGBFilter.GetOffsetG();
    aWriter->Store( lSS.str().c_str(), TAG_GREENOFFSET );

    // Offset B
    lSS.str( "" );
    lSS << mRGBFilter.GetOffsetB();
    aWriter->Store( lSS.str().c_str(), TAG_BLUEOFFSET );

    // Bayer filtering
    lStr = VAL_BAYERSIMPLE;
    switch ( mBayerFilter )
    {
    case PvBayerFilterSimple:
        lStr = VAL_BAYERSIMPLE;
        break;

    case PvBayerFilter3X3:
        lStr = VAL_BAYER3X3;
        break;

    default:
        assert( 0 );
    }
    aWriter->Store( lStr.c_str(), TAG_BAYERFILTERING );

	// Range filtering
	mRangeFilter.Save( aWriter );

    // Tap geometry
    uint32_t lTapCount = PvTapFilter::GetSupportedGeometryCount();
    for ( uint32_t i = 0; i < lTapCount; i++ )
    {
        if ( PvTapFilter::GetSupportedGeometryValue( i ) == mTapGeometry )
        {
            aWriter->Store( PvTapFilter::GetSupportedGeometryName( i ), TAG_TAPGEOMETRY );
            break;
        }
    }

    return PvResult::Code::OK;
}


///
/// \brief Persistence load
///

PvResult ImageFiltering::Load( PvConfigurationReader *aReader )
{
    PvResult lResult;
    PvString lPvStr;
    double lValueD = 0.0;
    int32_t lValueI = 0;

    // Always load from a blank setup!
    mRGBFilterEnabled = false;
    mRGBFilter.Reset();
    mBayerFilter = PvBayerFilterSimple;

    // RGB filter enabled
    lResult = aReader->Restore( PvString( TAG_FILTERRGBENABLED ), lPvStr );
    if ( lResult.IsOK() )
    {
        mRGBFilterEnabled = ( lPvStr == VAL_TRUE );
    }

    // Gains
    lResult = aReader->Restore( PvString( TAG_REDGAIN ), lPvStr );
    if ( lResult.IsOK() )
    {
        std::stringstream lSS( lPvStr.GetAscii() );
        lSS >> lValueD;
        mRGBFilter.SetGainR( lValueD );
    }
    lResult = aReader->Restore( PvString( TAG_GREENGAIN ), lPvStr );
    if ( lResult.IsOK() )
    {
        std::stringstream lSS( lPvStr.GetAscii() );
        lSS >> lValueD;
        mRGBFilter.SetGainG( lValueD );
    }
    lResult = aReader->Restore( PvString( TAG_BLUEGAIN ), lPvStr );
    if ( lResult.IsOK() )
    {
        std::stringstream lSS( lPvStr.GetAscii() );
        lSS >> lValueD;
        mRGBFilter.SetGainB( lValueD );
    }

    // Offsets
    lResult = aReader->Restore( PvString( TAG_REDOFFSET ), lPvStr );
    if ( lResult.IsOK() )
    {
        std::stringstream lSS( lPvStr.GetAscii() );
        lSS >> lValueI;
        mRGBFilter.SetOffsetR( lValueI );
    }
    lResult = aReader->Restore( PvString( TAG_GREENOFFSET ), lPvStr );
    if ( lResult.IsOK() )
    {
        std::stringstream lSS( lPvStr.GetAscii() );
        lSS >> lValueI;
        mRGBFilter.SetOffsetG( lValueI );
    }
    lResult = aReader->Restore( PvString( TAG_BLUEOFFSET ), lPvStr );
    if ( lResult.IsOK() )
    {
        std::stringstream lSS( lPvStr.GetAscii() );
        lSS >> lValueI;
        mRGBFilter.SetOffsetB( lValueI );
    }

    // Bayer filtering
    lResult = aReader->Restore( PvString( TAG_BAYERFILTERING ), lPvStr );
    if ( lResult.IsOK() )
    {
        if ( lPvStr == VAL_BAYERSIMPLE )
        {
            mBayerFilter = PvBayerFilterSimple;
        }
        else if ( lPvStr == VAL_BAYER3X3 )
        {
            mBayerFilter = PvBayerFilter3X3;
        }
    }

	// Range filtering
	mRangeFilter.Load( aReader );

    // Tap geometry
    mTapGeometry = PvTapGeometryUnknown;
    lResult = aReader->Restore( PvString( TAG_TAPGEOMETRY ), lPvStr );
    if ( lResult.IsOK() )
    {
        uint32_t lTapCount = PvTapFilter::GetSupportedGeometryCount();
        for ( uint32_t i = 0; i < lTapCount; i++ )
        {
            if ( PvTapFilter::GetSupportedGeometryName( i ) == lPvStr )
            {
                mTapGeometry = PvTapFilter::GetSupportedGeometryValue( i );
                break;
            }
        }
    }

    return PvResult::Code::OK;
}


///
/// \brief Configures a buffer converter using the current filtering configuration
///

void ImageFiltering::ConfigureConverter( PvBufferConverter &aConverter )
{
    aConverter.SetBayerFilter( mBayerFilter );
    if ( mRGBFilterEnabled )
    {
        aConverter.SetRGBFilter( mRGBFilter );
    }
    else
    {
        aConverter.ResetRGBFilter();
    }
}


///
/// \brief Performs white balance on the last acquired image
///

void ImageFiltering::WhiteBalance( DisplayThread *aDisplayThread )
{
    aDisplayThread->WhiteBalance( &mRGBFilter );
}


///
/// \brief Resets the content of the RGB filter
///

void ImageFiltering::ResetRGBFilter()
{
    mRGBFilter.Reset();
}


