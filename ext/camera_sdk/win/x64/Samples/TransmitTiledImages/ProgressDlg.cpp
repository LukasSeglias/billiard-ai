// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
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


// =============================================================================
ProgressDlg::ProgressDlg( Thread* aThread, CWnd* pParent /*=NULL*/ )    
    : CDialog( ProgressDlg::IDD, pParent )
    , mThread( aThread )
    , mCanCancel( false )
{

}


// =============================================================================
ProgressDlg::~ProgressDlg()
{

}


// =============================================================================
void ProgressDlg::DoDataExchange( CDataExchange* pDX )
{
    CDialog::DoDataExchange( pDX );
    DDX_Control(pDX, IDC_STATUS, mStatusLabel);
    DDX_Control(pDX, IDCANCEL, mCancelButton);
}


// =============================================================================
void ProgressDlg::OnOK()
{
}


// =============================================================================
void ProgressDlg::OnCancel()
{
    mStatusLabel.SetWindowText( _T( "Cancelling operation..." ) );
    mThread->Stop();
}


// =============================================================================
BOOL ProgressDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Load processing wheel bitmap
    VERIFY( mWheelBitmap.LoadBitmap( IDB_WHEEL ) );

    // Init UI - before the dialog is visible
    Update();

    // Create timer that will be used to pump status info to the UI
    mTimer = SetTimer( 1, 100, NULL );

    return TRUE;
}


// ==============================================================================
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

        // If the device thread is done, complete and start stream thread
        if ( mThread->IsDone() )
        {
            EndDialog( IDOK );
        }
    }

    CDialog::OnTimer( nIDEvent );
}


// ==========================================================================
void ProgressDlg::OnDestroy()
{
    CDialog::OnDestroy();

    if ( mTimer != 0 )
    {
        KillTimer( 1 );
        mTimer = 0;
    }
}


// ==========================================================================
INT_PTR ProgressDlg::DoModal()
{
    mThread->Start();

    return CDialog::DoModal();
}


// ==========================================================================
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

        // Only enable cancel button when its actually possible to cancel
        mCancelButton.ShowWindow( mCanCancel ? SW_SHOW : SW_HIDE );
        mCancelButton.EnableWindow( mCanCancel );
    mMutex.Unlock();
    /////////////////////////////////////////////////////////////////
}

void ProgressDlg::SetCanCancel( BOOL aCanCancel )
{
    /////////////////////////////////////////////////////////////////
    mMutex.Lock();
        mCanCancel = TRUE;
    mMutex.Unlock();
    /////////////////////////////////////////////////////////////////
}

void ProgressDlg::SetStatus( CString aStatus )
{
    /////////////////////////////////////////////////////////////////
    mMutex.Lock();
        mStatus = aStatus;
    mMutex.Unlock();
    /////////////////////////////////////////////////////////////////
}

