// *****************************************************************************
//
//     Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"

#include "SplashPage.h"

#include <PvVersion.h>


#ifdef _PT_DEBUG_
#pragma warning ( push )
#pragma warning ( disable : 4100 )
#endif // _PT_DEBUG_


BEGIN_MESSAGE_MAP( SplashPage, CDialog )
    ON_WM_TIMER()
END_MESSAGE_MAP()


// ==========================================================================
void SplashPage::Show()
{
    SplashPage lDlg;
    lDlg.DoModal();
}

#ifdef _PT_DEBUG_
#pragma warning( pop )
#endif // _PT_DEBUG_

// ==========================================================================
SplashPage::SplashPage( CWnd* pParent /*=NULL*/ )   
    : CDialog( SplashPage::IDD, pParent )
{
}

// ==========================================================================
SplashPage::~SplashPage()
{
}

// =============================================================================
void SplashPage::DoDataExchange( CDataExchange* pDX )
{
    CDialog::DoDataExchange( pDX );
    DDX_Control(pDX, IDC_APPNAME, mAppNameLabel);
    DDX_Control(pDX, IDC_PRODUCTNAME, mProductNameLabel);
    DDX_Control(pDX, IDC_COPYRIGHT, mCopyrightLabel);
    DDX_Control(pDX, IDC_COMPANY, mCompanyLabel);
}

// ==========================================================================
void SplashPage::OnTimer( UINT_PTR aEvent )
{
    KillTimer( aEvent );
    ShowWindow( SW_HIDE );

    EndDialog( IDOK );
}

// ==========================================================================
BOOL SplashPage::OnInitDialog()
{
    CDialog::OnInitDialog();
    SetTimer( 0, 2500, NULL );

    // Create bold font
    LOGFONT lLogFont;
    mAppNameLabel.GetFont()->GetLogFont( &lLogFont );
    lLogFont.lfWeight = FW_BOLD;
    mBoldFont.CreateFontIndirect( &lLogFont );

    mAppNameLabel.SetFont( &mBoldFont );

    mAppNameLabel.SetWindowText( _T( "NetCommand" ) );
    mProductNameLabel.SetWindowText( CString( _T( "eBUS SDK Version " ) ) + CString( NVERSION_STRING ) );
    mCopyrightLabel.SetWindowText( _T( VERSION_COPYRIGHT ) );
    mCompanyLabel.SetWindowText( _T( VERSION_COMPANY_NAME ) );

    return TRUE;
}
