// *****************************************************************************
//
//     Copyright (c) 2016, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "afxwin.h"
#include "afxdialogex.h"

#include "eBUsPlayerApp.h"
#include "eBUSPlayerDlg.h"


class PersistenceInfoDlg : public CDialogEx
{
	DECLARE_DYNAMIC( PersistenceInfoDlg )

public:

	PersistenceInfoDlg( CWnd* aParent = NULL );

	virtual ~PersistenceInfoDlg();
	virtual BOOL OnInitDialog();

	enum { IDD = IDD_PERSISTENCEINFO_DLG };

    bool IsDontShowAgain() const { return mDontShowAgain; }

    void SetDeviceContext();
    void SetPreferencesContext();

protected:

	virtual void DoDataExchange( CDataExchange* pDX );
	virtual void OnOK();
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()

private:

    CButton mDontShowAgainCheckBox;
    CStatic mMessageStatic;

    bool mDontShowAgain;
    
    CString mMessage;
    CString mTitle;

};

