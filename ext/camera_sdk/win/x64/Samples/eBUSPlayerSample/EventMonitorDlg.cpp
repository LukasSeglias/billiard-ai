// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"

#include "EventMonitorDlg.h"
#include "eBUSPlayerDlg.h"

#include <sstream>


#define MAX_LOG_SIZE ( 256 * 1024 )


IMPLEMENT_DYNAMIC(EventMonitorDlg, CDialog)

BEGIN_MESSAGE_MAP(EventMonitorDlg, CDialog)
    ON_BN_CLICKED(IDC_CLEAR_BUTTON, &EventMonitorDlg::OnBnClickedClearButton)
    ON_WM_SIZE()
    ON_WM_GETMINMAXINFO()
    ON_WM_TIMER()
    ON_WM_SHOWWINDOW()
    ON_WM_DESTROY()
    ON_WM_CREATE()
    ON_BN_CLICKED(IDC_SAVELOGCHECKBOX, &EventMonitorDlg::OnBnClickedSaveLogCheckBox)
    ON_BN_CLICKED(IDC_SAVELOGBUTTON, &EventMonitorDlg::OnBnClickedSaveLogButton)
    ON_BN_CLICKED(IDC_GENICAMCHECKBOX, &EventMonitorDlg::OnBnClickedGenICamCheckBox)
    ON_BN_CLICKED(IDC_EVENTSCHECKBOX, &EventMonitorDlg::OnBnClickedEventsCheckBox)
    ON_BN_CLICKED(IDC_BUFFERALLCHECKBOX, &EventMonitorDlg::OnBnClickedBufferAllCheckBox)
    ON_BN_CLICKED(IDC_CHUNKCHECKBOX, &EventMonitorDlg::OnBnClickedChunkCheckBox)
    ON_BN_CLICKED(IDC_BUFFERERRORCHECKBOX, &EventMonitorDlg::OnBnClickedBufferErrorCheckBox)
    ON_BN_CLICKED(IDC_SERIAL_COM_LOG_ENABLED, &EventMonitorDlg::OnBnClickedSerialComLogEnabled)
    ON_BN_CLICKED(IDC_LOGSINKCHECKBOX, &EventMonitorDlg::OnBnClickedLogSinkEnabled)
END_MESSAGE_MAP()


// ==========================================================================
EventMonitorDlg::EventMonitorDlg( LogBuffer *aLogBuffer, CWnd* pParent /*=NULL*/ )
    : CDialog( EventMonitorDlg::IDD, pParent )
    , mNeedInit( TRUE )
    , mLogEditTextLength( 0 )
    , mLastUpdateTime( 0 )
    , mTimer( 0 )
    , mLogBuffer( aLogBuffer )
{
}


// ==========================================================================
EventMonitorDlg::~EventMonitorDlg()
{
}


// ==========================================================================
void EventMonitorDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LOG_EDIT, mLogEdit);
    DDX_Control(pDX, IDC_SAVELOGCHECKBOX, mSaveLogCheckBox);
    DDX_Control(pDX, IDC_SAVELOGEDIT, mSaveLogEdit);
    DDX_Control(pDX, IDC_SAVELOGBUTTON, mSaveLogButton);
    DDX_Control(pDX, IDC_GENICAMCHECKBOX, mGenICamCheckBox);
    DDX_Control(pDX, IDC_EVENTSCHECKBOX, mEventsCheckBox);
    DDX_Control(pDX, IDC_BUFFERERRORCHECKBOX, mBufferErrorCheckBox);
    DDX_Control(pDX, IDC_BUFFERALLCHECKBOX, mBufferAllCheckBox);
    DDX_Control(pDX, IDC_CHUNKCHECKBOX, mChunkCheckBox);
    DDX_Control(pDX, IDC_SERIAL_COM_LOG_ENABLED, mSerialComLogCheck);
    DDX_Control(pDX, IDC_LOGSINKCHECKBOX, mLogSinkCheckBox);
}


// ==========================================================================
BOOL EventMonitorDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    GetClientRect( mCrt );
    mNeedInit = FALSE;

    mWindowBackgroundBrush.CreateSolidBrush( ::GetSysColor( COLOR_WINDOW ) );

    mLogEdit.SetLimitText( MAX_LOG_SIZE * 2 );
    mLogEdit.HideCaret();

    if ( mTerminalFont.GetSafeHandle() == 0 )
    {
        mTerminalFont.CreateFont( 14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, _T( "Courier New" ) );
    }

    mLogEdit.SetFont( &mTerminalFont );

    LoadData();

    return TRUE;
}


// ==============================================================================
void EventMonitorDlg::OnTimer( UINT_PTR nIDEvent )
{
    if ( nIDEvent == 1 )
    {
        std::list<std::string> &lEntries = mLogBuffer->Lock();
        if ( lEntries.size() <= 0 )
        {
            mLogBuffer->Unlock();
            return;
        }

        // Reduce flicker by ignoring WM_PAINT
        ::SendMessage( mLogEdit.GetSafeHwnd(), WM_SETREDRAW, false, 0 );

        std::wstringstream lSS;
        std::list<std::string>::iterator lIt =lEntries.begin();
        while ( lIt != lEntries.end() )
        {
            PvString lPvString( lIt->c_str() );
            lSS << lPvString.GetUnicode();
            lSS << _T( "\r\n" );

            lIt++;
        }

        std::wstring lStr = lSS.str();
        size_t lLength = lStr.length();

        mLogEditTextLength += static_cast<int>( lStr.length() );
        mLogEdit.SetSel( MAX_LOG_SIZE * 4, MAX_LOG_SIZE * 4, TRUE );
        mLogEdit.ReplaceSel( lStr.c_str() );

        mLogBuffer->Unlock();

        if ( mLogEditTextLength > MAX_LOG_SIZE )
        {
            mLogEdit.SetSel( 0, static_cast<int>( lStr.length() ) );
            mLogEdit.ReplaceSel( _T( "" ) );
            mLogEditTextLength -= static_cast<int>( lStr.length() );
        }

        // Make sure we scroll to the end. MAX_LOG_SIZE is the worst case 
        // scenario, one char per line...
        ::SendMessage( mLogEdit.GetSafeHwnd(), EM_LINESCROLL, 0, MAX_LOG_SIZE );
        ::SendMessage( mLogEdit.GetSafeHwnd(), WM_SETREDRAW, true, 0 );
    }

    CDialog::OnTimer( nIDEvent );
}


// ==========================================================================
void EventMonitorDlg::OnBnClickedClearButton()
{
    mLogBuffer->Reset();
    mLogEdit.SetWindowText( _T( "" ) );
    mLogEditTextLength = 0;
}


// ==========================================================================
void EventMonitorDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    CRect lR;
    this->GetWindowRect( &lR );
    
    if ( mNeedInit )
    {
        return;
    }

    int dx = cx - mCrt.Width();
    int dy = cy - mCrt.Height();
    GetClientRect(&mCrt);

    CRect r1;

    HDWP hDWP = ::BeginDeferWindowPos( 20 );

    //
    // Bottom left, resizing
    //

    int lBottomLeftResizing[] =
    {
        IDC_PARAMETERS_GROUP, IDC_LISTBOX
    };

    for ( int i = 0; i < sizeof( lBottomLeftResizing ) / sizeof ( lBottomLeftResizing[ 0 ] ); i++ )
    {
        CWnd *lChild = GetDlgItem( lBottomLeftResizing[ i ] );
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
    // Bottom left, sticking to the bottom
    //

    int lBottomLeft[] =
    {
        IDC_INFO, IDC_RESET_BUTTON
    };

    for ( int i = 0; i < sizeof( lBottomLeft ) / sizeof ( lBottomLeft[ 0 ] ); i++ )
    {
        CWnd *lChild = GetDlgItem( lBottomLeft[ i ] );
        if ( lChild != NULL )
        {
            lChild->GetWindowRect(&r1); ScreenToClient(&r1); 
            r1.top += dy;
            r1.bottom += dy;
            ::DeferWindowPos(
                hDWP, lChild->m_hWnd, NULL, 
                r1.left, r1.top, r1.Width(), r1.Height(),
                SWP_NOACTIVATE|SWP_NOZORDER);
        }
    }

    //
    // Right, make sure the log and group box fill the right part
    // of our window
    //

    int lRight[] = 
    { 
        IDC_LOG_EDIT, IDC_LOG_GROUP
    };

    for ( int i = 0; i < sizeof( lRight ) / sizeof ( lRight[ 0 ] ); i++ )
    {
        CWnd *lChild = GetDlgItem( lRight[ i ] );
        if ( lChild != NULL )
        {
            lChild->GetWindowRect(&r1); ScreenToClient(&r1); 
            r1.right += dx; r1.bottom += dy;
            ::DeferWindowPos(
                hDWP, lChild->m_hWnd, NULL, 
                r1.left, r1.top, r1.Width(), r1.Height(),
                SWP_NOACTIVATE|SWP_NOZORDER);
        }
    }

    //
    // Reset button box. Just like the log, but sticks to the bottom
    // and is not resized
    //

    int lBottomRight[] = 
    { 
        IDC_CLEAR_BUTTON, IDC_SAVELOGEDIT
    };

    for ( int i = 0; i < sizeof( lBottomRight ) / sizeof ( lBottomRight[ 0 ] ); i++ )
    {
        CWnd *lChild = GetDlgItem( lBottomRight[ i ] );
        if ( lChild != NULL )
        {
            lChild->GetWindowRect(&r1); ScreenToClient(&r1); 
            ::DeferWindowPos(
                hDWP, lChild->m_hWnd, NULL, 
                r1.left, r1.top + dy, r1.Width() + dx, r1.Height(),
                SWP_NOACTIVATE|SWP_NOZORDER);
        }
    }

    // Save log check box. Sticks to bottom left, not resized
    
    int lBottomRightFixed[] = 
    { 
        IDC_SAVELOGCHECKBOX
    };

    for ( int i = 0; i < sizeof( lBottomRightFixed ) / sizeof ( lBottomRightFixed[ 0 ] ); i++ )
    {
        CWnd *lChild = GetDlgItem( lBottomRightFixed[ i ] );
        if ( lChild != NULL )
        {
            lChild->GetWindowRect(&r1); ScreenToClient(&r1); 
            ::DeferWindowPos(
                hDWP, lChild->m_hWnd, NULL, 
                r1.left, r1.top + dy, r1.Width(), r1.Height(),
                SWP_NOACTIVATE|SWP_NOZORDER);
        }
    }

    // Bottom right, "..." button
    mSaveLogButton.GetWindowRect(&r1); ScreenToClient(&r1); 
    ::DeferWindowPos(
        hDWP, mSaveLogButton.m_hWnd, NULL, 
        r1.left + dx, r1.top + dy, r1.Width(), r1.Height(),
        SWP_NOACTIVATE|SWP_NOZORDER);

    //
    // Top right check boxes
    //

    int lTopRight[] = 
    { 
        IDC_GENICAMCHECKBOX, IDC_EVENTSCHECKBOX, IDC_BUFFERERRORCHECKBOX, IDC_BUFFERALLCHECKBOX,
        IDC_SERIAL_COM_LOG_ENABLED, IDC_SERIAL_COMMUNICATIONS_LABEL, IDC_LOGSINKCHECKBOX
    };

    for ( int i = 0; i < sizeof( lTopRight ) / sizeof ( lTopRight[ 0 ] ); i++ )
    {
        CWnd *lChild = GetDlgItem( lTopRight[ i ] );
        if ( lChild != NULL )
        {
            lChild->GetWindowRect(&r1); ScreenToClient(&r1); 
            ::DeferWindowPos(
                hDWP, lChild->m_hWnd, NULL, 
                r1.left + dx, r1.top, r1.Width(), r1.Height(),
                SWP_NOACTIVATE|SWP_NOZORDER);
        }
    }

    ::EndDeferWindowPos(hDWP);
}


// ==========================================================================
void EventMonitorDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
    lpMMI->ptMinTrackSize.x = 480;
    lpMMI->ptMinTrackSize.y = 200;
}


// ==========================================================================
void EventMonitorDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CDialog::OnShowWindow(bShow, nStatus);
    if ( bShow )
    {
        if ( mTimer == 0 )
        {
            mTimer = SetTimer( 1, 200, NULL );
        }
    }
    else
    {
        if ( mTimer != 0 )
        {
            KillTimer( 1 );
            mTimer = 0;
        }
    }
}


// ==========================================================================
void EventMonitorDlg::OnDestroy()
{
    CDialog::OnDestroy();

    if ( mTimer != 0 )
    {
        KillTimer( 1 );
        mTimer = 0;
    }
}


// ==========================================================================
void EventMonitorDlg::OnOK()
{
    // Do nothing
}


// ==========================================================================
void EventMonitorDlg::OnCancel()
{
    // This window life-cycle is managed from the outside
    ShowWindow( SW_HIDE );
}


// ==========================================================================
int EventMonitorDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if ( CDialog::OnCreate( lpCreateStruct ) == -1 )
    {
        return -1;
    }

    // This small change will hide the icon
    ModifyStyleEx( 0, WS_EX_DLGMODALFRAME );

    return 0;
}


// ==============================================================================
void EventMonitorDlg::OnBnClickedGenICamCheckBox()
{
    mLogBuffer->SetGenICamEnabled( mGenICamCheckBox.GetCheck() == BST_CHECKED );
}


// ==============================================================================
void EventMonitorDlg::OnBnClickedEventsCheckBox()
{
    mLogBuffer->SetEventsEnabled( mEventsCheckBox.GetCheck() == BST_CHECKED );
}


// ==============================================================================
void EventMonitorDlg::OnBnClickedBufferErrorCheckBox()
{
    mLogBuffer->SetBufferErrorEnabled( mBufferErrorCheckBox.GetCheck() == BST_CHECKED );
}


// ==============================================================================
void EventMonitorDlg::OnBnClickedBufferAllCheckBox()
{
    mLogBuffer->SetBufferAllEnabled( mBufferAllCheckBox.GetCheck() == BST_CHECKED );
    EnableInterface();
}


// ==============================================================================
void EventMonitorDlg::OnBnClickedChunkCheckBox()
{
    mLogBuffer->SetChunkEnabled( mChunkCheckBox.GetCheck() == BST_CHECKED );
}


// ==============================================================================
void EventMonitorDlg::OnBnClickedSaveLogCheckBox()
{
    mLogBuffer->SetWriteToFileEnabled( mSaveLogCheckBox.GetCheck() == BST_CHECKED );
    EnableInterface();
}


// ==============================================================================
void EventMonitorDlg::OnBnClickedSaveLogButton()
{
    CFileDialog lDlg( FALSE, NULL, mLogBuffer->GetFilename(), 
        OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
        _T( "Text files (*.txt)|All files (*.*)||" ), this );

    if ( lDlg.DoModal() == IDOK )
    {
        mLogBuffer->SetFilename( (LPCTSTR)lDlg.GetPathName() );
    }

    LoadData();
}


// ==============================================================================
void EventMonitorDlg::EnableInterface()
{
    bool lSaveEnabled = ( mSaveLogCheckBox.GetCheck() == BST_CHECKED );
    mSaveLogEdit.EnableWindow( false );
    mSaveLogButton.EnableWindow( lSaveEnabled );

    bool lBufferAllEnabled = ( mBufferAllCheckBox.GetCheck() == BST_CHECKED );
    mChunkCheckBox.EnableWindow( lBufferAllEnabled );
}


// ==============================================================================
void EventMonitorDlg::LoadData()
{
    mLogSinkCheckBox.SetCheck( ( mLogBuffer->IsLogSinkEnabled() ) ? BST_CHECKED : BST_UNCHECKED );
    mSerialComLogCheck.SetCheck( ( mLogBuffer->IsSerialComLogEnabled() ) ? BST_CHECKED : BST_UNCHECKED );
    mGenICamCheckBox.SetCheck( ( mLogBuffer->IsGenICamEnabled() ) ? BST_CHECKED : BST_UNCHECKED );
    mEventsCheckBox.SetCheck( ( mLogBuffer->IsEventsEnabled() ) ? BST_CHECKED : BST_UNCHECKED );
    mBufferErrorCheckBox.SetCheck( ( mLogBuffer->IsBufferErrorEnabled() ) ? BST_CHECKED : BST_UNCHECKED );
    mBufferAllCheckBox.SetCheck( ( mLogBuffer->IsBufferAllEnabled() ) ? BST_CHECKED : BST_UNCHECKED );
    mChunkCheckBox.SetCheck( ( mLogBuffer->IsChunkEnabled() ) ? BST_CHECKED : BST_UNCHECKED );
    mSaveLogCheckBox.SetCheck( ( mLogBuffer->IsWriteToFileEnabled() ) ? BST_CHECKED : BST_UNCHECKED );
    mSaveLogEdit.SetWindowText( mLogBuffer->GetFilename() );

    EnableInterface();
}


// ==============================================================================
void EventMonitorDlg::OnBnClickedSerialComLogEnabled()
{
    mLogBuffer->SetSerialComLogEnabled( mSerialComLogCheck.GetCheck() == BST_CHECKED );
}


// ==============================================================================
void EventMonitorDlg::OnBnClickedLogSinkEnabled()
{
    mLogBuffer->SetLogSinkEnabled( mLogSinkCheckBox.GetCheck() == BST_CHECKED );
}

