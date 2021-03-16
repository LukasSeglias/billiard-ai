// *****************************************************************************
//
//     Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "Resource.h"

#include <PvDevice.h>
#include <PvGenBrowserWnd.h>


class DefaultComParamsDlg : public CDialog
{
public:

    DefaultComParamsDlg( CWnd* pParent = NULL );
    virtual ~DefaultComParamsDlg();

    void SetGenParameter( PvGenParameterArray *aParameterArray );

    enum { IDD = IDD_DEFAULTCOMPARAMS };

protected:

    void OnOK();

    PvGenParameterArray *mGenParameterArray;
    PvGenBrowserWnd mGenBrowserWnd;

    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    DECLARE_MESSAGE_MAP()
};


