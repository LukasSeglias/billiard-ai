// *****************************************************************************
//
// Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// -----------------------------------------------------------------------------
//
// Pane containing a GenICam tree browser used to browser the properties of
// the selected project entity. The is just one GenICam tree browser. Changing
// current selection or selecting a different node map with the combo box just
// causes a different node map to be assigned to the same GenICam tree browser.
//
// If not entity is selected, the GenICam tree browser is hidden.
// 
// *****************************************************************************

#include "stdafx.h"

#include "PropertiesWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "NetCommandDoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define DATA_COMMLINKPARAMS ( 100 )
#define DATA_DEVICEPARAMS ( 101 )
#define DATA_STREAMPARAMS ( 102 )

#define ID_COMBO ( 1 )
#define ID_TREE ( 2 )


BEGIN_MESSAGE_MAP(CPropertiesWnd, CDockablePane)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_PAINT()
    ON_WM_SETFOCUS()
    ON_CBN_SELCHANGE(ID_COMBO, &CPropertiesWnd::OnObjectComboSelChanged)
    ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// ==========================================================================
CPropertiesWnd::CPropertiesWnd()
    : mDocument( NULL )
    , mCurrentArray( NULL )
{
}

// ==========================================================================
CPropertiesWnd::~CPropertiesWnd()
{
}

// ==========================================================================
void CPropertiesWnd::AdjustLayout()
{
    if (GetSafeHwnd() == NULL)
    {
        return;
    }

    CRect rectClient,rectCombo;

    GetClientRect(rectClient);
    mObjectCombo.GetWindowRect(&rectCombo);

    int cyCmb = rectCombo.Size().cy;

    mObjectCombo.SetWindowPos( NULL, rectClient.left + 1, rectClient.top + 1, rectClient.Width(), cyCmb, SWP_NOACTIVATE | SWP_NOZORDER );
    mGenBrowserWnd.SetPosition( rectClient.left + 1, rectClient.top + cyCmb + 1, rectClient.Width() - 2, rectClient.Height() - cyCmb - 2 );
}

// ==========================================================================
int CPropertiesWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if ( CDockablePane::OnCreate( lpCreateStruct ) == -1 )
    {
        return -1;
    }

    CRect rectDummy;
    rectDummy.SetRectEmpty();

    // Create combo:
    DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER | CBS_SORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    if ( !mObjectCombo.Create( dwViewStyle, rectDummy, this, ID_COMBO ) )
    {
        TRACE0("Failed to create Properties Combo \n");
        return -1;      // fail to create
    }
    mObjectCombo.SetFont( CFont::FromHandle( (HFONT)GetStockObject( DEFAULT_GUI_FONT ) ) );
    mObjectCombo.SetCurSel( 0 );

    // Create tree browser
    if ( !mGenBrowserWnd.Create( GetSafeHwnd(), ID_TREE ) )
    {
        TRACE0("Failed to create Properties Grid \n");
        return -1;      // fail to create
    }

    FillComboBox();
    ConfigureBrowser();

    AdjustLayout();

    return 0;
}

// ==========================================================================
void CPropertiesWnd::OnSize(UINT nType, int cx, int cy)
{
    CDockablePane::OnSize(nType, cx, cy);
    AdjustLayout();
}

// ==========================================================================
void CPropertiesWnd::OnPaint()
{
    CPaintDC dc( this ); // device context for painting

    CRect rect;
    GetClientRect( rect );

    CRect rectTree;
    ScreenToClient( rectTree );

    rectTree.InflateRect( 1, 1 );
    dc.Draw3dRect( rectTree, ::GetSysColor( COLOR_3DSHADOW ), ::GetSysColor( COLOR_3DSHADOW ) );
    dc.FillSolidRect( rect, ::GetSysColor( CTLCOLOR_DLG ) );
}

// ==========================================================================
void CPropertiesWnd::OnSetFocus( CWnd* pOldWnd )
{
    CDockablePane::OnSetFocus( pOldWnd );
}

// ==========================================================================
void CPropertiesWnd::SetDocument( NetCommandDoc *aDocument )
{
    if ( aDocument != mDocument )
    {
        mDocument = aDocument;
        
        FillComboBox();
        ConfigureBrowser();
    }
}

// ==========================================================================
void CPropertiesWnd::OnObjectComboSelChanged()
{
    ConfigureBrowser();
}

// ==========================================================================
void CPropertiesWnd::FillComboBox()
{
    // Save selection
    int lSelection = mObjectCombo.GetCurSel();
    CString lOldSelection;
    if ( lSelection != CB_ERR )
    {
        mObjectCombo.GetLBText( lSelection, lOldSelection );
    }

    mObjectCombo.ResetContent();
    if ( mDocument == NULL )
    {
    }
    else
    {
        if ( mDocument->GetDevice()->IsConnected() )
        {
            int lIndex = mObjectCombo.AddString( _T( "Communication" ) );
            mObjectCombo.SetItemData( lIndex, DATA_COMMLINKPARAMS );

            lIndex = mObjectCombo.AddString( _T( "GEVDevice" ) );
            mObjectCombo.SetItemData( lIndex, DATA_DEVICEPARAMS );
        }
        
        if ( mDocument->GetStream()->IsOpen() )
        {
            int lIndex = mObjectCombo.AddString( _T( "Image stream" ) );
            mObjectCombo.SetItemData( lIndex, DATA_STREAMPARAMS );
        }
    }

    // Try finding the previous selection
    for ( int i = 0; i < mObjectCombo.GetCount(); i++ )
    {
        CString lText;
        mObjectCombo.GetLBText( i, lText );
        if ( lOldSelection == lText )
        {
            mObjectCombo.SetCurSel( i );
            return;
        }
    }

    // No luck, select item 0
    if ( mObjectCombo.GetCount() > 0 )
    {
        mObjectCombo.SetCurSel( 0 );
    }
}

// ==========================================================================
void CPropertiesWnd::ConfigureBrowser()
{
    int lSel = mObjectCombo.GetCurSel();
    if ( ( mDocument == NULL ) || 
         ( lSel == CB_ERR ) )
    {
        mGenBrowserWnd.SetGenParameterArray( NULL );

        CWnd *lWnd = CWnd::FromHandle( mGenBrowserWnd.GetHandle() );
        lWnd->ShowWindow( SW_HIDE );
    }
    else
    {
        CWnd *lWnd = CWnd::FromHandle( mGenBrowserWnd.GetHandle() );
        lWnd->ShowWindow( SW_SHOW );

        static int lIndex = 0;
        DWORD_PTR lData = mObjectCombo.GetItemData( lSel );

        switch ( lData )
        {
        case DATA_COMMLINKPARAMS:
            if ( mCurrentArray != mDocument->GetDevice()->GetCommunicationParameters() )
            {
                mCurrentArray = mDocument->GetDevice()->GetCommunicationParameters();
                mGenBrowserWnd.SetGenParameterArray( mCurrentArray );
            }
            break;

        case DATA_DEVICEPARAMS:
            if ( mCurrentArray != mDocument->GetDevice()->GetParameters() )
            {
                mCurrentArray = mDocument->GetDevice()->GetParameters();
                mGenBrowserWnd.SetGenParameterArray( mCurrentArray );
            }
            break;

        case DATA_STREAMPARAMS:
            if ( mCurrentArray != mDocument->GetStream()->GetParameters() )
            {
                mCurrentArray = mDocument->GetStream()->GetParameters();
                mGenBrowserWnd.SetGenParameterArray( mCurrentArray );
            }
            break;

        default:
            ASSERT( 0 );
        }
    }
}

// ==========================================================================
void CPropertiesWnd::OnShowWindow( BOOL bShow, UINT nStatus )
{
}



