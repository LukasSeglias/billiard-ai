// *****************************************************************************
//
// Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// -----------------------------------------------------------------------------
//
// MDI interface view for our document. Owns the display and status bar. 
// Receives notifications from the document on when to update itself.
// 
// *****************************************************************************

#include "stdafx.h"
#include "NetCommand.h"

#include "NetCommandDoc.h"
#include "NetCommandView.h"
#include "Messages.h"

#include <vector>

#include <PvStreamInfo.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(NetCommandView, CView)

BEGIN_MESSAGE_MAP(NetCommandView, CView)
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_SIZE()
    ON_WM_TIMER()
END_MESSAGE_MAP()


// ==========================================================================
NetCommandView::NetCommandView()
    : mTimer( 0 )
{
    // TODO: add construction code here

}

// ==========================================================================
NetCommandView::~NetCommandView()
{
}

// ==========================================================================
BOOL NetCommandView::PreCreateWindow(CREATESTRUCT& cs)
{
    return CView::PreCreateWindow(cs);
}

// ==========================================================================
int NetCommandView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CView::OnCreate(lpCreateStruct) == -1)
        return -1;

    if ( !mDisplayWnd.Create( GetSafeHwnd(), 2 ) )
    {
        TRACE0("Failed to create display\n");
        return -1;      // fail to create
    }

    if ( !mStatusBar.Create( WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY, CRect( 0, 0, 0, 0 ), this, 3 ) )
    {
        TRACE0("Status bar\n");
    }
    mStatusBar.SetFont(CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT)));

    AdjustLayout();

    mTimer = SetTimer( 1, 250, NULL );

    return 0;
}

// ==========================================================================
void NetCommandView::OnDestroy()
{
    NetCommandDoc* pDoc = GetDocument();
    if ( pDoc != NULL )
    {
        ASSERT( !pDoc->m_bAutoDelete );
        pDoc->SetDisplay( NULL );
    }

    CView::OnDestroy();

    if ( mTimer != 0 )
    {
        KillTimer( 1 );
        mTimer = 0;
    }
}

// ==========================================================================
void NetCommandView::OnDraw(CDC* /*pDC*/)
{
    NetCommandDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if ( !pDoc )
    {
        return;
    }
}

// ==========================================================================
void NetCommandView::OnInitialUpdate()
{
    NetCommandDoc* pDoc = GetDocument();
    if ( pDoc != NULL )
    {
        pDoc->SetDisplay( &mDisplayWnd );
    }
}

// ==========================================================================
void NetCommandView::OnRButtonUp(UINT nFlags, CPoint point)
{
    ClientToScreen(&point);
    OnContextMenu(this, point);
}

// ==========================================================================
void NetCommandView::OnContextMenu(CWnd* pWnd, CPoint point)
{
}

// ==========================================================================
void NetCommandView::AdjustLayout()
{
    if (GetSafeHwnd() == NULL)
    {
        return;
    }

    CRect rectClient,rectCombo;
    GetClientRect(rectClient);

    mDisplayWnd.SetPosition( rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height() - 42 );
    mStatusBar.SetWindowPos( NULL, rectClient.left, rectClient.Height() - 42, rectClient.Width(), 42, SWP_NOACTIVATE | SWP_NOZORDER );
}

// ==========================================================================
void NetCommandView::OnSize(UINT nType, int cx, int cy)
{
    CView::OnSize(nType, cx, cy);
    AdjustLayout();
}

// ==========================================================================
void NetCommandView::OnActivateView( BOOL bActivate, CView *pActivateView, CView *pDeactiveView )
{
    ::AfxGetMainWnd()->SendMessage( WM_ACTIVEVIEWCHANGED, reinterpret_cast<WPARAM>( GetDocument() ), bActivate );
}

// ==========================================================================
void NetCommandView::OnActivateFrame( UINT nState, CFrameWnd *pFrameWnd )
{
}

// ==========================================================================
void NetCommandView::UpdateStatusBar()
{
    // Stream opened, image save dlg exists, thread display is up
    if ( !GetDocument()->GetStream()->IsOpen() )
    {
        return;
    }
    
    PvStreamInfo lInfo( GetDocument()->GetStream() );
    CString lStatistics = lInfo.GetStatistics( GetDocument()->GetDisplayFrameRate() ).GetUnicode();
    CString lErrors = lInfo.GetErrors().GetUnicode();
    CString lWarnings = lInfo.GetWarnings( false ).GetUnicode();

    CString lStr = lStatistics + _T( "\r\n" ) + lErrors + _T( "\r\n" ) + lWarnings;
    
    CString lCurrent;
    mStatusBar.GetWindowText( lCurrent );

    if ( lCurrent != lStr )
    {
        mStatusBar.SetWindowText( lStr );
    }
}

// ==========================================================================
void NetCommandView::OnTimer( UINT_PTR nIDEvent )
{
    if ( nIDEvent == 1 )
    {
        UpdateStatusBar();
    }

    CView::OnTimer( nIDEvent );
}

