// *****************************************************************************
//
//     Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include <PvDisplayWnd.h>


class NetCommandView : public CView
{
    DECLARE_DYNCREATE( NetCommandView )

protected: // create from serialization only

    NetCommandView();
    virtual ~NetCommandView();

public:

    NetCommandDoc* GetDocument() const;

    void AdjustLayout();

protected:

    virtual void OnInitialUpdate();
    virtual void OnActivateView( BOOL bActivate, CView *pActivateView, CView *pDeactiveView );
    virtual void OnActivateFrame( UINT nState, CFrameWnd *pFrameWnd );
    virtual void OnDraw( CDC* pDC );  // overridden to draw this view
    virtual BOOL PreCreateWindow( CREATESTRUCT& cs );

    void UpdateStatusBar();
    CString GetErrorsAndWarnings();

    PvDisplayWnd mDisplayWnd;
    CEdit mStatusBar;

    UINT_PTR mTimer;

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnTimer( UINT_PTR nIDEvent );
    DECLARE_MESSAGE_MAP()
};


inline NetCommandDoc* NetCommandView::GetDocument() const
{ 
    return reinterpret_cast<NetCommandDoc*>( m_pDocument ); 
}

