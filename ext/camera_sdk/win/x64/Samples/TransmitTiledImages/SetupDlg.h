// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "Setup.h"
#include "IPAddress.h"

#include <PvDeviceInfo.h>

///
/// \class Connection Setup dialog
///
/// \brief This is a dialog to allow the user to select the type of connection he wants to do
///
class SetupDlg 
    : public CDialog
{
public:

    ///
    /// \brief Constructor
    ///
    /// \param aSetup The intialized struct to retrieve the information
    /// \param aParent The parent window
    ///
    SetupDlg( Setup* aSetup, PvDeviceInfoType aSetupType, CWnd* pParent = NULL );

    ///
    /// \brief Destructor
    ///
    virtual ~SetupDlg();

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    BOOL OnInitDialog();
    void OnOK();
    void OnCancel();
    afx_msg void OnBnClicked();
    DECLARE_MESSAGE_MAP()

private:
    // GUI elements
    CButton mCtrlDataRadio;
    CButton mDataRadio;
    CButton mUnicastAutoRadio;
    CButton mUnicastSpecificRadio;
    CEdit mUnicastSpecificPortEdit;
    CButton mMulticastRadio;
    IPAddress mMulticastIPCtrl;
    CEdit mMulticastPortEdit;
    CStatic mDestinationGroup;
    CStatic mSpecificPortLabel;
    CStatic mMulticastIPLabel;
    CStatic mMulticastPortLabel;

private:
    // memory location to store the result of the dialog call
    Setup* mSetup;

    PvDeviceInfoType mSetupType;

    ///
    /// \brief Manage the enable / disable element of the GUI
    ///
    void EnableInterface();
};


