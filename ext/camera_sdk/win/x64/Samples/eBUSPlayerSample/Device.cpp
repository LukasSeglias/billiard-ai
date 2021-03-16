// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "eBUSPlayerShared.h"
#include "Device.h"

#include "DeviceGEV.h"
#include "DeviceU3V.h"

#include <PvAppUtils.h>
#include <PvDeviceSerialPort.h>
#include <PvGenFile.h>
#include <PvDeviceInfoGEV.h>

#include <assert.h>
#include <sstream>


#define SOURCE_SELECTOR ( "SourceSelector" )
#define GEV_DEVICE_CLASS ( "GevDeviceClass" )
#define TRANSMITTER ( "Transmitter" )
#define PIXEL_FORMAT ( "PixelFormat" )
#define WIDTH ( "Width" )
#define HEIGHT ( "Height" )
#define PAYLOAD_SIZE ( "PayloadSize" )
#define ACQUISITION_START ( "AcquisitionStart" )
#define ACQUISITION_STOP ( "AcquisitionStop" )
#define ACQUISITION_MODE ( "AcquisitionMode" )
#define DATA_LENGTH ( "DataLength" )
#define SOURCE_STREAM_CHANNEL ( "SourceStreamChannel" )
#define CHANNEL_SELECTOR ( "GevStreamChannelSelector" )
#define DEVICE_MODEL_NAME ( "DeviceModelName" )


#ifdef _AFXDLL
    IMPLEMENT_DYNAMIC( Device, CObject )
#endif // _AFXDLL


///
/// \brief Constructor
///

Device::Device( IMessageSink *aSink, LogBuffer *aLogBuffer )
    : mMessageSink( aSink )
    , mDevice( NULL )
    , mAcquisitionStateManager( NULL )
    , mUpdatingAcquisitionMode( false )
    , mUpdatingSources( false )
    , mLogBuffer( aLogBuffer )
    , mRecovery( false )
    , mSerialSupported( false )
    , mClass( PvDeviceClassUnknown )
    , mIsMultiSourceTransmitter( false )
    , mLogger( "eBUSPlayer" )
{
    assert( mMessageSink != NULL );
    assert( mLogBuffer != NULL );
}


///
/// \brief Destructor
///

Device::~Device()
{
    Disconnect();

    assert( mAcquisitionStateManager == NULL );

    assert( mDeviceAdapter != NULL );
    PVDELETE( mDeviceAdapter );

    assert( mDevice != NULL );
    PVDELETE( mDevice );
}


///
/// \brief Creates the right type of device object based on aDeviceInfo
///

Device *Device::Create( const PvDeviceInfo *aDeviceInfo, IMessageSink *aSink, LogBuffer *aLogBuffer )
{
    assert( aDeviceInfo != NULL );
    assert( aSink != NULL );
    assert( aLogBuffer != NULL );

    Device *lDevice = NULL;
    switch ( aDeviceInfo->GetType() )
    {
    case PvDeviceInfoTypeGEV:
        lDevice = new DeviceGEV( aSink, aLogBuffer );
        break;

    case PvDeviceInfoTypeU3V:
        lDevice = new DeviceU3V( aSink, aLogBuffer );
        break;
        
    default:
        break;
    }

    return lDevice;
}


///
/// \brief Completes the connection process (shared GEV, U3V)
///

void Device::CompleteConnect( PvStream *aStream )
{
    // Call early, needed to complete this method
    SetIsMultiSourceTransmitter();

    int64_t lSource = 0;
    if ( IsMultiSourceTransmitter() )
    {
        mDevice->GetParameters()->GetIntegerValue( SOURCE_SELECTOR, lSource );
    }

    // Register for device callbacks
    mDevice->RegisterEventSink( this );

    mAcquisitionStateManagerMutex.Lock();
    ///////////////////////////////////////////////////////////////////////////

    // Regardless of whether a corresponding stream is open or not, we can still pass it to the acquisition state manager
    assert( mAcquisitionStateManager == NULL );
    if ( IsTransmitter() )
    {
        mAcquisitionStateManager = new PvAcquisitionStateManager( mDevice, aStream, static_cast<uint32_t>( lSource ) );
        mAcquisitionStateManager->RegisterEventSink( this );
    }

    ///////////////////////////////////////////////////////////////////////////
    mAcquisitionStateManagerMutex.Unlock();

    // Create the event monitor *before* registering the device event sink
    mLogBuffer->SetParameters( mDevice->GetParameters() );

    // Check whether video interface serial communication is supported or not
    mSerialSupported = PvDeviceSerialPort::IsSupported( mDeviceAdapter, PvDeviceSerial0 );
    mSerialSupported |= PvDeviceSerialPort::IsSupported( mDeviceAdapter, PvDeviceSerialBulk0 );

    // Fill available sources
    GetSources( mAvailableSources );

    // Register for parameters callbacks
    uint32_t lParametersCount = GetParameters()->GetCount();
    for ( uint32_t i = 0; i < lParametersCount; i++ )
    {
        PvGenParameter *lParameter = GetParameters()->Get( i );
        lParameter->RegisterEventSink( this );
    }
    DetectFrameGrabber();
}


///
/// \brief Change the source
///

bool Device::ChangeSource( int64_t aSource )
{
    PvResult lResult;

    // Get source selector
    PvGenEnum *lSourceSelector = GetParameters()->GetEnum( SOURCE_SELECTOR );
    if ( lSourceSelector == NULL )
    {
        PV_LOGERROR( mLogger, "Source selector missing" );
        return false;
    }

    // Get channel selector
    PvGenInteger *lChannelSelector = GetParameters()->GetInteger( CHANNEL_SELECTOR );
    if ( lChannelSelector == NULL )
    {
        PV_LOGERROR( mLogger, "Channel selector missing" );
        return false;
    }

    // Read current source
    int64_t lCurrent = 0;
    lResult = lSourceSelector->GetValue( lCurrent );
    if ( !lResult.IsOK() )
    {
        PV_LOGERROR( mLogger, "Cannot read source selector" );
        return false;
    }

    // Do we need to select a new source?
    if ( lCurrent != aSource )
    {
        // Select source
        PV_LOGINFO( mLogger, "Selecting source " << lCurrent );
        lResult = lSourceSelector->SetValue( aSource );
        if ( !lResult.IsOK() )
        {
            PV_LOGERROR( mLogger, "Cannot set source selector" );
            return false;
        }
    }

    // Read source stream channel
    int64_t lChannel = 0;
    lResult = GetParameters()->GetIntegerValue( SOURCE_STREAM_CHANNEL, lChannel );
    if ( !lResult.IsOK() )
    {
        PV_LOGERROR( mLogger, "Cannot read source stream channel" );
        return false;
    }

    // Read current channel selector
    lCurrent = 0;
    lResult = lChannelSelector->GetValue( lCurrent );
    if ( !lResult.IsOK() )
    {
        PV_LOGERROR( mLogger, "Cannot read channel selector" );
        return false;
    }

    // Do we need to select the channel?
    if ( lCurrent != lChannel )
    {
        // Select channel
        PV_LOGINFO( mLogger, "Selecting stream channel " << lCurrent );
        lResult = lChannelSelector->SetValue( lChannel );
        if ( !lResult.IsOK() )
        {
            PV_LOGERROR( mLogger, "Cannot set channel selector" );
            return false;
        }
    }
    
    return true;
}


///
/// \brief Change the source
///

void Device::CompleteChangeSource( PvStream *aStream, int64_t aSource )
{
    mAcquisitionStateManagerMutex.Lock();
    ///////////////////////////////////////////////////////////////////////////

    // Reset acquisition state manager
    FreeAcquisitionStateManager( false );

    // Regardless of whether a corresponding stream is open or not, we can still pass it to the acquisition state manager
    mAcquisitionStateManager = new PvAcquisitionStateManager( GetDevice(), aStream, static_cast<uint32_t>( aSource ) );
    mAcquisitionStateManager->RegisterEventSink( this );

    ///////////////////////////////////////////////////////////////////////////
    mAcquisitionStateManagerMutex.Unlock();
}


///
/// \brief Disconnects the device
///

PvResult Device::Disconnect()
{
    if ( !IsConnected() )
    {
        return PvResult::Code::NOT_CONNECTED;
    }

    mLogBuffer->SetParameters( NULL );

    uint32_t lParametersCount = GetParameters()->GetCount();
    for ( uint32_t i = 0; i < lParametersCount; i++ )
    {
        PvGenParameter *lParameter = GetParameters()->Get( i );
        lParameter->UnregisterEventSink( this );
    }
    
    mDevice->UnregisterEventSink( this );
    mDevice->Disconnect();

    mUpdatingAcquisitionMode = false;
    mUpdatingSources = false;
    mRecovery = false;
    mSerialSupported = false;

    mAvailableSources.clear();

    FreeAcquisitionStateManager();

    return PvResult::Code::OK;
}


///
/// \brief True if the device is connected
///

bool Device::IsConnected()
{
    return ( mDevice != NULL ) && mDevice->IsConnected();
}


///
/// \brief Returns the enabled state of various controls
///

void Device::GetControlsEnabled( ControlsState &aState )
{
    aState.Reset();

    // Used to temporarily select the source in use
    PvGenStateStack lStack( GetParameters() );

    PvAcquisitionState lAcquisitionState = GetAcquisitionState();
    PushSource( &lStack );

    PvGenCommand *lStart = GetParameters()->GetCommand( ACQUISITION_START );
    PvGenCommand *lStop = GetParameters()->GetCommand( ACQUISITION_STOP );
    PvGenEnum *lMode = GetParameters()->GetEnum( ACQUISITION_MODE );
    PvGenEnum *lSource = GetParameters()->GetEnum( SOURCE_SELECTOR );

    if ( IsMultiSourceTransmitter() )
    {
        aState.mSource = 
            ( lSource != NULL ) &&
            ( lSource->IsWritable() ) &&
            ( lAcquisitionState != PvAcquisitionStateLocked );
    }

    if ( lMode != NULL )
    {
        aState.mAcquisitionMode = 
            ( lMode->IsWritable() ) &&
            ( lAcquisitionState != PvAcquisitionStateLocked );
    }

    if ( lStart != NULL )
    {
        aState.mStart = ( lAcquisitionState != PvAcquisitionStateLocked );
    }

    if ( lStop != NULL )
    {
        aState.mStop = ( lAcquisitionState != PvAcquisitionStateUnlocked );
    }
}


///
/// \brief Returns true if the device is a multi source transmitter
///

void Device::SetIsMultiSourceTransmitter()
{
    mIsMultiSourceTransmitter = false;

    PvGenEnum *lEnum = mDevice->GetParameters()->GetEnum( SOURCE_SELECTOR );
    PvGenEnum *lDeviceClass = mDevice->GetParameters()->GetEnum( GEV_DEVICE_CLASS );
    if ( ( lEnum != NULL ) && ( lDeviceClass != NULL ) )
    {
        PvString lClass;
        if ( lDeviceClass->GetValue( lClass ).IsOK() )
        {
            mIsMultiSourceTransmitter = ( lClass == TRANSMITTER );
        }
    }
}


///
/// \brief Returns the communication parameters for the device
///

PvGenParameterArray *Device::GetCommunicationParameters()
{
    if ( mDevice != NULL )
    {
        return mDevice->GetCommunicationParameters();
    }

    return NULL;
}


///
/// \brief Returns the parameters of the device
///

PvGenParameterArray *Device::GetParameters()
{
    if ( mDevice != NULL )
    {
        return mDevice->GetParameters();
    }

    return NULL;
}


///
/// \brief Instructs the device controller to start streaming
///

PvResult Device::StartAcquisition()
{
    // Set source selector if needed
    PvGenStateStack lStack( GetParameters() );
    PushSource( &lStack );

    PV_LOGINFO( mLogger, "Controller starting acquisition" );
    return mAcquisitionStateManager->Start();
}


///
/// \brief Instructs the device controller to stop streaming
///
/// Only stops if unconditional streaming is not enabled.
///
/// This method is likely called when disconnecting or on other implicit
/// acquisition stop request.
///

PvResult Device::StopAcquisitionIfApplicable()
{
    // Set source selector if needed
    PvGenStateStack lStack( GetParameters() );
    PushSource( &lStack );

    if ( !IsUnconditionalStreamingEnabled() )
    {
        PV_LOGINFO( mLogger, "Controller stopping acquisition" );
        return mAcquisitionStateManager->Stop();
    }
    else
    {
        PV_LOGINFO( mLogger, "Skipping stop acquisition: unconditional streaming enabled" );
    }

    return PvResult::Code::OK;
}


///
/// \brief Instructs the device controller to stop streaming
///
/// Stops regardless of unconditional streaming being enabled or not.
///
/// This method is likely called when the user explicitly requests
/// for streaming to stop.
///

PvResult Device::ForceStopAcquisition()
{
    // Set source selector if needed
    PvGenStateStack lStack( GetParameters() );
    PushSource( &lStack );

    PV_LOGINFO( mLogger, "Controller stopping acquisition" );
    return mAcquisitionStateManager->Stop();

    return PvResult::Code::OK;
}


///
/// \brief Sets a new acquisition mode
///

PvResult Device::SetAcquisitionMode( int64_t aNewMode )
{
    // Set source selector if needed
    PvGenStateStack lStack( GetParameters() );
    PushSource( &lStack );

    // Get acquisition mode parameter
    PvGenEnum *lMode = GetParameters()->GetEnum( ACQUISITION_MODE );
    if ( lMode == NULL )
    {
        return PvResult::Code::NOT_FOUND;
    }

    // Change mode
    return lMode->SetValue( aNewMode );
}


///
/// \brief Returns the default name to use when saving the GenICam XML file of the device
///

PvString Device::GetDeviceXMLDefaultName()
{
    PvString lFilename;
    if ( !mDevice->GetDefaultGenICamXMLFilename( lFilename ).IsOK() )
    {
        lFilename = "Default.xml";
    }

    return lFilename;
}


///
/// \brief Saves the GenICam XML file of the device to a file on the host
///

PvResult Device::SaveDeviceXML( const PvString &aFileName )
{
    if ( !IsConnected() )
    {
        return PvResult::Code::NOT_CONNECTED;
    }

    return mDevice->DumpGenICamXML( aFileName );
}


///
/// \brief Saves the device controller configuration
///

PvResult Device::Save( PvConfigurationWriter *aWriter )
{
    assert( aWriter != NULL );

    return aWriter->Store( mDevice );
}


///
/// \brief Loads the device controller configuration
///

PvResult Device::Load( PvConfigurationReader *aReader )
{
    assert( aReader != NULL );

    return aReader->Restore( 0, mDevice );
}


///
/// \brief Pushes the current source on the GenICam state stack (if the device is multi-source)
///

void Device::PushSource( PvGenStateStack *aStack )
{
    assert( aStack != NULL );

    // If multi source, push the active source on the stack
    if ( IsMultiSourceTransmitter() )
    {
        int64_t lSource1 = 0;
        PvResult lResult = GetParameters()->GetEnumValue( SOURCE_SELECTOR, lSource1 );
        if ( !lResult.IsOK() )
        {
            PV_LOGERROR( mLogger, "Cannot read source selector" );
            return;
        }

        int64_t lSource2 = GetCurrentSource();
        if ( lSource1 != lSource2 )
        {
            // Select source (will be reset when aStack goes out of scope)
            lResult = aStack->SetEnumValue( SOURCE_SELECTOR, mAcquisitionStateManager->GetSource() );
            if ( !lResult.IsOK() )
            {
                PV_LOGERROR( mLogger, "Cannot push source selector" );
                return;
            }

            // Read source stream channel
            int64_t lChannel = 0;
            lResult = GetParameters()->GetIntegerValue( SOURCE_STREAM_CHANNEL, lChannel );
            if ( !lResult.IsOK() )
            {
                PV_LOGERROR( mLogger, "Cannot read source stream channel" );
                return;
            }

            // Select channel (will be reset when aStack goes out of scope)
            lResult = aStack->SetIntegerValue( CHANNEL_SELECTOR, lChannel );
            if ( !lResult.IsOK() )
            {
                PV_LOGERROR( mLogger, "Cannot push channel selector" );
                return;
            }
        }
    }
}


///
/// \brief Returns the numerical value of the currently selected source on the device
///

int64_t Device::GetSelectedSource()
{
    if ( !IsConnected() )
    {
        return -1;
    }

    int64_t lValue = -1;
    PvGenEnum *lEnum = GetParameters()->GetEnum( SOURCE_SELECTOR );
    if ( lEnum != NULL )
    {
        lEnum->GetValue( lValue );
    }

    return lValue;
}



///
/// \brief Returns true if the device is streaming
///

bool Device::IsStreaming()
{
    return GetAcquisitionState() == PvAcquisitionStateLocked;
}


///
/// \brief Return the current acquisition mode
///

void Device::GetCurrentAcquisitionMode( ComboItem &aMode, bool &aWritable )
{
    ComboItemVector lModes;
    GetAcquisitionModes( lModes, aWritable, true );

    assert ( lModes.size() == 1 );
    if ( lModes.size() != 1 )
    {
        return;
    }

    aMode = *lModes.begin();
}


///
/// \brief Returns all acquisition modes
///

void Device::GetAcquisitionModes( ComboItemVector &aModes, bool &aEnabled )
{
    GetAcquisitionModes( aModes, aEnabled, false );
}


///
/// \brief Returns all possible acquisition modes for the device
///

void Device::GetAcquisitionModes( ComboItemVector &aModes, bool &aEnabled, bool aOnlyCurrent )
{
    aModes.clear();
    aEnabled = false;

    if ( !IsConnected() )
    {
        return;
    }

    PvGenEnum *lEnum = GetParameters()->GetEnum( "AcquisitionMode" );
    if ( lEnum != NULL )
    {
        PvGenStateStack lStack( GetParameters() );
        PushSource( &lStack );

        int64_t lCurrentValue = 0;
        lEnum->GetValue( lCurrentValue );

        int64_t lCount = 0;
        lEnum->GetEntriesCount( lCount );

        for ( int64_t i = 0; i < lCount; i++ )
        {
            const PvGenEnumEntry *lEE = NULL;
            lEnum->GetEntryByIndex( i, &lEE );

            PvString lName;
            lEE->GetName( lName );

            int64_t lValue;
            lEE->GetValue( lValue );

            if ( aOnlyCurrent && ( lCurrentValue != lValue ) )
            {
                continue;
            }

            ComboItem lMode;
            lMode.mName = lName;
            lMode.mValue = lValue;
            lMode.mSelected = ( lCurrentValue == lValue );

            aModes.push_back( lMode );
        }

        aEnabled = lEnum->IsWritable() && ( GetAcquisitionState() != PvAcquisitionStateLocked );
    }
}


///
/// \brief Returns all sources for the device.
///

void Device::GetSources( ComboItemVector &aSources )
{
    aSources.clear();

    PvGenEnum *lSourceSelector = GetParameters()->GetEnum( "SourceSelector" );
    if ( lSourceSelector == NULL )
    {
        return;
    }

    int64_t lSourceCount = 0;
    lSourceSelector->GetEntriesCount( lSourceCount );

    uint64_t lActiveSource = GetCurrentSource();

    // Fill source selector combo box
    for ( int64_t i = 0; i < lSourceCount; i++ )
    {
        const PvGenEnumEntry *lEE = NULL;
        lSourceSelector->GetEntryByIndex( i, &lEE );

        if ( lEE->IsAvailable() )
        {
            PvString lDisplayName;
            lEE->GetDisplayName( lDisplayName );

            int64_t lValue = 0;
            lEE->GetValue( lValue );

            ComboItem lComboItem;
            lComboItem.mName = lDisplayName.GetUnicode();
            lComboItem.mValue = lValue;
            if ( lActiveSource == static_cast<uint32_t>( lValue ) )
            {
                lComboItem.mSelected = true;
            }

            aSources.push_back( lComboItem );
        }
    }
}


///
/// \brief Returns the current acquisition state
///

PvAcquisitionState Device::GetAcquisitionState()
{
    if ( !IsConnected() )
    {
        return PvAcquisitionStateUnknown;
    }

    mAcquisitionStateManagerMutex.Lock();
    ///////////////////////////////////////////////////////////////////////////

    PvAcquisitionState lAcquisitionState = PvAcquisitionStateUnknown;
    if ( mAcquisitionStateManager != NULL )
    {
        lAcquisitionState = mAcquisitionStateManager->GetState();
    }

    ///////////////////////////////////////////////////////////////////////////
    mAcquisitionStateManagerMutex.Unlock();

    return lAcquisitionState;
}



///
/// \brief Returns the current source. -1 if the source cannot be retrieve (ie: not mutli-source)
///

int64_t Device::GetCurrentSource()
{
    if ( !IsConnected() || !IsMultiSourceTransmitter() )
    {
        return -1;
    }

    mAcquisitionStateManagerMutex.Lock();
    ///////////////////////////////////////////////////////////////////////////

    int64_t lRetVal = 0;
    if ( mAcquisitionStateManager != NULL )
    {
        lRetVal = mAcquisitionStateManager->GetSource();
    }

    ///////////////////////////////////////////////////////////////////////////
    mAcquisitionStateManagerMutex.Unlock();

    return lRetVal;
}


///
/// \brief Returns currently selected source channel
///

uint16_t Device::GetCurrentSourceChannel()
{
    if ( !IsConnected() || !IsMultiSourceTransmitter() )
    {
        return 0;
    }

    PvGenInteger *lSourceChannel = GetParameters()->GetInteger( SOURCE_STREAM_CHANNEL );
    if ( lSourceChannel == NULL )
    {
        return 0;
    }

    int64_t lValue = 0;
    lSourceChannel->GetValue( lValue );

    return static_cast<uint16_t>( lValue );
}


///
/// \brief Returns true if acquisition is currently locked
///

bool Device::IsAcquisitionLocked()
{
    return GetAcquisitionState() == PvAcquisitionStateLocked;
}


///
/// \brief Sets the device pointer
///

void Device::SetDevice( PvDevice *aDevice )
{
    mDevice = aDevice;

    // Create device adapter
    mDeviceAdapter = new PvDeviceAdapter( aDevice );
}


///
/// \brief PvGenEventSink
///

void Device::OnParameterUpdate( PvGenParameter *aParameter )
{
    if ( mAcquisitionStateManager == NULL )
    {
        return;
    }

    PvString lName = aParameter->GetName();
    if ( lName == ACQUISITION_MODE )
    {
        // Prevent re-entry
        if ( !mUpdatingAcquisitionMode )
        {
            if ( IsMultiSourceTransmitter() )
            {
                int64_t lSource1 = 0;
                GetParameters()->GetEnumValue( SOURCE_SELECTOR, lSource1 );

                int64_t lSource2 = mAcquisitionStateManager->GetSource();

                if ( lSource1 == lSource2 )
                {
                    // Only refresh if the source selector is that same as the source used by eBUSPlayer
                    mUpdatingAcquisitionMode = true;
                    mMessageSink->SendMsgIfPossible( WM_UPDATEACQUISITIONMODE, 1 );
                }
            }
            else
            {
                // No source selector, always refresh
                mUpdatingAcquisitionMode = true;
                mMessageSink->SendMsgIfPossible( WM_UPDATEACQUISITIONMODE, 1 );
            }
        }
    }

    if ( lName == SOURCE_SELECTOR )
    {
        if ( !mUpdatingSources && IsMultiSourceTransmitter() )
        {
            PvGenEnum *lSourceSelector = dynamic_cast<PvGenEnum *>( aParameter );
            if ( lSourceSelector == NULL )
            {
                return;
            }

            // Retrieve enum entry count from source selector
            int64_t lSourceCount = 0;
            lSourceSelector->GetEntriesCount( lSourceCount );

            // Build list of available sources
            Int64Vector lSources;
            for ( int64_t i = 0; i < lSourceCount; i++ )
            {
                const PvGenEnumEntry *lEE = NULL;
                lSourceSelector->GetEntryByIndex( i, &lEE );
                if ( lEE->IsAvailable() )
                {
                    int64_t lValue = 0;
                    lEE->GetValue( lValue );

                    lSources.push_back( lValue );
                }
            }

            // Compare available sources vs. cached combo box content
            bool lIsTheSame = false;
            if ( lSources.size() == mAvailableSources.size() )
            {
                lIsTheSame = true;
                Int64Vector::const_iterator lIt1 = lSources.begin();
                ComboItemVector::const_iterator lIt2 = mAvailableSources.begin();
                while ( ( lIt1 != lSources.end() ) && ( lIt2 != mAvailableSources.end() ) )
                {
                    if ( *lIt1 != lIt2->mValue )
                    {
                        lIsTheSame = false;
                        break;
                    }

                    lIt1++;
                    lIt2++;
                }
            }

            if ( !lIsTheSame )
            {
                mUpdatingSources = true;
                mMessageSink->PostMsg( WM_UPDATESOURCES, 0, -1 );
            }
        }
    }
}


///
/// PvDeviceEventSink
///

void Device::OnLinkDisconnected( PvDevice *aDevice )
{
    mRecovery = true;
    mMessageSink->PostMsg( WM_LINKDISCONNECTED );    
}


///
/// PvDeviceEventSink
///

void Device::OnLinkReconnected( PvDevice *aDevice )
{
    mMessageSink->PostMsg( WM_LINKRECONNECTED );
}


///
/// PvDeviceEventSink
///

void Device::OnEvent( PvDevice *aDevice, uint16_t aEventID, uint16_t aChannel, uint64_t aBlockID, uint64_t aTimestamp, const void *aData, uint32_t aDataLength )
{
}


///
/// PvDeviceEventSink
///

void Device::OnEventGenICam( PvDevice *aDevice, uint16_t aEventID, uint16_t aChannel, uint64_t aBlockID, uint64_t aTimestamp, PvGenParameterList *aData )
{
    if ( !mLogBuffer->IsEventsEnabled() )
    {
        return;
    }
     
    // Basic event log
    std::stringstream lLog;
    lLog << "Event ID: 0x" << std::hex << aEventID << " ";
    lLog << "Channel: 0x" << std::hex << aBlockID << " ";
    lLog << "Timestamp: 0x" << std::hex << aTimestamp;

    // Add parameters
    if ( aData != NULL )
    {
        // Is there a parameter containing "DataLength"? Get its value to query only subset of defined IRegister
        int64_t lDataLength = 0;
        PvGenParameter *lP = aData->GetFirst();
        while ( lP != NULL )
        {
            std::string lName = lP->GetName().GetAscii();
            if ( lName.find( DATA_LENGTH ) != std::string::npos )
            {
                PvGenInteger *lPI = dynamic_cast<PvGenInteger *>( lP );
                if ( lPI != NULL )
                {
                    lPI->GetValue( lDataLength );
                }

                break;
            }

            lP = aData->GetNext();
        }

        PvGenParameter *lParameter = aData->GetFirst();
        while ( lParameter != NULL )
        {
            lLog << "\r\n                ";
            lLog << lParameter->GetName().GetAscii();
            lLog << ": ";

            PvGenRegister *lRegister = dynamic_cast<PvGenRegister *>( lParameter );
            if ( lRegister == NULL )
            {
                lLog << lParameter->ToString().GetAscii();
            }
            else
            {
                lLog << "\r\n";

                if ( lDataLength <= 0 )
                {
                    lRegister->GetLength( lDataLength );
                }

                uint8_t *lData = new uint8_t[ static_cast<uint32_t>( lDataLength ) ];

                PvResult lResult = lRegister->Get( lData, static_cast<uint32_t>( lDataLength ) );
                lLog << PvBinaryFormat( "                    ", lData, static_cast<uint32_t>( lDataLength ), 0 ).GetAscii();

                delete []lData;
                lData = NULL;
            }

            lParameter = aData->GetNext();
        }
    }

    mLogBuffer->Log( lLog.str() );
}


///
/// \brief PvDeviceEventSink
///

void Device::OnCmdLinkRead( const void *aBuffer, int64_t aAddress, int64_t aLength )
{

}


///
/// \brief PvDeviceEventSink
///

void Device::OnCmdLinkWrite( const void *aBuffer, int64_t aAddress, int64_t aLength )
{
  
}


///
/// \brief PvAcquisitionStateEventSink
///

void Device::OnAcquisitionStateChanged( PvDevice* aDevice, PvStream* aStream, uint32_t aSource, PvAcquisitionState aState )
{
    mMessageSink->SendMsgIfPossible( WM_ACQUISITIONSTATECHANGED );
}


///
/// \brief Loads communication parameters from a string
///

void Device::LoadCommunicationParameters( const PvString &aParameters )
{
    PvConfigurationReader lReader;
    lReader.LoadFromString( aParameters );
    lReader.Restore( 0, mDevice->GetCommunicationParameters() );
}


///
/// \brief Frees the acquisition state manager
///

void Device::FreeAcquisitionStateManager( bool aLockMutex )
{
    if ( aLockMutex ) mAcquisitionStateManagerMutex.Lock();
    ///////////////////////////////////////////////////////////////////////////

    if ( mAcquisitionStateManager != NULL )
    {
        delete mAcquisitionStateManager;
        mAcquisitionStateManager = NULL;
    }

    ///////////////////////////////////////////////////////////////////////////
    if ( aLockMutex ) mAcquisitionStateManagerMutex.Unlock();
}


///
/// \brief Detects whether the device is a frame grabber or not
///

void Device::DetectFrameGrabber()
{
    mFrameGrabber = false;
    if ( !IsConnected() )
    {
        return;
    }

    // Get device model name
    PvString lModelName;
    PvResult lResult = mDevice->GetParameters()->GetStringValue( DEVICE_MODEL_NAME, lModelName );
    if ( !lResult.IsOK() )
    {
        PV_LOGWARNING( mLogger, "Could not read device model name" );
        return;
    }

    // For now we test for the model name to contain -CL (as in PT1000-CL4) or CL- (as in CL-GigE, CL-U3)
    std::string lString = lModelName.GetAscii();
    mFrameGrabber = ( ( lString.find( "-CL" ) != std::string::npos ) ||
                      ( lString.find( "CL-" ) != std::string::npos ) );
}

