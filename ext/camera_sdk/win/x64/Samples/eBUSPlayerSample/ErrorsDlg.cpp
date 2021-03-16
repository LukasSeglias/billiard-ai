// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"
#include "ErrorsDlg.h"

#include <PvStringList.h>


IMPLEMENT_DYNAMIC(ErrorsDlg, CDialog)


BEGIN_MESSAGE_MAP(ErrorsDlg, CDialog)
END_MESSAGE_MAP()


// =============================================================================
ErrorsDlg::ErrorsDlg( CWnd* pParent /*=NULL*/ )
    : CDialog( ErrorsDlg::IDD, pParent )
    , mErrors( NULL )
{

}


// =============================================================================
ErrorsDlg::~ErrorsDlg()
{
}


// =============================================================================
void ErrorsDlg::DoDataExchange( CDataExchange* pDX )
{
    CDialog::DoDataExchange( pDX );
    DDX_Control(pDX, IDC_TEXTBOX, mTextBox);
}


// =============================================================================
INT_PTR ErrorsDlg::DoModal( PvStringList *aErrors )
{
    mErrors = aErrors;
    return CDialog::DoModal();
}


// =============================================================================
BOOL ErrorsDlg::OnInitDialog()
{
    ASSERT( mErrors != NULL );
    
    CDialog::OnInitDialog();

    CString lBuffer;
    PvString *lError = mErrors->GetFirst();
    while ( lError != NULL )
    {
        lBuffer += lError->GetUnicode();
        lBuffer += _T( "\r\n\r\n" );

        lError = mErrors->GetNext();
    }

    mTextBox.SetWindowText( lBuffer );

    return TRUE;
}

