// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "eBUSPlayerShared.h"
#include "Player.h"

#include "DeviceGEV.h"
#include "DeviceU3V.h"
#include "StreamGEV.h"
#include "StreamU3V.h"

#include "ChangeSourceTask.h"
#include "ConnectTask.h"
#include "DisconnectTask.h"
#include "LoadTask.h"
#include "SaveTask.h"

#include <PvSystem.h>
#include <PvStreamInfo.h>
#include <PvNetworkAdapter.h>

#include <sstream>
#include <assert.h>
#include <iomanip>


#define TAG_DISPLAYOPTIONS ( "displayoptions" )
#define TAG_ACTIVESOURCE ( "activesource" )
#define TAG_COMMUNICATIONPARAMSGEV ( "communicationparamsgev" )
#define TAG_COMMUNICATIONPARAMSU3V ( "communicationparamsu3v" )

#define LINK_RECOVERY_ENABLED ( "LinkRecoveryEnabled" )

#define NA_STRING ( "N/A" )


#ifdef _AFXDLL
    IMPLEMENT_DYNAMIC( Player, CObject )
#endif // _AFXDLL


PvDeviceGEV Player::sDefaultDeviceGEV;
PvDeviceU3V Player::sDefaultDeviceU3V;
BufferOptions Player::sDefaultBufferOptions;


///
/// \brief Constructor
///

Player::Player( IPlayerController *aController, IPvDisplayAdapter *aDisplay )
    : mController( aController )
    , mDevice( NULL )
    , mStream( NULL )
    , mDisplayThread( NULL )
#ifdef SERIALBRIDGE
    , mCameraBridgeManagerWnd( NULL )
    , mSerialBridgeManagerWnd( NULL )
#endif
    , mImageFiltering( NULL )
    , mImageSaving( NULL )
    , mChangingSource( false )
    , mPreferredSource( -1 )
    , mLogger( "eBUSPlayer" )
{
    mImageFiltering = new ImageFiltering;
    mImageSaving = new ImageSaving( mImageFiltering );
    mDisplayThread = new DisplayThread( aDisplay, mImageFiltering, mImageSaving, &mLogBuffer );
    
    // Initialize buffer options from defaults
    mBufferOptions = sDefaultBufferOptions;

    // Build list of preferences implementing IChanged, take baseline
    mChangedList.push_back( &mSetup );
    mChangedList.push_back( &mLogBuffer );
    mChangedList.push_back( mImageFiltering );
    mChangedList.push_back( mImageSaving );
    mChangedList.push_back( mDisplayThread );
    mChangedList.push_back( &mBufferOptions );
    ResetPreferencesChanged();
}


///
/// \brief Destructor
///

Player::~Player()
{
    // Just in case...
    DisconnectTaskHandler( NULL );

    PVDELETE( mDisplayThread );
    PVDELETE( mImageSaving );
    PVDELETE( mImageFiltering );
}


///
/// Saves the application configuration to a file
///

PvResult Player::SaveConfig( PvConfigurationWriter *aWriter, PvStringList &aErrorList, bool aSaveConnectedDevice )
{
    PV_LOGINFO( mLogger, "Saving configuration to a PvConfigurationWriter" );

    if ( !aSaveConnectedDevice )
    {
        // Save everything but the connection: fast, no need for task/progress
        return SaveConfigTaskHandler( NULL, aWriter, aErrorList, false );
    }

    SaveTask lTask( this, aWriter, aErrorList );

    IProgress *lProgress = mController->CreateProgressDialog();
    lProgress->SetTitle( "Configuration Saving Progress" );
    lProgress->RunTask( &lTask );

    PVDELETE( lProgress );

    return lTask.GetResult();
}


///
/// Saves the application configuration to a file
///

PvResult Player::SaveConfigTaskHandler( IProgress *aProgress, PvConfigurationWriter *aWriter, PvStringList &aErrorList, bool aSaveConnectedDevice )
{
    SETSTATUS( aProgress, "Saving application setup..." );

    mSetup.Save( aWriter );

    SETSTATUS( aProgress, "Saving imaging filtering configuration..." );

    mImageFiltering->Save( aWriter );

    SETSTATUS( aProgress, "Saving imaging saving configuration..." );

    mImageSaving->Save( aWriter );

    SETSTATUS( aProgress, "Saving event monitor configuration..." );

    mLogBuffer.Save( aWriter );

    SETSTATUS( aProgress, "Saving buffer options..." );

    mBufferOptions.Save( aWriter );

    SETSTATUS( aProgress, "Saving display options..." );
 
    PvPropertyList lPropertyList;
    mDisplayThread->Save( lPropertyList );
    aWriter->Store( &lPropertyList, TAG_DISPLAYOPTIONS );

    if ( aSaveConnectedDevice )
    {
        mLogBuffer.EnableGenICamMonitoring( false );

        if ( IsDeviceConnected() )
        {
            SETSTATUS( aProgress, "Saving device configuration..." );

            // Save device configuration
            aWriter->SetErrorList( &aErrorList, "Device state save error. " );
            mDevice->Save( aWriter );

            // Save currently selected source
            if ( IsDeviceConnected() && mDevice->IsMultiSourceTransmitter() )
            {
                SETSTATUS( aProgress, "Saving selected source..." );

                int64_t lSource = mDevice->GetSelectedSource();

                std::stringstream lSS;
                lSS << lSource;

                PvString lSourceStr( lSS.str().c_str() );

                aWriter->Store( lSourceStr, TAG_ACTIVESOURCE );
            }

            // Save camera and serial bridges
            PvResult lResult = SaveBridges( aProgress, aWriter, aErrorList );
            if ( !lResult.IsOK() )
            {
                return lResult;
            }
        }

        // Save stream
        if ( IsStreamOpened() )
        {
            SETSTATUS( aProgress, "Saving stream configuration..." );

            aWriter->SetErrorList( &aErrorList, "Stream state save error. " );
            mStream->Save( aWriter );
        }

        mLogBuffer.EnableGenICamMonitoring( true );
    }
    else
    {
        SETSTATUS( aProgress, "Saving default communication parameters..." );
    }

    return PvResult::Code::OK;
}


///
/// \brief Opens a configuration
///

PvResult Player::OpenConfig( PvConfigurationReader *aReader, PvStringList &aErrorList, bool aShowProgress )
{
    PV_LOGINFO( mLogger, "Opening configuration from a PvConfigurationReader" );

    if ( !aShowProgress )
    {
        // Restore everything but the connection: fast, no need for task/progress
        PvResult lResult = OpenConfigTaskHandler( NULL, aReader, aErrorList );
        return lResult;
    }

    LoadTask lTask( this, aReader, aErrorList );

    IProgress *lProgress = mController->CreateProgressDialog();
    lProgress->SetTitle( "Configuration Restore Progress" );
    lProgress->RunTask( &lTask );

    PVDELETE( lProgress );

    UpdateController();

    return lTask.GetResult();
}


///
/// \brief Open configuration task handler
///

PvResult Player::OpenConfigTaskHandler( IProgress *aProgress, PvConfigurationReader *aReader, PvStringList &aErrorList )
{
    SETSTATUS( aProgress, "Restoring application setup..." );

    mSetup.Load( aReader );

    SETSTATUS( aProgress, "Restoring imaging filtering configuration..." );

    mImageFiltering->Load( aReader );

    SETSTATUS( aProgress, "Restoring event monitor configuration..." );

    mLogBuffer.Load( aReader );

    if ( ( aReader->GetDeviceCount() > 0 ) || ( aReader->GetStreamCount() > 0 ) )
    {
        // If we're connected, just apply the settings. Otherwise connect from the settings in the persistence file.
        if ( IsDeviceConnected() )
        {
            if ( IsControlledTransmitter() )
            {   
                SETSTATUS( aProgress, "Restoring device configuration..." );

                // Restore device properties
                aReader->SetErrorList( &aErrorList, "Device restore error. " );
                mDevice->Load( aReader );

                // Load camera and serial bridges
                PvResult lResult = LoadBridges( aProgress, aReader, aErrorList );
                if ( !lResult.IsOK() )
                {
                    return lResult;
                }
            }

            uint16_t lChannel = mSetup.GetDefaultChannel();

            if ( IsSetupRoleDataReceiver() )
            {
                SETSTATUS( aProgress, "Restoring streaming configuration..." );

                aReader->SetErrorList( &aErrorList, "Stream restore error. " );
                mStream->Load( aReader );

                // Obtain the proper channel from the stream we restored
                lChannel = mStream->GetChannel();
            }

            // Now that the stream is opened, set the destination on the device
            if ( IsSetupRoleController() )
            {
                if ( mStream != NULL )
                {
                    SETSTATUS( aProgress, "Setting stream destination..." );

                    // Set stream destination
                    mDevice->SetStreamDestination( &mSetup, mStream, lChannel );
                }
            }
        }
        else
        {
            PvResult lResult = ConnectTaskHandler( aProgress, NULL, aReader, &aErrorList );
            if ( !lResult.IsOK() )
            {
                // Adapted messsage for not found
                return PvResult( PvResult::Code::NOT_FOUND, "Unable to connect the device. It is either absent or already in use." );
            }
        }

        if ( IsDeviceConnected() && mDevice->IsMultiSourceTransmitter() )
        {
            PvString lSourceStr;
            if ( aReader->Restore( TAG_ACTIVESOURCE, lSourceStr ).IsOK() )
            {
                // Here we just save the preferred source, it will be selected later
                std::stringstream lSS( lSourceStr.GetAscii() );
                lSS >> mPreferredSource;
            }
        }
    }

    SETSTATUS( aProgress, "Restoring display configuration..." );

    PvPropertyList lPropertyList;
    aReader->Restore( TAG_DISPLAYOPTIONS, &lPropertyList );
    mDisplayThread->Load( lPropertyList );

    SETSTATUS( aProgress, "Restoring image saving configuration..." );

    mImageSaving->Load( aReader );

    SETSTATUS( aProgress, "Restoring buffer options..." );

    mBufferOptions.Load( aReader );
    if ( IsStreamOpened() )
    {
        mBufferOptions.Apply( mStream->GetPipeline() );
        StartStreaming();
    }

    return PvResult::Code::OK;
}


///
/// \brief Application connection operation
///

PvResult Player::Connect( const PvDeviceInfo *aDI, PvConfigurationReader *aCR )
{
    if ( aDI != NULL )
    {
        PV_LOGINFO( mLogger, "Connecting from device info: " << aDI->GetConnectionID().GetAscii() );
    }
    else if ( aCR != NULL )
    {
        PV_LOGINFO( mLogger, "Connecting from PvConfigurationReader" );
    }

    ConnectTask lTask( this, aDI, aCR );

    IProgress *lProgress = mController->CreateProgressDialog();
    lProgress->SetTitle( "Connection Progress" );
    lProgress->RunTask( &lTask );

    delete lProgress;
    lProgress = NULL;

    UpdateController();

    return lTask.GetResult();
}


///
/// \brief Connect task handler
///

PvResult Player::ConnectTaskHandler( IProgress *aProgress, const PvDeviceInfo *aDI, PvConfigurationReader *aCR, PvStringList *aErrorList )
{
    PV_LOGINFO( mLogger, "Player::ConnectTaskHandler begin" );
    assert( aDI != NULL || aCR != NULL );
    if ( aDI == NULL && aCR == NULL )   
    {
        PV_LOGERROR( mLogger, "Player::ConnectTaskHandler cannot connect with null device info or null configuration reader" );
        return PvResult::Code::INVALID_PARAMETER;
    }

    DisconnectTaskHandler( aProgress );

    mPreferredSource = -1;

    uint16_t lChannel = mSetup.GetDefaultChannel();
    PvResult lResult;

    if ( aDI != NULL )
    {
        if ( IsSetupRoleController() )
        {
            PV_LOGINFO( mLogger, "Player::ConnectTaskHandler Connecting device..." );
            SETSTATUS( aProgress, "Connecting device..." );

            PvString lCommunicationParameters;
            GetDefaultCommunicationParameters( aDI, lCommunicationParameters );

            mDevice = Device::Create( aDI, mController, &mLogBuffer );        
            lResult = mDevice->Connect( aProgress, &mSetup, aDI, lChannel, lCommunicationParameters );
            if ( !lResult.IsOK() )
            {
                PV_LOGERROR( mLogger, "Player::ConnectTaskHandler device connect failed " << lResult.GetCodeString().GetAscii() );
                DisconnectTaskHandler( aProgress );
                return lResult;
            }
        }

        if ( IsSetupRoleDataReceiver() )
        {
            PV_LOGINFO( mLogger, "Player::ConnectTaskHandler Opening stream..." );
            SETSTATUS( aProgress, "Opening stream..." );

            mStream = Stream::Create( aDI, mController );
            lResult = mStream->Open( &mSetup, aDI, lChannel );
            if ( !lResult.IsOK() )
            {
                PV_LOGERROR( mLogger, "Player::ConnectTaskHandler stream open failed " << lResult.GetCodeString().GetAscii() );
                DisconnectTaskHandler( aProgress );
                return lResult;
            }
        }
    }
    else if ( aCR != NULL )
    {
        if ( IsSetupRoleController() )
        {   
#ifdef SERIALBRIDGE
            if ( mCameraBridgeManagerWnd == NULL )
            {
                mCameraBridgeManagerWnd = new PvCameraBridgeManagerWnd();
            }
            if ( mSerialBridgeManagerWnd == NULL )
            {
                mSerialBridgeManagerWnd = new PvSerialBridgeManagerWnd();
            }
#endif // SERIALBRIDGE
            PV_LOGINFO( mLogger, "Player::ConnectTaskHandler Connecting and restoring device configuration..." );
            SETSTATUS( aProgress, "Connecting and restoring device configuration..." );

            // 1st try with a GEV device
            assert( mDevice == NULL );
            mDevice = new DeviceGEV( mController, &mLogBuffer );

            lResult = aCR->Restore( 0, mDevice->GetDevice() );
            if ( !lResult.IsOK() )
            {
                // Now try a U3V device
                delete mDevice;
                mDevice = new DeviceU3V( mController, &mLogBuffer );

                lResult = aCR->Restore( 0, mDevice->GetDevice() );
                if ( !lResult.IsOK() )
                {
                    PV_LOGERROR( mLogger, "Player::ConnectTaskHandler U3V device restore failed " << lResult.GetCodeString().GetAscii() );
                    DisconnectTaskHandler( aProgress );
                    return lResult;
                }
            }
        }
        if ( !lResult.IsOK() )
        {
            PV_LOGERROR( mLogger, "Player::ConnectTaskHandler Loading serial bridge failed " << lResult.GetCodeString().GetAscii() );
        }
        if ( IsSetupRoleDataReceiver() )
        {
            PV_LOGINFO( mLogger, "Player::ConnectTaskHandler Opening and restoring stream configuration..." );
            SETSTATUS( aProgress, "Opening and restoring stream configuration..." );

            // 1st try with a GEV stream
            assert( mStream == NULL );
            mStream = new StreamGEV( mController );

            lResult = aCR->Restore( 0, mStream->GetStream() );
            if ( !lResult.IsOK() )
            {
                // Now try with a U3V stream
                delete mStream;
                mStream = new StreamU3V( mController );

                lResult = aCR->Restore( 0, mStream->GetStream() );
                if ( !lResult.IsOK() )
                {
                    PV_LOGERROR( mLogger, "Player::ConnectTaskHandler U3V stream restore failed " << lResult.GetCodeString().GetAscii() );
                    DisconnectTaskHandler( aProgress );
                    return lResult;
                }
            }

            // Obtain the proper channel from the stream we restored
            lChannel = mStream->GetChannel();
        }

        // Load camera and serial bridges
        lResult = LoadBridges( aProgress, aCR, *aErrorList );
    }

    PvSystem lSystem;
    const PvDeviceInfo *lDI = aDI;

    if ( lDI == NULL )
    {
        PV_LOGINFO( mLogger, "Player::ConnectTaskHandler Retrieving device information..." );
        SETSTATUS( aProgress, "Retrieving device information..." );

        // Get IP address or GUID of the device of interest
        std::string lInfo;
        if ( IsDeviceConnected() )
        {
            lInfo = mDevice->GetFindString();
        }
        else if ( IsStreamOpened() )
        {
            lInfo = mStream->GetFindString();
        }

        if ( lInfo.length() > 0 )
        {
            // Attempt to get a device info
            lResult = lSystem.FindDevice( lInfo.c_str(), &lDI );
            if ( !lResult.IsOK() )
            {
                PV_LOGERROR( mLogger, "Player::ConnectTaskHandler system find device failed " << lResult.GetCodeString().GetAscii() );
                DisconnectTaskHandler( aProgress );
                return lResult;
            }
        }
    }

    // Now that we're sure to have a DI, set device class
    if ( IsDeviceConnected() && ( lDI != NULL ) )
    {
        mDevice->SetClass( lDI->GetClass() );
    }

    if ( IsDeviceConnected() && mDevice->IsTransmitter() ) 
    {
        PV_LOGINFO( mLogger, "Player::ConnectTaskHandler Setting stream destination..." );
        SETSTATUS( aProgress, "Setting stream destination..." );

        lResult = mDevice->SetStreamDestination( &mSetup, mStream, lChannel );
        if ( !lResult )
        {
            PV_LOGERROR( mLogger, "Player::ConnectTaskHandler device set stream destination failed " << lResult.GetCodeString().GetAscii() );
            DisconnectTaskHandler( aProgress );
            return lResult;
        }
    }

    // Update device attributes
    UpdateAttributes( lDI );

    if ( IsDeviceConnected() )
    {
#ifdef SERIALBRIDGE
        if ( mCameraBridgeManagerWnd == NULL )
        {
            mCameraBridgeManagerWnd = new PvCameraBridgeManagerWnd();
            mCameraBridgeManagerWnd->SetDevice( mDevice->GetDevice() );
            if ( mStream != NULL )
            {
                mCameraBridgeManagerWnd->SetStream( mStream->GetStream() );
            }
        }
        if ( mSerialBridgeManagerWnd == NULL )
        {
            mSerialBridgeManagerWnd = new PvSerialBridgeManagerWnd();
            mSerialBridgeManagerWnd->SetDevice( mDevice->GetDeviceAdapter() );
        }
#endif // SERIALBRIDGE

        PvStream *lStream = NULL;
        if ( IsStreamOpened() )
        {
            lStream = mStream->GetStream();
        }

        mDevice->CompleteConnect( lStream );
    }

    // Only start streaming if stream is opened and not restoring from persistence
    if ( IsStreamOpened() && ( aCR == NULL ) )
    {
        PV_LOGINFO( mLogger, "Player::ConnectTaskHandler Configuring streaming buffers..." );
        SETSTATUS( aProgress, "Configuring streaming buffers..." );
        mBufferOptions.Apply( mStream->GetPipeline() );
         
        PV_LOGINFO( mLogger, "Player::ConnectTaskHandler Starting display thread..." );
        SETSTATUS( aProgress, "Starting display thread..." );
        StartStreaming();
    }

    PV_LOGINFO( mLogger, "Player successful completion of connecting task handler" );
    return PvResult::Code::OK;
}


///
/// \brief Application connection operation
///

void Player::Disconnect()
{
    PV_LOGINFO( mLogger, "Disconnecting" );

    mImageSaving->NotifyStreamingStop();

    DisconnectTask lTask( this );

    IProgress *lProgress = mController->CreateProgressDialog();
    lProgress->SetTitle( "Disconnect Progress" );
    lProgress->RunTask( &lTask );

    delete lProgress;
    lProgress = NULL;
}
 

///
/// \brief Application connection operation task handler
///

PvResult Player::DisconnectTaskHandler( IProgress *aProgress )
{
    if ( IsStreaming() )
    {
        StopIfApplicable();
    }

    if ( mDevice != NULL )
    {
        mDevice->FreeAcquisitionStateManager();
    }

    mStreamMutex.Lock();
    if ( mStream != NULL )
    {
        SETSTATUS( aProgress, "Closing stream..." );

        delete mStream;
        mStream = NULL;
    }
    mStreamMutex.Unlock();

#ifdef SERIALBRIDGE
    if ( mCameraBridgeManagerWnd != NULL )
    {
        SETSTATUS( aProgress, "Closing camera bridge manager..." );
        PVDELETE( mCameraBridgeManagerWnd );
    }
    if ( mSerialBridgeManagerWnd != NULL )
    {
        SETSTATUS( aProgress, "Closing serial bridge manager..." );
        PVDELETE( mSerialBridgeManagerWnd );
    }
#endif // SERIALBRIDGE

    if ( mDevice != NULL )
    {
        if ( mDevice->IsConnected() )
        {
            SETSTATUS( aProgress, "Disconnecting device..." );
            mDevice->Disconnect();
        }

        PVDELETE( mDevice );
    }

    mDeviceAttributes.Reset();
    return PvResult::Code::OK;
}


///
/// \brief Change application source
///

PvResult Player::ChangeSource( int64_t aNewSource )
{
    PV_LOGINFO( mLogger, "Changing to source " << aNewSource );

    if ( !IsDeviceConnected() )
    {
        return PvResult::Code::NOT_CONNECTED;
    }

    // Get current source, channel
    int64_t lCurrentSource = mDevice->GetCurrentSource();

    // Is source already active?
    if ( mChangingSource || ( aNewSource == lCurrentSource ) )
    {
        return PvResult::Code::ABORTED;
    }

    mChangingSource = true;

    ChangeSourceTask lTask( this, aNewSource );

    IProgress *lProgress = mController->CreateProgressDialog();
    lProgress->SetTitle( "Source Change Progress" );
    lProgress->RunTask( &lTask );

    delete lProgress;
    lProgress = NULL;

    mChangingSource = false;

    return lTask.GetResult();
}


///
/// \brief Change application source
///

PvResult Player::ChangeSourceTaskHandler( IProgress *aProgress, int64_t aNewSource )
{
    if ( !IsDeviceConnected() )
    {
        return PvResult::Code::NOT_CONNECTED;
    }

    StreamGEV *lStreamGEV = dynamic_cast<StreamGEV *>( mStream );
    DeviceGEV *lDeviceGEV = dynamic_cast<DeviceGEV *>( mDevice );

    // Get current channel
    uint16_t lCurrentChannel = mDevice->GetCurrentSourceChannel();

    SETSTATUS( aProgress, "Stopping acquisition..." );

    // Stop acquisition
    mDevice->StopAcquisitionIfApplicable();

    SETSTATUS( aProgress, "Resetting stream destination..." );

    // Reset stream destination
    lDeviceGEV->ResetStreamDestination( lCurrentChannel );

    SETSTATUS( aProgress, "Closing stream..." );

    // Close stream
    if ( IsStreamOpened() )
    {
        mStream->Close();
    }

    if ( lDeviceGEV != NULL )
    {
        // Obtain the local IP address and device IP address
        std::string lDeviceIPAddress = lDeviceGEV->GetIPAddress();

        SETSTATUS( aProgress, "Selecting new source..." );

        // Change the source on the controller
        mDevice->ChangeSource( aNewSource );
        lCurrentChannel = mDevice->GetCurrentSourceChannel();

        SETSTATUS( aProgress, "Re-opening stream..." );

        // Re-open the stream
        PvStream *lStream = NULL;
        if ( lStreamGEV != NULL )
        {
            lStream = lStreamGEV->GetStream();

            std::string lLocalIPAddress = lStreamGEV->GetLocalIPAddress();
            PvResult lResult = lStreamGEV->Open( &mSetup, lDeviceIPAddress, lLocalIPAddress, lCurrentChannel );
            if ( !lResult.IsOK() )
            {
                return lResult;
            }
        }

        SETSTATUS( aProgress, "Setting stream destination..." );

        // Set the stream destination
        mDevice->SetStreamDestination( &mSetup, mStream, lCurrentChannel );

        // Update the acquisition manager
        mDevice->CompleteChangeSource( lStream, aNewSource );

        if ( IsStreamOpened() )
        {
            SETSTATUS( aProgress, "Restarting display thread..." );
            StartStreaming();
        }
    }

    return PvResult::Code::OK;
}


///
/// \brief Application connection operation
///

void Player::GetControlsEnabled( ControlsState &aState )
{
    if ( !IsDeviceConnected() )
    {
        return;
    }

    mDevice->GetControlsEnabled( aState );   
}


///
/// \brief Returns true if the information in the configuration reader is the same as what we now have
///

bool Player::IsSameAsCurrent( PvConfigurationReader *aReader )
{
    bool lSetupIsTheSame = mSetup.IsTheSame( aReader );
    return ( IsDeviceConnected() || IsStreamOpened() ) && !lSetupIsTheSame;
}


///
/// \brief Returns the streaming status
///

void Player::GetStatusText( PvString &aText, bool &aRecording )
{
    aText = "";
    aRecording = false;

    if ( !IsStreamOpened() )
    {
        return;
    }

    std::stringstream lSS;

    if ( mImageSaving->GetEnabled() )
    {
        aRecording = true;

        std::stringstream lAvgFPS( "N/A" );
        if ( mImageSaving->GetFrames() > 0 )
        {
            lAvgFPS << std::fixed << std::setprecision( 1 ) << mImageSaving->GetFPS();
        }

        PvString lLastError;
        mImageSaving->GetLastError( lLastError );

        lSS << "RECORDING ";
        lSS << mImageSaving->GetFrames() << " images    ";
        lSS << mImageSaving->GetTotalSize() << " MB    ";
        lSS << lAvgFPS.str() << " FPS    ";
        lSS << std::fixed << std::setprecision( 1 ) << mImageSaving->GetMbps() << " Mbps" << "\r\n";
        lSS << lLastError.GetAscii();
    }
    else
    {
        PvStreamInfo lInfo( mStream->GetStream() );
        PvString lStatistics = lInfo.GetStatistics( mDisplayThread->GetFPS() );
        PvString lErrors = lInfo.GetErrors();
        PvString lWarnings = lInfo.GetWarnings( mStream->IsPipelineReallocated() );

        lSS << lStatistics.GetAscii() << "\r\n";
        lSS << lErrors.GetAscii() << "\r\n";
        lSS << lWarnings.GetAscii();
    }

    aText = lSS.str().c_str();
}


///
/// \brief Saves the current image.
///

void Player::SaveCurrentImage()
{
    if ( !IsStreamStarted() )
    {
        return;
    }

    mImageSaving->SaveCurrentImage( mDisplayThread );
}


///
/// \brief Play button operation
///

void Player::Play()
{
    PV_LOGINFO( mLogger, "Play invoked" );

    // Try getting payload size from the device
    uint32_t lPayloadSize = 0;
    if ( IsDeviceConnected() )
    {
        lPayloadSize = mDevice->GetPayloadSize();
    }

    // Not payload size? Take the default from the buffer options
    if ( lPayloadSize == 0 )
    {
        lPayloadSize = mBufferOptions.GetBufferSize();
    }

    // Arm streaming
    if ( IsStreamOpened() )
    {
        mStream->Reset( lPayloadSize );
    }

    // Instruct controller to start streaming
    if ( IsDeviceConnected() )
    {
        // Always start the device/grabber 1st
        mDevice->StartAcquisition();

        // Start grabber camera if applicable
        PvCameraBridge *lBridge = GetSelectedBridge();
        if ( lBridge != NULL )
        {
            lBridge->StartAcquisition();
        }
    }
}


///
/// \brief Force streaming stop
///

void Player::ForceStop()
{
    PV_LOGINFO( mLogger, "ForceStop invoked" );

    if ( IsDeviceConnected() )
    {
        // Stop the grabber camera if applicable
        PvCameraBridge *lBridge = GetSelectedBridge();
        if ( lBridge != NULL )
        {
            lBridge->StopAcquisition();
        }

        // Stop device/grabber last
        mDevice->ForceStopAcquisition();

        mImageSaving->NotifyStreamingStop();
    }
}


///
/// \brief Stop acquisition if current device state allows/requires it
///

void Player::StopIfApplicable()
{
    PV_LOGINFO( mLogger, "StopIfApplicable invoked" );

    if ( IsDeviceConnected() )
    {
        // Stop the grabber camera if applicable
        if ( !mDevice->IsUnconditionalStreamingEnabled() )
        {
            PvCameraBridge *lBridge = GetSelectedBridge();
            if ( lBridge != NULL )
            {
                lBridge->StopAcquisition();
            }
        }

        // Stop device/grabber last
        mDevice->StopAcquisitionIfApplicable();
    }
}


///
/// \brief Returns true if link recovery is enabled for the connected device
///

bool Player::IsLinkRecoveryEnabled()
{
    if ( !IsDeviceConnected() )
    {
        return false;
    }

    PvGenBoolean *lLinkRecoveryEnabled = mDevice->GetCommunicationParameters()->GetBoolean( LINK_RECOVERY_ENABLED );
    if ( lLinkRecoveryEnabled == NULL )
    {
        return false;
    }

    bool lEnabled = false;
    lLinkRecoveryEnabled->GetValue( lEnabled );

    return lEnabled;
}


///
/// \brief Application notified us that the link is lost.
///

void Player::LinkDisconnected()
{
    PV_LOGINFO( mLogger, "Link disconnected notification" );

    if ( IsStreamOpened() )
    {
        StopStreaming();
        mStream->Close();
    }
}


///
/// \brief Perform recovery following a device reconnection event
///

void Player::Recover()
{
    PV_LOGINFO( mLogger, "Recovering connection" );

    if ( !IsDeviceConnected() )
    {
        return;
    }

    DeviceGEV *lDeviceGEV = dynamic_cast<DeviceGEV *>( mDevice );
    if ( lDeviceGEV != NULL )
    {
        RecoverGEV();
    }

    DeviceU3V *lDeviceU3V = dynamic_cast<DeviceU3V *>( mDevice );
    if ( lDeviceU3V != NULL )
    {
        RecoverU3V();
    }

#ifdef SERIALBRIDGE
    if ( mCameraBridgeManagerWnd != NULL )
    {
        mCameraBridgeManagerWnd->Recover();
    }
    if ( mSerialBridgeManagerWnd != NULL )
    {
        mSerialBridgeManagerWnd->Recover();
    }
#endif // SERIALBRIDGE
}


///
/// \brief Complete recovery of a GEV device
///

void Player::RecoverGEV()
{
    assert( ( mSetup.GetRole() == Setup::RoleCtrlData ) ||
            ( mSetup.GetRole() == Setup::RoleCtrl ) );

    DeviceGEV *lDeviceGEV = dynamic_cast<DeviceGEV *>( mDevice );
    StreamGEV *lStreamGEV = dynamic_cast<StreamGEV *>( mStream );
    
    std::string lIP = lDeviceGEV->GetIPAddress();
    std::string lLocalIP = lDeviceGEV->GetLocalIPAddress();

    int64_t lChannel = mDevice->GetCurrentSourceChannel();
    if ( mSetup.GetRole() == Setup::RoleCtrlData )
    {
        // Open stream
        PvResult lResult = lStreamGEV->Open( &mSetup, lIP, lLocalIP, static_cast<uint16_t>( lChannel ) );
        if ( !lResult.IsOK() )
        {
            return;
        }

        // Stream is opened, now start it
        StartStreaming();
    }

    // Set stream destination
    lDeviceGEV->SetStreamDestination( &mSetup, mStream, (uint16_t)lChannel );

    // Start acquisition, if needed
    if ( mDevice->IsAcquisitionLocked() && 
        !mDevice->IsUnconditionalStreamingEnabled() )
    {
        ForceStop();
        Play();
    }

    mDevice->ResetRecovery();
}


///
/// \brief Complete recovery of a U3V device
///

void Player::RecoverU3V()
{
    assert( ( mSetup.GetRole() == Setup::RoleCtrlData ) ||
            ( mSetup.GetRole() == Setup::RoleCtrl ) );

    // End point was likely reset, re-open stream
    if ( mStream != NULL )
    {
        StreamU3V *lStreamU3V = dynamic_cast<StreamU3V *>( mStream );
        lStreamU3V->Recover();

        StartStreaming();
    }

    // Start acquisition, if needed
    if ( mDevice != NULL )
    {
        ForceStop();
        Play();
        
        mDevice->ResetRecovery();
    }
}


///
/// \brief Starts streaming
///

void Player::StartStreaming()
{
    PV_LOGINFO( mLogger, "Start streaming" );

    if ( !IsStreamOpened() )
    {
        return;
    }

    mStream->Start( mDisplayThread, mImageSaving, &mBufferOptions, GetDeviceParameters() );
    mController->StartTimer();
}


///
/// \brief Starts streaming
///

void Player::StopStreaming()
{
    PV_LOGINFO( mLogger, "Stop streaming" );

    if ( !IsStreamOpened() )
    {
        return;
    }

    mController->StopTimer();
    mStream->Stop();
}


///
/// \brief Returns communication parameters
///

PvGenParameterArray *Player::GetCommunicationParameters()
{
    if ( !IsDeviceConnected() )
    {
        return NULL;
    }

    return mDevice->GetCommunicationParameters();
}


///
/// \brief Returns device parameters
///

PvGenParameterArray *Player::GetDeviceParameters()
{
    if ( !IsDeviceConnected() )
    {
        return NULL;
    }

    return mDevice->GetParameters();
}


///
/// \brief Returns stream parameters
///

PvGenParameterArray *Player::GetStreamParameters()
{
    if ( !IsStreamOpened() )
    {
        return NULL;
    }

    return mStream->GetParameters();
}


///
/// \brief Loads serial bridge configuration (bridges and camera configuration)
///

PvResult Player::LoadBridges( IProgress *aProgress, PvConfigurationReader *aReader, PvStringList &aErrorList )
{
#ifdef SERIALBRIDGE
    PV_LOGINFO( mLogger, "Loading camera bridges configuration from a PvConfigurationReader" );
    if ( mCameraBridgeManagerWnd != NULL )
    {
        SETSTATUS( aProgress, "Restoring camera bridges configuration..." );

        // Let the serial bridge manager to perform the load
        mCameraBridgeManagerWnd->SetDevice( mDevice->GetDevice() );
        mCameraBridgeManagerWnd->SetStream( mStream->GetStream() );
        mCameraBridgeManagerWnd->Load( *aReader, aErrorList );
    }

    PV_LOGINFO( mLogger, "Loading serial bridges configuration from a PvConfigurationReader" );
    if ( mSerialBridgeManagerWnd != NULL )
    {
        SETSTATUS( aProgress, "Restoring serial communication bridges configuration..." );

        PvPropertyList lList;
        aReader->Restore( "SerialBridgeManager", &lList );

        // Let the serial bridge manager to perform the load
        mSerialBridgeManagerWnd->SetDevice( mDevice->GetDeviceAdapter() );
        mSerialBridgeManagerWnd->Load( lList, aErrorList );
    }
#endif

    return PvResult::Code::OK;
}


///
/// \brief Saves a serial bridge configuration (bridges and camera configuration)
///

PvResult Player::SaveBridges( IProgress *aProgress, PvConfigurationWriter *aWriter, PvStringList &aErrorList )
{
#ifdef SERIALBRIDGE
    PV_LOGINFO( mLogger, "Saving camera bridges configuration to a PvConfigurationWriter" );
    if ( mCameraBridgeManagerWnd != NULL )
    {
        SETSTATUS( aProgress, "Saving camera bridges configuration..." );

        // Save persistence data
        PvResult lResult = mCameraBridgeManagerWnd->Save( *aWriter );
        if ( !lResult.IsOK() )
        {
            return lResult;
        }
    }

    PV_LOGINFO( mLogger, "Saving serial bridges configuration to a PvConfigurationWriter" );
    if ( mSerialBridgeManagerWnd != NULL )
    {
        SETSTATUS( aProgress, "Saving serial communication bridges configuration..." );

        // Get persistence data
        PvPropertyList lList;
        PvResult lResult = mSerialBridgeManagerWnd->Save( lList );
        if ( !lResult.IsOK() )
        {
            return lResult;
        }

        // Write to persistence file
        aWriter->Store( &lList, "SerialBridgeManager" );
    }
#endif

    return PvResult::Code::OK;
}


///
/// \brief Returns true if the buffer options are in sync with the pipeline configuration
///

bool Player::DoBufferOptionsRequireApply()
{
    if ( !IsStreamOpened() )
    {
        return true;
    }

    return mBufferOptions.HasChanged( mStream->GetPipeline() );
}


///
/// \brief Applies the buffer options to the pipeline
///

PvResult Player::ApplyBufferOptions()
{
    if ( !IsStreamOpened() )
    {
        return PvResult::Code::NOT_CONNECTED;
    }

    return mBufferOptions.Apply( mStream->GetPipeline() );
}


///
/// \brief Resets a flag preventing re-entry
///

void Player::ResetUpdatingSources()
{
    if ( IsDeviceConnected() )
    {
        mDevice->ResetUpdatingSources();
    }
}


///
/// \brief Resets a flag preventing re-entry
///

void Player::ResetUpdatingAcquisitionMode()
{
    if ( IsDeviceConnected() )
    {
        mDevice->ResetUpdatingAcquisitionMode();
    }
}


///
/// \brief Resets streaming statistics
///

void Player::ResetStreamingStatistics()
{
	if ( IsStreamOpened() )
	{
		mStream->ResetStatistics();
	}
}


///
/// \brief Returns the default name of the GenCIAm XML file of the device
///

PvString Player::GetDeviceXMLDefaultName()
{
    if ( !IsDeviceConnected() )
    {
        return "";
    }

    return mDevice->GetDeviceXMLDefaultName();
}


///
/// \brief Saves the GenICam XML file of the device
///

PvResult Player::SaveDeviceXML( const PvString &aFilename )
{
    PV_LOGINFO( mLogger, "Reading the GenICam XML file from the device and saving it" );

    if ( !IsDeviceConnected() )
    {
        return PvResult::Code::NOT_CONNECTED;
    }

    return mDevice->SaveDeviceXML( aFilename );

}


///
/// \brief Returns the PvDevice used for device control.
///
/// We attempt not to use it except where necessary.
///

PvDevice *Player::GetPvDevice()
{
    if ( !IsDeviceConnected() )
    {
        return NULL;
    }

    return mDevice->GetDevice();
}


///
/// \brief Returns the adapter to the PvDevice used for device control.
///

IPvDeviceAdapter *Player::GetPvDeviceAdapter()
{
    if ( !IsDeviceConnected() )
    {
        return NULL;
    }

    return mDevice->GetDeviceAdapter();
}


///
/// \brief Returns the current acquisition mode
///

void Player::GetCurrentAcquisitionMode( ComboItem &aMode, bool &aWritable )
{
    if ( !IsDeviceConnected() )
    {
        return;
    }

    mDevice->GetCurrentAcquisitionMode( aMode, aWritable );
}


///
/// \brief Returns all acquisition modes for the device
///

void Player::GetAcquisitionModes( ComboItemVector &aModes, bool &aWritable )
{
    if ( !IsDeviceConnected() )
    {
        return;
    }


    mDevice->GetAcquisitionModes( aModes, aWritable );
}


///
/// \brief Sets a new acquisition mode on the device
///

PvResult Player::SetAcquisitionMode( int64_t aNewMode )
{
    PV_LOGINFO( mLogger, "Setting acquisition mode to " << aNewMode );

    if ( !IsDeviceConnected() )
    {
        return PvResult::Code::NOT_CONNECTED;
    }

    return mDevice->SetAcquisitionMode( aNewMode );
}


///
/// \brief Returns all sources supported by the device
///

void Player::GetSources( ComboItemVector &aSources )
{
    if ( !IsDeviceConnected() )
    {
        return;
    }

    mDevice->GetSources( aSources );
}


///
/// \brief Returns the default communication parameters in a string
///

void Player::GetDefaultCommunicationParameters( const PvDeviceInfo *aDeviceInfo, PvString &aParameters )
{
    PvConfigurationWriter lWriter;

    PvGenParameterArray *lParameters = NULL;
    switch ( aDeviceInfo->GetType() )
    {
    case PvDeviceInfoTypeGEV:
        lParameters = sDefaultDeviceGEV.GetCommunicationParameters();
        break;

    case PvDeviceInfoTypeU3V:
        lParameters = sDefaultDeviceU3V.GetCommunicationParameters();
        break;

    default:
        assert( 0 );
    }

    if ( lParameters == NULL )
    {
        return;
    }

    lWriter.Store( lParameters, "Communication" );
    lWriter.SaveToString( aParameters );
}


///
///  \brief Updates the device attributes from a device info
///

void Player::UpdateAttributes( const PvDeviceInfo *aDI )
{
    if ( aDI == NULL )
    {
        PV_LOGWARNING( mLogger, "No device info required to update attributes" );
        return;
    }

    mDeviceAttributes.Reset();

    mDeviceAttributes.mVendor = aDI->GetVendorName().GetAscii();
    mDeviceAttributes.mModel = aDI->GetModelName().GetAscii();
    mDeviceAttributes.mName = aDI->GetUserDefinedName().GetAscii();

    const PvDeviceInfoGEV *lDeviceInfoGEV = dynamic_cast<const PvDeviceInfoGEV *>( aDI );
    if ( lDeviceInfoGEV != NULL )
    {
        mDeviceAttributes.mIP = lDeviceInfoGEV->GetIPAddress().GetAscii();
        mDeviceAttributes.mMAC = lDeviceInfoGEV->GetMACAddress().GetAscii();
    }
    else
    {
        mDeviceAttributes.mIP = NA_STRING;
        mDeviceAttributes.mMAC = NA_STRING;
    }

    const PvDeviceInfoU3V *lDeviceInfoU3V = dynamic_cast<const PvDeviceInfoU3V *>( aDI );
    if ( lDeviceInfoU3V != NULL )
    {
        mDeviceAttributes.mGUID = lDeviceInfoU3V->GetDeviceGUID().GetAscii();
    }
    else
    {
        mDeviceAttributes.mGUID = NA_STRING;
    }
}


///
/// \brief Is the stream opened?
///

bool Player::IsStreamOpened() 
{ 
    mStreamMutex.Lock();
    bool lOpened = ( mStream != NULL ) && mStream->IsOpened(); 
    mStreamMutex.Unlock();

    return lOpened;
}


///
/// \brief Is the stream started? (display thread running)
///

bool Player::IsStreamStarted() 
{ 
    mStreamMutex.Lock();
    bool lStarted = ( mStream != NULL ) && mStream->IsOpened() && mStream->IsStarted(); 
    mStreamMutex.Unlock();

    return lStarted;
}


///
/// \brief Sends messages to the controller to update itself
///

void Player::UpdateController()
{
    mController->SendMsg( WM_UPDATEACQUISITIONMODES );
    mController->SendMsg( WM_UPDATESOURCES, 0, mPreferredSource );
    mController->SendMsg( WM_UPDATEACQUISITIONMODE );
}


///
/// \brief Returns the selected bridge vs device's selected source.
///
/// For now we assume that source = camrea bridge ordering in manager.
///
/// This method can and does return NULL if we cannot map the selected source
/// to a serial port or if the camera bridge related to the selected source
/// is not active.
///
/// This only applies to camera bridges, not serial bridges.
///

PvCameraBridge *Player::GetSelectedBridge()
{
#ifdef SERIALBRIDGE
    if ( ( mDevice != NULL ) &&
         ( mCameraBridgeManagerWnd != NULL ) )
    {
        int64_t lSource = mDevice->GetSelectedSource();
        if ( lSource < 0 )
        {
            // Non multi source devices return negative, we want 0
            lSource = 0;
        }

        if ( lSource < mCameraBridgeManagerWnd->GetBridgeCount() )
        {
            PvCameraBridge *lBridge = mCameraBridgeManagerWnd->GetBridge( static_cast<int>( lSource ) );
            if ( lBridge->IsConnected() )
            {
                return lBridge;
            }
        }
    }
#endif

    return NULL;
}


///
/// \brief Returns true if app preferences changed.
///
/// Includes all preferences *but* the default communication parameters.
/// The default communication parameters are handled at the browser level.
///

bool Player::HavePreferencesChanged() const
{
    ChangedList::const_iterator lIt = mChangedList.begin();
    while ( lIt != mChangedList.end() )
    {
        if ( ( *lIt++ )->HasChanged() )
        {
            return true;
        }
    }

    return false;
}


///
/// \brief Resets app preferences changed to false.
///

void Player::ResetPreferencesChanged()
{
    ChangedList::iterator lIt = mChangedList.begin();
    while ( lIt != mChangedList.end() )
    {
        ( *lIt++ )->ResetChanged();
    }
}

