// *****************************************************************************
//
//     Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "afxwin.h"
#include "Resource.h"

#include "Thread.h"

#include <PvStreamGEV.h>
#include <PvDevice.h>
#include <PvConfigurationReader.h>




class ProgressDlg : public CDialog
{
    DECLARE_DYNAMIC( ProgressDlg )

public:

    ProgressDlg( Thread *aThread, CWnd* aParentWnd = NULL );
    virtual ~ProgressDlg();

    virtual INT_PTR DoModal();

    void SetStatus( CString aStatus );

    enum { IDD = IDD_PROGRESS };

protected:

    virtual void DoDataExchange( CDataExchange* pDX );
    void OnOK();
    void OnCancel();
    BOOL OnInitDialog();
    afx_msg void OnTimer( UINT_PTR nIDEvent );
    afx_msg void OnDestroy();
    DECLARE_MESSAGE_MAP()

    void Update();

    CStatic mStatusLabel;
    CBitmap mWheelBitmap;

    UINT_PTR mTimer;

    DWORD mWheelIndex;

private:

    Thread *mThread;
    PvResult mResult;
    CString mStatus;
    CMutex mMutex;

};

