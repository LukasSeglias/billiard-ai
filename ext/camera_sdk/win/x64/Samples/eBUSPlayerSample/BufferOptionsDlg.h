// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include <BufferOptions.h>

#include "Resource.h"


class eBUSPlayerDlg;


class BufferOptionsDlg : public CDialog
{
    DECLARE_DYNAMIC(BufferOptionsDlg)

public:

    BufferOptionsDlg( Player *aPlayer, CWnd* pParent = NULL );
    virtual ~BufferOptionsDlg();

    enum { IDD = IDD_BUFFER_OPTIONS };

protected:

    static bool StrToUInt32( const CString &aString, uint32_t &aValue );
    static bool EditToUInt32( CEdit &aEdit, uint32_t &aValue );

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();

    afx_msg void OnBnClickedOk();
    DECLARE_MESSAGE_MAP()

    Player *mPlayer;

    CEdit mBufferCountEdit;
    CEdit mBufferSizeEdit;
    CButton mAutoResizeCheck;
};



