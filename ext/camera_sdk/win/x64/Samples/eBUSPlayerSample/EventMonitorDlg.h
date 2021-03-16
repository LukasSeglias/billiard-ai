// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once
#include "afxcmn.h"
#include "afxwin.h"

#include <map>

#include "ParameterInfo.h"
#include "LogBuffer.h"

#include "resource.h"


class EventMonitorDlg : public CDialog
{
    DECLARE_DYNAMIC(EventMonitorDlg)

public:

    EventMonitorDlg( LogBuffer *aLogBuffer, CWnd* pParent = NULL );   // standard constructor
    virtual ~EventMonitorDlg();

    enum { IDD = IDD_EVENTMONITOR };

    void LoadData();

protected:

    virtual void DoDataExchange( CDataExchange* pDX );    // DDX/DDV support
    BOOL OnInitDialog();
    afx_msg void OnBnClickedSelectall();
    afx_msg void OnBnClickedAddButton();
    afx_msg void OnBnClickedRemoveButton();
    afx_msg void OnBnClickedClearButton();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
    afx_msg void OnTimer( UINT_PTR nIDEvent );
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg void OnDestroy();
    afx_msg void OnOK();
    afx_msg void OnCancel();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnBnClickedGenICamCheckBox();
    afx_msg void OnBnClickedEventsCheckBox();
    afx_msg void OnBnClickedBufferErrorCheckBox();
    afx_msg void OnBnClickedBufferAllCheckBox();
    afx_msg void OnBnClickedChunkCheckBox();
    afx_msg void OnBnClickedSaveLogCheckBox();
    afx_msg void OnBnClickedSaveLogButton();
    afx_msg void OnBnClickedCheck1();
    afx_msg void OnBnClickedSerialComLogEnabled();
    afx_msg void OnBnClickedLogSinkEnabled();
    DECLARE_MESSAGE_MAP()

    void EnableInterface();

private:

    CBrush mWindowBackgroundBrush;

    CEdit mLogEdit;
    CButton mSaveLogCheckBox;
    CEdit mSaveLogEdit;
    CButton mSaveLogButton;
    CButton mGenICamCheckBox;
    CButton mEventsCheckBox;
    CButton mBufferErrorCheckBox;
    CButton mBufferAllCheckBox;
    CButton mChunkCheckBox;
    CButton mSerialComLogCheck;
    CButton mLogSinkCheckBox;

    UINT_PTR mTimer;

    int mLogEditTextLength;
    int mLastUpdateTime;

    CRect mCrt;
    BOOL mNeedInit;

    LogBuffer *mLogBuffer;
    CFont mTerminalFont;
};

