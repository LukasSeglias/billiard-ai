// *****************************************************************************
//
// Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// -----------------------------------------------------------------------------
//
// The core of the MDI application. The main frame owns the main menu, all 
// panes, toolbars and the frames containing the views attached to the 
// documents.
// 
// *****************************************************************************

#include "stdafx.h"
#include "NetCommand.h"

#include "MainFrm.h"
#include "Messages.h"

#include "ChildFrm.h"
#include "NetCommandView.h"
#include "DefaultComParamsDlg.h"

#include <PvDeviceGEV.h>


#define DEFAULT_PROJECT_WIDTH ( 375 )
#define DEFAULT_PROJECT_HEIGHT ( 350 )

#define DEFAULT_CONTROL_WIDTH ( 375 )
#define DEFAULT_CONTROL_HEIGHT ( 120 )

#define DEFAULT_ATTRIBUTES_WIDTH ( 375 )
#define DEFAULT_ATTRIBUTES_HEIGHT ( 200 )

#define DEFAULT_PROPERTIES_WIDTH ( 350 )
#define DEFAULT_PROPERTIES_HEIGHT ( 200 )


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Default PvDevice. Never connected, only used to hold default communication parameters
PvDeviceGEV sDefaultDevice;


IMPLEMENT_DYNAMIC(MainFrame, CMDIFrameWndEx)

const int  iMaxUserToolbars = 10;
const UINT uiFirstUserToolBarId = AFX_IDW_CONTROLBAR_FIRST + 40;
const UINT uiLastUserToolBarId = uiFirstUserToolBarId + iMaxUserToolbars - 1;

BEGIN_MESSAGE_MAP(MainFrame, CMDIFrameWndEx)
    ON_WM_CREATE()
    ON_COMMAND(ID_VIEW_CUSTOMIZE, &MainFrame::OnViewCustomize)
    ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &MainFrame::OnToolbarCreateNew)
    ON_MESSAGE(WM_NEWDEVICE, &MainFrame::OnNewDevice)
    ON_MESSAGE(WM_CLOSEDEVICE, &MainFrame::OnRemoveDevice)
    ON_COMMAND(ID_VIEW_PROJECT, &MainFrame::OnViewProject)
    ON_COMMAND(ID_VIEW_ACQUISITIONCONTROLS, &MainFrame::OnViewControls)
    ON_COMMAND(ID_VIEW_ATTRIBUTES, &MainFrame::OnViewAttributes)
    ON_COMMAND(ID_VIEW_GENICAMBROWSER, &MainFrame::OnViewProperties)
    ON_MESSAGE(WM_ACTIVEVIEWCHANGED, &MainFrame::OnActiveViewChanged)
    ON_MESSAGE(WM_NEWDOCUMENT, &MainFrame::OnNewDocument)
    ON_MESSAGE(WM_DOCUMENTCLOSING, &MainFrame::OnDocumentClosing)
    ON_MESSAGE(WM_DOCUMENTOPENVIEW, &MainFrame::OnDocumentOpenView)
    ON_MESSAGE(WM_LINKDISCONNECTED, &MainFrame::OnLinkDisconnected)
    ON_COMMAND(ID_TOOLS_DEFAULTCOMMUNICATIONPARAMETERS, &MainFrame::OnDefaultCommunicationParameters)
    ON_WM_CLOSE()
END_MESSAGE_MAP()


// ==========================================================================
MainFrame::MainFrame()
{
    // TODO: add member initialization code here
    theApp.mnAppLook = theApp.GetInt( _T ("ApplicationLook" ), ID_VIEW_APPLOOK_VS_2005 );
}

// ==========================================================================
MainFrame::~MainFrame()
{
}

// ==========================================================================
int MainFrame::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    if ( CMDIFrameWndEx::OnCreate( lpCreateStruct ) == -1 )
    {
        return -1;
    }

    CMFCVisualManager::SetDefaultManager( RUNTIME_CLASS( CMFCVisualManagerVS2005 ) );
    CDockingManager::SetDockingMode( DT_SMART );

    if ( !mMenuBar.Create( this) )
    {
        TRACE0( "Failed to create menubar\n" );
        return -1; // fail to create
    }

    mMenuBar.SetPaneStyle( mMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY );

    // prevent the menu bar from taking the focus on activation
    CMFCPopupMenu::SetForceMenuFocus( FALSE );

    // Allow user-defined toolbars operations:
    InitUserToolbars( NULL, uiFirstUserToolBarId, uiLastUserToolBarId );

    if ( !mStatusBar.Create( this ) )
    {
        TRACE0( "Failed to create status bar\n" );
        return -1;      // fail to create
    }

    mMenuBar.EnableDocking( CBRS_ALIGN_TOP );
    EnableDocking( CBRS_ALIGN_TOP );
    DockPane( &mMenuBar );

    CDockingManager::SetDockingMode( DT_SMART );
    EnableAutoHidePanes( CBRS_ALIGN_ANY );

    // create docking windows
    if ( !CreateDockingWindows() )
    {
        TRACE0("Failed to create docking windows\n");
        return -1;
    }

    mAttributesPane.EnableDocking( CBRS_ALIGN_ANY );
    DockPane( &mAttributesPane );

    mControlPane.EnableDocking( CBRS_ALIGN_ANY );
    mControlPane.DockToWindow( &mAttributesPane, CBRS_TOP );

    mProjectPane.EnableDocking( CBRS_ALIGN_ANY );
    mProjectPane.DockToWindow( &mControlPane, CBRS_TOP );

    HDWP lHdwp; // Unused
    CRect lRect;

    mProjectPane.GetWindowRect( &lRect );
    lRect.bottom = lRect.top + DEFAULT_PROJECT_HEIGHT;
    mProjectPane.MovePane( lRect, FALSE, lHdwp );

    lRect.bottom = lRect.bottom + DEFAULT_CONTROL_HEIGHT;
    lRect.top = lRect.bottom - DEFAULT_CONTROL_HEIGHT;
    mControlPane.MovePane( lRect, FALSE, lHdwp );

    mProperties.EnableDocking( CBRS_ALIGN_ANY );
    DockPane( &mProperties );

    return 0;
}

// ==========================================================================
BOOL MainFrame::PreCreateWindow( CREATESTRUCT& cs )
{
    if( !CMDIFrameWndEx::PreCreateWindow( cs ) )
    {
        return FALSE;
    }

    return TRUE;
}

// ==========================================================================
BOOL MainFrame::CreateDockingWindows()
{
    BOOL bNameValid;

    // Create project view
    CString strProjectView;
    bNameValid = strProjectView.LoadString( IDS_PROJECT_VIEW );
    ASSERT( bNameValid );
    if ( !mProjectPane.Create( strProjectView, this, CRect( 0, 0, DEFAULT_PROJECT_WIDTH, DEFAULT_PROJECT_HEIGHT ), TRUE, 
        ID_VIEW_PROJECTVIEW, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT| CBRS_FLOAT_MULTI ) )
    {
        TRACE0( "Failed to create Project View window\n" );
        return FALSE; // failed to create
    }

    // Create control view
    CString strControlView;
    bNameValid = strControlView.LoadString( IDS_CONTROL_VIEW );
    ASSERT( bNameValid );
    if ( !mControlPane.Create( strControlView, this, CRect( 0, 0, DEFAULT_CONTROL_WIDTH, DEFAULT_CONTROL_HEIGHT ), TRUE, 
        ID_VIEW_CONTROLVIEW, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT| CBRS_FLOAT_MULTI ) )
    {
        TRACE0( "Failed to create Control View window\n" );
        return FALSE; // failed to create
    }

    // Create attributes view
    CString strAttributesView;
    bNameValid = strAttributesView.LoadString( IDS_ATTRIBUTES_VIEW );
    ASSERT( bNameValid );
    if (!mAttributesPane.Create( strAttributesView, this, CRect( 0, 0, DEFAULT_ATTRIBUTES_WIDTH, DEFAULT_ATTRIBUTES_HEIGHT ), TRUE, 
        ID_VIEW_ATTRIBUTESVIEW, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT| CBRS_FLOAT_MULTI ) )
    {
        TRACE0( "Failed to create Attributes View window\n" );
        return FALSE; // failed to create
    }

    // Create properties window
    CString strPropertiesWnd;
    bNameValid = strPropertiesWnd.LoadString( IDS_PROPERTIES_WND );
    ASSERT( bNameValid );
    if ( !mProperties.Create( strPropertiesWnd, this, CRect( 0, 0, DEFAULT_PROPERTIES_WIDTH, DEFAULT_PROPERTIES_HEIGHT ), TRUE, 
        ID_VIEW_PROPERTIESWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI ) )
    {
        TRACE0( "Failed to create Properties window\n" );
        return FALSE; // failed to create
    }

    SetDockingWindowIcons( theApp.mbHiColorIcons );
    return TRUE;
}

// ==========================================================================
void MainFrame::SetDockingWindowIcons(BOOL bHiColorIcons)
{
    HICON hProjectViewIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_FILE_VIEW_HC : IDI_FILE_VIEW), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
    mProjectPane.SetIcon(hProjectViewIcon, FALSE);

    HICON hControlViewIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_FILE_VIEW_HC : IDI_FILE_VIEW), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
    mProjectPane.SetIcon(hControlViewIcon, FALSE);

    HICON hPropertiesBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_PROPERTIES_WND_HC : IDI_PROPERTIES_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
    mProperties.SetIcon(hPropertiesBarIcon, FALSE);

}

// ==========================================================================
void MainFrame::OnWindowManager()
{
    ShowWindowsDialog();
}

// ==========================================================================
void MainFrame::OnViewCustomize()
{
    CMFCToolBarsCustomizeDialog* pDlgCust = new CMFCToolBarsCustomizeDialog(this, TRUE /* scan menus */);
    pDlgCust->EnableUserDefinedToolbars();
    pDlgCust->Create();
}

// ==========================================================================
LRESULT MainFrame::OnToolbarCreateNew( WPARAM wp,LPARAM lp )
{
    LRESULT lres = CMDIFrameWndEx::OnToolbarCreateNew( wp,lp );
    if (lres == 0)
    {
        return 0;
    }

    CMFCToolBar* pUserToolbar = (CMFCToolBar*)lres;
    ASSERT_VALID(pUserToolbar);

    BOOL bNameValid;
    CString strCustomize;
    bNameValid = strCustomize.LoadString( IDS_TOOLBAR_CUSTOMIZE );
    ASSERT(bNameValid);

    pUserToolbar->EnableCustomizeButton( TRUE, ID_VIEW_CUSTOMIZE, strCustomize );

    return lres;
}

// ==========================================================================
BOOL MainFrame::LoadFrame( UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext ) 
{
    // base class does the real work
    if (!CMDIFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
    {
        return FALSE;
    }

    // enable customization button for all user toolbars
    BOOL bNameValid;
    CString strCustomize;
    bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
    ASSERT(bNameValid);

    for (int i = 0; i < iMaxUserToolbars; i ++)
    {
        CMFCToolBar* pUserToolbar = GetUserToolBarByIndex(i);
        if (pUserToolbar != NULL)
        {
            pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
        }
    }

    return TRUE;
}

// ==========================================================================
LRESULT MainFrame::OnNewDevice( WPARAM wp, LPARAM lp )
{
    POSITION lPos = AfxGetApp()->GetFirstDocTemplatePosition();
    ASSERT( lPos != NULL ); // We have a template?
    
    // Get document template
    CDocTemplate *lTemplate = AfxGetApp()->GetNextDocTemplate( lPos );
    ASSERT( lTemplate != NULL ); // Double check on template?
    ASSERT( lPos == NULL ); // There should be only one template

    // Document creation will fire OnDocumentCreate callback where we add it to the project
    CDocument *lDoc = lTemplate->CreateNewDocument();
    if ( !lDoc->OnNewDocument() )
    {
        lDoc->m_bAutoDelete = TRUE;
        lTemplate->RemoveDocument( lDoc );
        return 0;
    }

    lDoc->m_bAutoDelete = FALSE;

    NetCommandDoc *lNetCommandDoc = dynamic_cast<NetCommandDoc *>( lDoc );
    ASSERT( lNetCommandDoc != NULL );

    // Only open the view is the stream is opened
    if ( lNetCommandDoc->GetStream()->IsOpen() )
    {
        CFrameWnd *lFrame = lTemplate->CreateNewFrame( lDoc, NULL );
        lTemplate->InitialUpdateFrame( lFrame, lDoc );
    }

    return 0;
}

// ==========================================================================
LRESULT MainFrame::OnRemoveDevice(WPARAM wp, LPARAM lp)
{
    POSITION lPos = AfxGetApp()->GetFirstDocTemplatePosition();
    ASSERT( lPos != NULL ); // We have a template?
    
    // Get document template
    CDocTemplate *lTemplate = AfxGetApp()->GetNextDocTemplate( lPos );
    ASSERT( lTemplate != NULL ); // Double check on template?
    ASSERT( lPos == NULL ); // There should be only one template

    NetCommandDoc *lDoc = reinterpret_cast<NetCommandDoc *>( wp );
    if ( lDoc != NULL )
    {
        lDoc->m_bAutoDelete = TRUE;
        lTemplate->RemoveDocument( lDoc );
        lDoc->OnCloseDocument();
    }

    return 0;
}

// ==========================================================================
void MainFrame::OnViewProject()
{
    mProjectPane.ShowPane( !mProjectPane.IsVisible(), FALSE, FALSE );
}

// ==========================================================================
void MainFrame::OnViewControls()
{
    mControlPane.ShowPane( !mControlPane.IsVisible(), FALSE, FALSE );
}

// ==========================================================================
void MainFrame::OnViewAttributes()
{
    mAttributesPane.ShowPane( !mAttributesPane.IsVisible(), FALSE, FALSE );
}

// ==========================================================================
void MainFrame::OnViewProperties()
{
    mProperties.ShowPane( !mProperties.IsVisible(), FALSE, FALSE );
}

// ==========================================================================
LRESULT MainFrame::OnActiveViewChanged(WPARAM wp, LPARAM lp)
{
    NetCommandDoc *lDoc = NULL;
    if ( lp != 0 )
    {
        lDoc = reinterpret_cast<NetCommandDoc *>( wp );

        mControlPane.SetDocument( lDoc );
        mProperties.SetDocument( lDoc );
        mAttributesPane.SetDocument( lDoc );

        mProjectPane.SelectDocument( lDoc );
    }

    return 0;
}

// ==========================================================================
LRESULT MainFrame::OnNewDocument(WPARAM wp, LPARAM lp)
{
    NetCommandDoc *lDoc = reinterpret_cast<NetCommandDoc *>( wp );

    ASSERT( lDoc != NULL );
    if ( lDoc == NULL )
    {
        return -1;
    }

#ifdef DEBUG
    // Make sure document is not already in doc array
    std::vector<NetCommandDoc *>::iterator lIt = mDocuments.begin();
    while ( lIt != mDocuments.end() )
    {
        ASSERT( *lIt != lDoc );
        lIt++;
    }
#endif // DEBUG

    mDocuments.push_back( lDoc );
    mProjectPane.Update( &mDocuments );        

    return 0;
}

// ==========================================================================
LRESULT MainFrame::OnDocumentClosing(WPARAM wp, LPARAM lp)
{
    NetCommandDoc *lDoc = reinterpret_cast<NetCommandDoc *>( wp );

    ASSERT( lDoc != NULL );
    if ( lDoc == NULL )
    {
        return -1;
    }

    if ( mControlPane.GetDocument() == lDoc ) mControlPane.SetDocument( NULL );
    if ( mProperties.GetDocument() == lDoc ) mProperties.SetDocument( NULL );
    if ( mAttributesPane.GetDocument() == lDoc ) mAttributesPane.SetDocument( NULL );

    std::vector<NetCommandDoc *>::iterator lIt = mDocuments.begin();
    while ( lIt != mDocuments.end() )
    {
        if ( lDoc == ( *lIt ) )
        {
            ( *lIt )->m_bAutoDelete = TRUE;
            mDocuments.erase( lIt );
            mProjectPane.Update( &mDocuments );        

            return 0;
        }

        lIt++;
    }

    return -2;
}

// ==========================================================================
LRESULT MainFrame::OnLinkDisconnected( WPARAM wp, LPARAM lp )
{
    MessageBox( _T( "Connection to device lost." ), _T( "NetCommand" ), MB_OK | MB_ICONINFORMATION );

    return SendMessage( WM_CLOSEDEVICE, wp );
}

// ==========================================================================
LRESULT MainFrame::OnDocumentOpenView(WPARAM wp, LPARAM lp)
{
    NetCommandDoc *lDoc = reinterpret_cast<NetCommandDoc *>( wp );
    if ( lDoc == NULL )
    {
        return -1;
    }

    POSITION lPos = lDoc->GetFirstViewPosition();
    if ( lPos == NULL )
    {
        // Get document template
        lPos = AfxGetApp()->GetFirstDocTemplatePosition();
        ASSERT( lPos != NULL ); // We have a template?
        CDocTemplate *lTemplate = AfxGetApp()->GetNextDocTemplate( lPos );
        ASSERT( lTemplate != NULL ); // Double check on template
        ASSERT( lPos == NULL ); // There should be only one template

        if ( lDoc->GetStream()->IsOpen() )
        {
            CFrameWnd *lFrame = lTemplate->CreateNewFrame( lDoc, NULL );
            lTemplate->InitialUpdateFrame( lFrame, lDoc );
        }
    }

    return 0;
}

// ==========================================================================
void MainFrame::OnClose()
{
    CMDIFrameWndEx::OnClose();
}

// ==========================================================================
void MainFrame::OnDefaultCommunicationParameters()
{
    // Show dialog with tree browser on communication parameter of a PvDevice
    DefaultComParamsDlg lDlg;
    lDlg.SetGenParameter( sDefaultDevice.GetCommunicationParameters() );
    lDlg.DoModal();
}


