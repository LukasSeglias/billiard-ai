// *****************************************************************************
//
// Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// -----------------------------------------------------------------------------
//
// Control view. Owns two buttons (start and stop) and a combo used to select
// the acquisition mode. The combo box is filled from the AcquisitionMode
// parameter of the currently selected device. The buttons notifies
// the document or the currently selected project entity to start or stop
// streaming.
// 
// *****************************************************************************


#include "stdafx.h"
#include "mainfrm.h"
#include "ControlPane.h"
#include "Resource.h"
#include "NetCommand.h"
#include "PvMessageBox.h"


#define ID_MODECOMBO ( 100 )
#define ID_PLAYBUTTON ( 101 )
#define ID_STOPBUTTON ( 102 )


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


BEGIN_MESSAGE_MAP(ControlPane, CDockablePane)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_PAINT()
    ON_WM_SETFOCUS()
    ON_COMMAND(ID_PLAYBUTTON, OnPlayButton)
    ON_UPDATE_COMMAND_UI(ID_PLAYBUTTON, OnUpdatePlayButton)
    ON_COMMAND(ID_STOPBUTTON, OnStopButton)
    ON_UPDATE_COMMAND_UI(ID_STOPBUTTON, OnUpdateStopButton)
    ON_CBN_SELCHANGE(ID_MODECOMBO, OnCbnSelchangeMode)
END_MESSAGE_MAP()


// ==========================================================================
ControlPane::ControlPane()
    : mDocument( NULL )
{
    mPlayButton.SetBitmap( IDB_PLAY );
    mStopButton.SetBitmap( IDB_STOP );
}

// ==========================================================================
ControlPane::~ControlPane()
{
}

// ==========================================================================
int ControlPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDockablePane::OnCreate(lpCreateStruct) == -1)
        return -1;

    CRect rectDummy;
    rectDummy.SetRectEmpty();

    // Create combo:
    DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER | CBS_SORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    if (!mObjectCombo.Create( dwViewStyle, rectDummy, this, ID_MODECOMBO ) )
    {
        TRACE0("Failed to create Properties Combo \n");
        return -1;      // fail to create
    }
    mObjectCombo.SetFont( CFont::FromHandle( (HFONT)GetStockObject( DEFAULT_GUI_FONT ) ) );
    mObjectCombo.SetCurSel(0);

    // Create start button:
    dwViewStyle = WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | BS_PUSHBUTTON;
    if ( !mPlayButton.Create( _T( "Start" ), dwViewStyle, rectDummy, this, ID_PLAYBUTTON  ) )
    {
        TRACE0("Failed to create start button\n");
        return -1;      // fail to create
    }
    mPlayButton.ModifyStyleEx( 0, WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR | WS_EX_NOPARENTNOTIFY );
    mPlayButton.SetFont(CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT)));

    // Create stop button:
    dwViewStyle = WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | BS_PUSHBUTTON;
    if ( !mStopButton.Create( _T( "Stop" ), dwViewStyle, rectDummy, this, ID_STOPBUTTON  ) )
    {
        TRACE0("Failed to create stop button\n");
        return -1;      // fail to create
    }
    mStopButton.SetFont(CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT)));

    OnChangeVisualStyle();

    AdjustLayout();
    EnableInterface();

    return 0;
}

// ==========================================================================
void ControlPane::OnSize(UINT nType, int cx, int cy)
{
    CDockablePane::OnSize(nType, cx, cy);
    AdjustLayout();
}

// ==========================================================================
void ControlPane::AdjustLayout()
{
    if (GetSafeHwnd() == NULL)
    {
        return;
    }

    CRect rectClient, rectCombo;
    GetClientRect(rectClient);
    mObjectCombo.GetClientRect(rectCombo);

    int cyCombo = rectCombo.Height();
    int cxButton = ( rectClient.Width() / 2 ) - 2;
    int cyButton = rectClient.Height() - 1 - cyCombo - 4;

    mObjectCombo.SetWindowPos( NULL, rectClient.left + 1, rectClient.top + 1, rectClient.Width() - 2, cyCombo, SWP_NOACTIVATE | SWP_NOZORDER );
    mPlayButton.SetWindowPos( NULL, rectClient.left, rectClient.top + 1 + cyCombo + 4, cxButton, cyButton, SWP_NOACTIVATE | SWP_NOZORDER );
    mStopButton.SetWindowPos( NULL, rectClient.left + cxButton + 4, rectClient.top + 1 + cyCombo + 4, cxButton, cyButton, SWP_NOACTIVATE | SWP_NOZORDER );
}

// ==========================================================================
void ControlPane::OnPaint()
{
    CPaintDC dc(this); // device context for painting

    CRect rect;
    GetClientRect(rect);

    CRect rectTree;
    ScreenToClient(rectTree);

    rectTree.InflateRect( 1, 1 );
    dc.Draw3dRect( rectTree, ::GetSysColor( COLOR_3DSHADOW ), ::GetSysColor( COLOR_3DSHADOW ) );
    dc.FillSolidRect( rect, ::GetSysColor( CTLCOLOR_DLG ) );
}

// ==========================================================================
void ControlPane::OnSetFocus(CWnd* pOldWnd)
{
    CDockablePane::OnSetFocus( pOldWnd );
}

// ==========================================================================
void ControlPane::OnChangeVisualStyle()
{
}

// ==========================================================================
void ControlPane::OnUpdatePlayButton( CCmdUI* pCmdU )
{
}

// ==========================================================================
void ControlPane::OnPlayButton()
{
    if ( mDocument != NULL )
    {
        mDocument->StartAcquisition();
    }
}

// ==========================================================================
void ControlPane::OnUpdateStopButton( CCmdUI* pCmdU )
{
}

// ==========================================================================
void ControlPane::OnStopButton()
{
    if ( mDocument != NULL )
    {
        mDocument->StopAcquisition();
    }
}

// ==========================================================================
void ControlPane::SetDocument( NetCommandDoc *aDoc )
{
    if ( mDocument != NULL )
    {
        PvGenEnum *lMode = dynamic_cast<PvGenEnum *>( mDocument->GetDevice()->GetParameters()->Get( "AcquisitionMode" ) );
        if ( lMode != NULL )
        {
            lMode->UnregisterEventSink( this );
        }
    }

    mDocument = aDoc;
    FillComboBox();
    EnableInterface();

    if ( mDocument != NULL )
    {
        PvGenEnum *lMode = dynamic_cast<PvGenEnum *>( mDocument->GetDevice()->GetParameters()->Get( "AcquisitionMode" ) );
        if ( lMode != NULL )
        {
            lMode->RegisterEventSink( this );
        }
    }
}

// ==========================================================================
void ControlPane::EnableInterface()
{
    bool lEnabled = ( mDocument != NULL ) && ( mDocument->GetDevice()->IsConnected() );
    PvGenParameter *lMode = NULL;
    PvGenParameter *lStart = NULL;
    PvGenParameter *lStop = NULL;
    
    if ( ( mDocument != NULL ) && mDocument->GetDevice()->IsConnected() )
    {
        lMode = mDocument->GetDevice()->GetParameters()->Get( "AcquisitionMode" );
        lStart = mDocument->GetDevice()->GetParameters()->Get( "AcquisitionStart" );
        lStop = mDocument->GetDevice()->GetParameters()->Get( "AcquisitionStop" );
    }

    mObjectCombo.EnableWindow( lEnabled );
    mPlayButton.EnableWindow( lEnabled && ( lStart != NULL ) );
    mStopButton.EnableWindow( lEnabled && ( lStop != NULL ) );
}

// ==========================================================================
void ControlPane::FillComboBox()
{
    if ( ( mDocument == NULL ) ||
         !mDocument->GetDevice()->IsConnected() )
    {
        mObjectCombo.ResetContent();
        return;
    }

    // Fill acquisition mode combo box
    mObjectCombo.ResetContent();
    PvGenEnum *lMode = dynamic_cast<PvGenEnum *>( mDocument->GetDevice()->GetParameters()->Get( "AcquisitionMode" ) );
    if ( lMode != NULL )
    {
        ASSERT( lMode != NULL ); // Mandatory parameter
        int64_t lEntriesCount = 0;
        VERIFY( lMode->GetEntriesCount( lEntriesCount ) ); // Failure here would be totally unexpected
        for ( uint32_t i = 0; i < lEntriesCount; i++ )
        {
            const PvGenEnumEntry *lEntry = NULL;
            VERIFY( lMode->GetEntryByIndex( i, &lEntry ) ); // Not expecting any failure on this one
            
            bool lAvailable = false;
            lEntry->IsAvailable( lAvailable );
            if ( lAvailable )
            {
                PvString lEEName;
                VERIFY( lEntry->GetName( lEEName ).IsOK() );

                int64_t lEEValue;
                VERIFY( lEntry->GetValue( lEEValue ).IsOK() );

                int lIndex = mObjectCombo.AddString( lEEName.GetUnicode() );
                mObjectCombo.SetItemData( lIndex, static_cast<DWORD_PTR>( lEEValue ) );
            }
        }

        // Set mode combo box to value currently used by the device
        int64_t lValue = 0;
        lMode->GetValue( lValue );
        for ( int i = 0; i < mObjectCombo.GetCount(); i++ )
        {
            if ( lValue == mObjectCombo.GetItemData( i ) )
            {
                mObjectCombo.SetCurSel( i );
                break;
            }
        }
    }
}

// ==========================================================================
void ControlPane::OnCbnSelchangeMode()
{
    if ( mDocument == NULL )
    {
        return;
    }

    if ( mObjectCombo.GetCurSel() < 0 )
    {
        return;
    }

    PvGenEnum *lMode = dynamic_cast<PvGenEnum *>( mDocument->GetDevice()->GetParameters()->Get( "AcquisitionMode" ) );
    if ( lMode == NULL )
    {
        return;
    }

    uint64_t lValue = mObjectCombo.GetItemData( mObjectCombo.GetCurSel() );
    PvResult lResult = lMode->SetValue( lValue );
    if ( !lResult.IsOK() )
    {
        PvMessageBox( this, lResult );
    }
}

// ==========================================================================
void ControlPane::OnParameterUpdate( PvGenParameter *aParameter )
{
    ASSERT( mDocument != NULL );

    PvString lName;
    if ( !aParameter->GetName( lName ).IsOK() )
    {
        ASSERT( 0 ); // Totally unexpected  
        return;
    }

    if ( ( lName == "AcquisitionMode" ) &&
         ( mObjectCombo.GetSafeHwnd() != 0 ) )
    {
        bool lAvailable = false, lWritable = false;
        VERIFY( aParameter->IsAvailable( lAvailable ).IsOK() );
        if ( lAvailable )
        {
            VERIFY( aParameter->IsWritable( lWritable ).IsOK() );
        }

        mObjectCombo.EnableWindow( lAvailable && lWritable );

        PvGenEnum *lEnum = dynamic_cast<PvGenEnum *>( aParameter );
        ASSERT( lEnum != NULL );

        if ( lEnum != NULL )
        {
            int64_t lEEValue = 0;
            VERIFY( lEnum->GetValue( lEEValue ) );      

            for ( int i = 0; i < mObjectCombo.GetCount(); i++ )
            {
                DWORD_PTR lData = mObjectCombo.GetItemData( i );
                if ( lData == lEEValue )
                {
                    mObjectCombo.SetCurSel( i );
                    break;
                }
            }
        }
    }
}

