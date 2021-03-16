// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"
#include "TransmitTiledImagesApp.h"
#include "TransmitTiledImagesDlg.h"

#include "TransmitterConfig.h"
#include <PvConfigurationWriter.h>
#include <PvInterface.h>
#include <PvVersion.h>
#include <PvSystem.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

// Dialog Data
    enum { IDD = IDD_ABOUTBOX };

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();

    CStatic mVersion;
    CStatic mCompanyName;
    CStatic mCopyright;
// Implementation
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COPYRIGHT, mCopyright);
    DDX_Control(pDX, IDC_VERSION, mVersion);
    DDX_Control(pDX, IDC_COMPANYNAME, mCompanyName);    
}

BOOL CAboutDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    mCompanyName.SetWindowText( _T( VERSION_COMPANY_NAME ) );
    mCopyright.SetWindowText( _T( VERSION_COPYRIGHT ) );
    mVersion.SetWindowText( _T( VERSION_STRING ) );

    return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// CTransmitTiledImagesDlg dialog

CTransmitTiledImagesDlg::CTransmitTiledImagesDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CTransmitTiledImagesDlg::IDD, pParent)
    , mTimer( 0 )
    , mMinSizeX( 0 )
    , mMinSizeY( 0 )
    , mActiveStreamThread( NULL )
{
    bool lReturn;

    m_hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );

    for( int i = 0; i < MAX_TILES_ROW; i++ )
    {
        for( int j = 0; j < MAX_TILES_COLUMN; j++ )
        {
            lReturn = mStreamThreads[ i ][ j ].Initialize( this, i, j, &mCurrentBuffersTable );
            ASSERT( lReturn );
        }
    }

    lReturn = mTransmitterThread.Initialize( this, &mDisplay, &mCurrentBuffersTable );
    ASSERT( lReturn );

    // Set bitmap buttons
    mButtonStart.SetBitmap( IDB_PLAY );
    mButtonStop.SetBitmap( IDB_STOP );
}

CTransmitTiledImagesDlg::~CTransmitTiledImagesDlg()
{
}

void CTransmitTiledImagesDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_IPADDRESS_DESTINATION, mEditAddress);
    DDX_Control(pDX, IDC_EDIT_PORT, mEditPort);
    DDX_Control(pDX, IDC_COMBO_IF_ADDRESS, mComboIfAddress);
    DDX_Control(pDX, IDC_EDIT_FPS, mEditFps);
    DDX_Control(pDX, IDC_EDIT_PACKET_SIZE, mEditPacketSize);
    DDX_Control(pDX, IDC_EDIT_WIDTH, mEditWidth);
    DDX_Control(pDX, IDC_EDIT_HEIGHT, mEditHeight);
    DDX_Control(pDX, IDC_COMBO_TILING_MODE, mTilingCombo);
    DDX_Control(pDX, IDC_STATUS_BAR, mStatusBar);
    DDX_Control(pDX, IDC_BUTTON_CONNECT_11, mButtonConnect[ 0 ][ 0 ]);
    DDX_Control(pDX, IDC_BUTTON_CONNECT_21, mButtonConnect[ 1 ][ 0 ]);
    DDX_Control(pDX, IDC_BUTTON_CONNECT_12, mButtonConnect[ 0 ][ 1 ]);
    DDX_Control(pDX, IDC_BUTTON_CONNECT_22, mButtonConnect[ 1 ][ 1 ]);
    DDX_Control(pDX, IDC_BUTTON_OPTIONS_11, mButtonOptions[ 0 ][ 0 ]);
    DDX_Control(pDX, IDC_BUTTON_OPTIONS_21, mButtonOptions[ 1 ][ 0 ]);
    DDX_Control(pDX, IDC_BUTTON_OPTIONS_12, mButtonOptions[ 0 ][ 1 ]);
    DDX_Control(pDX, IDC_BUTTON_OPTIONS_22, mButtonOptions[ 1 ][ 1 ]);
    DDX_Control(pDX, IDC_STATIC_CONNECTION_11, mStaticConnection[ 0 ] [ 0 ]);
    DDX_Control(pDX, IDC_STATIC_CONNECTION_12, mStaticConnection[ 0 ] [ 1 ]);
    DDX_Control(pDX, IDC_STATIC_CONNECTION_21, mStaticConnection[ 1 ] [ 0 ]);
    DDX_Control(pDX, IDC_STATIC_CONNECTION_22, mStaticConnection[ 1 ] [ 1 ]);
    DDX_Control(pDX, IDC_BTN_START, mButtonStart);
    DDX_Control(pDX, IDC_BTN_STOP, mButtonStop);
}

BEGIN_MESSAGE_MAP(CTransmitTiledImagesDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    //}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_BTN_START, &CTransmitTiledImagesDlg::OnBnClickedBtnStart)
    ON_BN_CLICKED(IDC_BTN_STOP, &CTransmitTiledImagesDlg::OnBnClickedBtnStop)
    ON_CBN_SELCHANGE(IDC_COMBO_IF_ADDRESS, &CTransmitTiledImagesDlg::OnCbnSelchangeComboIfAddress)
    ON_WM_DESTROY()
    ON_WM_TIMER()
    ON_EN_KILLFOCUS(IDC_EDIT_FPS, &CTransmitTiledImagesDlg::OnEnKillfocusEditFps)
    ON_EN_KILLFOCUS(IDC_EDIT_WIDTH, &CTransmitTiledImagesDlg::OnEnKillfocusEditWidth)
    ON_EN_KILLFOCUS(IDC_EDIT_HEIGHT, &CTransmitTiledImagesDlg::OnEnKillfocusEditHeight)
    ON_BN_CLICKED(IDC_BUTTON_CONNECT_11, &CTransmitTiledImagesDlg::OnBnClickedButtonConnect11)
    ON_BN_CLICKED(IDC_BUTTON_OPTIONS_11, &CTransmitTiledImagesDlg::OnBnClickedButtonOptions11)
    ON_BN_CLICKED(IDC_BUTTON_CONNECT_21, &CTransmitTiledImagesDlg::OnBnClickedButtonConnect21)
    ON_BN_CLICKED(IDC_BUTTON_OPTIONS_21, &CTransmitTiledImagesDlg::OnBnClickedButtonOptions21)
    ON_BN_CLICKED(IDC_BUTTON_CONNECT_12, &CTransmitTiledImagesDlg::OnBnClickedButtonConnect12)
    ON_BN_CLICKED(IDC_BUTTON_OPTIONS_12, &CTransmitTiledImagesDlg::OnBnClickedButtonOptions12)
    ON_BN_CLICKED(IDC_BUTTON_CONNECT_22, &CTransmitTiledImagesDlg::OnBnClickedButtonConnect22)
    ON_BN_CLICKED(IDC_BUTTON_OPTIONS_22, &CTransmitTiledImagesDlg::OnBnClickedButtonOptions22)
    ON_COMMAND(ID_FILE_OPEN, &CTransmitTiledImagesDlg::OnFileOpen)
    ON_COMMAND(ID_FILE_SAVE, &CTransmitTiledImagesDlg::OnFileSave)
    ON_COMMAND(ID_FILE_SAVEAS, &CTransmitTiledImagesDlg::OnFileSaveAs)
    ON_COMMAND(ID_FILE_EXIT, &CTransmitTiledImagesDlg::OnFileExit)
    ON_COMMAND(ID_HELP_ABOUT, &CTransmitTiledImagesDlg::OnHelpAbout)
    ON_COMMAND(ID_ACCELERATOR_OPEN, &CTransmitTiledImagesDlg::OnAcceleratorOpen)
    ON_COMMAND(ID_ACCELERATOR_SAVE, &CTransmitTiledImagesDlg::OnAcceleratorSave)
    ON_COMMAND(ID_ACCELERATOR_SAVEAS, &CTransmitTiledImagesDlg::OnAcceleratorSaveAs)
    ON_COMMAND(ID_ACCELERATOR_EXIT, &CTransmitTiledImagesDlg::OnAcceleratorExit)
    ON_MESSAGE(WM_LINKDISCONNECTED, &CTransmitTiledImagesDlg::OnLinkDisconnected)
    ON_MESSAGE(WM_REALLOCATIONFAIL, &CTransmitTiledImagesDlg::OnReAllocationFail)
    ON_MESSAGE(WM_CONVERSIONFAIL, &CTransmitTiledImagesDlg::OnConversionFail)    
    ON_WM_SIZE()
    ON_WM_GETMINMAXINFO()
    ON_COMMAND( ID_PARAMETERS_COMMUNICATIONPARAMETERS, &CTransmitTiledImagesDlg::OnCommunicationParameters)
    ON_COMMAND( ID_PARAMETERS_DEVICEPARAMETERS, &CTransmitTiledImagesDlg::OnDeviceParameters )
    ON_COMMAND( ID_PARAMETERS_STREAMPARAMETERS, &CTransmitTiledImagesDlg::OnStreamParameters )
END_MESSAGE_MAP()


// ==============================================================================
void CTransmitTiledImagesDlg::OnTimer( UINT_PTR nIDEvent )
{
    if ( nIDEvent == 1 )
    {
        // Periodically refresh the stats
        UpdateStats();
    }

    CDialog::OnTimer( nIDEvent );
}

void CTransmitTiledImagesDlg::UpdateStats()
{
    CString lStr;
    TransmitterStats lStats;

    // Get the stats
    mTransmitterThread.GetStats( lStats );

    // Format the stats
    lStr.Format( _T( "Transmitted: %d images    %.01f FPS    %.01f Mbps     Display: %i FPS" ), 
        lStats.BlocksTransmitted, lStats.InstantaneousTransmissionRate, lStats.InstantaneousPayloadThroughput, lStats.DisplayFrameRate );

    // Display the stats
    mStatusBar.SetWindowText( lStr );
}

// CTransmitTiledImagesDlg message handlers

BOOL CTransmitTiledImagesDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    int lIndex;

    // Get the current size
    GetClientRect( mClientRectangle );
    
    // Load the accelerator
    mAccelerator = ::LoadAccelerators( AfxGetResourceHandle(), MAKEINTRESOURCE( IDR_ACCELERATOR ) );

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

    // Populate the interface selection combo box
    PvSystem lSystem;
    lSystem.Find();

    bool lFound = false;
    for ( uint32_t i = 0; i < lSystem.GetInterfaceCount(); i++ )
    {
        const PvNetworkAdapter *lNetworkAdapter = dynamic_cast<const PvNetworkAdapter *>( lSystem.GetInterface( i ) );
        if ( lNetworkAdapter != NULL )
        {
            for ( uint32_t j = 0; j < lNetworkAdapter->GetIPAddressCount(); j++ )
            {
                if ( strcmp( "0.0.0.0", lNetworkAdapter->GetIPAddress( j ).GetAscii() ) != 0 )
                {
                    mComboIfAddress.AddString( lNetworkAdapter->GetIPAddress( j ).GetUnicode() );
                    lFound = true;
                }
            }
        }
    }

    if ( !lFound )
    {
        MessageBox( _T( "No valid interfaces found. Make sure you have at least one network adapter that is connected to a network." ) );
        EndDialog(IDCANCEL);
        return FALSE;
    }

    // Configure the display modes combo box
    lIndex = mTilingCombo.AddString( _T( "Stretch to Fit" ) );
    mTilingCombo.SetItemData( lIndex, TILING_MODE_STRETCH );
    lIndex = mTilingCombo.AddString( _T( "Maintain Resolution" ) );
    mTilingCombo.SetItemData( lIndex, TILING_MODE_CROP);
    lIndex = mTilingCombo.AddString( _T( "Maintain Aspect Ratio") );
    mTilingCombo.SetItemData( lIndex, TILING_MODE_RATIO );

    // Now initialize the default value into the control
    TransmitterConfig lTransmitterConfig;
    SetTransmitterConfig( lTransmitterConfig );

    CRect lRect;
    GetWindowRect( lRect );
    mMinSizeX = lRect.Width();
    mMinSizeY = lRect.Height();

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTransmitTiledImagesDlg::SetTransmitterConfig( TransmitterConfig& aTransmitterConfig )
{
    CString lTemp;

    lTemp.Format( _T( "%d" ), aTransmitterConfig.GetWidth() );
    mEditWidth.SetWindowText( lTemp );

    lTemp.Format( _T( "%d" ), aTransmitterConfig.GetHeight() );
    mEditHeight.SetWindowText( lTemp );

    lTemp.Format( _T( "%d" ), aTransmitterConfig.GetFrameRate() );
    mEditFps.SetWindowTextW( lTemp );

    for( int i = 0; i < mTilingCombo.GetCount(); i++ )
    {
        if( mTilingCombo.GetItemData( i ) == aTransmitterConfig.GetTilingMode() )
        {
            mTilingCombo.SetCurSel( i );
            break;
        } 
    }

    mEditAddress.SetAsText( aTransmitterConfig.GetDestinationIPAddress().GetBuffer() );

    lTemp.Format( _T( "%d" ), aTransmitterConfig.GetDestinationPort() );
    mEditPort.SetWindowTextW( lTemp );

    lTemp.Format( _T( "%d" ), aTransmitterConfig.GetPacketSize() );
    mEditPacketSize.SetWindowTextW( lTemp );

    CString lIfAddress = aTransmitterConfig.GetSourceIPAddress();
    if( lIfAddress == _T( "" ) )
    {
        mComboIfAddress.SetCurSel( 0 );
        mComboIfAddress.GetLBText( mComboIfAddress.GetCurSel(), lIfAddress );
    }
    else
    {
        for( int i = 0; i < mTilingCombo.GetCount(); i++ )
        {
            mComboIfAddress.GetLBText( i, lTemp );
            if( lTemp == lIfAddress )
            {
                mComboIfAddress.SetCurSel( i );
                break;
            }
        }
    }
    mVirtualDeviceGEV.StartListening( lIfAddress.GetBuffer() );
}

void CTransmitTiledImagesDlg::GetTransmitterConfig( TransmitterConfig& aTransmitterConfig )
{
    CString lTemp;

    // Read the Information from the GUI on the configuration of the stream
    mEditWidth.GetWindowText( lTemp );
    aTransmitterConfig.SetWidth( _wtoi( lTemp.GetBuffer() ) );

    mEditHeight.GetWindowText( lTemp );
    aTransmitterConfig.SetHeight( _wtoi( lTemp.GetBuffer() ) );

    aTransmitterConfig.SetTilingMode( ( TilingMode ) mTilingCombo.GetItemData( mTilingCombo.GetCurSel() ) );

    mEditFps.GetWindowText( lTemp );
    aTransmitterConfig.SetFrameRate( _wtoi( lTemp.GetBuffer() ) );

    mEditPacketSize.GetWindowText( lTemp );
    aTransmitterConfig.SetPacketSize( _wtoi( lTemp.GetBuffer() ) );

    mEditAddress.GetWindowText( lTemp );
    aTransmitterConfig.SetDestinationIPAddress( lTemp );

    mEditPort.GetWindowText( lTemp );
    aTransmitterConfig.SetDestinationPort( _wtoi( lTemp.GetBuffer() ) );

    mComboIfAddress.GetLBText( mComboIfAddress.GetCurSel(), lTemp );
    aTransmitterConfig.SetSourceIPAddress( lTemp );
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTransmitTiledImagesDlg::OnPaint()
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

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTransmitTiledImagesDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}


void CTransmitTiledImagesDlg::OnBnClickedBtnStart()
{
    Start();
}

void CTransmitTiledImagesDlg::OnBnClickedBtnStop()
{
    Stop();
}

void CTransmitTiledImagesDlg::Start()
{
    unsigned int lMaxWidth;
    unsigned int lMaxHeight;
    unsigned int lCurrentWidth;
    unsigned int lCurrentHeight;
    bool lResult;
    TransmitterConfig lTransmitterConfig;

    CWaitCursor lWait;

    // Update the configuration of the transmitter from the GUI
    GetTransmitterConfig( lTransmitterConfig );

    // Here we will start by initializing all the threads up front. 
    // This allows to fail when out of the memory without too much side effects!
    lMaxWidth = DEFAULT_RX_BUFFER_ALLOCATION_WIDTH;
    lMaxHeight = DEFAULT_RX_BUFFER_ALLOCATION_HEIGHT;
    for( int i = 0; i < MAX_TILES_ROW; i++ )
    {
        for( int j = 0; j < MAX_TILES_COLUMN; j++ )
        {
            if( mStreamThreads[ i ][ j ].IsConnected() )
            {
                lResult = mStreamThreads[ i ][ j ].Configure( lCurrentWidth, lCurrentHeight );
                if( !lResult )
                {        
                    MessageBox( _T( "Unable to configure the stream thread" ), _T( "Configuration failure" ), MB_OK | MB_ICONHAND );
                    return;
                }

                lMaxWidth = max( lMaxWidth, lCurrentWidth );
                lMaxHeight = max( lMaxHeight, lCurrentHeight );
            }
        }
    }

    // Now the transmitter
    lResult = mTransmitterThread.Configure( lMaxWidth, lMaxHeight, lTransmitterConfig );
    if( !lResult )
    {        
        MessageBox( _T( "Unable to configure the transmitter thread" ), _T( "Configuration failure" ), MB_OK | MB_ICONHAND );
        return;
    }


    // Now that all the threads are initialized, we can start them
    for( int i = 0; i < MAX_TILES_ROW; i++ )
    {
        for( int j = 0; j < MAX_TILES_COLUMN; j++ )
        {
            if( mStreamThreads[ i ][ j ].IsConnected() )
            {
                lResult = mStreamThreads[ i ][ j ].Start();
                if( !lResult )
                {        
                    Stop();
                    MessageBox( _T( "Unable to start the stream thread" ), _T( "Configuration failure" ), MB_OK | MB_ICONHAND );
                    return;
                }
            }
        }
    }
    // Now the transmitter
    lResult = mTransmitterThread.Start();
    if( !lResult )
    {        
        Stop();
        MessageBox( _T( "Unable to start the transmitter thread" ), _T( "Configuration failure" ), MB_OK | MB_ICONHAND );
        return;
    }

    // Create timer that will be used to periodically update the status bar
    mTimer = SetTimer( 1, 250, NULL );

    // Disable the interface to ensure the start will only happen once
    SetGUIInRunningState( TRUE );
}

void CTransmitTiledImagesDlg::Stop()
{
    CWaitCursor lWait;    
    
    // Stop the stats update
    if ( mTimer != 0 )
    {
        KillTimer( 1 );
        mTimer = 0;
    }

    // Stop the transmitter thread
    mTransmitterThread.Stop();

    // Stop the stream thread
    for( int i = 0; i < MAX_TILES_ROW; i++ )
    {
        for( int j = 0; j < MAX_TILES_COLUMN; j++ )
        {
            mStreamThreads[ i ][ j ].Stop();
        }
    }

    // And now we can restore the access to the GUI
    SetGUIInRunningState( FALSE );
}

void CTransmitTiledImagesDlg::SetGUIInRunningState( BOOL aRunning )
{   
    mButtonStart.EnableWindow( !aRunning );
    mButtonStop.EnableWindow( aRunning );

    mEditAddress.EnableWindow( !aRunning );
    mEditPort.EnableWindow( !aRunning );
    mComboIfAddress.EnableWindow( !aRunning );
    mEditFps.EnableWindow( !aRunning );
    mEditPacketSize.EnableWindow( !aRunning );
    mEditWidth.EnableWindow( !aRunning );
    mEditHeight.EnableWindow( !aRunning );
    mTilingCombo.EnableWindow( !aRunning );

    for( int i = 0; i < MAX_TILES_ROW; i++ )
    {
        for( int j = 0; j < MAX_TILES_COLUMN; j++ )
        {
            mButtonConnect[ i ][ j ].EnableWindow( !aRunning );
        }
    } 
}

void CTransmitTiledImagesDlg::OnCbnSelchangeComboIfAddress()
{
    CString lIfAddress;

    // Stop the current virtual device
    mVirtualDeviceGEV.StopListening();
    
    // Get the new virtual address
    mComboIfAddress.GetLBText( mComboIfAddress.GetCurSel(), lIfAddress );

    // and start listing with the new virtual device
    mVirtualDeviceGEV.StartListening( lIfAddress.GetBuffer() );
}

void CTransmitTiledImagesDlg::OnDestroy()
{
    CDialog::OnDestroy();

    Stop();
}

LRESULT CTransmitTiledImagesDlg::OnLinkDisconnected( WPARAM wp, LPARAM lp )
{
    CString lTemp;
    int lRow = ( int ) wp;
    int lColumn = ( int ) lp;
    
    // Run the normal disconnection thread and manage the GUI
    // we need to have everything in synch
    Disconnect( lRow, lColumn );

    // If not more device streaming, let stop automatically
    if( !IsConnected() )
    {
        Stop();
    }

    // Let the user know about the lost connection
    lTemp.Format( _T( "Lost the connection to the device in %d,%d" ), lRow, lColumn );
    MessageBox( lTemp.GetBuffer(), _T( "Connection Lost" ), MB_OK | MB_ICONHAND );

    return 0;
}

LRESULT CTransmitTiledImagesDlg::OnReAllocationFail( WPARAM wp, LPARAM lp )
{
    CString lTemp;
    int lRow = ( int ) wp;
    int lColumn = ( int ) lp;
    
    // Run the normal disconnection thread and manage the GUI
    // we need to have everything in synch
    Disconnect( lRow, lColumn );

    // If not more device streaming, let stop automatically
    if( !IsConnected() )
    {
        Stop();
    }

    // Let the user know about the lost connection
    lTemp.Format( _T( "The run-time reallocation of the memory for the stream in %d,%d fail" ), lRow, lColumn );
    MessageBox( lTemp.GetBuffer(), _T( "Out of memory" ), MB_OK | MB_ICONHAND );

    return 0;
}

bool CTransmitTiledImagesDlg::IsConnected()
{
    bool lReturn = false;

    for( int i = 0; i < MAX_TILES_ROW; i++ )
    {
        for( int j = 0; j < MAX_TILES_COLUMN; j++ )
        {
            lReturn = mStreamThreads[ i ][ j ].IsConnected();
            if( lReturn )
            {
                return true;
            }
        }
    }
    return false;
}

LRESULT CTransmitTiledImagesDlg::OnConversionFail( WPARAM wp, LPARAM lp )
{
    // Stop the system, we cannot continue!
    Stop();
    
    // Let the user know about the lost connection
    MessageBox( _T( "The system cannot re-allocate the buffer for the pixels conversion" ), _T( "Out of memory" ), MB_OK | MB_ICONHAND );

    return 0;
}


void CTransmitTiledImagesDlg::OnEnKillfocusEditFps()
{
    CString lFps;
    uint32_t lFPSInt;

    // remove the potential usage of 0 
    mEditFps.GetWindowText( lFps );
    lFPSInt = _wtoi( lFps.GetBuffer() );
    if( lFPSInt <= 0 )
    {
        mEditFps.SetWindowTextW( _T( "1" ) );
    }
}

void CTransmitTiledImagesDlg::OnEnKillfocusEditWidth()
{
    CString lWidth;
    uint32_t lWidthInt;

    // Ensure a 128 bytes width. 
    // This is ensuring we do not hit limitation on the image manipulation.
    mEditWidth.GetWindowText( lWidth );
    lWidthInt = _wtoi( lWidth.GetBuffer() );
    if( lWidthInt < 128 )
    {
        mEditWidth.SetWindowTextW( _T( "64" ) );
    }
}

void CTransmitTiledImagesDlg::OnEnKillfocusEditHeight()
{
    CString lHeight;
    uint32_t lHeightInt;

    // Ensure a 128 bytes Height. 
    // This is ensuring we do not hit limitation on the image manipulation.
    mEditHeight.GetWindowText( lHeight );
    lHeightInt = _wtoi( lHeight.GetBuffer() );
    if( lHeightInt < 128 )
    {
        mEditHeight.SetWindowTextW( _T( "64" ) );
    }
}

BOOL CTransmitTiledImagesDlg::PreTranslateMessage( MSG* pMsg )
{
    if ( WM_KEYFIRST <= pMsg->message && pMsg->message <= WM_KEYLAST )
    {
        if ( mAccelerator && ::TranslateAccelerator( m_hWnd, mAccelerator, pMsg ) )
        {
            return TRUE;
        }
    }

    if ( pMsg->wParam == VK_ESCAPE ) 
    { 
        return TRUE; 
    }
  
    return CDialog::PreTranslateMessage(pMsg);
}

void CTransmitTiledImagesDlg::ProcessConnection( int aRow, int aColumn )
{
    ASSERT( aRow >= 0 );
    ASSERT( aRow < MAX_TILES_ROW );
    ASSERT( aColumn >= 0 );
    ASSERT( aColumn < MAX_TILES_COLUMN );

    CWaitCursor lWait;

    if( aRow < 0 || aRow >= MAX_TILES_ROW || aColumn < 0 || aColumn >= MAX_TILES_COLUMN )
    {
        return;
    }

    // Depending of the current state of the stream thread, we need to toggle the code to execute
    if( mStreamThreads[ aRow ][ aColumn ].IsConnected() )
    {
        Disconnect( aRow, aColumn );
    }
    else
    {
        Connect( aRow, aColumn );
    }
}

void CTransmitTiledImagesDlg::Connect( int aRow, int aColumn )
{
    CString lDisplayString;

    // Make the connection of the thread
    CString lIfAddress;
    mComboIfAddress.GetWindowText( lIfAddress );
    PvResult lResult = mStreamThreads[ aRow ][ aColumn ].Connect( lIfAddress, this );
    if ( lResult.GetCode() == PvResult::Code::CANCEL )
    {
        return;
    }

    // Ensure the connection is done
    if( !mStreamThreads[ aRow ][ aColumn ].IsConnected() )
    {
        MessageBox( _T( "Connection Fail" ) , _T( "Connection" ),  MB_OK | MB_ICONHAND );  
        return;
    }

    // Update the GUI
    mButtonConnect[ aRow ][ aColumn ].SetWindowTextW( _T( "Disconnect" ) );
    FormatDisplay( &mStreamThreads[ aRow ][ aColumn ], lDisplayString ); 
    mStaticConnection[ aRow ][ aColumn ].SetWindowText( lDisplayString );
    mStaticConnection[ aRow ][ aColumn ].EnableWindow( TRUE );
}

void CTransmitTiledImagesDlg::FormatDisplay( StreamThread* aStreamThread, CString& aDisplay )
{
	if ( !aStreamThread->GetIPAddress().IsEmpty() )
	{
		// An IP address is available; display GEV device information
		aDisplay = _T( "IP: " ) + aStreamThread->GetIPAddress() + _T( "\n" );
		aDisplay += _T( "MAC: " ) + aStreamThread->GetMACAddress() + _T( "\n" );
	}
	else
	{
		// Display U3V device information
		aDisplay = _T( "Serial number: " ) + aStreamThread->GetSerialNumber() + _T( "\n" );
	}

    if( aStreamThread->GetUserDefinedName().IsEmpty() ) 
    {
        aDisplay += aStreamThread->GetModel();
    }
    else
    {
        aDisplay += aStreamThread->GetUserDefinedName();
    }
}

void CTransmitTiledImagesDlg::Disconnect( int aRow, int aColumn )
{
    mStreamThreads[ aRow ][ aColumn ].Disconnect();

    mButtonConnect[ aRow ][ aColumn ].SetWindowText( _T( "Select/Connect" ) );
    mStaticConnection[ aRow ][ aColumn ].SetWindowTextW( _T( "Not connected" ) );
    mStaticConnection[ aRow ][ aColumn ].EnableWindow( FALSE );
}

void CTransmitTiledImagesDlg::ProcessOptions( int aRow, int aColumn )
{
    CWaitCursor lWait;

    ASSERT( ( aRow >= 0 ) && ( aRow < MAX_TILES_ROW ) );
    ASSERT( ( aColumn >= 0 ) && ( aColumn < MAX_TILES_COLUMN ) );

    // Get matrix resources
    mActiveStreamThread = &mStreamThreads[ aRow ][ aColumn ];
    CButton *lButton = &mButtonOptions[ aRow ][ aColumn ];
    
    // Get parameters button rect
    CRect lButtonRect;
    lButton->GetWindowRect( lButtonRect );

    // Initialize popup menu
    CMenu lParametersMenu;
    lParametersMenu.LoadMenu( IDR_PARAMETERSMENU );
    CMenu *lPopup = lParametersMenu.GetSubMenu( 0 );
    ASSERT( lPopup != NULL );

    // Enable menu
    lPopup->EnableMenuItem( ID_PARAMETERS_COMMUNICATIONPARAMETERS, MF_ENABLED );
    lPopup->EnableMenuItem( ID_PARAMETERS_DEVICEPARAMETERS, mActiveStreamThread->IsDeviceConnected() ? MF_ENABLED : MF_DISABLED );
    lPopup->EnableMenuItem( ID_PARAMETERS_STREAMPARAMETERS, mActiveStreamThread->IsStreamOpened() ? MF_ENABLED : MF_DISABLED );

    // Finally, display the menu
    lPopup->TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON, lButtonRect.left, lButtonRect.bottom, this );
}

void CTransmitTiledImagesDlg::OnBnClickedButtonConnect11()
{
    ProcessConnection( 0, 0 );
}

void CTransmitTiledImagesDlg::OnBnClickedButtonOptions11()
{
    ProcessOptions( 0, 0 );
}

void CTransmitTiledImagesDlg::OnBnClickedButtonConnect21()
{
    ProcessConnection( 1, 0 );
}

void CTransmitTiledImagesDlg::OnBnClickedButtonOptions21()
{
    ProcessOptions( 1, 0 );
}

void CTransmitTiledImagesDlg::OnBnClickedButtonConnect12()
{
    ProcessConnection( 0, 1 );
}

void CTransmitTiledImagesDlg::OnBnClickedButtonOptions12()
{
    ProcessOptions( 0, 1 );
}

void CTransmitTiledImagesDlg::OnBnClickedButtonConnect22()
{
    ProcessConnection( 1, 1 );
}

void CTransmitTiledImagesDlg::OnBnClickedButtonOptions22()
{
    ProcessOptions( 1, 1 );
}

void CTransmitTiledImagesDlg::OnGetMinMaxInfo( MINMAXINFO *lpMMI )
{
    if ( ( mMinSizeX > 0 ) && ( mMinSizeY > 0 ) )
    {
        lpMMI->ptMinTrackSize.x = mMinSizeX;
        lpMMI->ptMinTrackSize.y = mMinSizeY;
    }
}

void CTransmitTiledImagesDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    CRect lRect;
    int32_t lX, lY, lSizeX, lSizeY;
    int32_t lDeltaX, lDeltaY;

    if ( ( mMinSizeX == 0 ) || ( mMinSizeY == 0 ) || 
         ( cx == 0 ) || ( cy == 0 ) )
    {
        return;
    }

    lDeltaX = cx - mClientRectangle.Width();
    lDeltaY = cy - mClientRectangle.Height();

    // Update the client rectangle
    GetClientRect( mClientRectangle );

    HDWP hDWP = ::BeginDeferWindowPos( 20 );

    // Move transmission group box
    CWnd *lTGB = GetDlgItem( IDC_TRANSMISSION_GROUPBOX );
    lTGB->GetWindowRect( lRect );
    ScreenToClient( lRect );
    lRect.bottom += ( int ) lDeltaY;
    ::DeferWindowPos( hDWP, lTGB->GetSafeHwnd(), NULL, 
        lRect.left, lRect.top, lRect.Width(), lRect.Height(),
        SWP_NOACTIVATE|SWP_NOZORDER );

    // Move the display group box
    CWnd *lDGB = GetDlgItem( IDC_DISPLAY_GROUPBOX );
    lDGB->GetWindowRect( lRect );
    ScreenToClient( lRect );
    lRect.right += ( int ) lDeltaX;
    lRect.bottom += ( int ) lDeltaY;
    ::DeferWindowPos( hDWP, lDGB->GetSafeHwnd(), NULL, 
        lRect.left, lRect.top, lRect.Width(), lRect.Height(),
        SWP_NOACTIVATE|SWP_NOZORDER );

    // Move the display
    mDisplay.GetPosition( lX, lY, lSizeX, lSizeY );
    mDisplay.SetPosition( lX, lY, lSizeX + lDeltaX, lSizeY + lDeltaY );

    // Move the status bar
    mStatusBar.GetWindowRect( lRect ); 
    ScreenToClient( lRect ); 
    lRect.right += ( int ) lDeltaX;
    lRect.bottom += ( int ) lDeltaY;
    lRect.top += ( int ) lDeltaY;
    ::DeferWindowPos( hDWP, mStatusBar.GetSafeHwnd(), NULL, 
        lRect.left, lRect.top, lRect.Width(), lRect.Height(),
        SWP_NOACTIVATE|SWP_NOZORDER );

    ::EndDeferWindowPos( hDWP );
}


void CTransmitTiledImagesDlg::OnFileOpen()
{
    Open();
}

void CTransmitTiledImagesDlg::OnFileSave()
{
    SaveAs( mLastFileName );
}

void CTransmitTiledImagesDlg::OnFileSaveAs()
{
    SaveAs( CString() );
}

void CTransmitTiledImagesDlg::OnFileExit()
{
    EndDialog( IDOK );
}

void CTransmitTiledImagesDlg::OnHelpAbout()
{
    CAboutDlg lDlg;
    lDlg.DoModal();
}

void CTransmitTiledImagesDlg::OnAcceleratorOpen()
{
    Open();
}

void CTransmitTiledImagesDlg::OnAcceleratorSave()
{
    SaveAs( mLastFileName );
}

void CTransmitTiledImagesDlg::OnAcceleratorSaveAs()
{
    SaveAs( CString() );
}

void CTransmitTiledImagesDlg::OnAcceleratorExit()
{
    EndDialog( IDOK );
}

void CTransmitTiledImagesDlg::Open()
{
    PvConfigurationReader lReader;
    CString lFileName;
    TransmitterConfig lTransmitterConfig;
    CWaitCursor lWait;
    CString lDisplayString;
    CFileDialog lFileDlg( TRUE, _T( ".pvt" ), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T( "TransmitTiledImages Sample Configuration Files (*.pvt)|*.pvt|All files (*.*)|*.*||" ), this );

    // Get a filename
    lFileDlg.m_ofn.lpstrTitle = _T( "Open Project" );
    if( lFileDlg.DoModal() != IDOK )
    {
        return;
    }
    lFileName = lFileDlg.GetPathName();        

    Stop();

    // Disconnect all the cameras
    for( int i = 0; i < MAX_TILES_ROW; i++ )
    {
        for( int j = 0; j < MAX_TILES_COLUMN; j++ )
        {
            // Ensure the stream is disconnected and the GUI is disabled
            Disconnect( i, j );
        }
    }

    // Now load the file
    if( lReader.Load( lFileName.GetBuffer() ).IsOK() )
    {
        // Parse the file
        for( int i = 0; i < MAX_TILES_ROW; i++ )
        {
            for( int j = 0; j < MAX_TILES_COLUMN; j++ )
            {
                // Ensure the stream is disconnected and the GUI is disabled
                Disconnect( i, j );

                // Load the connection from the XML
                mStreamThreads[ i ][ j ].Restore( lReader );

                // Update the GUI
                if( mStreamThreads[ i ][ j ].IsConnected() )
                {
                    mButtonConnect[ i ][ j ].SetWindowTextW( _T( "Disconnect" ) );
                    FormatDisplay( &mStreamThreads[ i ][ j ], lDisplayString ); 
                    mStaticConnection[ i ][ j ].SetWindowText( lDisplayString );
                    mStaticConnection[ i ][ j ].EnableWindow( TRUE );
                }
            }
        }

        // Read the config and reset the GUI
        lTransmitterConfig.Restore( lReader );
        SetTransmitterConfig( lTransmitterConfig );

        // Update the save menu state
        mLastFileName = lFileName;
        GetMenu()->GetSubMenu( 0 )->EnableMenuItem( ID_FILE_SAVE, MF_BYCOMMAND | MF_ENABLED | MF_GRAYED );
    }
}

void CTransmitTiledImagesDlg::SaveAs( CString& aFileName )
{
    PvConfigurationWriter lWriter;
    TransmitterConfig lTransmitterConfig;
    CWaitCursor lWait;

    if( aFileName.IsEmpty() )
    {
        CFileDialog lFileDlg( FALSE, _T( ".pvt" ), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T( "PvTransmitTileImage Sample Configuration Files (*.pvt)|*.pvt|All files (*.*)|*.*||" ), this );

        lFileDlg.m_ofn.lpstrTitle = _T( "Save Project" );

        if( lFileDlg.DoModal() != IDOK )
        {
            return;
        }
        aFileName = lFileDlg.GetPathName();         
    }

    for( int i = 0; i < MAX_TILES_ROW; i++ )
    {
        for( int j = 0; j < MAX_TILES_COLUMN; j++ )
        {
            mStreamThreads[ i ][ j ].Store( lWriter );
        }
    }
  
    // Now read the Transmitter configuration and save it
    GetTransmitterConfig( lTransmitterConfig );
    lTransmitterConfig.Store( lWriter );

    // Store the configuration
    if( lWriter.Save( aFileName.GetBuffer() ).IsOK() )
    {
        mLastFileName = aFileName;
        GetMenu()->GetSubMenu( 0 )->EnableMenuItem( ID_FILE_SAVE, MF_BYCOMMAND | MF_ENABLED | MF_GRAYED );
    }
}


///
/// \brief Displays communication parameters for the currently active source
///

void CTransmitTiledImagesDlg::OnCommunicationParameters()
{
    ASSERT( mActiveStreamThread != NULL );
    mActiveStreamThread->ShowCommunication();
    mActiveStreamThread = NULL;
}


///
/// \brief Displays device parameters for the currently active source
///

void CTransmitTiledImagesDlg::OnDeviceParameters()
{
    ASSERT( mActiveStreamThread != NULL );
    mActiveStreamThread->ShowDevice();
    mActiveStreamThread = NULL;
}


///
/// \brief Displays stream parameters for the currently active source
///

void CTransmitTiledImagesDlg::OnStreamParameters()
{
    ASSERT( mActiveStreamThread != NULL );
    mActiveStreamThread->ShowStream();
    mActiveStreamThread = NULL;
}

