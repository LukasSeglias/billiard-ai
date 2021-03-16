// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "resource.h"

#include <PvDevice.h>


class PvGenFile;


class FileTransferDlg : public CDialog, PvDeviceEventSink
{
    DECLARE_DYNAMIC(FileTransferDlg)

public:

    FileTransferDlg( PvDevice *aDevice, CWnd* pParent = NULL );
    virtual ~FileTransferDlg();

    enum { IDD = IDD_FILETRANSFER };

protected:

    virtual void DoDataExchange( CDataExchange *pDX );
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void OnCancel();

    afx_msg void OnCbnSelchangeFile();
    afx_msg void OnBnClickedUpload();
    afx_msg void OnBnClickedDownload();
    afx_msg void OnTimer( UINT_PTR nIDEvent );
    afx_msg void OnDestroy();
    DECLARE_MESSAGE_MAP()

    void LoadFileCombo();
    void EnableInterface();
    void Transfer();

    // Thread hook (used to drive progress)
    static unsigned long WINAPI Link( void *aParam );

    // PvDeviceEventSink callbacks
    virtual void OnLinkDisconnected( PvDevice *aDevice );

private:

    CStatic mGroup1;
    CStatic mGroup2;
    CComboBox mFileCombo;
    CButton mDownloadButton;
    CButton mUploadButton;
    CProgressCtrl mProgress;
    CStatic mProgressLabel;
    CButton mCancelButton;

    UINT_PTR mTimer;
    HANDLE mThreadHandle;

    PvDevice *mDevice;
    PvGenParameterArray *mParameters;

    // GenICam file object used to perform the file operation
    PvGenFile *mFile;

    // Operation info extracted from UI
    bool mWrite;
    CString mDeviceFile;
    CString mHostFile;
    PvResult mResult;

};


