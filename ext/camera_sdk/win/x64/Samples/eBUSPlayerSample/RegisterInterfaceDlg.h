// *****************************************************************************
//
//     Copyright (c) 2009, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "resource.h"

#include <PvDeviceGEV.h>


class RegisterInterfaceDlg : public CDialog
{
public:

    RegisterInterfaceDlg( CWnd* pParent = NULL );
    virtual ~RegisterInterfaceDlg();

    void SetDevice( PvDeviceGEV *aDevice ) { mDevice = aDevice; }

protected:

    virtual void DoDataExchange(CDataExchange* pDX);
    BOOL OnInitDialog();
    void OnOK();
    void OnCancel();
    afx_msg void OnReadButtonClicked();
    afx_msg void OnWriteButtonClicked();
    DECLARE_MESSAGE_MAP()

    void EnableInterface();

    bool ReadValue( CEdit &aEdit, uint32_t &aValue );
    void WriteValue( uint32_t aValue, CEdit &aEdit );

private:

    PvDeviceGEV *mDevice;

    CEdit mReadAddressEdit;
    CEdit mReadValueEdit;
    CEdit mReadResultEdit;
    CEdit mWriteAddressEdit;
    CEdit mWriteValueEdit;
    CEdit mWriteResultEdit;
};


