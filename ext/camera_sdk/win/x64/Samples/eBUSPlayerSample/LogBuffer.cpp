// *****************************************************************************
//
//     Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "eBUSPlayerShared.h"
#include "LogBuffer.h"

#include <PvAppUtils.h>

#ifdef WIN32
#include <shlobj.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif

#include <assert.h>
#include <string.h>
#include <fstream>
#include <iomanip>


#define LOGBUFFER_VERSION ( "1.0.0.0" )

#define TAG_VERSION ( "logbufferversion" )
#define TAG_GENICAMENABLED ( "loggenicamenabled" )
#define TAG_EVENTSENABLED ( "logeventsenabled" )
#define TAG_BUFFERALLENABLED ( "logbufferallenabled" )
#define TAG_CHUNKENABLED ( "chunkenabled" )
#define TAG_BUFFERERRORENABLED ( "logbuffererrorenabled" )
#define TAG_LOGFILENAME ( "logfilename" )
#define TAG_WRITETOFILEENABLED ( "logwritetofileenabled" )
#define TAG_SERIALCOMLOGENABLED ( "logserialcomenabled" )
#define TAG_LOGSINKENABLED ( "logsinkenabled" )

#define VAL_TRUE ( "true" )
#define VAL_FALSE ( "false" )


#define MAX_LOG_SIZE ( 1024 )
#define TERMINAL_COLUMNS ( 8 )


#ifdef _AFXDLL
    IMPLEMENT_DYNAMIC( LogBuffer, CObject )
#endif // _AFXDLL


std::string PortToStr( PvDeviceSerial aPort )
{
    switch ( aPort )
    {
    case PvDeviceSerial0: 
        return "Serial0";

    case PvDeviceSerial1:
        return "Serial1";

    case PvDeviceSerialBulk0:
        return "Bulk0";

    case PvDeviceSerialBulk1:
        return "Bulk1";

    case PvDeviceSerialBulk2:
        return "Bulk2";

    case PvDeviceSerialBulk3:
        return "Bulk3";

    case PvDeviceSerialBulk4:
        return "Bulk4";

    case PvDeviceSerialBulk5:
        return "Bulk5";

    case PvDeviceSerialBulk6:
        return "Bulk6";

    case PvDeviceSerialBulk7:
        return "Bulk7";
        
    default:
        break;
    }

    // assert( 0 );
    return "Unknown";
}


///
/// \brief Constructor
///

LogBuffer::LogBuffer()
    : mGenICamEnabled( true )
    , mEventsEnabled( false )
    , mBufferAllEnabled( false )
    , mChunkEnabled( false )
    , mBufferErrorEnabled( true )
    , mWriteToFileEnabled( false )
    , mSerialComLogEnabled( false )
    , mLogSinkEnabled( false )
    , mParameters( NULL )
{
    PvDeviceSerialPort::RegisterEchoSink( this );

    ResetConfig();
    Reset();
}


///
/// \brief Destructor
///

LogBuffer::~LogBuffer()
{
    PvDeviceSerialPort::UnregisterEchoSink( this );
}


///
/// \brief Resets the log buffer configuration
///

void LogBuffer::ResetConfig()
{
    mGenICamEnabled = true;
    mEventsEnabled = false;
    mBufferAllEnabled = false;
    mChunkEnabled = false;
    mBufferErrorEnabled = true;
    mWriteToFileEnabled = false;
    mSerialComLogEnabled = false;
    mLogSinkEnabled = false;

#ifdef WIN32

    char lDesktop[ MAX_PATH ];
    SHGetSpecialFolderPathA( NULL, lDesktop, CSIDL_DESKTOP, true );

    mFilename = lDesktop;
    mFilename += "\\EventMonitor.txt";

#else

    struct passwd *pw = getpwuid( getuid() );
    
    mFilename = pw->pw_dir;
    mFilename += "/Desktop/EventMonitor.txt";

#endif //_LINUX_

}


///
/// \brief Resets the buffer
///

void LogBuffer::Reset()
{
    mMutex.Lock();
    //////////////////////////////////

#ifdef QT_VERSION
    mTimer.start();
#else
    mStartTimestamp = ::GetTickCount();
#endif

    mBuffer.clear();

    //////////////////////////////////
    mMutex.Unlock();
}


///
/// \brief Add a string to the log buffer
///

void LogBuffer::Log( const std::string &aItem )
{
#ifdef _AFXDLL
#endif

#ifdef QT_VERSION
    int lElapsed = mTimer.elapsed();
#else
    int64_t lElapsed = ::GetTickCount() - mStartTimestamp;
#endif

    std::stringstream lStr;
    if ( lElapsed >= 0 )
    {
        lStr << std::setfill( '0' ) << std::setw( 4 ) << ( lElapsed / 1000 );
        lStr << ".";
        lStr << std::setfill( '0' ) << std::setw( 3 ) << ( lElapsed % 1000 );
    }
    else
    {
        lStr << "0";
    }

    lStr << "    " << aItem;

    mMutex.Lock();
    //////////////////////////////////

    mBuffer.push_back( lStr.str() );
    while ( mBuffer.size() > MAX_LOG_SIZE )
    {
        mBuffer.pop_front();
    }

    if ( mWriteToFileEnabled )
    {
        std::ofstream lFile;

#ifdef WIN32
		std::wstring lFilename = mFilename.GetUnicode();
#else
		const char *lFilename = mFilename.GetAscii();
#endif // WIN32

        lFile.open( lFilename, std::ios_base::app );
        if ( lFile.is_open() )
        {
            lFile << lStr.str() << "\n";
            lFile.close();
        }
    }

    //////////////////////////////////
    mMutex.Unlock();
}


///
/// \brief Locks the buffer
///

std::list<std::string> &LogBuffer::Lock()
{
    mMutex.Lock();
    //////////////////////////////////

    return mBuffer;
}


///
/// \brief Unlocks the buffer
///

void LogBuffer::Unlock()
{
	mBuffer.clear();

    //////////////////////////////////
    mMutex.Unlock();
}


///
/// \brief Sets the name of the file where we output the logging events
///

void LogBuffer::SetFilename( const PvString &aFilename ) 
{ 
    mMutex.Lock();
    //////////////////////////////////

    mFilename = aFilename; 

    //////////////////////////////////
    mMutex.Unlock();
}


///
/// \brief Enable/disable logging to file
///

void LogBuffer::SetWriteToFileEnabled( bool aEnabled ) 
{ 
    mMutex.Lock();
    //////////////////////////////////

    mWriteToFileEnabled = aEnabled; 

    //////////////////////////////////
    mMutex.Unlock();
}


///
/// \brief Save the current logging configuration
///

PvResult LogBuffer::Save( PvConfigurationWriter *aWriter )
{
    // Save a version string, just in case we need it in the future
    aWriter->Store( LOGBUFFER_VERSION, TAG_VERSION );

    // bool mGenICamEnabled;
    aWriter->Store( mGenICamEnabled ? VAL_TRUE : VAL_FALSE, TAG_GENICAMENABLED );

    // bool mEventsEnabled;
    aWriter->Store( mEventsEnabled ? VAL_TRUE : VAL_FALSE, TAG_EVENTSENABLED );

    // bool mBufferErrorEnabled;
    aWriter->Store( mBufferErrorEnabled ? VAL_TRUE : VAL_FALSE, TAG_BUFFERERRORENABLED );

    // bool mBufferAllEnabled;
    aWriter->Store( mBufferAllEnabled ? VAL_TRUE : VAL_FALSE, TAG_BUFFERALLENABLED );

    // bool mChunkEnabled;
    aWriter->Store( mChunkEnabled ? VAL_TRUE : VAL_FALSE, TAG_CHUNKENABLED );

    // CString mLogFilename
    aWriter->Store( mFilename, TAG_LOGFILENAME );

    // bool mWriteToFileEnabled;
    aWriter->Store( mWriteToFileEnabled ? VAL_TRUE : VAL_FALSE, TAG_WRITETOFILEENABLED );

    // bool mSerialComLogEnabled
    aWriter->Store( mSerialComLogEnabled ? VAL_TRUE : VAL_FALSE, TAG_SERIALCOMLOGENABLED );

    // bool mLogSinkEnabled
    aWriter->Store( mLogSinkEnabled ? VAL_TRUE : VAL_FALSE, TAG_LOGSINKENABLED );

    return PvResult::Code::OK;
}


///
/// \brief Load the logging configuration
///

PvResult LogBuffer::Load( PvConfigurationReader *aReader )
{
    PvResult lResult;
    PvString lPvStr;

    // Always load from a blank setup!
    ResetConfig();

    // bool mGenICamEnabled;
    lResult = aReader->Restore( TAG_GENICAMENABLED, lPvStr );
    if ( lResult.IsOK() )
    {
        mGenICamEnabled = ( lPvStr == VAL_TRUE );
    }

    // bool mEventsEnabled;
    lResult = aReader->Restore( TAG_EVENTSENABLED, lPvStr );
    if ( lResult.IsOK() )
    {
        mEventsEnabled = ( lPvStr == VAL_TRUE );
    }

    // bool mBufferErrorEnabled;
    lResult = aReader->Restore( TAG_BUFFERERRORENABLED, lPvStr );
    if ( lResult.IsOK() )
    {
        mBufferErrorEnabled = ( lPvStr == VAL_TRUE );
    }

    // bool mBufferAllEnabled;
    lResult = aReader->Restore( TAG_BUFFERALLENABLED, lPvStr );
    if ( lResult.IsOK() )
    {
        mBufferAllEnabled = ( lPvStr == VAL_TRUE );
    }

    // bool mChunkEnabled;
    lResult = aReader->Restore( TAG_CHUNKENABLED, lPvStr );
    if ( lResult.IsOK() )
    {
        mChunkEnabled = ( lPvStr == VAL_TRUE );
    }

    // CString mFilename;
    lResult = aReader->Restore( TAG_LOGFILENAME, lPvStr );
    if ( lResult.IsOK() )
    {
        SetFilename( lPvStr.GetUnicode() );
    }

    // bool mWriteToFileEnabled;
    lResult = aReader->Restore( TAG_WRITETOFILEENABLED, lPvStr );
    if ( lResult.IsOK() )
    {
        SetWriteToFileEnabled( lPvStr == VAL_TRUE );
    }

    // bool mSerialComLogEnabled
    lResult = aReader->Restore( TAG_SERIALCOMLOGENABLED, lPvStr );
    if ( lResult.IsOK() )
    {
        mSerialComLogEnabled = ( lPvStr == VAL_TRUE );
    }

    // bool mLogSinkEnabled
    lResult = aReader->Restore( TAG_LOGSINKENABLED, lPvStr );
    if ( lResult.IsOK() )
    {
        mLogSinkEnabled = ( lPvStr == VAL_TRUE );
    }

    return PvResult::Code::OK;
}


///
/// \brief Enable device serial communication logging
///

void LogBuffer::SetSerialComLogEnabled( bool aEnabled )
{
    mSerialComLogEnabled = aEnabled;
}


///
/// \brief Sets a parameter array we will be monitoring for logging
///

void LogBuffer::SetParameters( PvGenParameterArray *aParameters )
{
    if ( aParameters == NULL )
    {
        mInfoMap.clear();
        mParameters = NULL;
        return;
    }

    assert( mInfoMap.size() <= 0 );
    assert( mParameters == NULL );

    mParameters = aParameters;
    for ( uint32_t i = 0; i < mParameters->GetCount(); i++ )
    {
        PvGenParameter *lParameter = mParameters->Get( i );

        // We only display invisible parameter events when in debug mode
#ifndef _DEBUG
        PvGenVisibility lVisibility = PvGenVisibilityUndefined;
        lParameter->GetVisibility( lVisibility );
        if ( lVisibility < PvGenVisibilityInvisible )
#endif // _DEBUG
        {
            PvString lName;
            lParameter->GetName( lName );

            mInfoMap[ lName.GetAscii() ].Initialize( lParameter, this );
        }
    }
}


///
/// \brief Enables GenApi logging
///

void LogBuffer::EnableGenICamMonitoring( bool aEnabled )
{
    ParameterInfoMap::iterator lIt = mInfoMap.begin();
    while ( lIt != mInfoMap.end() )
    {
        lIt->second.Enable( aEnabled );
        lIt++;
    }
}


///
/// \brief eBUS SDK log sink: we take log events and them to our log buffer
///

void LogBuffer::Log( PvLogLevelEnum aLevel, const char *aFile, uint32_t aLine, const char *aFunction, const char *aCategory, const char *aMessage )
{
    if ( !mLogSinkEnabled )
    {
        return;
    }

    int lLevelIndex = 0;
    const char *cLogLevels[] = { "UNKNOWN", "INFO", "ERROR", "WARNING", "CRITICAL", "DEBUG" };
    switch ( aLevel )
    {
    case PvLogLevelInfo:
        lLevelIndex = 1;
        break;

    case PvLogLevelError:
        lLevelIndex = 2;
        break;

    case PvLogLevelWarning:
        lLevelIndex = 3;
        break;

    case PvLogLevelCritical:
        lLevelIndex = 4;
        break;

    case PvLogLevelDebug:
        lLevelIndex = 5;
        break;

    default:
        break;
    }

    std::stringstream lSS;
    lSS << "[" << cLogLevels[ lLevelIndex ] << "|" << aCategory << "] " << aMessage;

    Log( lSS.str() );
}


///
/// \brief Echo of all device Tx serial activity in the process
///

void LogBuffer::OnTransmitted( const PvString &aDeviceID, PvDeviceSerial aPort, const uint8_t *aBytes, uint32_t aByteCount, uint64_t aTimestamp )
{
    if ( !mSerialComLogEnabled )
    {
        return;
    }

    // Get Rx buffer
    std::string lKey = aDeviceID.GetAscii() + static_cast<int>( aPort );
    RxBufferMap::iterator lIt = mRxBufferMap.find( lKey );
    if ( lIt != mRxBufferMap.end() )
    {
        // Force Rx log flush before we add Tx data
        FlushRxBuffer( lIt );
    }

    Log( std::string( "  " ) + PortToStr( aPort ) + " Tx:\r\n" + PvBinaryFormat( "    ", aBytes, aByteCount, 0 ).GetAscii() );
}


///
/// \brief Echo of all device Rx serial activity in the process
///

void LogBuffer::OnRead( const PvString &aDeviceID, PvDeviceSerial aPort, const uint8_t *aBytes, uint32_t aByteCount, uint64_t aTimestamp )
{
    if ( !mSerialComLogEnabled )
    {
        return;
    }

    if ( aByteCount > 0 )
    {
        // Get Rx buffer
        std::string lKey = aDeviceID.GetAscii() + static_cast<int>( aPort );
        RxBufferMap::iterator lIt = mRxBufferMap.find( lKey );
        if ( lIt == mRxBufferMap.end() )
        {
            mRxBufferMap[ lKey ] = RxBuffer();
            lIt = mRxBufferMap.find( lKey );
        }

        if ( lIt->second.mDataSize == 0 )
        {
            // Empty, first data
            lIt->second.mStartTime = aTimestamp;
            lIt->second.mPort = aPort;

            // Read up to RX_BUFFER_MAX
            uint32_t lByteCount = aByteCount;
            if ( aByteCount > RX_BUFFER_MAX )
            {
                lByteCount = RX_BUFFER_MAX;
            }

            // Copy data
            lIt->second.mDataSize += lByteCount;
            for ( uint32_t i = 0; i < lByteCount; i++ )
            {
                lIt->second.mData[ i ] = aBytes[ i ];
            }
            lIt->second.mDataSize = lByteCount;
            assert( lIt->second.mDataSize <= RX_BUFFER_MAX );
        }
        else
        {
            // Append up to RX_BUFFER_MAX
            uint32_t lByteCount = aByteCount;
            if ( ( lByteCount + lIt->second.mDataSize ) > RX_BUFFER_MAX )
            {
                lByteCount = RX_BUFFER_MAX - lIt->second.mDataSize;
            }

            // Copy data
            for ( uint32_t i = 0; i < lByteCount; i++ )
            {
                lIt->second.mData[ i + lIt->second.mDataSize ] = aBytes[ i ];
            }
            lIt->second.mDataSize += lByteCount;
            assert( lIt->second.mDataSize <= RX_BUFFER_MAX );
        }
    }

    FlushFullOrStale( aTimestamp );
}


///
/// \brief Flushes full or stale Rx buffers
///

void LogBuffer::FlushFullOrStale( uint64_t aTimestamp )
{
    RxBufferMap::iterator lIt = mRxBufferMap.begin();
    while ( lIt != mRxBufferMap.end() )
    {
        if ( lIt->second.mDataSize > 0 )
        {
            if ( ( lIt->second.mDataSize >= RX_BUFFER_MAX ) ||
                 ( ( aTimestamp - lIt->second.mStartTime ) >= RX_BUFFER_STALE ) )
            {
                FlushRxBuffer( lIt );
            }
        }

        lIt++;
    }
}


///
/// \brief Flushes a Rx buffer to the log
///

void LogBuffer::FlushRxBuffer( RxBufferMap::iterator &aIt )
{
    if ( aIt->second.mDataSize > 0 )
    {
        Log( std::string( "  " ) + PortToStr( aIt->second.mPort ) + " Rx:\r\n" + PvBinaryFormat( "    ", aIt->second.mData, aIt->second.mDataSize, 0 ).GetAscii() );
        aIt->second.Reset();
    }
}


///
/// \brief Notification from serial port that a read operation is complete
///

void LogBuffer::OnReadComplete( const PvString &aDeviceID, PvDeviceSerial aPort, uint64_t aTimestamp )
{
    // Get Rx buffer
    std::string lKey = aDeviceID.GetAscii() + static_cast<int>( aPort );
    RxBufferMap::iterator lIt = mRxBufferMap.find( lKey );
    if ( lIt != mRxBufferMap.end() )
    {
        FlushRxBuffer( lIt );
    }
}



