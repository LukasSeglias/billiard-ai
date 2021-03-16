// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"
#include "eBUSPlayerDlg.h"

#include "PvMessageBox.h"
#include "ErrorsDlg.h"
#include "ProgressDlg.h"
#include "PersistenceInfoDlg.h"
#include "eBUSPlayerFactory.h"

#include <PvDeviceInfoGEV.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define TAG_APP ( "ebusplayerconfig" ) 
#define TAG_DEFAULTCOMMGEVBROWSER ( "defaultcommgevbrowser" )
#define TAG_DEFAULTCOMMU3VBROWSER ( "defaultcommu3vbrowser" )
#define TAG_COMMUNICATIONBROWSER ( "communicationbrowser" )
#define TAG_DEVICEBROWSER ( "devicebrowser" )
#define TAG_STREAMBROWSER ( "streambrowser" )
#define TAG_GEVCOMMUNICATION ( "gevcommunication" )
#define TAG_U3VCOMMUNICATION ( "u3vcommunication" )


BEGIN_MESSAGE_MAP(eBUSPlayerDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_SIZE()
    ON_WM_GETMINMAXINFO()
    ON_BN_CLICKED(IDC_DEVICE_BUTTON, &eBUSPlayerDlg::OnBnClickedDeviceButton)
    ON_BN_CLICKED(IDC_LINK_BUTTON, &eBUSPlayerDlg::OnBnClickedLinkButton)
    ON_BN_CLICKED(IDC_STREAMPARAMS_BUTTON, &eBUSPlayerDlg::OnBnClickedStreamparamsButton)
    ON_BN_CLICKED( IDC_CAMERAPARAMS_BUTTON, &eBUSPlayerDlg::OnToolsCameraBridge)
    ON_BN_CLICKED(IDC_CONNECT_BUTTON, &eBUSPlayerDlg::OnBnClickedConnectButton)
    ON_BN_CLICKED(IDC_DISCONNECT_BUTTON, &eBUSPlayerDlg::OnBnClickedDisconnectButton)
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_START, &eBUSPlayerDlg::OnBnClickedStart)
    ON_BN_CLICKED(IDC_STOP, &eBUSPlayerDlg::OnBnClickedStop)
    ON_CBN_SELCHANGE(IDC_MODE, &eBUSPlayerDlg::OnCbnSelchangeMode)
    ON_MESSAGE(WM_IMAGEDISPLAYED, OnImageDisplayed)
    ON_COMMAND(ID_FILE_LOAD, &eBUSPlayerDlg::OnFileLoad)
    ON_COMMAND(ID_FILE_SAVE, &eBUSPlayerDlg::OnFileSave)
    ON_COMMAND(ID_FILE_EXIT, &eBUSPlayerDlg::OnFileExit)
    ON_COMMAND(ID_TOOLS_EVENTMONITOR, &eBUSPlayerDlg::OnToolsEventMonitor)
    ON_COMMAND(ID_HELP_ABOUT, &eBUSPlayerDlg::OnHelpAbout)
    ON_COMMAND_RANGE(ID_FILE_RECENT1, ID_FILE_RECENT10, &eBUSPlayerDlg::OnRecent)
    ON_WM_INITMENU()
    ON_WM_MOVE()
    ON_WM_CTLCOLOR()
    ON_COMMAND(ID_FILE_SAVEAS, &eBUSPlayerDlg::OnFileSaveAs)
    ON_COMMAND(ID_ACCELERATOR_SAVE, &eBUSPlayerDlg::OnAcceleratorSave)
    ON_COMMAND(ID_ACCELERATOR_OPEN, &eBUSPlayerDlg::OnAcceleratorOpen)
    ON_COMMAND(ID_TOOLS_SETUP, &eBUSPlayerDlg::OnToolsSetup)
    ON_COMMAND(ID_TOOLS_IMAGEFILTERING, &eBUSPlayerDlg::OnToolsImagefiltering)
    ON_COMMAND(ID_TOOLS_SERIALCOMMUNICATIONBRIDGE, &eBUSPlayerDlg::OnToolsSerialCommunicationBridge)
    ON_COMMAND(ID_TOOLS_CAMERABRIDGE, &eBUSPlayerDlg::OnToolsCameraBridge)
    ON_COMMAND(ID_TOOLS_ACTIONCOMMAND, &eBUSPlayerDlg::OnToolsActionCommand)    
    ON_COMMAND(ID_TOOLS_SAVEIMAGES, &eBUSPlayerDlg::OnToolsSaveimages)
    ON_COMMAND(ID_TOOLS_SAVECURRENTIMAGE, &eBUSPlayerDlg::OnToolsSavecurrentimage)
    ON_COMMAND(ID_TOOLS_DEVICESERIALCOMMUNICATION, &eBUSPlayerDlg::OnToolsSerialCommunication)
    ON_WM_TIMER()
    ON_WM_DESTROY()
    ON_COMMAND(ID_REGISTERINTERFACE, &eBUSPlayerDlg::OnRegisterInterface)
    ON_COMMAND(ID_DISPLAYIMAGES_DISABLED, &eBUSPlayerDlg::OnDisplayDisabled)
    ON_COMMAND(ID_DISPLAYIMAGES_30FPS, &eBUSPlayerDlg::OnDisplay30FPS)
    ON_COMMAND(ID_DISPLAYIMAGES_60FPS, &eBUSPlayerDlg::OnDisplay60FPS)
    ON_COMMAND(ID_TOOLS_SAVEPREFERENCES, &eBUSPlayerDlg::OnToolsSavepreferences)
    ON_COMMAND(ID_TOOLS_RESTOREDEFAULTPREFERENCES, &eBUSPlayerDlg::OnToolsRestoredefaultpreferences)
    ON_COMMAND(ID_TOOLS_BUFFEROPTIONS, &eBUSPlayerDlg::OnToolsBufferoptions)
	ON_COMMAND(ID_TOOLS_RESET_STREAMING_STATISTICS, &eBUSPlayerDlg::OnToolsResetStreamingStatistics)
    ON_COMMAND(ID_TOOLS_DISPLAYPARTIALIMAGES, &eBUSPlayerDlg::OnToolsDisplaypartialimages)
    ON_COMMAND(ID_TOOLS_DISPLAYCHUNKDATA, &eBUSPlayerDlg::OnToolsDisplaychunkdata)
    ON_COMMAND(ID_TOOLS_SAVE_XML, &eBUSPlayerDlg::OnToolsSaveXml)
    ON_MESSAGE(WM_LINKDISCONNECTED, &eBUSPlayerDlg::OnLinkDisconnected)
    ON_MESSAGE(WM_LINKRECONNECTED, &eBUSPlayerDlg::OnLinkReconnected)
    ON_MESSAGE(WM_ACQUISITIONSTATECHANGED, &eBUSPlayerDlg::OnAcquisitionStateChanged)
    ON_CBN_SELCHANGE(IDC_COMBO_INPUT, &eBUSPlayerDlg::OnCbnSelchangeComboSource)
    ON_COMMAND(ID_TOOLS_DEINTERLACING_DISABLED, &eBUSPlayerDlg::OnToolsDeinterlacingDisabled)
    ON_COMMAND(ID_TOOLS_DEINTERLACING_WEAVE_HALF, &eBUSPlayerDlg::OnToolsDeinterlacingWeavingHalf)
    ON_COMMAND(ID_TOOLS_DEINTERLACING_WEAVE_FULL, &eBUSPlayerDlg::OnToolsDeinterlacingWeavingFull)
    ON_COMMAND(ID_TOOLS_DEINTERLACING_BLENDING, &eBUSPlayerDlg::OnToolsDeinterlacingBlending)
    ON_COMMAND(ID_TOOLS_DEINTERLACING_LINEDOUBLING, &eBUSPlayerDlg::OnToolsDeinterlacingLineDoubling)
    ON_MESSAGE(WM_UPDATESOURCE, &eBUSPlayerDlg::OnUpdateSource)
    ON_MESSAGE(WM_UPDATESOURCES, &eBUSPlayerDlg::OnUpdateSources)
    ON_MESSAGE(WM_UPDATEACQUISITIONMODE, &eBUSPlayerDlg::OnUpdateAcquisitionMode)
    ON_MESSAGE(WM_UPDATEACQUISITIONMODES, &eBUSPlayerDlg::OnUpdateAcquisitionModes)
    ON_COMMAND(ID_TOOLS_DEFAULTGEVCOMMUNICATIONPARAMETERS, &eBUSPlayerDlg::OnToolsDefaultGEVCommunicationParameters)
    ON_COMMAND(ID_TOOLS_DEFAULTU3VCOMMUNICATIONPARAMETERS, &eBUSPlayerDlg::OntoolsDefaultU3VCommunicationParameters)
    ON_COMMAND(ID_TOOLS_FILETRANSFER, &eBUSPlayerDlg::OnToolsFileTransfer)
END_MESSAGE_MAP()


// =============================================================================
eBUSPlayerDlg::eBUSPlayerDlg( CString aFileName, CWnd* pParent /* =NULL */ )
    : CDialog( eBUSPlayerDlg::IDD, pParent )
    , mNeedInit( TRUE )
    , mEventMonitorDlg( NULL )
    , mActionCommandDlg( NULL )
    , mMRUMenu( NULL )
    , mFileName( aFileName )
    , mStatusColor( SCDefault )
    , mTimer( 0 )
    , mFilteringDlg( NULL )
    , mPlayer( NULL )
    , mDeviceWnd( NULL )
{
    m_hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );

    mPlayButton.SetBitmap( IDB_PLAY );
    mStopButton.SetBitmap( IDB_STOP );

    mPlayer = new Player( this, &mDisplay );
    mFilteringDlg = new FilteringDlg( mPlayer->GetImageFiltering(), mPlayer->GetDisplayThread(), this );
    mMRUMenu = new CMenu;

    eBUSPlayerFactory lFactory;
    mDeviceWnd = lFactory.CreateDeviceBrowser();

    ResetChangedSession();
    ResetChangedPreferences();
}


// =============================================================================
eBUSPlayerDlg::~eBUSPlayerDlg()
{
    PVDELETE( mMRUMenu );
    PVDELETE( mFilteringDlg );
    PVDELETE( mPlayer );
    PVDELETE( mEventMonitorDlg );
    PVDELETE( mDeviceWnd );
}


// =============================================================================
void eBUSPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_MODE, mModeCombo);
    DDX_Control(pDX, IDC_START, mPlayButton);
    DDX_Control(pDX, IDC_STOP, mStopButton);
    DDX_Control(pDX, IDC_STATUS, mStatusTextBox);
    DDX_Control(pDX, IDC_IP_EDIT, mIPEdit);
    DDX_Control(pDX, IDC_MAC_EDIT, mMACEdit);
    DDX_Control(pDX, IDC_GUID_EDIT, mGUIDEdit);
    DDX_Control(pDX, IDC_MODEL_EDIT, mModelEdit);
    DDX_Control(pDX, IDC_MANUFACTURER_EDIT, mVendorEdit);
    DDX_Control(pDX, IDC_NAME_EDIT2, mNameEdit);
    DDX_Control(pDX, IDC_COMBO_INPUT, mSourceCombo);
    DDX_Control( pDX, IDC_LINK_BUTTON, mCommunicationButton );
    DDX_Control( pDX, IDC_DEVICE_BUTTON, mDeviceButton );
    DDX_Control( pDX, IDC_CAMERAPARAMS_BUTTON, mCameraButton );
    DDX_Control( pDX, IDC_STREAMPARAMS_BUTTON, mStreamButton );
}


// =============================================================================
void eBUSPlayerDlg::EnableInterface()
{
    // This method can be called really early or late when the window is not created
    if ( GetSafeHwnd() == 0 )
    {
        return;
    }

    GetDlgItem( IDC_CONNECT_BUTTON )->EnableWindow( !mPlayer->IsDeviceConnected() && !mPlayer->IsStreamOpened() );
    GetDlgItem( IDC_DISCONNECT_BUTTON )->EnableWindow( mPlayer->IsDeviceConnected() || mPlayer->IsStreamOpened() );

    EnableTreeBrowsers( TRUE );
    EnableControls( mPlayer->IsDeviceConnected() );
}


// =============================================================================
void eBUSPlayerDlg::EnableControls( BOOL aEnabled )
{
    ControlsState lState;
    mPlayer->GetControlsEnabled( lState );

    mPlayButton.EnableWindow( aEnabled && lState.mStart );
    mStopButton.EnableWindow( aEnabled && lState.mStop );
    mModeCombo.EnableWindow( aEnabled && lState.mAcquisitionMode );
    mSourceCombo.EnableWindow( aEnabled && lState.mSource );
}


// =============================================================================
void eBUSPlayerDlg::EnableTreeBrowsers( BOOL aEnabled )
{
    mCommunicationButton.EnableWindow( aEnabled && mPlayer->IsDeviceConnected() );
    mDeviceButton.EnableWindow( aEnabled && mPlayer->IsDeviceConnected() );
    mStreamButton.EnableWindow( aEnabled && mPlayer->IsStreamOpened() );

#ifdef SERIALBRIDGE
    if ( mPlayer->IsDeviceConnected() && mPlayer->IsDeviceFrameGrabber() )
    {
        mCameraButton.ShowWindow( SW_SHOW );
        mCameraButton.EnableWindow( aEnabled );

        CRect lRect;
        GetWindowRect( lRect );
        if ( lRect.Height() < MIN_FG_HEIGHT )
        {
            lRect.bottom = lRect.top + MIN_FG_HEIGHT;
            MoveWindow( lRect );
        }
    }
    else
#endif
    {
        mCameraButton.ShowWindow( SW_HIDE );
        mCameraButton.EnableWindow( FALSE );
    }
}


// =============================================================================
void eBUSPlayerDlg::Connect( const PvDeviceInfo *aDI )
{
    ASSERT( aDI != NULL );
    if ( aDI == NULL )  
    {
        return;
    }

    if ( aDI != NULL )
    {
        PvDeviceClass lClass = PvDeviceClassTransmitter;

        const PvDeviceInfoGEV *lDeviceInfoGEV = dynamic_cast<const PvDeviceInfoGEV *>( aDI );
        if ( lDeviceInfoGEV != NULL )
        {
            lClass = lDeviceInfoGEV->GetClass();
        }

        Setup::Role lRole = mPlayer->GetSetupRole();

        if ( ( lClass == PvDeviceClassReceiver ) || 
             ( lClass == PvDeviceClassPeripheral ) ||
             ( lClass == PvDeviceClassTransceiver ) )
        {
            // For receiver and peripheral, we make sure  role is controller only
            if ( lRole == Setup::RoleCtrlData || lRole == Setup::RoleData )
            {
                CString lMessage;

                if ( ( lClass == PvDeviceClassReceiver ) ||
                     ( lClass == PvDeviceClassPeripheral ) )
                {
                    // Receiver and peripheral message
                    lMessage = _T( "You have chosen to connect to a " );
                    lMessage += ( lClass == PvDeviceClassReceiver ) ? _T( "receiver" ) : _T( "peripheral" );
                    lMessage += _T( " however " ) + GetAppName() + _T( " is currently set up to connect as a " );
                    lMessage += ( lRole == Setup::RoleCtrlData ) ? _T( "controller and data receiver" ) : _T( "data receiver" );
                    lMessage += _T( ". \r\n\r\nRe-attempt connection as a controller?" );
                }
                else if ( lClass == PvDeviceClassTransceiver )
                {
                    // Transceiver message
                    lMessage = _T( "You have chosen to connect to a transceiver. " ) + GetAppName() + _T( " does not fully\r\n" );
                    lMessage += _T( "support transceivers in this version. Re-attempt connection as a \r\n" );
                    lMessage += _T( "controller?" );
                }
                else
                {
                    ASSERT( 0 );
                }

                if ( MessageBox( lMessage, GetAppName(), MB_YESNO | MB_ICONQUESTION ) == IDNO )
                {
                    return;
                }
                else
                {
                    mPlayer->ResetSetup();
                    mPlayer->GetSetup()->SetRole( Setup::RoleCtrl );
                }
            }
        }
    }

    // Connect the device and/or stream
    PvResult lResult = mPlayer->Connect( aDI, NULL );
    if ( !lResult.IsOK() )
    {
        if ( lResult.GetCode() == PvResult::Code::CANT_READ_MANIFEST )
        {
            // This error typically happens with the (u3v) device is not powered properly and so 
            // the FX3 board is up and the device can be discovered but Nios is not running and so
            // it cannot be connected to. 
            PvString lConnectPowerError( "Connection Failed. Ensure the device has the correct amount of power supplied to it. "
                                         "For example, your device may require an external power supply when powering a camera. "
                                         "See the documentation accompanying the video interface for detailed power information and troubleshooting tips. "
                                         "Details: " );
            PvString lManifestReadError = lResult.GetCodeString().GetUnicode();
            lConnectPowerError += lManifestReadError;
            lResult.SetDescription( lConnectPowerError );
            PvMessageBox( this, lResult );
        }
        else
        {
            PvMessageBox( this, lResult );
        }
    }

    CreateEventMonitor();
    UpdateDeviceAttributes();
    EnableInterface();

    ResetChangedSession();
}


// =============================================================================
void eBUSPlayerDlg::Disconnect( BOOL aLinkDisconnected )
{
    // Close all configuration child windows
    CloseWnd( &mCommunicationWnd );
    CloseWnd( mDeviceWnd );
    CloseWnd( &mStreamParametersWnd );
    mFilteringDlg->ShowWindow( SW_HIDE );
    if ( mSerialTerminalWnd.GetHandle() != 0 )
    {
        mSerialTerminalWnd.Close();
    }
#ifdef SERIALBRIDGE
    CloseWnd( mPlayer->GetCameraBridgeManager() );
    CloseWnd( mPlayer->GetSerialBridgeManager() );
#endif // SERIALBRIDGE
    if ( mRegisterInterfaceDlg.GetSafeHwnd() != 0 )
    {
        mRegisterInterfaceDlg.EndDialog( IDOK );
    }

    // Show the persistence warning if needed
    if ( !aLinkDisconnected && 
         ( mPlayer->IsDeviceConnected() || 
           mPlayer->IsStreamOpened() ) )
    {
        // Display persistence warning for device connection
        PersistenceWarning();
    }

    CWaitCursor lCursor;

    // Call the model's disconnect method
    mPlayer->Disconnect();

    // Clear the display
    mDisplay.Clear();

    // Blank out UI
    if ( GetSafeHwnd() != 0 )
    {
        mVendorEdit.SetWindowText( _T( "" ) );
        mModelEdit.SetWindowText( _T( "" ) );
        mIPEdit.SetWindowText( _T( "" ) );
        mMACEdit.SetWindowText( _T( "" ) );
        mNameEdit.SetWindowText( _T( "" ) );
        mGUIDEdit.SetWindowText( _T( "" ) );
    }

    mStatusTextBox.SetWindowText( _T( "" ) );
    SetStatusColor( SCDefault );

    // Sync the UI with our new status
    EnableInterface();
    SetWindowText( GetAppName() );

    mSourceCombo.ResetContent();
    mModeCombo.ResetContent();

    DrawMenuBar();
}


// =============================================================================
void eBUSPlayerDlg::PersistenceWarning()
{
    UINT lDontShowAgain = AfxGetApp()->GetProfileInt( _T(""), 
        _T( "DontShowPersistenceWarning" ), 0 );
    if ( lDontShowAgain == 0 )
    {
        PersistenceInfoDlg lDlg;

        bool lChanged = false;
        if ( !mPlayer->IsDeviceConnected() )
        {
            // Check preferences
            lDlg.SetPreferencesContext();
            lChanged = mPlayer->HavePreferencesChanged() ||
                mDefaultCommGEVParametersWnd.HasChanged() ||
                mDefaultCommU3VParametersWnd.HasChanged() ||
                HaveBrowserOptionsChanged();
        }
        else
        {
            // Check session status
            lDlg.SetDeviceContext();
            lChanged = ( ( mDeviceWnd != NULL ) && mDeviceWnd->HasChanged() ) ||
                mCommunicationWnd.HasChanged() ||
                mStreamParametersWnd.HasChanged();
        }

        // If the configuration changed, display the info/warning dialog
        if ( lChanged )
        {
            lDlg.DoModal();
            if ( lDlg.IsDontShowAgain() )
            {
                AfxGetApp()->WriteProfileInt( _T( "" ), 
                    _T( "DontShowPersistenceWarning" ), 1 );
            }
        }
    }

    if ( !mPlayer->IsDeviceConnected() )
    {
        ResetChangedPreferences();
    }
    else
    {
        ResetChangedSession();
    }
}


// =============================================================================
void eBUSPlayerDlg::ShowGenWnd( PvGenBrowserWnd *aWnd, PvGenParameterArray *aParams, const CString &aTitle )
{
    if ( aWnd->GetHandle() != 0 )
    {
        // If already open, just toggle to closed...     
        CloseWnd( aWnd );
        return;
    }

    // Create, assign parameters, set title and show modeless
    aWnd->SetTitle( PvString( aTitle ) );
    aWnd->SetGenParameterArray( aParams );
    aWnd->ShowModeless( GetSafeHwnd() );
}


// =============================================================================
void eBUSPlayerDlg::CloseWnd( PvWnd *aWnd )
{
    // If the window object is currently created (visible), close/destroy it
    if ( ( aWnd != NULL ) && 
         ( aWnd->GetHandle() != 0 ) )
    {
        ( aWnd )->Close();
    }
}


// =============================================================================
void eBUSPlayerDlg::SetStatusColor( StatusColor aColor )
{
    mStatusColor = aColor;
    mStatusTextBox.Invalidate();
}


// =============================================================================
DWORD eBUSPlayerDlg::ReadRegistryDWORD( const CString &aName )
{
    HKEY hKey = 0;
    DWORD lType = REG_DWORD;
    DWORD lSize = 4;
    DWORD lValue = 0;
    LONG lWinResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE, _T( "SOFTWARE\\Pleora Technologies Inc\\eBUSPlayer" ), 0L,  KEY_READ, &hKey );
    if ( lWinResult == ERROR_SUCCESS )
    {
        lWinResult = RegQueryValueEx( hKey, aName, NULL, &lType, (LPBYTE)&lValue, &lSize );
        if ( lWinResult == ERROR_SUCCESS )
        {
            return lValue;
        }

        ::CloseHandle( hKey );
    }

    return 0;
}


// =============================================================================
intptr_t eBUSPlayerDlg::SendMsgIfPossible( uint32_t Msg, uintptr_t wParam, intptr_t lParam )
{
    if ( AfxGetApp()->m_nThreadID == ::GetCurrentThreadId() )
    {
        // Same thread, use send (ends up being a direct call)
        return CWnd::SendMessage( Msg, wParam, lParam );
    }


    // Different thread, we want to use post to prevent deadlocks
    return CWnd::PostMessage( Msg, wParam, lParam );
}


// =============================================================================
void eBUSPlayerDlg::OpenConfig( const CString &aFileName )
{
    if ( mCommunicationWnd.GetHandle() != 0 )
    {
        MessageBox( _T( "Please close the communication control window before loading a new configuration." ), GetAppName() );
        return;
    }
    if ( mStreamParametersWnd.GetHandle() != 0 )
    {
        MessageBox( _T( "Please close the image stream control window before loading a new configuration." ), GetAppName() );
        return;
    }
    if ( ( mDeviceWnd != NULL ) && ( mDeviceWnd->GetHandle() != 0 ) )
    {
        MessageBox( _T( "Please close the Device control window before loading a new configuration." ), GetAppName() );
        return;
    }
#ifdef SERIALBRIDGE
    if ( ( mPlayer->GetCameraBridgeManager() != NULL ) &&
         ( mPlayer->GetCameraBridgeManager()->GetHandle() != 0 ) )
    {
        MessageBox( _T( "Please close the Camera control window before loading a new configuration." ), GetAppName() );
        return;
    }
#endif

    // Create reader, load file
    PvConfigurationReader lReader;
    PvResult lResult = lReader.Load( (LPCTSTR)aFileName );
    if ( !lResult.IsOK() )
    {
        PvMessageBox( this, lResult );
        return;
    }

    // we're loading a new persistence file. Report it now so that it's visible in the title bar while we're loading.
    if ( !mFileName.IsEmpty() && ( aFileName == mFileName ) )
    {
        ReportMRU( aFileName );
    }

    // Check if roles are the same if not, we must disconnect first
    if ( mPlayer->IsSameAsCurrent( &lReader ) )
    {
        if ( MessageBox( _T( "Setup of this configuration file does not match the current ")
            _T( "connection.\r\n\r\nIn order to proceed with the operation, ")
            _T( "the device will be disconnected.\r\n\r\nContinue ")
            _T( "restoring configuration?"), 
            GetAppName(), MB_YESNO | MB_ICONQUESTION ) == IDNO )
        {
            return;
        }    

        Disconnect();
    }

    bool lContextOnly = ( lReader.GetDeviceCount() <= 0 ) && ( lReader.GetStreamCount() <= 0 ); 
    if ( !lContextOnly )
    {
        // If we're connected, just apply the settings. Otherwise connect from the settings in the persistence file.
        if ( mPlayer->IsDeviceConnected() )
        {
            if ( MessageBox( _T( "Opening a file now will apply the settings to the currently connected " )
                _T( "device. \r\n\r\nIf you wish to connect to a different device associated " )
                _T( "with the file, disconnect first, then open the file. \r\n\r\nContinue opening file?" ), 
                GetAppName(), MB_YESNO | MB_ICONQUESTION ) == IDNO )
            {
                return;
            }
        }
    }

    PvStringList lErrorList;
    mPlayer->OpenConfig( &lReader, lErrorList, !lContextOnly );

    if ( lErrorList.GetSize() > 0 )
    {
        ErrorsDlg lDlg;
        lDlg.DoModal( &lErrorList );
    }

    OpenBrowserOptions( &lReader );

    lReader.Restore( TAG_GEVCOMMUNICATION, Player::GetDefaultGEVCommunicationParameters() );
    lReader.Restore( TAG_U3VCOMMUNICATION, Player::GetDefaultU3VCommunicationParameters() );

    CreateEventMonitor();
    UpdateDeviceAttributes();
    mFilteringDlg->ToDialog();
    EnableInterface();

    ResetChangedSession();
    ResetChangedPreferences();
}


// =============================================================================
void eBUSPlayerDlg::OpenBrowserOptions( PvConfigurationReader *aReader )
{
    PvPropertyList lCommunicationBrowserOptions;
    if ( aReader->Restore( TAG_COMMUNICATIONBROWSER, &lCommunicationBrowserOptions ).IsOK() )
    {
        mCommunicationWnd.Load( lCommunicationBrowserOptions );        
    }
    PvPropertyList lDeviceBrowserOptions;
    if ( aReader->Restore( TAG_DEVICEBROWSER, &lDeviceBrowserOptions ).IsOK() && 
         ( mDeviceWnd != NULL ) )
    {
        mDeviceWnd->Load( lDeviceBrowserOptions );        
    }
    PvPropertyList lStreamBrowserOptions;
    if ( aReader->Restore( TAG_STREAMBROWSER, &lStreamBrowserOptions ).IsOK() )
    {
        mStreamParametersWnd.Load( lStreamBrowserOptions );        
    }
    PvPropertyList lDefaultCommU3VBrowserOptions;
    if ( aReader->Restore( TAG_DEFAULTCOMMU3VBROWSER, &lDefaultCommU3VBrowserOptions ).IsOK() )
    {
        mDefaultCommU3VParametersWnd.Load( lDefaultCommU3VBrowserOptions );        
    }
    PvPropertyList lDefaultCommGEVBrowserOptions;
    if ( aReader->Restore( TAG_DEFAULTCOMMGEVBROWSER, &lDefaultCommGEVBrowserOptions ).IsOK() )
    {
        mDefaultCommGEVParametersWnd.Load( lDefaultCommGEVBrowserOptions );        
    }
}


// =============================================================================
void eBUSPlayerDlg::SaveConfig( const CString &aLocation, bool aSaveConnectedDevice )
{
    // Create configuration writer
    PvConfigurationWriter lWriter;
    lWriter.Store( "1", TAG_APP ); 

    // Save GenICam browsers configuration
    SaveBrowserOptions( &lWriter );

    // GEV default communication parameters
    lWriter.Store( Player::GetDefaultGEVCommunicationParameters(), TAG_GEVCOMMUNICATION );

    // U3V default communication parameters
    lWriter.Store( Player::GetDefaultU3VCommunicationParameters(), TAG_U3VCOMMUNICATION );

    // Save application model
    PvStringList lErrorList;
    mPlayer->SaveConfig( &lWriter, lErrorList, aSaveConnectedDevice );

    // Save the data contained in the writer to file
    lWriter.Save( (LPCTSTR)aLocation );

    // If the error list is not empty, show the errors
    if ( lErrorList.GetSize() > 0 )
    {
        ErrorsDlg lDlg;
        lDlg.DoModal( &lErrorList );
    }

    ResetChangedSession();
    ResetChangedPreferences();
}


// =============================================================================
void eBUSPlayerDlg::SaveBrowserOptions( PvConfigurationWriter *aWriter )
{
    PvPropertyList lPropertyList;

    mCommunicationWnd.Save( lPropertyList );
    aWriter->Store( &lPropertyList, TAG_COMMUNICATIONBROWSER );

    if ( mDeviceWnd != NULL )
    {
        mDeviceWnd->Save( lPropertyList );
        aWriter->Store( &lPropertyList, TAG_DEVICEBROWSER );
    }

    mStreamParametersWnd.Save( lPropertyList );
    aWriter->Store( &lPropertyList, TAG_STREAMBROWSER );

    mDefaultCommU3VParametersWnd.Save( lPropertyList );
    aWriter->Store( &lPropertyList, TAG_DEFAULTCOMMU3VBROWSER );

    mDefaultCommGEVParametersWnd.Save( lPropertyList );
    aWriter->Store( &lPropertyList, TAG_DEFAULTCOMMGEVBROWSER );
}


// =============================================================================
CString eBUSPlayerDlg::GetStickyPath()
{
#ifdef WIN32

    CString lLocation;
    TCHAR lAppData[ MAX_PATH ];
    if ( SHGetSpecialFolderPath( NULL, lAppData, CSIDL_APPDATA, TRUE ) )
    {
        lLocation += lAppData;
    }

    lLocation += _T( "\\Pleora" );
    ::CreateDirectory( lLocation, NULL );
    lLocation += _T( "\\eBUSPlayer.pvcfg" );

    return lLocation;

#endif // WIN32
}


// =============================================================================
CString eBUSPlayerDlg::GetDefaultPath()
{
#ifdef WIN32

    CString lLocation;
    TCHAR lAppData[MAX_PATH];
    if ( SHGetSpecialFolderPath( NULL, lAppData, CSIDL_APPDATA, TRUE ) )
    {
        lLocation += lAppData;
    }

    lLocation += _T( "\\Pleora" );
    ::CreateDirectory( lLocation, NULL );
    lLocation += _T( "\\eBUSPlayerDefaults.pvcfg" );

    return lLocation;

#endif // WIN32
}


// =============================================================================
IProgress *eBUSPlayerDlg::CreateProgressDialog()
{
    return new ProgressDlg( this );
}


// =============================================================================
void eBUSPlayerDlg::UpdateDeviceAttributes()
{
    if ( mPlayer->IsDeviceConnected() || mPlayer->IsStreamOpened() )
    {
        DeviceAttributes &lAttributes = mPlayer->GetDeviceAttributes();

        mVendorEdit.SetWindowText( lAttributes.mVendor.GetUnicode() );
        mModelEdit.SetWindowText( lAttributes.mModel.GetUnicode() );
        mNameEdit.SetWindowText( lAttributes.mName.GetUnicode() );

        mIPEdit.SetWindowText( lAttributes.mIP.GetUnicode() );
        mMACEdit.SetWindowText( lAttributes.mMAC.GetUnicode() );
        mGUIDEdit.SetWindowText( lAttributes.mGUID.GetUnicode() );
    }
}


// =============================================================================
void eBUSPlayerDlg::CreateEventMonitor()
{
    if ( mEventMonitorDlg == NULL )
    {
        mEventMonitorDlg = new EventMonitorDlg( mPlayer->GetLogBuffer(), this );
        mEventMonitorDlg->Create( EventMonitorDlg::IDD, this );
    }

    mEventMonitorDlg->LoadData();
}


// =============================================================================
void eBUSPlayerDlg::StartTimer()
{
    mTimer = SetTimer( 1, 200, NULL );
}


// =============================================================================
void eBUSPlayerDlg::StopTimer()
{
    if ( mTimer != 0 )
    {
        KillTimer( 1 );
        mTimer = 0;
    }
}


// =============================================================================
bool eBUSPlayerDlg::HaveBrowserOptionsChanged()
{
    PvConfigurationWriter lWriter;
    SaveBrowserOptions( &lWriter );

    PvString lNow;
    lWriter.SaveToString( lNow );

    return lNow != mBrowserOptionsBaseline;
}

    
// =============================================================================
void eBUSPlayerDlg::ResetBrowserOptionsChanged()
{
    PvConfigurationWriter lWriter;
    SaveBrowserOptions( &lWriter );

    lWriter.SaveToString( mBrowserOptionsBaseline );
}


// =============================================================================
void eBUSPlayerDlg::ResetChangedSession()
{
    if ( mDeviceWnd != NULL )
    {
        mDeviceWnd->ResetChanged();
    }

    mCommunicationWnd.ResetChanged();
    mStreamParametersWnd.ResetChanged();
}


// =============================================================================
void eBUSPlayerDlg::ResetChangedPreferences()
{
    mDefaultCommGEVParametersWnd.ResetChanged();
    mDefaultCommU3VParametersWnd.ResetChanged();
    ResetBrowserOptionsChanged();
    mPlayer->ResetPreferencesChanged();
}

