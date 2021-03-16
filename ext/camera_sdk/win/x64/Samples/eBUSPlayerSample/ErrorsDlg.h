// *****************************************************************************
//
//     Copyright (c) 2011, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "resource.h"

class PvStringList;


class ErrorsDlg : public CDialog
{
    DECLARE_DYNAMIC( ErrorsDlg )

public:

    ErrorsDlg( CWnd* pParent = NULL );   // standard constructor
    virtual ~ErrorsDlg();

    enum { IDD = IDD_ERRORS };

    INT_PTR DoModal( PvStringList *aErrors );

protected:

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    BOOL OnInitDialog();
    DECLARE_MESSAGE_MAP()

    CEdit mTextBox;

private:

    PvStringList *mErrors;

};
