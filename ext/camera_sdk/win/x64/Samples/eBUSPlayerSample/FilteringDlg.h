// *****************************************************************************
//
//     Copyright (c) 2008, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "resource.h"

#include <ImageFiltering.h>
#include <DisplayThread.h>

#include "HistogramCtrl.h"


class FilteringDlg : public CDialog
{
public:

    FilteringDlg( ImageFiltering *aImageFiltering, DisplayThread *aDisplayThread, CWnd* pParent = NULL );
    virtual ~FilteringDlg();

    enum { IDD = IDD_FILTERING };

    void ToDialog();
    void FromDialog();

protected:

    CButton mRGBFilterCheckBox;
    CEdit mRGEdit;
    CEdit mGGEdit;
    CEdit mBGEdit;
    CEdit mROEdit;
    CEdit mGOEdit;
    CEdit mBOEdit;
    CSpinButtonCtrl mRGSpin;
    CSpinButtonCtrl mGGSpin;
    CSpinButtonCtrl mBGSpin;
    CSpinButtonCtrl mROSpin;
    CSpinButtonCtrl mGOSpin;
    CSpinButtonCtrl mBOSpin;
    CSliderCtrl mRGSlider;
    CSliderCtrl mGGSlider;
    CSliderCtrl mBGSlider;
    CSliderCtrl mROSlider;
    CSliderCtrl mGOSlider;
    CSliderCtrl mBOSlider;
    CButton mWBButton;
    CButton mResetButton;
    CComboBox mBayerCombo;
    CComboBox mTapCombo;
    CStatic mGainsLabel;
    CStatic mOffsetsLabel;
    CStatic mRedLabel;
    CStatic mGreenLabel;
    CStatic mBlueLabel;

    CButton mHistogramCheck;
    HistogramCtrl *mHistogramCtrl;
    CSliderCtrl mDarkSlider;
    CSliderCtrl mLightSlider;
    CButton mAutoConfigButton;

    CFont mBoldFont;

    virtual void DoDataExchange(CDataExchange* pDX);
    BOOL PreTranslateMessage(MSG *pMsg);
    afx_msg void OnOK();
    afx_msg void OnCancel();
    BOOL OnInitDialog();
    afx_msg void OnBnClickedEnabledcheck();
    afx_msg void OnEnChange();
    afx_msg void OnBnClickedWbbutton();
    afx_msg void OnBnClickedResetbutton();
    afx_msg void OnCbnSelchange();
    afx_msg void OnDeltapos(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnBnClickedHistogramenabled();
    afx_msg void OnBnClickedAutoconfigure();
    DECLARE_MESSAGE_MAP()

    void EnableInterface();
    void LoadHistogram();

    bool GetValueD( CEdit *aEdit, double &aValue );
    bool GetValueD( const CString &aStr, double &aValue );
    bool GetValueI( CEdit *aEdit, int32_t &aValue );
    bool GetValueI( const CString &aStr, int32_t &aValue );
    double FromGainSlider( CSliderCtrl *aSlider );
    void ToGainSlider( CSliderCtrl *aSlider, double &aValue );

    CString ToStr( double aValue );
    CString ToStr( int32_t aValue );
    void SyncSpins();
    void SyncSliders();
    
    bool mLoading;
    int mSkipSyncSpinsCount;
    int mSkipSyncSlidersCount;

    ImageFiltering *mImageFiltering;
    DisplayThread *mDisplayThread;

};
