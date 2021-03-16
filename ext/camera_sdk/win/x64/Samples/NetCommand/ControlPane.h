// *****************************************************************************
//
//     Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "BitmapButton.h"
#include "NetCommandDoc.h"


class ControlPane : public CDockablePane, PvGenEventSink
{
public:

    ControlPane();
    virtual ~ControlPane();

    void AdjustLayout();
    void OnChangeVisualStyle();

    const NetCommandDoc *GetDocument() const { return mDocument; }
    void SetDocument( NetCommandDoc *aDoc );

protected:

    afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
    afx_msg void OnSize( UINT nType, int cx, int cy );
    afx_msg void OnPaint();
    afx_msg void OnSetFocus( CWnd *pOldWnd );
    afx_msg void OnUpdatePlayButton( CCmdUI *pCmdU );
    afx_msg void OnPlayButton();
    afx_msg void OnUpdateStopButton( CCmdUI *pCmdU );
    afx_msg void OnStopButton();
    afx_msg void OnCbnSelchangeMode();
    DECLARE_MESSAGE_MAP()

    void OnParameterUpdate( PvGenParameter *aParameter );

    void EnableInterface();
    void FillComboBox();

    CComboBox mObjectCombo;
    BitmapButton mPlayButton;
    BitmapButton mStopButton;

    NetCommandDoc *mDocument;
};

