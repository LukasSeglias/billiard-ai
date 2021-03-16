// *****************************************************************************
//
//     Copyright (c) 2008, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"
#include "ProgressDlg.h"

#include <PvDevice.h>
#include <PvInterface.h>


IMPLEMENT_DYNAMIC(ProgressDlg, CDialog)


BEGIN_MESSAGE_MAP(ProgressDlg, CDialog)
    ON_WM_DESTROY()
    ON_WM_TIMER()
END_MESSAGE_MAP()


///
/// \brief Constructor
///

ProgressDlg::ProgressDlg( CWnd* pParent /*=NULL*/ ) 
    : CDialog( ProgressDlg::IDD, pParent )
    , mTask( NULL )
    , mThreadHandle( 0 )
    , mTitle( _T( "Progress Dialog" ) )
{

}


///
/// \brief Destructor
///

ProgressDlg::~ProgressDlg()
{

}


///
/// \brief DoDataExchange
///

void ProgressDlg::DoDataExchange( CDataExchange* pDX )
{
    CDialog::DoDataExchange( pDX );
    DDX_Control( pDX, IDC_STATUS, mStatusLabel );
}


///
/// \brief OK override: do nothing!
///

void ProgressDlg::OnOK()
{
}


///
/// \brief Cancel override: do nothing!
///

void ProgressDlg::OnCancel()
{
}


///
/// \brief Runs a task
///

int ProgressDlg::RunTask( Task *aTask )
{
    mTask = aTask;
    mTask->SetProgress( this );

    INT_PTR lReturn = DoModal();

    mTask->SetProgress( NULL );
    mTask = NULL;
    
    return static_cast<int>( lReturn );
}


///
/// \brief Dialog init handler
///

BOOL ProgressDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Load processing wheel bitmap
    VERIFY( mWheelBitmap.LoadBitmap( IDB_WHEEL ) );

    // Init UI - before the dialog is visible
    Update();

    // Start task thread (before we start the timer)
    DWORD lID = 0;
    mThreadHandle = ::CreateThread(
        NULL,               // Security attributes
        0,                  // Stack size, 0 is default
        Link,               // Start address
        this,               // Parameter
        0,                  // Creation flags
        &lID );             // Thread ID

    // Create timer that will be used to pump status info to the UI
    mTimer = SetTimer( 1, 50, NULL );

    SetWindowText( mTitle );

    return TRUE;
}


///
/// \brief Timer event handler
///

void ProgressDlg::OnTimer( UINT_PTR nIDEvent )
{
    if ( nIDEvent == 1 )
    {
        // Pain DC
        CClientDC lClientDC( this );

        // Bitmap DC
        CDC lBitmapDC;
        lBitmapDC.CreateCompatibleDC( &lClientDC );
        lBitmapDC.SelectObject( mWheelBitmap );

        // Blit the wheel on screen
        lClientDC.FillSolidRect( 
            12, 20 - 1, 16, 16, 
            ::GetSysColor( COLOR_3DFACE ) );
        lClientDC.TransparentBlt(
            12, 20 - 1, 16, 16, 
            &lBitmapDC, 
            mWheelIndex * 16, 0, 16, 16, 
            RGB( 0xFF, 0xFF, 0xFF ) );

        // Advance to next image in sprite
        ( ++mWheelIndex ) %= 8;

        // Update dialog
        Update();

        // If the task thread is done, complete
        if ( ::WaitForSingleObject( mThreadHandle, 0 ) == WAIT_OBJECT_0 )
        {
            EndDialog( IDOK );
        }
    }

    CDialog::OnTimer( nIDEvent );
}


///
/// \brief On destroy handler
///

void ProgressDlg::OnDestroy()
{
    CDialog::OnDestroy();

    if ( mTimer != 0 )
    {
        KillTimer( 1 );
        mTimer = 0;
    }

    if ( mThreadHandle != 0 )
    {
        ::WaitForSingleObject( mThreadHandle, INFINITE );
        mThreadHandle = 0;
    }
}


///
/// \brief Progress dialog update method. Can be called from UI or task thread.
///

void ProgressDlg::Update()
{
    CString lOldStr;
    mStatusLabel.GetWindowText( lOldStr );

    /////////////////////////////////////////////////////////////////
    mMutex.Lock();
        
    if ( lOldStr != mStatus )
    {
        mStatusLabel.SetWindowText( mStatus );
    }
        
    mMutex.Unlock();
    /////////////////////////////////////////////////////////////////
}


///
/// \brief Sets the progress status. Can be called from UI or task thread.
///

void ProgressDlg::SetStatus( const std::string &aStatus )
{
    /////////////////////////////////////////////////////////////////
    mMutex.Lock();

    PvString lString( aStatus.c_str() );
    mStatus = lString.GetUnicode();
    
    mMutex.Unlock();
    /////////////////////////////////////////////////////////////////

    // Make sure every step is visible, if only for a little while
    ::Sleep( 50 );
}


///
/// \brief Shows a warning (status change + delay)
///

void ProgressDlg::ShowWarning( const std::string &aStatus )
{
    SetStatus( aStatus );
    ::Sleep( 3000 );
}


///
/// \brief The thread entry point. Simply runs the task.
///

unsigned long WINAPI ProgressDlg::Link( void *aParam )
{
    ProgressDlg *lThis = reinterpret_cast<ProgressDlg *>( aParam );
    lThis->mTask->Execute();
    return 0;
}

