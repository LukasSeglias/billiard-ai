// *****************************************************************************
//
//     Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "afxwin.h"

#include "Setup.h"


class PvDeviceInfoGEV;


class SetupDlg : public CDialog
{
public:

    SetupDlg( CWnd* pParent = NULL );
    virtual ~SetupDlg();

    void SetSetup( Setup *aSetup ) { mSetup = *aSetup; }
    const Setup *GetSetup() const { return &mSetup; }

    void SetDevice( const PvDeviceInfoGEV *aDeviceInfo ) { mDeviceInfo = aDeviceInfo; }

    void SetEnabled( bool aEnabled ) { mEnabled = aEnabled; }

protected:

    void EnableInterface();
    void IPStrToCtrl( const CString &aIPStr, CIPAddressCtrl &aCtrl );

    virtual void DoDataExchange(CDataExchange* pDX);
    BOOL OnInitDialog();
    void OnOK();
    void OnCancel();
    afx_msg void OnBnClicked();
    DECLARE_MESSAGE_MAP()

    CButton mCtrlDataRadio;
    CButton mCtrlRadio;
    CButton mDataRadio;
    CButton mUnicastSpecificRadio;
    CButton mUnicastAutoRadio;
    CButton mUnicastOtherRadio;
    CIPAddressCtrl mUnicastIPCtrl;
    CEdit mUnicastSpecificPortEdit;
    CEdit mUnicastPortEdit;
    CButton mMulticastRadio;
    CIPAddressCtrl mMulticastIPCtrl;
    CStatic mUnicastSpecificPortLabel;
    CStatic mUnicastIPLabel;
    CStatic mUnicastPortLabel;
    CEdit mMulticastPortEdit;
    CStatic mMulticastIPLabel;
    CStatic mMulticastPortLabel;

private:

    Setup mSetup;
    bool mEnabled;

    const PvDeviceInfoGEV *mDeviceInfo;
};


