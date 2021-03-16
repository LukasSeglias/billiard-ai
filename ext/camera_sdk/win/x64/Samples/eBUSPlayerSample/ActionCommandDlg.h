// *****************************************************************************
//
//     Copyright (c) 2008, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "resource.h"

#include <PvActionCommand.h>
#include "HexEdit.h"


// ActionCommand dialog
class ActionCommandDlg : public CDialog
{
    DECLARE_DYNAMIC( ActionCommandDlg )

public:

    ActionCommandDlg( CWnd* pParent = NULL );
    virtual ~ActionCommandDlg();

    enum { IDD = IDD_ACTIONCOMMAND };

protected:

    void EnableInterface();

    virtual void DoDataExchange( CDataExchange* pDX );
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void OnCancel();

    DECLARE_MESSAGE_MAP()
    afx_msg void OnBnClickedCheckscheduled();
    afx_msg void OnBnClickedButtonsend();
    afx_msg void OnClBnChkChange();

    CHexEdit mDeviceKeyEdit;
    CHexEdit mGroupKeyEdit;
    CHexEdit mGroupMaskEdit;
    CHexEdit mScheduledTime;
    CButton mRequestAcknowledgesCheckBox;
    CButton mScheduledCheck;
    CStatic mActionTimeLabel;
    CButton mSendButton;
    CListBox mAcknowledgeList;
    CCheckListBox mInterfaceList;

private:

    PvActionCommand mActionCommand;

};

