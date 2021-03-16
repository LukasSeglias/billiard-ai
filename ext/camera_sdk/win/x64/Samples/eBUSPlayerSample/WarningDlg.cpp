// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"
#include "WarningDlg.h"


IMPLEMENT_DYNAMIC(WarningDlg, CDialog)

BEGIN_MESSAGE_MAP(WarningDlg, CDialog)
    ON_BN_CLICKED(IDOK, &WarningDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDC_DONT_SHOW_AGAIN, &WarningDlg::OnBnClickedDontShowAgain)
END_MESSAGE_MAP()


WarningDlg::WarningDlg(CWnd* pParent /*=NULL*/)
    : CDialog(WarningDlg::IDD, pParent)
    , mChecked( false )
{

}

WarningDlg::~WarningDlg()
{
}


void WarningDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_DONT_SHOW_AGAIN, mDontShowAgainCheck);
    DDX_Control(pDX, IDC_WARNING_MESSAGE, mWarningMessage);
}


void WarningDlg::OnBnClickedOk()
{
    mChecked = mDontShowAgainCheck.GetCheck() == 0 ? false : true;

    OnOK();
}


void WarningDlg::OnBnClickedDontShowAgain()
{
}


void WarningDlg::SetWarning( const CString &aWarning )
{
    mWarningMsg = aWarning;
}


void WarningDlg::SetCheckboxMessage( const CString &aCheckboxMessage )
{
    mCheckboxMsg = aCheckboxMessage;
}


BOOL WarningDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    mDontShowAgainCheck.SetWindowText( mCheckboxMsg );
    mWarningMessage.SetWindowText( mWarningMsg );

    return TRUE;
}

