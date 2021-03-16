// *****************************************************************************
//
//     Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include <PvGenBrowserWnd.h>
#include <PvDevice.h>

class NetCommandDoc;


class CPropertiesWnd : public CDockablePane
{
public:

    CPropertiesWnd();
    virtual ~CPropertiesWnd();

    void AdjustLayout();

    const NetCommandDoc *GetDocument() const { return mDocument; }
    void SetDocument( NetCommandDoc *aDocument );

protected:

    void FillComboBox();
    void ConfigureBrowser();

    NetCommandDoc *mDocument;

    CComboBox mObjectCombo;
    PvGenBrowserWnd mGenBrowserWnd;

    afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
    afx_msg void OnSize( UINT nType, int cx, int cy );
    afx_msg void OnPaint();
    afx_msg void OnSetFocus( CWnd* pOldWnd );
    afx_msg void OnObjectComboSelChanged();
    afx_msg void OnShowWindow( BOOL bShow, UINT nStatus );
    DECLARE_MESSAGE_MAP()

    PvGenParameterArray *mCurrentArray;

};


