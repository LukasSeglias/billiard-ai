// *****************************************************************************
//
//     Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "Resource.h"


class SplashDlg : public CDialog
{
public:

    SplashDlg( const CString &aAppName, CWnd* pParent = NULL );
    virtual ~SplashDlg();

    enum { IDD = IDD_SPLASHPAGE };

protected:

    CString mAppName;

    CFont mBoldFont;

    CStatic mAppNameLabel;
    CStatic mProductNameLabel;
    CStatic mCopyrightLabel;
    CStatic mCompanyLabel;

    virtual void DoDataExchange(CDataExchange* pDX);
    virtual void OnTimer( UINT_PTR aEvent );
    virtual BOOL OnInitDialog();
    DECLARE_MESSAGE_MAP()

};


