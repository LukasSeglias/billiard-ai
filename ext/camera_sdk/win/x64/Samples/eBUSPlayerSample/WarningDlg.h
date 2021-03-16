// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "resource.h"


class WarningDlg : public CDialog
{
    DECLARE_DYNAMIC(WarningDlg)

public:

    WarningDlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~WarningDlg();
    
    void SetWarning( const CString &aWarning );
    void SetCheckboxMessage( const CString &aMessage );
    bool IsChecked() { return mChecked; }

    enum { IDD = IDD_NO_DRIVER };

protected:

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedDontShowAgain();

    CButton mDontShowAgainCheck;
    CStatic mWarningMessage;

private:

    bool mChecked;
    CString mWarningMsg;
    CString mCheckboxMsg;

};


