// *****************************************************************************
//
//     Copyright (c) 2008, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "resource.h"

#include <Setup.h>


class SetupDlg : public CDialog
{
public:

    SetupDlg( Setup *aSetup, CWnd* pParent = NULL );
    virtual ~SetupDlg();

protected:

    void SetCurrentRole( Setup::Role aRole );
    void SetCurrentDestination( Setup::Destination aDestination );

    Setup::Role GetCurrentRole();
    Setup::Destination GetCurrentDestination();

    void EnableInterface();
    void IPStrToCtrl( const CString &aIPStr, CIPAddressCtrl &aCtrl );

    virtual void DoDataExchange(CDataExchange* pDX);
    BOOL OnInitDialog();
    void OnOK();
    void OnCancel();
    afx_msg void OnBnClicked();
    DECLARE_MESSAGE_MAP()

private:

    CButton mCtrlDataRadio;
    CButton mCtrlRadio;
    CButton mDataRadio;
    CButton mUnicastSpecificRadio;
    CButton mUnicastAutoRadio;
    CButton mUnicastOtherRadio;
    CIPAddressCtrl mUnicastIPCtrl;
    CEdit mUnicastSpecificPortEdit;
    CStatic mChannelLabel;
    CEdit mChannelEdit;
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

    Setup *mSetup;

};


