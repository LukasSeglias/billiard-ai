// *****************************************************************************
//
// Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// -----------------------------------------------------------------------------
//
// Project view pane. Contains a toolbar used to perform project operations
// (add, remove entity, etc.) and a tree control showing all instantiated
// NetCommand project entities.
//
// The pane is not the final owner of the documents, even though it keeps
// track of references to them.
// 
// *****************************************************************************

#include "stdafx.h"
#include "mainfrm.h"
#include "ProjectPane.h"
#include "Resource.h"
#include "NetCommand.h"
#include "Messages.h"
#include "NetCommandDoc.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#define ID_TREE ( 4 )

#define INDEX_GEV ( 1 )
#define INDEX_FOLDER ( 12 )
#define INDEX_FOLDER_OPEN ( 13 )


BEGIN_MESSAGE_MAP(ProjectPane, CDockablePane)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_CONTEXTMENU()
    ON_WM_PAINT()
    ON_WM_SETFOCUS()
    ON_NOTIFY(TVN_SELCHANGED, ID_TREE, &ProjectPane::OnTvnSelchangedTree)
    ON_NOTIFY(NM_DBLCLK, ID_TREE, &ProjectPane::OnTvnDblclkTree)
    ON_COMMAND(ID_ADDDEVICE, &ProjectPane::OnAddDevice)
    ON_COMMAND(ID_REMOVEDEVICE, &ProjectPane::OnRemoveDevice)
    ON_UPDATE_COMMAND_UI(ID_REMOVEDEVICE, &ProjectPane::OnUpdateRemoveDevice)
END_MESSAGE_MAP()


// ==========================================================================
ProjectPane::ProjectPane()
    : mDevices( 0 )
    , mSoftware( 0 )
    , mPeripherals( 0 )
    , mReceivers( 0 )
    , mTransceivers( 0 )
    , mTransmitters( 0 )
{
}

// ==========================================================================
ProjectPane::~ProjectPane()
{
}

// ==========================================================================
int ProjectPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if ( CDockablePane::OnCreate( lpCreateStruct ) == -1 )
    {
        return -1;
    }

    CRect rectDummy;
    rectDummy.SetRectEmpty();

    // Create tree
    DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_NOHSCROLL | TVS_SHOWSELALWAYS;
    if ( !mFileView.Create( dwViewStyle, rectDummy, this, ID_TREE ) )
    {
        TRACE0("Failed to create project view\n");
        return -1;      // fail to create
    }

    // Load view images:
    mFileViewImages.Create( IDB_PROJECT_ICONS_16, 16, 0, RGB( 255, 0, 255 ) );
    mFileView.SetImageList( &mFileViewImages, TVSIL_NORMAL );

    // Load images:
    mToolBar.Create( this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROJECT );
    mToolBar.LoadToolBar( IDR_PROJECT, 0, 0, TRUE /* Is locked */ );

    OnChangeVisualStyle();

    mToolBar.SetPaneStyle( mToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY );
    mToolBar.SetPaneStyle( mToolBar.GetPaneStyle() & ~( CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT ) );
    mToolBar.SetOwner(this);

    // All commands will be routed via this control , not via the parent frame:
    mToolBar.SetRouteCommandsViaFrame( FALSE );

    // Fill in some static tree view data (dummy code, nothing magic here)
    FillProjectView();
    AdjustLayout();

    return 0;
}

// ==========================================================================
void ProjectPane::OnSize( UINT nType, int cx, int cy )
{
    CDockablePane::OnSize( nType, cx, cy );
    AdjustLayout();
}

// ==========================================================================
void ProjectPane::OnContextMenu( CWnd* pWnd, CPoint point )
{
}

// ==========================================================================
void ProjectPane::AdjustLayout()
{
    if ( GetSafeHwnd() == NULL )
    {
        return;
    }

    CRect rectClient;
    GetClientRect( rectClient );

    int cyTlb = mToolBar.CalcFixedLayout( FALSE, TRUE ).cy;

    mToolBar.SetWindowPos( NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER );
    mFileView.SetWindowPos( NULL, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER );
}

// ==========================================================================
void ProjectPane::OnPaint()
{
    CPaintDC dc( this ); // device context for painting

    CRect rect;
    GetClientRect( rect );

    CRect rectTree;
    mFileView.GetWindowRect( rectTree );
    ScreenToClient( rectTree );

    rectTree.InflateRect( 1, 1 );
    dc.Draw3dRect( rectTree, ::GetSysColor( COLOR_3DSHADOW ), ::GetSysColor( COLOR_3DSHADOW ) );
    dc.FillSolidRect( rect, ::GetSysColor( CTLCOLOR_DLG ) );
}

// ==========================================================================
void ProjectPane::OnSetFocus( CWnd* pOldWnd )
{
    CDockablePane::OnSetFocus( pOldWnd );

    mFileView.SetFocus();
}

// ==========================================================================
void ProjectPane::OnChangeVisualStyle()
{
    mToolBar.CleanUpLockedImages();
    mToolBar.LoadBitmap( IDR_PROJECT, 0, 0, TRUE );

    mFileViewImages.DeleteImageList();

    CBitmap bmp;
    if ( !bmp.LoadBitmap( IDB_PROJECT_ICONS_16 ) )
    {
        TRACE( _T( "Can't load bitmap: %x\n" ), IDB_PROJECT_ICONS_16 );
        ASSERT( FALSE );
        return;
    }

    BITMAP bmpObj;
    bmp.GetBitmap( &bmpObj );

    UINT nFlags = ILC_MASK;

    nFlags |= ILC_COLOR24;

    mFileViewImages.Create( 16, bmpObj.bmHeight, nFlags, 0, 0 );
    mFileViewImages.Add( &bmp, RGB( 255, 0, 255 ) );

    mFileView.SetImageList( &mFileViewImages, TVSIL_NORMAL );
}

// ==========================================================================
void ProjectPane::FillProjectView()
{
    mDevices = mFileView.InsertItem( _T( "GigE Vision Devices" ), INDEX_FOLDER, INDEX_FOLDER_OPEN );
    mFileView.SetItemState( mDevices, TVIS_BOLD, TVIS_BOLD );

    mTransmitters = mFileView.InsertItem( _T( "Transmitters" ), INDEX_FOLDER, INDEX_FOLDER_OPEN, mDevices );
    mFileView.SetItemState( mTransmitters, TVIS_BOLD, TVIS_BOLD );

    mTransceivers = mFileView.InsertItem( _T( "Transceivers" ), INDEX_FOLDER, INDEX_FOLDER_OPEN, mDevices );
    mFileView.SetItemState( mTransceivers, TVIS_BOLD, TVIS_BOLD );

    mReceivers = mFileView.InsertItem( _T("Receivers" ), INDEX_FOLDER, INDEX_FOLDER_OPEN, mDevices );
    mFileView.SetItemState( mReceivers, TVIS_BOLD, TVIS_BOLD );

    mPeripherals = mFileView.InsertItem( _T( "Peripherals" ), INDEX_FOLDER, INDEX_FOLDER_OPEN, mDevices );
    mFileView.SetItemState( mPeripherals, TVIS_BOLD, TVIS_BOLD );

    mSoftware = mFileView.InsertItem( _T( "GigE Vision Software Receivers" ), INDEX_FOLDER, INDEX_FOLDER_OPEN );
    mFileView.SetItemState( mSoftware, TVIS_BOLD, TVIS_BOLD );

    mFileView.Expand( mDevices, TVE_EXPAND );
    mFileView.Expand( mSoftware, TVE_EXPAND );
}

// ==========================================================================
void ProjectPane::Update( std::vector<NetCommandDoc *> *aDocuments )
{
    bool lTransmitters = false;
    bool lTransceivers = false;
    bool lReceivers = false;
    bool lPeripherals = false;
    bool lSoftwareReceivers = false;

    DeleteChildren( mTransmitters );
    DeleteChildren( mTransceivers );
    DeleteChildren( mReceivers );
    DeleteChildren( mPeripherals );
    DeleteChildren( mSoftware );
    mDocumentMap.clear();

    HTREEITEM lItem = 0;

    std::vector<NetCommandDoc *>::iterator lIt = aDocuments->begin();
    while ( lIt != aDocuments->end() )
    {
        switch ( ( *lIt )->GetRole() )
        {
        case NetCommandDoc::RoleDeviceTransmitter:
            lTransmitters = true;
            lItem = mFileView.InsertItem( ( *lIt )->GetDescription(), INDEX_GEV, INDEX_GEV, mTransmitters );
            mDocumentMap[ *lIt ] = lItem;
            mFileView.SetItemData( lItem, reinterpret_cast<DWORD_PTR>( *lIt ) );
            break;

        case NetCommandDoc::RoleDeviceReceiver:
            lReceivers = true;
            lItem = mFileView.InsertItem( ( *lIt )->GetDescription(), INDEX_GEV, INDEX_GEV, mReceivers );
            mDocumentMap[ *lIt ] = lItem;
            mFileView.SetItemData( lItem, reinterpret_cast<DWORD_PTR>( *lIt ) );
            break;

        case NetCommandDoc::RoleDeviceTransceiver:
            lTransceivers = true;
            lItem = mFileView.InsertItem( ( *lIt )->GetDescription(), INDEX_GEV, INDEX_GEV, mTransceivers );
            mDocumentMap[ *lIt ] = lItem;
            mFileView.SetItemData( lItem, reinterpret_cast<DWORD_PTR>( *lIt ) );
            break;

        case NetCommandDoc::RoleDevicePeripheral:
            lPeripherals = true;
            lItem = mFileView.InsertItem( ( *lIt )->GetDescription(), INDEX_GEV, INDEX_GEV, mPeripherals );
            mDocumentMap[ *lIt ] = lItem;
            mFileView.SetItemData( lItem, reinterpret_cast<DWORD_PTR>( *lIt ) );
            break;

        case NetCommandDoc::RoleSoftwareReceiver:
            lSoftwareReceivers = true;
            lItem = mFileView.InsertItem( ( *lIt )->GetDescription(), INDEX_GEV, INDEX_GEV, mSoftware );
            mDocumentMap[ *lIt ] = lItem;
            mFileView.SetItemData( lItem, reinterpret_cast<DWORD_PTR>( *lIt ) );
            break;

        default:
            ASSERT( 0 );
        }

        lIt++;
    }

    if ( lTransmitters ) mFileView.Expand( mTransmitters, TVE_EXPAND );
    if ( lTransceivers ) mFileView.Expand( mTransceivers, TVE_EXPAND );
    if ( lReceivers ) mFileView.Expand( mReceivers, TVE_EXPAND );
    if ( lPeripherals ) mFileView.Expand( mPeripherals, TVE_EXPAND );
    if ( lSoftwareReceivers ) mFileView.Expand( mSoftware, TVE_EXPAND );
}

// ==========================================================================
void ProjectPane::DeleteChildren( HTREEITEM aNode )
{
    while ( mFileView.ItemHasChildren( aNode ) )
    {
        HTREEITEM lChildItem = mFileView.GetChildItem( aNode );
        mFileView.DeleteItem( lChildItem );
    }
}

// ==========================================================================
void ProjectPane::OnTvnSelchangedTree( NMHDR *pNMHDR, LRESULT *pResult )
{
    HTREEITEM lSelection = mFileView.GetSelectedItem();
    if ( lSelection == 0 )
    {
        return;
    }

    DWORD_PTR lData = mFileView.GetItemData( lSelection );
    if ( lData != 0 )
    {
        NetCommandDoc *lDoc = reinterpret_cast<NetCommandDoc *>( lData );
        if ( lDoc != NULL )
        {
            POSITION lPos = lDoc->GetFirstViewPosition();
            while ( lPos != NULL )
            {
                CView *lView = lDoc->GetNextView( lPos );
                lView->GetParent()->SetWindowPos( &wndTop, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE );
            }   
        }

        ::AfxGetMainWnd()->SendMessage( WM_ACTIVEVIEWCHANGED, lData, 1 );
    }
}

// ==========================================================================
void ProjectPane::OnTvnDblclkTree( NMHDR *pNMHDR, LRESULT *pResult )
{
    HTREEITEM lSelection = mFileView.GetSelectedItem();
    if ( lSelection == 0 )
    {
        return;
    }

    DWORD_PTR lData = mFileView.GetItemData( lSelection );
    if ( lData != 0 )
    {
        ::AfxGetMainWnd()->SendMessage( WM_DOCUMENTOPENVIEW, lData );
    }
}

// ==========================================================================
void ProjectPane::SelectDocument( NetCommandDoc *aDocument )
{
    std::map<NetCommandDoc *, HTREEITEM>::const_iterator lIt = mDocumentMap.find( aDocument );
    if ( lIt != mDocumentMap.end() )
    {
        HTREEITEM lSelection = mFileView.GetSelectedItem();
        if ( lIt->second != lSelection )
        {
            mFileView.Select( lIt->second, TVGN_CARET );
        }
    }
}

// ==========================================================================
void ProjectPane::OnAddDevice()
{
    ::AfxGetMainWnd()->SendMessage( WM_NEWDEVICE );
}

// ==========================================================================
void ProjectPane::OnRemoveDevice()
{
    bool lEnabled = false;
    HTREEITEM lSelection = mFileView.GetSelectedItem();
    if ( lSelection != 0 )
    {
        DWORD_PTR lData = mFileView.GetItemData( lSelection );
        ::AfxGetMainWnd()->SendMessage( WM_CLOSEDEVICE, lData );
    }
}

// ==========================================================================
void ProjectPane::OnUpdateRemoveDevice( CCmdUI *pCmdUI )
{
    bool lEnabled = false;
    HTREEITEM lSelection = mFileView.GetSelectedItem();
    if ( lSelection != 0 )
    {
        DWORD_PTR lData = mFileView.GetItemData( lSelection );
        lEnabled = lData != 0;
    }

    pCmdUI->Enable( lEnabled );
}

