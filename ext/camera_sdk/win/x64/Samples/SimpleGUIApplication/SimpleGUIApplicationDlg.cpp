// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"

#include "SimpleGUIApplication.h"
#include "SimpleGUIApplicationDlg.h"

#include <string.h>


#define DEFAULT_PAYLOAD_SIZE ( 1920 * 1080 * 2 )

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


BEGIN_MESSAGE_MAP(SimpleGUIApplicationDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_SIZE()
    ON_WM_GETMINMAXINFO()
    ON_BN_CLICKED(IDC_DEVICE_BUTTON, &SimpleGUIApplicationDlg::OnBnClickedDeviceButton)
    ON_BN_CLICKED(IDC_LINK_BUTTON, &SimpleGUIApplicationDlg::OnBnClickedLinkButton)
    ON_BN_CLICKED(IDC_STREAMPARAMS_BUTTON, &SimpleGUIApplicationDlg::OnBnClickedStreamparamsButton)
    ON_BN_CLICKED(IDC_CONNECT_BUTTON, &SimpleGUIApplicationDlg::OnBnClickedConnectButton)
    ON_BN_CLICKED(IDC_DISCONNECT_BUTTON, &SimpleGUIApplicationDlg::OnBnClickedDisconnectButton)
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_START, &SimpleGUIApplicationDlg::OnBnClickedStart)
    ON_BN_CLICKED(IDC_STOP, &SimpleGUIApplicationDlg::OnBnClickedStop)
    ON_CBN_SELCHANGE(IDC_MODE, &SimpleGUIApplicationDlg::OnCbnSelchangeMode)
    ON_BN_CLICKED(IDC_DEVICEEVENTS_BUTTON, &SimpleGUIApplicationDlg::OnBnClickedDeviceEvents)
    ON_WM_MOVE()
    ON_WM_CTLCOLOR()
    ON_MESSAGE(WM_ACQUISITIONSTATECHANGED, &SimpleGUIApplicationDlg::OnAcquisitionStateChanged)
END_MESSAGE_MAP()


// =============================================================================
SimpleGUIApplicationDlg::SimpleGUIApplicationDlg( CWnd* pParent /* =NULL */ )
    : CDialog( SimpleGUIApplicationDlg::IDD, pParent )
    , mNeedInit( TRUE )
    , mDeviceWnd( NULL )
    , mCommunicationWnd( NULL )
    , mStreamParametersWnd( NULL )
    , mDisplayThread( NULL )
    , mAcquiringImages( false )
    , mAcquisitionStateManager( NULL )
{
    m_hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );

    // Create display thread
    mDisplayThread = new DisplayThread( &mDisplay );
    mDevice = NULL;
    mStream = NULL;
    mPipeline = NULL;
}


// =============================================================================
SimpleGUIApplicationDlg::~SimpleGUIApplicationDlg()
{
    if ( mDisplayThread != NULL )
    {
        delete mDisplayThread;
        mDisplayThread = NULL;
    }

    if ( mAcquisitionStateManager != NULL )
    {
        delete mAcquisitionStateManager;
        mAcquisitionStateManager = NULL;
    }
    if ( mDevice != NULL )
    {
        PvDevice::Free( mDevice );
    }

    if ( mStream != NULL )
    {
        PvStream::Free( mStream );
    }

    if ( mPipeline != NULL )
    {
        delete mPipeline;
        mPipeline = NULL;
    }
}


// =============================================================================
void SimpleGUIApplicationDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_MODE, mModeCombo);
    DDX_Control(pDX, IDC_START, mPlayButton);
    DDX_Control(pDX, IDC_STOP, mStopButton);
    DDX_Control(pDX, IDC_IP_EDIT, mIPEdit);
    DDX_Control(pDX, IDC_MAC_EDIT, mMACEdit);
    DDX_Control(pDX, IDC_GUID_EDIT, mGUIDEdit);
    DDX_Control(pDX, IDC_MODEL_EDIT, mModelEdit);
    DDX_Control(pDX, IDC_MANUFACTURER_EDIT, mManufacturerEdit);
    DDX_Control(pDX, IDC_NAME_EDIT, mNameEdit);
}


// =============================================================================
BOOL SimpleGUIApplicationDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    SetWindowText( _T( "SimpleGUIApplication" ) );

    GetClientRect( mCrt );
    mNeedInit = FALSE;

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);         // Set big icon
    SetIcon(m_hIcon, FALSE);        // Set small icon

    CRect lDisplayRect;
    GetDlgItem( IDC_DISPLAYPOS )->GetClientRect( &lDisplayRect );
    GetDlgItem( IDC_DISPLAYPOS )->ClientToScreen( &lDisplayRect );
    ScreenToClient( &lDisplayRect );
    mDisplay.Create( GetSafeHwnd(), 10000 );
    mDisplay.SetPosition( lDisplayRect.left, lDisplayRect.top, lDisplayRect.Width(), lDisplayRect.Height() );
    mDisplay.SetBackgroundColor( 0x80, 0x80, 0x80 );
    
    EnableInterface();

    return TRUE;  // return TRUE  unless you set the focus to a control
}


// =============================================================================
void SimpleGUIApplicationDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialog::OnPaint();
    }
}


// =============================================================================
HCURSOR SimpleGUIApplicationDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}


// =============================================================================
void SimpleGUIApplicationDlg::OnClose()
{
    // Make sure we cleanup before we leave
    Disconnect();

    CDialog::OnClose();
}


// =============================================================================
void SimpleGUIApplicationDlg::OnBnClickedConnectButton()
{
    // create a device finder wnd and open the select device dialog
    PvDeviceFinderWnd lFinder;
    PvResult lResult = lFinder.ShowModal();

    if ( lResult.GetCode() == PvResult::Code::ABORTED )
    {
        MessageBox( _T( "Invalid selection. Please select a device." ), _T("SimpleGUIApplication") );
        return;
    }

    if ( !lResult.IsOK() || ( lFinder.GetSelected() == NULL ) )
    {
        return;
    }

    CWaitCursor lCursor;

    UpdateWindow();

    const PvDeviceInfo* lDeviceInfo = lFinder.GetSelected();

    if ( lDeviceInfo != NULL )
    {
        Connect( lDeviceInfo );
    }
    else
    {
        MessageBox( _T( "No device selected." ), _T( "Error" ), 
            MB_OK | MB_ICONINFORMATION );
        return;
    }

}


// =============================================================================
void SimpleGUIApplicationDlg::OnBnClickedDisconnectButton()
{
    CWaitCursor lCursor;

    Disconnect();
}


// =============================================================================
void SimpleGUIApplicationDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    CRect lR;
    this->GetWindowRect( &lR );
    // TRACE( "%i %i\n", lR.Width(), lR.Height() );

    if ( mNeedInit || cx == 0 || cy == 0 )
    {
        return;
    }

    int dx = cx - mCrt.Width();
    int dy = cy - mCrt.Height();

    GetClientRect(&mCrt);

    CRect r1;

    HDWP hDWP = ::BeginDeferWindowPos(20);

    //
    // Bottom left, just bring the group box down
    //

    int lBottomLeft[] =
    {
        IDC_CONTROL_GROUP
    };

    for ( int i = 0; i < sizeof( lBottomLeft ) / sizeof ( lBottomLeft[ 0 ] ); i++ )
    {
        CWnd *lChild = GetDlgItem( lBottomLeft[ i ] );
        if ( lChild != NULL )
        {
            lChild->GetWindowRect(&r1); ScreenToClient(&r1); 
            r1.bottom += dy;
            ::DeferWindowPos(
                hDWP, lChild->m_hWnd, NULL, 
                r1.left, r1.top, r1.Width(), r1.Height(),
                SWP_NOACTIVATE|SWP_NOZORDER);
        }
    }

    //
    // Right, make sure the display and group box fill the right part
    // of our window
    //

    int lDisplayGroup[] =
    {
        IDC_DISPLAY_GROUP, IDC_DISPLAYPOS
    };

    for ( int i = 0; i < sizeof( lDisplayGroup ) / sizeof ( lDisplayGroup[ 0 ] ); i++ )
    {
        CWnd *lChild = GetDlgItem( lDisplayGroup[ i ] );
        if ( lChild != NULL )
        {
            lChild->GetWindowRect(&r1); ScreenToClient(&r1); 
            r1.bottom += dy;
            r1.right += dx;
            ::DeferWindowPos(
                hDWP, lChild->m_hWnd, NULL, 
                r1.left, r1.top, r1.Width(), r1.Height(),
                SWP_NOACTIVATE|SWP_NOZORDER);

            if ( lDisplayGroup[i] == IDC_DISPLAYPOS )
            {
                mDisplay.SetPosition( 
                    r1.left, r1.top, 
                    r1.Width(), r1.Height() );
            }
        }
    }

    ::EndDeferWindowPos(hDWP);
}


// =============================================================================
void SimpleGUIApplicationDlg::OnGetMinMaxInfo( MINMAXINFO *lpMMI )
{
    lpMMI->ptMinTrackSize.x = 870;
    lpMMI->ptMinTrackSize.y = 507;
}


// =============================================================================
void SimpleGUIApplicationDlg::EnableInterface()
{
    // This method can be called really early or late when the window is not created
    if ( GetSafeHwnd() == 0 )
    {
        return;
    }

    bool lConnected = ( mDevice != NULL ) && mDevice->IsConnected();

    GetDlgItem( IDC_CONNECT_BUTTON )->EnableWindow( !lConnected );
    GetDlgItem( IDC_DISCONNECT_BUTTON )->EnableWindow( lConnected );

    GetDlgItem( IDC_COMMUNICATION_BUTTON )->EnableWindow( lConnected ); 
    GetDlgItem( IDC_DEVICE_BUTTON )->EnableWindow( lConnected );
    GetDlgItem( IDC_STREAMPARAMS_BUTTON )->EnableWindow( lConnected );

    bool lLocked = false;
    if ( mAcquisitionStateManager != NULL )
    {
        lLocked = mAcquisitionStateManager->GetState() == PvAcquisitionStateLocked;
    }

    mPlayButton.EnableWindow( lConnected && !lLocked );
    mStopButton.EnableWindow( lConnected && lLocked );

    // If not connected, disable the acquisition mode control. If enabled,
    // it will be managed automatically by events from the GenICam parameters
    if ( !lConnected || lLocked )
    {
        mModeCombo.EnableWindow( FALSE );
    }
}


// =============================================================================
void SimpleGUIApplicationDlg::Connect( const PvDeviceInfo *aDI )
{
    ASSERT( aDI != NULL );
    if ( aDI == NULL )  
    {
        return;
    }

    const PvDeviceInfoGEV *lDIGEV = dynamic_cast<const PvDeviceInfoGEV *>( aDI );
    const PvDeviceInfoU3V *lDIU3V = dynamic_cast<const PvDeviceInfoU3V *>( aDI );

    // Just in case we came here still connected...
    Disconnect();

    // Device connection, packet size negotiation and stream opening
    PvResult lResult = PvResult::Code::NOT_CONNECTED;

    // Connect device
    mDevice = PvDevice::CreateAndConnect( aDI, &lResult );
    if ( !lResult.IsOK() )
    {
        Disconnect();
        return;
    }

    // Open stream
    mStream = PvStream::CreateAndOpen( aDI->GetConnectionID(), &lResult );
    if ( !lResult.IsOK() )
    {
        Disconnect();
        return;
    }

    // GigE Vision devices only connection steps
    if ( aDI->GetType() == PvDeviceInfoTypeGEV )
    {
        PvDeviceGEV *lDeviceGEV = static_cast<PvDeviceGEV *>( mDevice );
        PvStreamGEV *lStreamGEV = static_cast<PvStreamGEV *>( mStream );

        PvString lLocalIpAddress = lStreamGEV->GetLocalIPAddress();
        uint16_t lLocalPort = lStreamGEV->GetLocalPort();

        // Perform automatic packet size negotiation
        lDeviceGEV->NegotiatePacketSize();

        // Now that the stream is opened, set the destination on the device
        lDeviceGEV->SetStreamDestination( lLocalIpAddress, lLocalPort );
    }

    mPipeline = new PvPipeline( mStream );

    // Register to all events of the parameters in the device's node map
    PvGenParameterArray *lGenDevice = mDevice->GetParameters();
    for ( uint32_t i = 0; i < lGenDevice->GetCount(); i++ )
    {
        lGenDevice->Get( i )->RegisterEventSink( this );
    }

    PvString lManufacturerStr = aDI->GetVendorName();
    PvString lModelNameStr = aDI->GetModelName();
    PvString lDeviceVersionStr = aDI->GetVersion();

    // GigE Vision only parameters
    PvString lIPStr = "N/A";
    PvString lMACStr = "N/A";
    if ( lDIGEV != NULL )
    {
        // IP (GigE Vision only)
        lIPStr = lDIGEV->GetIPAddress();

        // MAC address (GigE Vision only)
        lMACStr = lDIGEV->GetMACAddress();
    }

    // USB3 Vision only parameters
    PvString lDeviceGUIDStr = "N/A";
    if ( lDIU3V != NULL )
    {
        // Device GUID (USB3 Vision only)
        lDeviceGUIDStr = lDIU3V->GetDeviceGUID();
    }

    // Device name (User ID)
    PvString lNameStr = aDI->GetUserDefinedName();

    mManufacturerEdit.SetWindowText( lManufacturerStr );
    mModelEdit.SetWindowText( lModelNameStr );
    mIPEdit.SetWindowText( lIPStr );
    mMACEdit.SetWindowText( lMACStr );
    mGUIDEdit.SetWindowText( lDeviceGUIDStr );
    mNameEdit.SetWindowText( lNameStr );

    // Get acquisition mode GenICam parameter
    PvGenEnum *lMode = lGenDevice->GetEnum( "AcquisitionMode" );
    int64_t lEntriesCount = 0;
    lMode->GetEntriesCount( lEntriesCount );

    // Fill acquisition mode combo box
    mModeCombo.ResetContent();
    for ( uint32_t i = 0; i < lEntriesCount; i++ )
    {
        const PvGenEnumEntry *lEntry = NULL;
        lMode->GetEntryByIndex( i, &lEntry );
        
        if ( lEntry->IsAvailable() )
        {
            PvString lEEName;
            lEntry->GetName( lEEName );

            int64_t lEEValue;
            lEntry->GetValue( lEEValue );

            int lIndex = mModeCombo.AddString( lEEName.GetUnicode() );
            mModeCombo.SetItemData( lIndex, static_cast<DWORD_PTR>( lEEValue ) );
        }
    }

    // Set mode combo box to value currently used by the device
    int64_t lValue = 0;
    lMode->GetValue( lValue );
    for ( int i = 0; i < mModeCombo.GetCount(); i++ )
    {
        if ( lValue == mModeCombo.GetItemData( i ) )
        {
            mModeCombo.SetCurSel( i );
            break;
        }
    }

    // Create acquisition state manager
    mAcquisitionStateManager = new PvAcquisitionStateManager( mDevice, mStream );
    mAcquisitionStateManager->RegisterEventSink( this );

    mAcquiringImages = false;

    // Force an update on all the parameters on acquisition mode
    OnParameterUpdate( lMode );

    // Ready image reception
    StartStreaming();

    // Sync up UI
    EnableInterface();
}


// =============================================================================
void SimpleGUIApplicationDlg::Disconnect()
{
    
    if ( mDevice != NULL )
    {
        // Unregister all events of the parameters in the device's node map
        PvGenParameterArray *lGenDevice = mDevice->GetParameters();
        for ( uint32_t i = 0; i < lGenDevice->GetCount(); i++ )
        {
            lGenDevice->Get( i )->UnregisterEventSink( this );
        }
    }   
    
    // Close all configuration child windows
    CloseGenWindow( &mDeviceWnd );
    CloseGenWindow( &mCommunicationWnd );
    CloseGenWindow( &mStreamParametersWnd );

    // If streaming, stop streaming
    StopStreaming();

    // Release acquisition state manager
    if ( mAcquisitionStateManager != NULL )
    {
        delete mAcquisitionStateManager;
        mAcquisitionStateManager = NULL;
    }

    // Reset device ID - can be called by the destructor when the window
    // no longer exists, be careful...
    if ( GetSafeHwnd() != 0 )
    {
        mManufacturerEdit.SetWindowText( _T( "" ) );
        mModelEdit.SetWindowText( _T( "" ) );
        mIPEdit.SetWindowText( _T( "" ) );
        mMACEdit.SetWindowText( _T( "" ) );
        mGUIDEdit.SetWindowText( _T( "" ) );
        mNameEdit.SetWindowText( _T( "" ) );
    }

    if ( mDevice != NULL )
    {
        PvDevice::Free( mDevice );
        mDevice = NULL;
    }
    
    if ( mPipeline != NULL )
    {
        delete mPipeline;
        mPipeline = NULL;
    }

    if ( mStream != NULL )
    {
        PvStream::Free( mStream );
        mStream = NULL;
    }
    
    EnableInterface();
}


// =============================================================================
void SimpleGUIApplicationDlg::OnBnClickedDeviceButton()
{
    if ( !mDevice->IsConnected() )
    {
        return;
    }

    ShowGenWindow( 
        &mDeviceWnd, 
        mDevice->GetParameters(), 
        _T( "Device Control" ) );
}


// =============================================================================
void SimpleGUIApplicationDlg::OnBnClickedDeviceEvents()
{
}


// =============================================================================
void SimpleGUIApplicationDlg::OnBnClickedLinkButton()
{
    ShowGenWindow( 
        &mCommunicationWnd, 
        mDevice->GetCommunicationParameters(), 
        _T( "Communication Control" ) );
}


// =============================================================================
void SimpleGUIApplicationDlg::OnBnClickedStreamparamsButton()
{
    if ( !mDevice->IsConnected() )
    {
        return;
    }
    
    ShowGenWindow( 
        & mStreamParametersWnd, 
        mStream->GetParameters(), 
        _T( "Image Stream Control" ) );
}


// =============================================================================
void SimpleGUIApplicationDlg::ShowGenWindow( PvGenBrowserWnd **aWnd, PvGenParameterArray *aParams, const CString &aTitle )
{
    if ( ( *aWnd ) != NULL )
    {
        if ( ( *aWnd )->GetHandle() != 0 )
        {
            CWnd lWnd;
            lWnd.Attach( ( *aWnd )->GetHandle() );

            // Window already visible, give it focus and bring it on top
            lWnd.BringWindowToTop();
            lWnd.SetFocus();

            lWnd.Detach();
            return;
        }

        // Window object exists but was closed/destroyed. Free it before re-creating
        CloseGenWindow( aWnd );
    }

    // Create, assign parameters, set title and show modeless
    ( *aWnd ) = new PvGenBrowserWnd;
    ( *aWnd )->SetTitle( PvString( aTitle ) );
    ( *aWnd )->SetGenParameterArray( aParams );
    ( *aWnd )->ShowModeless( GetSafeHwnd() );
}


// =============================================================================
void SimpleGUIApplicationDlg::CloseGenWindow( PvGenBrowserWnd **aWnd )
{
    // If the window object does not even exist, do nothing
    if ( ( *aWnd ) == NULL )
    {
        return;
    }

    // If the window object exists and is currently created (visible), close/destroy it
    if ( ( *aWnd )->GetHandle() != 0 )
    {
        ( *aWnd )->Close();
    }

    // Finally, release the window object
    delete ( *aWnd );
    ( *aWnd ) = NULL;
}


// =============================================================================
void SimpleGUIApplicationDlg::StartStreaming()
{
    // Start threads
    mDisplayThread->Start( mPipeline, mDevice->GetParameters() );
    mDisplayThread->SetPriority( THREAD_PRIORITY_ABOVE_NORMAL );

    // Start pipeline
    mPipeline->Start();
}


// =============================================================================
void SimpleGUIApplicationDlg::StopStreaming()
{
    // Stop display thread
    if ( mDisplayThread != NULL )
    {
        mDisplayThread->Stop( false );
    }

    if ( mPipeline != NULL )
    {
        // Stop stream thread
        if ( mPipeline->IsStarted() )
        {
            mPipeline->Stop();
        }
    }   

    // Wait for display thread to be stopped
    if ( mDisplayThread != NULL )
    {
        mDisplayThread->WaitComplete();
    }
}


// =============================================================================
void SimpleGUIApplicationDlg::StartAcquisition()
{
    // Get payload size from device
    int64_t lPayloadSizeValue = DEFAULT_PAYLOAD_SIZE;
    if ( ( mDevice != NULL ) && mDevice->IsConnected() )
    {
        lPayloadSizeValue = mDevice->GetPayloadSize();
    }

    // If payload size is valid, force buffers re-alloc - better than 
    // adjusting as images are coming in
    if ( lPayloadSizeValue > 0 )
    {
        mPipeline->SetBufferSize( static_cast<uint32_t>( lPayloadSizeValue ) );
    }

    // Never hurts to start streaming on a fresh pipeline/stream...
    mPipeline->Reset();

    // Reset stream statistics
    mStream->GetParameters()->ExecuteCommand( "Reset" );

    mAcquisitionStateManager->Start();
}


// =============================================================================
void SimpleGUIApplicationDlg::StopAcquisition()
{
    mAcquisitionStateManager->Stop();
}


// =============================================================================
void SimpleGUIApplicationDlg::OnBnClickedStart()
{
    if ( !mDevice->IsConnected() )
    {
        return;
    }

    StartAcquisition();

    EnableInterface();
}


// =============================================================================
void SimpleGUIApplicationDlg::OnBnClickedStop()
{
    if ( !mDevice->IsConnected() )
    {
        return;
    }

    mAcquiringImages = false;

    StopAcquisition();

    EnableInterface();
}


// =============================================================================
void SimpleGUIApplicationDlg::OnCbnSelchangeMode()
{
    if ( !mDevice->IsConnected() )
    {
        return;
    }

    if ( mModeCombo.GetCurSel() < 0 )
    {
        return;
    }

    PvGenParameterArray *lDeviceParams = mDevice->GetParameters();

    uint64_t lValue = mModeCombo.GetItemData( mModeCombo.GetCurSel() );
    PvResult lResult = lDeviceParams->SetEnumValue( "AcquisitionMode", lValue );
    if ( !lResult.IsOK() )
    {
        MessageBox( _T( "Unable to set AcquisitionMode value." ), _T( "Error" ), 
            MB_OK | MB_ICONINFORMATION );
    }
}


// =============================================================================
void SimpleGUIApplicationDlg::OnParameterUpdate( PvGenParameter *aParameter )
{
    bool bBufferResize =false;
    PvString lName;
    aParameter->GetName( lName );

    if ( ( lName == "AcquisitionMode" ) &&
         ( mModeCombo.GetSafeHwnd() != 0 ) )
    {
        bool lAvailable = false, lWritable = false;
        aParameter->IsAvailable( lAvailable );
        if ( lAvailable )
        {
            aParameter->IsWritable( lWritable );
        }

        mModeCombo.EnableWindow( lAvailable && lWritable );

        PvGenEnum *lEnum = dynamic_cast<PvGenEnum *>( aParameter );
        if ( lEnum != NULL )
        {
            int64_t lEEValue = 0;
            lEnum->GetValue( lEEValue );

            for ( int i = 0; i < mModeCombo.GetCount(); i++ )
            {
                DWORD_PTR lData = mModeCombo.GetItemData( i );
                if ( lData == lEEValue )
                {
                    mModeCombo.SetCurSel( i );
                    break;
                }
            }
        }
    }
}


// =============================================================================
void SimpleGUIApplicationDlg::OnMove(int x, int y)
{
    CDialog::OnMove(x, y);
}


// =============================================================================
void SimpleGUIApplicationDlg::OnAcquisitionStateChanged( PvDevice* aDevice, PvStream* aStream, uint32_t aSource, PvAcquisitionState aState )
{
    PostMessage( WM_ACQUISITIONSTATECHANGED );
}


// =============================================================================
LRESULT SimpleGUIApplicationDlg::OnAcquisitionStateChanged( WPARAM wParam, LPARAM lParam )
{
    EnableInterface();
    return 0;
}
