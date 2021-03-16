// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <IMessageSink.h>
#include <IProgress.h>
#include <LogBuffer.h>
#include <Setup.h>

#include <PvAcquisitionStateManager.h>
#include <PvDevice.h>
#include <PvDeviceInfo.h>
#include <PvGenStateStack.h>
#include <PvStream.h>
#include <PvDeviceAdapter.h>
#include <PvLogger.h>

#include <vector>
#include <list>


class Stream;


struct ComboItem
{
    ComboItem()
        : mValue( -1 )
        , mSelected( false )
    {
    }

    PvString mName;
    int64_t mValue;
    bool mSelected;
};

struct ControlsState
{
    ControlsState()
    {
        Reset();
    }

    void Reset()
    {
        mStart = false;
        mStop = false;
        mAcquisitionMode = false;
        mSource = false;
    }

    bool mStart;
    bool mStop;
    bool mAcquisitionMode;
    bool mSource;
};


typedef std::vector<int64_t> Int64Vector;
typedef std::vector<ComboItem> ComboItemVector;
typedef std::list<PvString> StringList;

#ifndef WM_USER
#define WM_USER ( 0 )
#endif // WM_USER

#define WM_LINKDISCONNECTED ( WM_USER + 0x3000 )
#define WM_LINKRECONNECTED ( WM_USER + 0x3001 )
#define WM_UPDATESOURCE ( WM_USER + 0x3002 )
#define WM_UPDATESOURCES ( WM_USER + 0x3003 )
#define WM_UPDATEACQUISITIONMODE ( WM_USER + 0x3004 )
#define WM_UPDATEACQUISITIONMODES ( WM_USER + 0x3005 )
#define WM_ACQUISITIONSTATECHANGED ( WM_USER + 0x3006 )


class Device : public Persistent, PvGenEventSink, PvDeviceEventSink, PvAcquisitionStateEventSink
#ifdef _AFXDLL
    , public CObject
#endif // _AFXDLL
{
#ifdef _AFXDLL
    DECLARE_DYNAMIC( Device )
#endif // _AFXDLL

public:

    Device( IMessageSink *aSink, LogBuffer *aLogBuffer );
    virtual ~Device();

    static Device *Create( const PvDeviceInfo *aDeviceInfo, IMessageSink *aSink, LogBuffer *aLogBuffer );

    PvDevice *GetDevice() { return mDevice; }
    IPvDeviceAdapter *GetDeviceAdapter() { return mDeviceAdapter; }

    void CompleteConnect( PvStream *aStream );
    bool ChangeSource( int64_t aSource );
    void CompleteChangeSource( PvStream *aStream, int64_t aSource );

    PvResult Disconnect();
    bool IsConnected();
    bool IsTransmitter() { return ( mClass == PvDeviceClassTransmitter ) || ( mClass == PvDeviceClassTransceiver ); }
    bool IsInRecovery() { return mRecovery; }
    bool IsStreaming();
    bool IsSerialSupported() { return mSerialSupported; }
    bool IsAcquisitionLocked();
    bool IsFrameGrabber() const { return mFrameGrabber; }
    virtual bool IsUnconditionalStreamingEnabled() { return false; }

    void GetControlsEnabled( ControlsState &aState );
    bool IsMultiSourceTransmitter() const { return mIsMultiSourceTransmitter; }

    PvGenParameterArray *GetCommunicationParameters();
    PvGenParameterArray *GetParameters();

    PvResult StartAcquisition();
    PvResult StopAcquisitionIfApplicable();
    PvResult ForceStopAcquisition();

    void GetCurrentAcquisitionMode( ComboItem &aMode, bool &aWritable );
    void GetAcquisitionModes( ComboItemVector &aModes, bool &aWritable );
    PvResult SetAcquisitionMode( int64_t aNewMode );
    
    PvAcquisitionState GetAcquisitionState();

    void GetSources( ComboItemVector &aSources );
    int64_t GetCurrentSource();
    uint16_t GetCurrentSourceChannel();
    uint32_t GetPayloadSize() { if ( !IsConnected() ) { return 0; } return mDevice->GetPayloadSize(); }

    PvString GetDeviceXMLDefaultName();
    PvResult SaveDeviceXML( const PvString &aFileName );

    // Persistent
    PvResult Save( PvConfigurationWriter *aWriter );
    PvResult Load( PvConfigurationReader *aReader );

    int64_t GetSelectedSource();

    void ResetRecovery() { mRecovery = false; }
    void ResetUpdatingAcquisitionMode() { mUpdatingAcquisitionMode = false; }
    void ResetUpdatingSources() { mUpdatingSources = false; }

    virtual PvResult Connect( IProgress *aProgress, Setup *aSetup, const PvDeviceInfo *aDeviceInfo, uint16_t &aChannel, const PvString &aCommunicationParameters ) = 0;
    virtual PvResult SetStreamDestination( Setup *aSetup, Stream *aStream, uint16_t aChannel ) = 0;
    virtual std::string GetFindString() = 0;

    void SetClass( PvDeviceClass aClass ) { mClass = aClass; }

    void FreeAcquisitionStateManager( bool aLockMutex = true );

protected:

    void LoadCommunicationParameters( const PvString &aParameters );
    void PushSource( PvGenStateStack *aStack );
    void GetAcquisitionModes( ComboItemVector &aModes, bool &aWritable, bool aOnlyCurrent );
    void SetIsMultiSourceTransmitter();
    void DetectFrameGrabber();

    // PvGenEventSink
    void OnParameterUpdate( PvGenParameter *aParameter );

    // PvDeviceEventSink
    void OnLinkDisconnected( PvDevice *aDevice );
    void OnLinkReconnected( PvDevice *aDevice );
    void OnEvent( PvDevice *aDevice, uint16_t aEventID, uint16_t aChannel, uint64_t aBlockID, uint64_t aTimestamp, const void *aData, uint32_t aDataLength );
    void OnEventGenICam( PvDevice *aDevice, uint16_t aEventID, uint16_t aChannel, uint64_t aBlockID, uint64_t aTimestamp, PvGenParameterList *aData );
    void OnCmdLinkRead( const void *aBuffer, int64_t aAddress, int64_t aLength );
    void OnCmdLinkWrite( const void *aBuffer, int64_t aAddress, int64_t aLength );

    // PvAcquisitionStateEventSink
    void OnAcquisitionStateChanged( PvDevice* aDevice, PvStream* aStream, uint32_t aSource, PvAcquisitionState aState );

    void SetDevice( PvDevice *aDevice );

    PvLogger mLogger;

private:

    PvDevice *mDevice;
    PvDeviceAdapter *mDeviceAdapter;
    
    PvAcquisitionStateManager *mAcquisitionStateManager;
    Mutex mAcquisitionStateManagerMutex;

    PvDeviceClass mClass;

    IMessageSink *mMessageSink;
    LogBuffer *mLogBuffer;

    bool mRecovery;
    bool mUpdatingAcquisitionMode;
    bool mUpdatingSources;
    bool mSerialSupported;
    bool mFrameGrabber;
    
    ComboItemVector mAvailableSources;
    bool mIsMultiSourceTransmitter;
};


#endif // __DEVICE_H__

