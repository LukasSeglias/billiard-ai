// *****************************************************************************
//
// Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// -----------------------------------------------------------------------------
//
// Dialog used to display an embedded GenICam tree browser window. We use it
// to display the default communication parameters used with new devices.
// 
// *****************************************************************************


#include "stdafx.h"

#include "DefaultComParamsDlg.h"

#include <PvVersion.h>


#ifdef _PT_DEBUG_
#pragma warning ( push )
#pragma warning ( disable : 4100 )
#endif // _PT_DEBUG_


BEGIN_MESSAGE_MAP( DefaultComParamsDlg, CDialog )
END_MESSAGE_MAP()


#ifdef _PT_DEBUG_
#pragma warning( pop )
#endif // _PT_DEBUG_


// ==========================================================================
DefaultComParamsDlg::DefaultComParamsDlg( CWnd* pParent /*=NULL*/ ) 
    : CDialog( DefaultComParamsDlg::IDD, pParent )
    , mGenParameterArray( NULL )
{
}

// ==========================================================================
DefaultComParamsDlg::~DefaultComParamsDlg()
{
}

// =============================================================================
void DefaultComParamsDlg::DoDataExchange( CDataExchange* pDX )
{
    CDialog::DoDataExchange( pDX );
}

// ==========================================================================
BOOL DefaultComParamsDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    ASSERT( mGenParameterArray != NULL );

    CWnd *lWnd = GetDlgItem( IDC_POS );
    ASSERT( lWnd != NULL );

    CRect lRect;
    lWnd->GetWindowRect( &lRect );
    ScreenToClient( &lRect );

    mGenBrowserWnd.Create( GetSafeHwnd(), 2 );
    mGenBrowserWnd.SetPosition( lRect.left, lRect.top, lRect.Width(), lRect.Height() );
    mGenBrowserWnd.SetGenParameterArray( mGenParameterArray );

    return TRUE;
}

// ==========================================================================
void DefaultComParamsDlg::SetGenParameter( PvGenParameterArray *aParameterArray )
{
    mGenParameterArray = aParameterArray;
}

// ==========================================================================
void DefaultComParamsDlg::OnOK()
{
}

