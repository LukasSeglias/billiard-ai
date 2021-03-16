// *****************************************************************************
//
//     Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "Resource.h"


class SplashPage : public CDialog
{
public:

    static void Show();

    SplashPage( CWnd* pParent = NULL );
    virtual ~SplashPage();

    enum { IDD = IDD_SPLASHPAGE };

protected:

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


