// *****************************************************************************
//
// Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// -----------------------------------------------------------------------------
//
// Grid dockable pane used to display attributes for the current project
// selection.
// 
// *****************************************************************************


#include "stdafx.h"
#include "mainfrm.h"
#include "AttributesPane.h"
#include "Resource.h"
#include "NetCommand.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


BEGIN_MESSAGE_MAP( AttributesPane, CDockablePane )
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_PAINT()
END_MESSAGE_MAP()


// ==========================================================================
AttributesPane::AttributesPane()
{
}

// ==========================================================================
AttributesPane::~AttributesPane()
{
}

// ==========================================================================
int AttributesPane::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    if ( CDockablePane::OnCreate( lpCreateStruct ) == -1 )
    {
        return -1;
    }

    CRect rectDummy;
    rectDummy.SetRectEmpty();

    // Create attributes
    DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | LVS_REPORT;
    if ( !mListCtrl.Create( dwViewStyle, rectDummy, this, 5 ) )
    {
        TRACE0("Failed to create attributes list\n");
        return -1;      // fail to create
    }
    mListCtrl.InsertColumn( 0, _T( "Parameter" ) );
    mListCtrl.InsertColumn( 1, _T( "Value" ) );
    mListCtrl.SetColumnWidth( 0, 100 );
    mListCtrl.SetExtendedStyle( mListCtrl.GetExtendedStyle() | 
        LVS_EX_HEADERDRAGDROP | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT );

    OnChangeVisualStyle();

    AdjustLayout();

    return 0;
}

// ==========================================================================
void AttributesPane::OnSize( UINT nType, int cx, int cy )
{
    CDockablePane::OnSize(nType, cx, cy);
    AdjustLayout();
}

// ==========================================================================
void AttributesPane::AdjustLayout()
{
    if ( GetSafeHwnd() == NULL )
    {
        return;
    }

    CRect rectClient;
    GetClientRect( rectClient );

    mListCtrl.SetWindowPos( NULL, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER );
    mListCtrl.SetColumnWidth( 1, rectClient.Width() - mListCtrl.GetColumnWidth( 0 ) );
}

// ==========================================================================
void AttributesPane::OnPaint()
{
    CPaintDC dc( this ); // device context for painting
}

// ==========================================================================
void AttributesPane::OnChangeVisualStyle()
{
}

// ==========================================================================
void AttributesPane::SetDocument( NetCommandDoc *aDoc )
{
    mDocument = aDoc;
    FillAttributesView();
}

// ==========================================================================
void AttributesPane::FillAttributesView()
{
    mListCtrl.DeleteAllItems();
    if ( mDocument != NULL )
    {
        mListCtrl.InsertItem( 0, _T( "IP address" ) );
        mListCtrl.SetItemText( 0, 1, mDocument->GetIPAddress() );
        mListCtrl.InsertItem( 1, _T( "MAC address" ) );
        mListCtrl.SetItemText( 1, 1, mDocument->GetMACAddress() );
        mListCtrl.InsertItem( 2, _T( "Manufacturer" ) );
        mListCtrl.SetItemText( 2, 1, mDocument->GetManufacturer() );
        mListCtrl.InsertItem( 3, _T( "Model" ) );
        mListCtrl.SetItemText( 3, 1, mDocument->GetModel() );
        mListCtrl.InsertItem( 4, _T( "Name" ) );
        mListCtrl.SetItemText( 4, 1, mDocument->GetName() );
    }
}


