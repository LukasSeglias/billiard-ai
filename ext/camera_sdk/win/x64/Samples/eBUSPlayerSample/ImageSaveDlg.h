// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "resource.h"

#include <ImageSaving.h>


class ImageSaveDlg : public CDialog
{
    DECLARE_DYNAMIC( ImageSaveDlg )

public:

    ImageSaveDlg( ImageSaving *aImageSaving, CWnd* pParent = NULL ); 
    virtual ~ImageSaveDlg();

    enum { IDD = IDD_IMAGE_SAVE_OPTIONS };

    void EnableInterface();

    void ToDialog();
	bool FromDialog();

protected:

    virtual void DoDataExchange(CDataExchange* pDX);
    DECLARE_MESSAGE_MAP()

private:

    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedOneoutofRadio();
    afx_msg void OnBnClickedMaxrateRadio();
    afx_msg void OnBnClickedAveragethroughputRadio();
    afx_msg void OnBnClickedNothrottleRadio();
    afx_msg void OnBnClickedLocationButton();
    afx_msg void OnBnClickedImagesaveenable();
    afx_msg void OnCbnSelchangeFormat();
	bool ValidateImageFolder();

    bool FolderExists( const CString lSavePath );

    bool FolderPathIsEmpty( const CString &lSavePath );

    CEdit mOneOutOfEdit;
    CEdit mMaxRateEdit;
    CEdit mAverageThroughputEdit;
    CEdit mSavePathEdit;
    CButton mSaveEnabledCheck;
    CButton mLocationGroup;
    CButton mLocationButton;
    CButton mThrottlingGroup;
    CButton mOneOutOfRadio;
    CButton mMaxRateRadio;
    CButton mAverageThroughputRadio;
    CButton mNoThrottleRadio;
    CStatic mCapturedImagesLabel;
    CStatic mMsLabel;
    CStatic mAverageLabel;
    CComboBox mFormatCombo;
    CEdit mAvgBitrateEdit;

    ImageSaving *mImageSaving;
};

