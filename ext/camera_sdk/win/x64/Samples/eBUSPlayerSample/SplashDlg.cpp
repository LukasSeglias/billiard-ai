// *****************************************************************************
//
//     Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"

#include "SplashDlg.h"

#include <PvVersion.h>


#define TIMER_ID ( 100 )


BEGIN_MESSAGE_MAP( SplashDlg, CDialog )
    ON_WM_TIMER()
END_MESSAGE_MAP()


// ==========================================================================
SplashDlg::SplashDlg( const CString &aAppName, CWnd* pParent ) 
    : CDialog( SplashDlg::IDD, pParent )
    , mAppName( aAppName )
{
}

// ==========================================================================
SplashDlg::~SplashDlg()
{
}

// =============================================================================
void SplashDlg::DoDataExchange( CDataExchange* pDX )
{
    CDialog::DoDataExchange( pDX );
    DDX_Control(pDX, IDC_APPNAME, mAppNameLabel);
    DDX_Control(pDX, IDC_PRODUCTNAME, mProductNameLabel);
    DDX_Control(pDX, IDC_COPYRIGHT, mCopyrightLabel);
    DDX_Control(pDX, IDC_COMPANY, mCompanyLabel);
}

// ==========================================================================
void SplashDlg::OnTimer( UINT_PTR aEvent )
{
    KillTimer( aEvent );
    EndDialog( IDOK );
}

// ==========================================================================
BOOL SplashDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    SetTimer( TIMER_ID, 2500, NULL );
    
    // Create bold font
    LOGFONT lLogFont;
    mAppNameLabel.GetFont()->GetLogFont( &lLogFont );
    lLogFont.lfWeight = FW_BOLD;
    mBoldFont.CreateFontIndirect( &lLogFont );

    mAppNameLabel.SetFont( &mBoldFont );
    
    mAppNameLabel.SetWindowText( mAppName );
    mProductNameLabel.SetWindowText( CString( _T( "eBUS SDK Version " ) ) + _T( NVERSION_STRING ) );
    mCopyrightLabel.SetWindowText( _T( VERSION_COPYRIGHT ) );
    mCompanyLabel.SetWindowText( _T( VERSION_COMPANY_NAME ) );

    return TRUE;
}
