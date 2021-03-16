// *****************************************************************************
//
//     Copyright (c) 2016, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "resource.h"

#include <PvCameraBridge.h>


class CameraControlDlg : public CDialog
{
    DECLARE_DYNAMIC( CameraControlDlg )

public:

    CameraControlDlg( PvCameraBridge *aCameraBridge, CWnd* pParent, bool aPCFVisible ); 
    virtual ~CameraControlDlg();

    enum { IDD = IDD_CAMERACONTROL };

    void EnableInterface();

    bool ConfigurePCF();
    bool ConfigureCLP();
    bool ConfigureGenCP();

    bool GetDontShowAgain() const { return mDontShowAgain; }

protected:

    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnBnClickedRadio();
    afx_msg void OnBnClickedPCFPathButton();
    afx_msg void OnEditChangePathEdit();
    afx_msg void OnCbnSelchangeCLPTemplate();
    DECLARE_MESSAGE_MAP()

private:

    CButton mPCFRadio;
    CStatic mPCFPathLabel;
    CEdit mPCFPathEdit;
    CButton mPCFPathButton;

    CButton mCLPRadio;
    CStatic mCLPTemplateLabel;
    CComboBox mCLPTemplateCombo;

    CButton mGenCPRadio;

    CButton mManualRadio;
    CStatic mManualDescription;

    CButton mNoPoCLRadio;
    CButton mPoCLRadio;

    PvCameraBridge *mCameraBridge;
    PvStringList mTemplates;

    CButton mDontShowAgainCheckBox;
    bool mDontShowAgain;

    CButton mOKButton;
    CButton mCancelButton;

    bool mPCFVisible;

};

