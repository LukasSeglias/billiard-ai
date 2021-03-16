// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "eBUSPlayerShared.h"
#include "BufferOptions.h"
#include "Player.h"

#include <assert.h>
#include <sstream>


#define DEFAULT_BUFFER_COUNT ( 16 )
#define DEFAULT_BUFFER_SIZE ( 1920 * 1080 * 2 )
#define DEFAULT_AUTO_RESIZE ( true )

#define WARNING_MEM_THRESHOLD ( 1 * 1024 * 1024 * 1024 ) // 1 GB


#define BUFFEROPTIONSDLG_VERSION ( "1.0.0.0" )

#define TAG_VERSION ( "bufferoptionsdlgversion" )
#define TAG_STREAMINGBUFFERCOUNT ( "streamingbuffercount" )
#define TAG_STREAMINGDEFAULTBUFFERSIZE ( "streamingdefaultbuffersize" )
#define TAG_AUTORESIZE ( "streamingautoresize" )


#ifdef _AFXDLL
    IMPLEMENT_DYNAMIC( BufferOptions, CObject )
#endif // _AFXDLL


///
/// \brief Constructor
///

BufferOptions::BufferOptions()
    : mBufferCount( DEFAULT_BUFFER_COUNT )
    , mBufferSize( DEFAULT_BUFFER_SIZE )
    , mAutoResize( DEFAULT_AUTO_RESIZE )
{

}


///
/// \brief Constructor
///

BufferOptions::~BufferOptions()
{
    
}


///
/// \brief Equal operator
///

void BufferOptions::operator=( const BufferOptions& aObjectSource )
{
    mBufferCount = aObjectSource.mBufferCount;
    mBufferSize = aObjectSource.mBufferSize;
    mAutoResize = aObjectSource.mAutoResize;
}


///
/// \brief Persistence save
///

PvResult BufferOptions::Save( PvConfigurationWriter *aWriter )
{
    std::stringstream lSS;

    // Save a version string, just in case we need it in the future
    aWriter->Store( BUFFEROPTIONSDLG_VERSION, TAG_VERSION );

    // Buffer count
    lSS << mBufferCount;
    aWriter->Store( lSS.str().c_str(), TAG_STREAMINGBUFFERCOUNT );

    // Default buffer size
    lSS.str( "" );
    lSS << mBufferSize;
    aWriter->Store( lSS.str().c_str(), TAG_STREAMINGDEFAULTBUFFERSIZE );

    // Auto resizing of buffers
    lSS.str( "" );
    lSS << mAutoResize;
    aWriter->Store( lSS.str().c_str(), TAG_AUTORESIZE );

    return PvResult::Code::OK;
}


///
/// \brief Persistence load
///

PvResult BufferOptions::Load( PvConfigurationReader *aReader )
{
    PvResult lResult;
    PvString lPvStr;

    mBufferCount = DEFAULT_BUFFER_COUNT;
    lResult = aReader->Restore( TAG_STREAMINGBUFFERCOUNT, lPvStr );
    if ( lResult.IsOK() )
    {
        std::stringstream lSS( lPvStr.GetAscii() );
        lSS >> mBufferCount;
    }

    mBufferSize = DEFAULT_BUFFER_SIZE;
    lResult = aReader->Restore( TAG_STREAMINGDEFAULTBUFFERSIZE, lPvStr );
    if ( lResult.IsOK() )
    {
        std::stringstream lSS( lPvStr.GetAscii() );
        lSS >> mBufferSize;
    }

    mAutoResize = DEFAULT_AUTO_RESIZE;
    lResult = aReader->Restore( TAG_AUTORESIZE, lPvStr );
    if ( lResult.IsOK() )
    {
        // Comparing to false as the default, usual behavior is true
        mAutoResize = lPvStr != "false";
    }

    return PvResult::Code::OK;
}


///
/// \brief Help for buffer count parameter
///

PvString BufferOptions::GetHelpBufferCount() const
{
    return PvString(
        "Increasing the buffer count can make streaming more tolerant "
        "to missing block IDs, but at the expense of using more memory "
        "and increasing latency.\r\n\r\n"

        "Using more than 16 buffers is typically used in high frame rate, "
        "small buffer size applications.\r\n\r\n"

        "Applications using low frame rates or using very large buffers "
        "are not as sensitive to missing block IDs and can thus save "
        "memory and latency by only using 4 or 8 buffers."
    );
}
    

///
/// \brief Help for buffer size parameter
///

PvString BufferOptions::GetHelpBufferSize() const
{
    return PvString(
        "The default buffer size is used to allocate acquisition pipeline "
        "buffers when it is not possible to read the payload "
        "size directly from the device.\r\n\r\n"

        "The default buffer size can be calculated using this formula:\r\n"
        "(((width * pixel bytes) + padding x) * height) + padding y."
    );
}


///
/// \brief Help for auto resizing parameter
///

PvString BufferOptions::GetHelpAutoResizing() const
{
    return PvString(
        "If enabled, buffers are automatically resized by the acquisition "
        "pipeline when the BUFFER_TOO_SMALL operation result is returned."
    );
}


///
/// \brief Returns true if the pipeline is not in sync with our parameters
///

bool BufferOptions::HasChanged( PvPipeline *aPipeline )
{
    return ( mBufferCount != aPipeline->GetBufferCount() ) ||
        ( mBufferSize != aPipeline->GetBufferSize() ) ||
        ( mAutoResize != aPipeline->GetHandleBufferTooSmall() );
}


///
/// \brief Returns true if the current options would allocate memory above our comfort threshold
///

bool BufferOptions::ShowWarning()
{
    uint64_t lTotal = mBufferSize * mBufferCount;
    return ( lTotal >= WARNING_MEM_THRESHOLD );
}


///
/// \brief Loads the buffer configuration from pipeline object
///

void BufferOptions::Init( PvPipeline *aPipeline )
{
    mBufferCount = aPipeline->GetBufferCount();
    mBufferSize = aPipeline->GetBufferSize();
    mAutoResize = aPipeline->GetHandleBufferTooSmall();
}


///
/// \brief Applies the parameters to the pipeline object
///

PvResult BufferOptions::Apply( PvPipeline *aPipeline )
{
    // Apply parameters
    aPipeline->SetBufferSize( mBufferSize );
    aPipeline->SetHandleBufferTooSmall( mAutoResize );
    PvResult lResult = aPipeline->SetBufferCount( mBufferCount );

    // aPlayer->StartStreaming();

    if ( !lResult.IsOK() )
    {   
        std::stringstream lSS;
        lSS << "Could not change pipeline configuration to requested values.\r\n";
        lSS << "Only " << aPipeline->GetBufferCount() << " buffers could be allocated.";

        return PvResult( lResult.GetCode(), lSS.str().c_str() );
    }

    return PvResult::Code::OK;
}


