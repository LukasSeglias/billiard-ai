// *****************************************************************************
//
//     Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "Resource.h"


class AboutBox : public CDialog
{
public:

    AboutBox( CWnd* pParent = NULL );
    virtual ~AboutBox();

    enum { IDD = IDD_ABOUTBOX };

protected:

    CFont mBoldFont;

    CStatic mAppNameLabel;
    CStatic mProductNameLabel;
    CStatic mCopyrightLabel;
    CStatic mCompanyLabel;

    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    DECLARE_MESSAGE_MAP()
};


