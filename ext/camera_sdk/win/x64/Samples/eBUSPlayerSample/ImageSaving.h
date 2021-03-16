// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __IMAGE_SAVING_H__
#define __IMAGE_SAVING_H__

#include "Persistent.h"
#include "DisplayThread.h"


#ifdef QT_VERSION
#include <QtCore/QTime>
#endif


class ImageFiltering;
class PvBufferWriter;
class IMp4Writer;

class ImageSaving : public Persistent
{
#ifdef _AFXDLL
    DECLARE_DYNAMIC( ImageSaving )
#endif // _AFXDLL

public:

    ImageSaving( ImageFiltering *aImageFiltering );
    ~ImageSaving();

    // Persistent
    PvResult Save( PvConfigurationWriter *aWriter );
    PvResult Load( PvConfigurationReader *aReader );

    void Reset();
    void ResetStats();

    typedef enum
    {
        ThrottleOneOutOf = 0,
        ThrottleMaxRate = 1,
        ThrottleAverageThroughput = 2,
        ThrottleNone = 3

    } Throttle;

    typedef enum
    {
        FormatBmp = 0,
        FormatRaw = 1,
        FormatTiff = 2,
        FormatMp4 = 3

    } Format;

    bool GetEnabled() const { return mEnabled; }
    Throttle GetThrottling() const { return mThrottling; }
    uint32_t GetOneOutOf() const { return mOneOutOf; }
    uint32_t GetMaxRate() const { return mMaxRate; }
    uint32_t GetAverageThroughput() const { return mAverageThroughput; }
    PvString GetPath() const { return mPath.c_str(); }
    Format GetFormat() const { return mFormat; }
    uint32_t GetAvgBitrate() const;
        
    void SetEnabled( bool aValue ) { mEnabled = aValue; }
    void SetThrottling( Throttle aValue ) { mThrottling = aValue; }
    void SetOneOutOf( uint32_t aValue ) { mOneOutOf = aValue; }
    void SetMaxRate( uint32_t aValue ) { mMaxRate = aValue; }
    void SetAverageThroughput( uint32_t aValue ) { mAverageThroughput = aValue; }
    void SetPath( const PvString &aValue ) { mPath = aValue.GetAscii(); }
    void SetFormat( Format aValue ) { mFormat = aValue; }
    void SetAvgBitrate( uint32_t aValue );
    
    double GetFPS() { return mFPS; }
    double GetMbps() { return mMbps; }
    int32_t GetFrames() { return mFrames; }
    int64_t GetTotalSize() { return mTotalSize / 1048576; }

    bool SavePure( PvBuffer *aBuffer );
    bool SaveDisplay( PvBuffer *aBuffer );

    bool SaveCurrentImage( DisplayThread *aDisplayThread );

    bool IsFormatVideo();
    bool IsMp4Supported();
    void NotifyStreamingStop();
	void StopMp4();

    void GetLastError( PvString &aString );

protected:

    ImageSaving(); // Not implemented

    bool SaveIfNecessary( PvBuffer *aBuffer );
    bool SaveImage( PvBuffer *aRawBuffer, bool aUpdateStats );

    void GetPath( PvBuffer *aBuffer, std::string &aLocation, std::string &aFilename );
    void UpdateStats( PvBuffer *aBuffer, uint32_t aLastSize );

    bool SaveMp4( PvBuffer *aBuffer );

private:

    bool mEnabled;
    double mFPS;
    double mMbps;
    uint32_t mFrames;
    int64_t mTotalSize;

    Throttle mThrottling;

    uint32_t mOneOutOf;
    uint32_t mMaxRate;
    uint32_t mAverageThroughput;

#ifdef QT_VERSION
    QTime mTimer;
    QTime mTimerLong;
#else
    int64_t mStartTime;
    int64_t mElapsedTime;
#endif

    int64_t mCapturedSince;
    int64_t mPrevious;

    Format mFormat;

    PvBufferWriter *mBufferWriter;

    std::string mPath;

    uint32_t mCount;
    bool mStopped;

    ImageFiltering *mImageFiltering;

    // Used for MP4/H.264
    IMp4Writer *mMp4Writer;
    bool mMp4WriterOpenFailed;

};


#endif // __IMAGE_SAVING_H__

