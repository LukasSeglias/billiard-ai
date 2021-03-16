// *****************************************************************************
//
//     Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "BitmapButton.h"


class AttributesPane : public CDockablePane
{
public:

    AttributesPane();
    virtual ~AttributesPane();

    void AdjustLayout();
    void OnChangeVisualStyle();

    const NetCommandDoc *GetDocument() const { return mDocument; }
    void SetDocument( NetCommandDoc *aDoc );

protected:

    void FillAttributesView();

    CListCtrl mListCtrl;
    NetCommandDoc *mDocument;

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnPaint();
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    DECLARE_MESSAGE_MAP()
};

