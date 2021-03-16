// *****************************************************************************
//
//     Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __LOG_BUFFER_H__
#define __LOG_BUFFER_H__

#include "Persistent.h"

#include <ParameterInfo.h>
#include <Mutex.h>

#include <PvConfigurationReader.h>
#include <PvConfigurationWriter.h>
#include <PvGenParameterArray.h>
#include <PvLogger.h>
#include <PvDeviceSerialPortEchoSink.h>

#include <list>
#include <map>

#ifdef _LINUX_
#include <QtCore/QTime>
#endif // _AFXDLL


#define RX_BUFFER_MAX ( 1024 )
#define RX_BUFFER_STALE ( 250 )

struct RxBuffer
{
    RxBuffer()
    {
        Reset();
    }

    void Reset()
    {
        mDataSize = 0;
        mStartTime = 0;
        mPort = PvDeviceSerialInvalid;
    }

    uint8_t mData[ RX_BUFFER_MAX ];
    uint32_t mDataSize;

    PvDeviceSerial mPort;
    uint64_t mStartTime;
};

typedef std::map<std::string, ParameterInfo> ParameterInfoMap;
typedef std::map<std::string, RxBuffer> RxBufferMap;


class LogBuffer : public PvLogSink, public Persistent, PvDeviceSerialPortEchoSink
{
#ifdef _AFXDLL
    DECLARE_DYNAMIC( LogBuffer )
#endif

public:

    LogBuffer();
    ~LogBuffer();

    void Log( const std::string &aItem );
    
    std::list<std::string> &Lock();
    void Unlock();

    void EnableGenICamMonitoring( bool aEnabled );

    void Reset();

    void SetParameters( PvGenParameterArray *aParameters );

    void SetGenICamEnabled( bool aEnabled ) { mGenICamEnabled = aEnabled; }
    void SetEventsEnabled( bool aEnabled ) { mEventsEnabled = aEnabled; }
    void SetBufferErrorEnabled( bool aEnabled ) { mBufferErrorEnabled = aEnabled; }
    void SetBufferAllEnabled( bool aEnabled ) { mBufferAllEnabled = aEnabled; }
    void SetChunkEnabled( bool aEnabled ) { mChunkEnabled = aEnabled; }
    void SetFilename( const PvString &aFilename );
    void SetWriteToFileEnabled( bool aEnabled );
    void SetSerialComLogEnabled( bool aEnabled );
    void SetLogSinkEnabled( bool aEnabled ) { mLogSinkEnabled = aEnabled; }

    bool IsGenICamEnabled() const { return mGenICamEnabled; }
    bool IsEventsEnabled() const { return mEventsEnabled; }
    bool IsBufferErrorEnabled() const { return mBufferErrorEnabled; }
    bool IsBufferAllEnabled() const { return mBufferAllEnabled; }
    bool IsChunkEnabled() const { return mChunkEnabled; }
    PvString GetFilename() const { return mFilename; }
    bool IsWriteToFileEnabled() const { return mWriteToFileEnabled; }
    bool IsSerialComLogEnabled() const { return mSerialComLogEnabled; }
    bool IsLogSinkEnabled() const { return mLogSinkEnabled;  }

    // Persistent
    PvResult Save( PvConfigurationWriter *aWriter );
    PvResult Load( PvConfigurationReader *aReader );

protected:

    void ResetConfig();
    void FlushFullOrStale( uint64_t aTimestamp );
    void FlushRxBuffer( RxBufferMap::iterator &aIt );

    void Log( PvLogLevelEnum aLevel, const char *aFile, uint32_t aLine, const char *aFunction, const char *aCategory, const char *aMessage );
    void OnTransmitted( const PvString &aDeviceID, PvDeviceSerial aPort, const uint8_t *aBytes, uint32_t aByteCount, uint64_t aTimestamp );
    void OnRead( const PvString &aDeviceID, PvDeviceSerial aPort, const uint8_t *aBytes, uint32_t aByteCount, uint64_t aTimestamp );
    void OnReadComplete( const PvString &aDeviceID, PvDeviceSerial aPort, uint64_t aTimestamp );

private:

    Mutex mMutex;

#ifdef QT_VERSION
    QTime mTimer;
#else
    uint64_t mStartTimestamp;
#endif 

    std::list<std::string> mBuffer;

    bool mGenICamEnabled;
    bool mEventsEnabled;
    bool mBufferErrorEnabled;
    bool mBufferAllEnabled;
    bool mChunkEnabled;
    bool mSerialComLogEnabled;
    bool mLogSinkEnabled;

    PvString mFilename;
    bool mWriteToFileEnabled;

    PvGenParameterArray *mParameters;
    ParameterInfoMap mInfoMap;

    RxBufferMap mRxBufferMap;
};


#endif // __LOG_BUFFER_H__
