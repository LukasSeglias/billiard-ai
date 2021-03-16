// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "afxwin.h"

#include "DisplayThread.h"
#include "Resource.h"

#include <PvDeviceFinderWnd.h>
#include <PvDevice.h>
#include <PvGenParameter.h>
#include <PvGenBrowserWnd.h>
#include <PvBuffer.h>
#include <PvStream.h>
#include <PvPipeline.h>
#include <PvDisplayWnd.h>
#include <PvAcquisitionStateManager.h>
#include <PvDeviceGEV.h>
#include <PvDeviceInfoGEV.h>
#include <PvDeviceInfoU3V.h>
#include <PvStreamGEV.h>


#include "afxcmn.h"


#define WM_ACQUISITIONSTATECHANGED ( WM_USER + 0x4440 )


class SimpleGUIApplicationDlg : public CDialog, PvGenEventSink, PvAcquisitionStateEventSink
{
public:

    SimpleGUIApplicationDlg( CWnd *pParent = NULL );
    virtual ~SimpleGUIApplicationDlg();

    enum { IDD = IDD_SIMPLEGUIDAPPLICATION };

    void StartStreaming();
    void StopStreaming();

    bool IsAcquiringImages() const { return mAcquiringImages; }

protected:

    HICON m_hIcon;

    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnGetMinMaxInfo( MINMAXINFO *lpMMI );
    afx_msg void OnBnClickedDeviceButton();
    afx_msg void OnBnClickedLinkButton();
    afx_msg void OnBnClickedStreamparamsButton();
    afx_msg void OnBnClickedConnectButton();
    afx_msg void OnBnClickedDisconnectButton();
    afx_msg void OnClose();
    afx_msg void OnBnClickedStart();
    afx_msg void OnBnClickedStop();
    afx_msg void OnCbnSelchangeMode();
    afx_msg void OnBnClickedDeviceEvents();
    afx_msg void OnMove(int x, int y);
    LRESULT OnAcquisitionStateChanged(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()

    void EnableInterface();

    void Connect( const PvDeviceInfo *aDI );
    void Disconnect();
    void StartAcquisition();
    void StopAcquisition();

    void ShowGenWindow( PvGenBrowserWnd **aWnd, PvGenParameterArray *aParams, const CString &aTitle );
    void CloseGenWindow( PvGenBrowserWnd **aWnd );

    // PvGenEventSink implementation
    void OnParameterUpdate( PvGenParameter *aParameter );

    // PvAcquisitionStateEventSink implementation
    void OnAcquisitionStateChanged( PvDevice* aDevice, PvStream* aStream, uint32_t aSource, PvAcquisitionState aState );

    CRect mCrt;
    BOOL mNeedInit;

    CComboBox mModeCombo;
    CButton mPlayButton;
    CButton mStopButton;

    CEdit mIPEdit;
    CEdit mMACEdit;
    CEdit mGUIDEdit;
    CEdit mManufacturerEdit;
    CEdit mModelEdit;
    CEdit mNameEdit;

    PvDevice* mDevice;
    PvStream* mStream;
    PvPipeline* mPipeline;

    bool mAcquiringImages;

    DisplayThread *mDisplayThread;

    PvGenBrowserWnd *mDeviceWnd;
    PvGenBrowserWnd *mCommunicationWnd;
    PvGenBrowserWnd *mStreamParametersWnd;

    PvDisplayWnd mDisplay;

    PvAcquisitionStateManager *mAcquisitionStateManager;
};

