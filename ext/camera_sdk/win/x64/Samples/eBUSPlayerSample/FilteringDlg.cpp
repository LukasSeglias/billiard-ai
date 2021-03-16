// *****************************************************************************
//
//     Copyright (c) 2008, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"
#include "FilteringDlg.h"
#include "DisplayThread.h"

#include <math.h>


BEGIN_MESSAGE_MAP(FilteringDlg, CDialog)
    ON_BN_CLICKED(IDC_ENABLEDCHECK, &FilteringDlg::OnBnClickedEnabledcheck)
    ON_EN_CHANGE(IDC_RGEDIT, &FilteringDlg::OnEnChange)
    ON_EN_CHANGE(IDC_GGEDIT, &FilteringDlg::OnEnChange)
    ON_EN_CHANGE(IDC_BGEDIT, &FilteringDlg::OnEnChange)
    ON_EN_CHANGE(IDC_ROEDIT, &FilteringDlg::OnEnChange)
    ON_EN_CHANGE(IDC_GOEDIT, &FilteringDlg::OnEnChange)
    ON_EN_CHANGE(IDC_BOEDIT, &FilteringDlg::OnEnChange)
    ON_BN_CLICKED(IDC_WBBUTTON, &FilteringDlg::OnBnClickedWbbutton)
    ON_BN_CLICKED(IDC_RESETBUTTON, &FilteringDlg::OnBnClickedResetbutton)
    ON_CBN_SELCHANGE(IDC_BAYERCOMBO, &FilteringDlg::OnCbnSelchange)
    ON_CBN_SELCHANGE( IDC_TAPCOMBO, &FilteringDlg::OnCbnSelchange)
    ON_NOTIFY(UDN_DELTAPOS, IDC_RGSPIN, &FilteringDlg::OnDeltapos)
    ON_NOTIFY(UDN_DELTAPOS, IDC_GGSPIN, &FilteringDlg::OnDeltapos)
    ON_NOTIFY(UDN_DELTAPOS, IDC_BGSPIN, &FilteringDlg::OnDeltapos)
    ON_WM_HSCROLL()
    ON_BN_CLICKED(IDC_HISTOGRAMENABLED, &FilteringDlg::OnBnClickedHistogramenabled)
    ON_BN_CLICKED(IDC_AUTOCONFIGURE, &FilteringDlg::OnBnClickedAutoconfigure)
END_MESSAGE_MAP()


#define GAIN_TICK_COUNT ( 1000 )
#define GAIN_MAX ( 16 )


const double cNormalizer = log( static_cast<double>( GAIN_TICK_COUNT ) );


// =============================================================================
FilteringDlg::FilteringDlg( ImageFiltering *aImageFiltering, DisplayThread *aDisplayThread, CWnd* pParent /* = NULL */ )
    : CDialog(FilteringDlg::IDD, pParent)
    , mImageFiltering( aImageFiltering )
    , mDisplayThread( aDisplayThread )
    , mLoading( true )
    , mSkipSyncSpinsCount( 0 )
    , mSkipSyncSlidersCount( 0 )
    , mHistogramCtrl( NULL )
{
    mHistogramCtrl = new HistogramCtrl( mImageFiltering->GetRangeFilter(), this );
}


// =============================================================================
FilteringDlg::~FilteringDlg()
{
    if ( mHistogramCtrl != NULL )
    {
        delete mHistogramCtrl;
        mHistogramCtrl = NULL;
    }
}


// =============================================================================
void FilteringDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_ENABLEDCHECK, mRGBFilterCheckBox);
    DDX_Control(pDX, IDC_RGEDIT, mRGEdit);
    DDX_Control(pDX, IDC_GGEDIT, mGGEdit);
    DDX_Control(pDX, IDC_BGEDIT, mBGEdit);
    DDX_Control(pDX, IDC_ROEDIT, mROEdit);
    DDX_Control(pDX, IDC_GOEDIT, mGOEdit);
    DDX_Control(pDX, IDC_BOEDIT, mBOEdit);
    DDX_Control(pDX, IDC_RGSPIN, mRGSpin);
    DDX_Control(pDX, IDC_GGSPIN, mGGSpin);
    DDX_Control(pDX, IDC_BGSPIN, mBGSpin);
    DDX_Control(pDX, IDC_ROSPIN, mROSpin);
    DDX_Control(pDX, IDC_GOSPIN, mGOSpin);
    DDX_Control(pDX, IDC_BOSPIN, mBOSpin);
    DDX_Control(pDX, IDC_RGSLIDER, mRGSlider);
    DDX_Control(pDX, IDC_GGSLIDER, mGGSlider);
    DDX_Control(pDX, IDC_BGSLIDER, mBGSlider);
    DDX_Control(pDX, IDC_ROSLIDER, mROSlider);
    DDX_Control(pDX, IDC_GOSLIDER, mGOSlider);
    DDX_Control(pDX, IDC_BOSLIDER, mBOSlider);
    DDX_Control(pDX, IDC_WBBUTTON, mWBButton);
    DDX_Control(pDX, IDC_RESETBUTTON, mResetButton);
    DDX_Control(pDX, IDC_BAYERCOMBO, mBayerCombo);
    DDX_Control( pDX, IDC_TAPCOMBO, mTapCombo );
    DDX_Control(pDX, IDC_GAINSLABEL, mGainsLabel);
    DDX_Control(pDX, IDC_OFFSETSLABEL, mOffsetsLabel);
    DDX_Control(pDX, IDC_REDLABEL, mRedLabel);
    DDX_Control(pDX, IDC_GREENLABEL, mGreenLabel);
    DDX_Control(pDX, IDC_BLUELABEL, mBlueLabel);
    DDX_Control(pDX, IDC_HISTOGRAMENABLED, mHistogramCheck);
    DDX_Control(pDX, IDC_DARKSLIDER, mDarkSlider);
    DDX_Control(pDX, IDC_LIGHTSLIDER, mLightSlider);
    DDX_Control(pDX, IDC_AUTOCONFIGURE, mAutoConfigButton);
}


// =============================================================================
void FilteringDlg::OnOK()
{
    ShowWindow( SW_HIDE );
}


// =============================================================================
void FilteringDlg::OnCancel()
{
    ShowWindow( SW_HIDE );
}


// =============================================================================
BOOL FilteringDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

	CRect lRect;

	// Position histogram according to invisible resource item
	GetDlgItem( IDC_IPOS )->GetClientRect( &lRect );
	GetDlgItem( IDC_IPOS )->ClientToScreen( &lRect );
	ScreenToClient( &lRect );
    mHistogramCtrl->Create( HistogramCtrl::IDD, this );
	mHistogramCtrl->MoveWindow( lRect );
	mHistogramCtrl->ShowWindow( SW_SHOW );

    // Create bold font
    LOGFONT lLogFont;
    mRedLabel.GetFont()->GetLogFont( &lLogFont );
    lLogFont.lfWeight = FW_BOLD;
    mBoldFont.CreateFontIndirect( &lLogFont );

    mGainsLabel.SetFont( &mBoldFont );
    mOffsetsLabel.SetFont( &mBoldFont );
    mRedLabel.SetFont( &mBoldFont );
    mGreenLabel.SetFont( &mBoldFont );
    mBlueLabel.SetFont( &mBoldFont );

    mRGSpin.SetRange( 0, 2550 );
    mGGSpin.SetRange( 0, 2550 );
    mBGSpin.SetRange( 0, 2550 );

    mRGSlider.SetRange( 0, GAIN_TICK_COUNT );
    mGGSlider.SetRange( 0, GAIN_TICK_COUNT );
    mBGSlider.SetRange( 0, GAIN_TICK_COUNT );

    mROSpin.SetRange( -255, 255 );
    mGOSpin.SetRange( -255, 255 );
    mBOSpin.SetRange( -255, 255 );

    mROSlider.SetRange( 0, 510 );
    mGOSlider.SetRange( 0, 510 );
    mBOSlider.SetRange( 0, 510 );

    mBayerCombo.AddString( _T( "Simple (2x2)" ) );
    mBayerCombo.AddString( _T( "Interpolation (3x3)" ) );

    // Default, tap reconstruction disabled
    mTapCombo.AddString( _T( "Disabled" ) );
    mTapCombo.SetItemData( 0, static_cast<DWORD_PTR>( PvTapGeometryUnknown ) );
    mTapCombo.SetCurSel( 0 );

    // Add all available tap geometries
    uint32_t lCount = PvTapFilter::GetSupportedGeometryCount();
    for ( uint32_t i = 0; i < lCount; i++ )
    {
        mTapCombo.AddString( PvTapFilter::GetSupportedGeometryName( i ).GetUnicode() );
        mTapCombo.SetItemData( mTapCombo.GetCount() - 1, static_cast<DWORD_PTR>( PvTapFilter::GetSupportedGeometryValue( i ) ) );
    }

    mDarkSlider.SetRange( 0, 255 );
    mLightSlider.SetRange( 0, 255 );

    ToDialog();

    SyncSpins();
    SyncSliders();

    mLoading = false;
    mSkipSyncSpinsCount = 0;
    mSkipSyncSlidersCount = 0;

    return TRUE;
}


// =============================================================================
void FilteringDlg::EnableInterface()
{
    BOOL lRGBEnabled = ( mRGBFilterCheckBox.GetCheck() == BST_CHECKED );

    mRGEdit.EnableWindow( lRGBEnabled );
    mGGEdit.EnableWindow( lRGBEnabled );
    mBGEdit.EnableWindow( lRGBEnabled );
    mROEdit.EnableWindow( lRGBEnabled );
    mGOEdit.EnableWindow( lRGBEnabled );
    mBOEdit.EnableWindow( lRGBEnabled );

    mRGSpin.EnableWindow( lRGBEnabled );
    mGGSpin.EnableWindow( lRGBEnabled );
    mBGSpin.EnableWindow( lRGBEnabled );
    mROSpin.EnableWindow( lRGBEnabled );
    mGOSpin.EnableWindow( lRGBEnabled );
    mBOSpin.EnableWindow( lRGBEnabled );

    mRGSlider.EnableWindow( lRGBEnabled );
    mGGSlider.EnableWindow( lRGBEnabled );
    mBGSlider.EnableWindow( lRGBEnabled );
    mROSlider.EnableWindow( lRGBEnabled );
    mGOSlider.EnableWindow( lRGBEnabled );
    mBOSlider.EnableWindow( lRGBEnabled );

    mWBButton.EnableWindow( lRGBEnabled );
    mResetButton.EnableWindow( lRGBEnabled );

    mOffsetsLabel.EnableWindow( lRGBEnabled );
    mGainsLabel.EnableWindow( lRGBEnabled );
    mRedLabel.EnableWindow( lRGBEnabled );
    mGreenLabel.EnableWindow( lRGBEnabled );
    mBlueLabel.EnableWindow( lRGBEnabled );

    BOOL lHistogramEnabled = ( mHistogramCheck.GetCheck() == BST_CHECKED );

    mHistogramCtrl->EnableWindow( lHistogramEnabled );
    mDarkSlider.EnableWindow( lHistogramEnabled );
    mLightSlider.EnableWindow( lHistogramEnabled );
    mAutoConfigButton.EnableWindow( lHistogramEnabled );
}


// =============================================================================
void FilteringDlg::OnBnClickedEnabledcheck()
{
    FromDialog();
    EnableInterface();
}


// =============================================================================
void FilteringDlg::ToDialog()
{
    mLoading = true;
   
    // Filter RGB enabled?
    mRGBFilterCheckBox.SetCheck( 
        ( mImageFiltering->GetRGBFilterEnabled() ) ? 
            BST_CHECKED : 
            BST_UNCHECKED );

    PvBufferConverterRGBFilter &lFilter = mImageFiltering->GetRGBFilter();

    // Gains
    mRGEdit.SetWindowText( ToStr( lFilter.GetGainR() ) );
    mGGEdit.SetWindowText( ToStr( lFilter.GetGainG() ) );
    mBGEdit.SetWindowText( ToStr( lFilter.GetGainB() ) );

    // Offsets
    mROEdit.SetWindowText( ToStr( lFilter.GetOffsetR() ) );
    mGOEdit.SetWindowText( ToStr( lFilter.GetOffsetG() ) );
    mBOEdit.SetWindowText( ToStr( lFilter.GetOffsetB() ) );

    // Bayer filtering
    switch ( mImageFiltering->GetBayerFilter() )
    {
        case PvBayerFilterSimple:
            mBayerCombo.SetCurSel( 0 );
            break;

        case PvBayerFilter3X3:
            mBayerCombo.SetCurSel( 1 );
            break;

        default:
            ASSERT( 0 );
    }

    // Tap geometry
    mTapCombo.SetCurSel( 0 ); // Default if no match found
    PvTapGeometryEnum lTapGeometry = mImageFiltering->GetTapGeometry();
    for ( int i = 0; i < mTapCombo.GetCount(); i++ )
    {
        if ( lTapGeometry == static_cast<PvTapGeometryEnum>( mTapCombo.GetItemData( i ) ) )
        {
            mTapCombo.SetCurSel( i );
            break;
        }
    }

    LoadHistogram();

    mLoading = false;

    // Make sure UI is setup properly!
    EnableInterface();
}


// =============================================================================
void FilteringDlg::FromDialog()
{
    if ( mLoading )
    {
        return;
    }
    
    // Filter RGB enabled?
    bool lEnabled = ( mRGBFilterCheckBox.GetCheck() == BST_CHECKED );
    mImageFiltering->SetRGBFilterEnabled( lEnabled );

    PvBufferConverterRGBFilter &lFilter = mImageFiltering->GetRGBFilter();

    // Gain R
    double lValueD = 0.0;
    if ( GetValueD( &mRGEdit, lValueD ) )
    {
        lFilter.SetGainR( lValueD );
    }

    // Gain G
    if ( GetValueD( &mGGEdit, lValueD ) )
    {
        lFilter.SetGainG( lValueD );
    }

    // Gain B
    if ( GetValueD( &mBGEdit, lValueD ) )
    {
        lFilter.SetGainB( lValueD );
    }

    // Offset R
    int32_t lValueI = 0;
    if ( GetValueI( &mROEdit, lValueI ) )
    {
        lFilter.SetOffsetR( lValueI );
    }

    // Offset G
    if ( GetValueI( &mGOEdit, lValueI ) )
    {
        lFilter.SetOffsetG( lValueI );
    }

    // Offset B
    if ( GetValueI( &mBOEdit, lValueI ) )
    {
        lFilter.SetOffsetB( lValueI );
    }

    // Bayer filtering
    int lSelected = mBayerCombo.GetCurSel();
    switch ( lSelected )
    {
        case 0:
            mImageFiltering->SetBayerFilter( PvBayerFilterSimple );
            break;

        case 1:
            mImageFiltering->SetBayerFilter( PvBayerFilter3X3 );
            break;

        default:
            ASSERT( 0 );
    }

    // Tap geometry
    if ( mTapCombo.GetCurSel() >= 0 )
    {
        PvTapGeometryEnum lTapGeometry = static_cast<PvTapGeometryEnum>( mTapCombo.GetItemData( mTapCombo.GetCurSel() ) );
        mImageFiltering->SetTapGeometry( lTapGeometry );
    }
    else
    {
        mImageFiltering->SetTapGeometry( PvTapGeometryUnknown );
    }
}


// =============================================================================
bool FilteringDlg::GetValueD( CEdit *aEdit, double &aValue )
{
    CString lStr;
    aEdit->GetWindowText( lStr );

    return GetValueD( lStr, aValue );
}


// =============================================================================
bool FilteringDlg::GetValueD( const CString &aStr, double &aValue )
{
    int lCount = swscanf_s( (LPCTSTR)aStr, _T( "%lf" ), &aValue );

    return ( lCount == 1 );
}


// =============================================================================
bool FilteringDlg::GetValueI( CEdit *aEdit, int32_t &aValue )
{
    CString lStr;
    aEdit->GetWindowText( lStr );

    return GetValueI( lStr, aValue );
}


// =============================================================================
bool FilteringDlg::GetValueI( const CString &aStr, int32_t &aValue )
{
    int lCount = swscanf_s( (LPCTSTR)aStr, _T( "%i" ), &aValue );

    return ( lCount == 1 );
}


// =============================================================================
void FilteringDlg::OnEnChange()
{
    FromDialog();

    if ( mSkipSyncSpinsCount > 0 )
    {
        mSkipSyncSpinsCount--;
    }
    else
    {
        mSkipSyncSpinsCount = 0;
        SyncSpins();
    }

    if ( mSkipSyncSlidersCount > 0 )
    {
        mSkipSyncSlidersCount--;
    }
    else
    {
        mSkipSyncSlidersCount = 0;
        SyncSliders();
    }
}


// =============================================================================
CString FilteringDlg::ToStr( double aValue )
{
    CString lStr;
    lStr.Format( _T( "%.3f" ), aValue );

    return lStr;
}


// =============================================================================
CString FilteringDlg::ToStr( int32_t aValue )
{
    CString lStr;
    lStr.Format( _T( "%i" ), aValue );

    return lStr;
}


// =============================================================================
void FilteringDlg::OnBnClickedWbbutton()
{
    mImageFiltering->WhiteBalance( mDisplayThread );
    ToDialog();
}


// =============================================================================
void FilteringDlg::OnBnClickedResetbutton()
{
    mImageFiltering->ResetRGBFilter();
    ToDialog();
}


// =============================================================================
void FilteringDlg::OnCbnSelchange()
{
    FromDialog();
}


// =============================================================================
void FilteringDlg::SyncSpins()
{
    if ( ( mRGSpin.GetSafeHwnd() == NULL ) ||
         ( mGGSpin.GetSafeHwnd() == NULL ) ||
         ( mBGSpin.GetSafeHwnd() == NULL ) ||
         ( mROSpin.GetSafeHwnd() == NULL ) ||
         ( mGOSpin.GetSafeHwnd() == NULL ) ||
         ( mBOSpin.GetSafeHwnd() == NULL ) )
    {
        return;
    }

    double lValueD = 0.0;
    int lValueI = 0;
    int lLower, lUpper;

    // RGain
    if ( GetValueD( &mRGEdit, lValueD ) )
    {
        mRGSpin.GetRange32( lLower, lUpper );
        mRGSpin.SetPos( max( lLower, min( lUpper, static_cast<int>( lValueD * 10.0 + 0.5 ) ) ) );
    }

    // GGain
    if ( GetValueD( &mGGEdit, lValueD ) )
    {
        mGGSpin.GetRange32( lLower, lUpper );
        mGGSpin.SetPos( max( lLower, min( lUpper, static_cast<int>( lValueD * 10.0 + 0.5 ) ) ) );
    }

    // BGain
    if ( GetValueD( &mBGEdit, lValueD ) )
    {
        mBGSpin.GetRange32( lLower, lUpper );
        mBGSpin.SetPos( max( lLower, min( lUpper, static_cast<int>( lValueD * 10.0 + 0.5 ) ) ) );
    }

    // ROffset
    if ( GetValueI( &mROEdit, lValueI ) )
    {
        mROSpin.GetRange32( lLower, lUpper );
        mROSpin.SetPos( max( lLower, min( lUpper, lValueI ) ) );
    }

    // GOffset
    if ( GetValueI( &mGOEdit, lValueI ) )
    {
        mGOSpin.GetRange32( lLower, lUpper );
        mGOSpin.SetPos( max( lLower, min( lUpper, lValueI ) ) );
    }

    // BOffset
    if ( GetValueI( &mBOEdit, lValueI ) )
    {
        mBOSpin.GetRange32( lLower, lUpper );
        mBOSpin.SetPos( max( lLower, min( lUpper, lValueI ) ) );
    }
}


// =============================================================================
void FilteringDlg::SyncSliders()
{
    if ( ( mRGSlider.GetSafeHwnd() == NULL ) ||
         ( mGGSlider.GetSafeHwnd() == NULL ) ||
         ( mBGSlider.GetSafeHwnd() == NULL ) ||
         ( mROSlider.GetSafeHwnd() == NULL ) ||
         ( mGOSlider.GetSafeHwnd() == NULL ) ||
         ( mBOSlider.GetSafeHwnd() == NULL ) )
    {
        return;
    }

    double lValueD = 0.0;
    int lValueI = 0;
    int lLower, lUpper;

    // RGain
    if ( GetValueD( &mRGEdit, lValueD ) )
    {
        ToGainSlider( &mRGSlider, lValueD );
    }

    // GGain
    if ( GetValueD( &mGGEdit, lValueD ) )
    {
        ToGainSlider( &mGGSlider, lValueD );
    }

    // BGain
    if ( GetValueD( &mBGEdit, lValueD ) )
    {
        ToGainSlider( &mBGSlider, lValueD );
    }

    // ROffset
    if ( GetValueI( &mROEdit, lValueI ) )
    {
        mROSlider.GetRange( lLower, lUpper );
        mROSlider.SetPos( max( lLower, min( lUpper, lValueI + 255 ) ) );
    }

    // GOffset
    if ( GetValueI( &mGOEdit, lValueI ) )
    {
        mGOSlider.GetRange( lLower, lUpper );
        mGOSlider.SetPos( max( lLower, min( lUpper, lValueI + 255 ) ) );
    }

    // BOffset
    if ( GetValueI( &mBOEdit, lValueI ) )
    {
        mBOSlider.GetRange( lLower, lUpper );
        mBOSlider.SetPos( max( lLower, min( lUpper, lValueI + 255 ) ) );
    }
}


// =============================================================================
void FilteringDlg::OnDeltapos( NMHDR *pNMHDR, LRESULT *pResult )
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

    int lNewVal = min( 2550, max( 0, pNMUpDown->iPos + pNMUpDown->iDelta ) );

    if ( pNMHDR->hwndFrom == mRGSpin.GetSafeHwnd() )
    {
        mSkipSyncSpinsCount++;
        mRGEdit.SetWindowText( ToStr( lNewVal / 10.0 ) );
    }
    else if ( pNMHDR->hwndFrom == mGGSpin.GetSafeHwnd() )
    {
        mSkipSyncSpinsCount++;
        mGGEdit.SetWindowText( ToStr( lNewVal / 10.0 ) );
    }
    else if ( pNMHDR->hwndFrom == mBGSpin.GetSafeHwnd() )
    {
        mSkipSyncSpinsCount++;
        mBGEdit.SetWindowText( ToStr( lNewVal / 10.0 ) );
    }

    *pResult = 0;
}


// =============================================================================
void FilteringDlg::OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar )
{
    CDialog::OnHScroll( nSBCode, nPos, pScrollBar );

    if ( pScrollBar->GetSafeHwnd() == mRGSlider.GetSafeHwnd() )
    {
        mSkipSyncSlidersCount++;
        mRGEdit.SetWindowText( ToStr( FromGainSlider( &mRGSlider ) ) );
    }
    else if ( pScrollBar->GetSafeHwnd() == mGGSlider.GetSafeHwnd() )
    {
        mSkipSyncSlidersCount++;
        mGGEdit.SetWindowText( ToStr( FromGainSlider( &mGGSlider ) ) );
    }
    else if ( pScrollBar->GetSafeHwnd() == mBGSlider.GetSafeHwnd() )
    {
        mSkipSyncSlidersCount++;
        mBGEdit.SetWindowText( ToStr( FromGainSlider( &mBGSlider ) ) );
    }
    else if ( pScrollBar->GetSafeHwnd() == mROSlider.GetSafeHwnd() )
    {
        mSkipSyncSlidersCount++;
        int lPos = mROSlider.GetPos();
        mROEdit.SetWindowText( ToStr( lPos - 255 ) );
    }
    else if ( pScrollBar->GetSafeHwnd() == mGOSlider.GetSafeHwnd() )
    {
        mSkipSyncSlidersCount++;
        int lPos = mGOSlider.GetPos();
        mGOEdit.SetWindowText( ToStr( lPos - 255 ) );
    }
    else if ( pScrollBar->GetSafeHwnd() == mBOSlider.GetSafeHwnd() )
    {
        mSkipSyncSlidersCount++;
        int lPos = mBOSlider.GetPos();
        mBOEdit.SetWindowText( ToStr( lPos - 255 ) );
    }
    else if ( pScrollBar->GetSafeHwnd() == mDarkSlider.GetSafeHwnd() )
    {
        PvRangeFilter *lRangeFilter = mImageFiltering->GetRangeFilter();

        int lValue = mDarkSlider.GetPos();
        if ( lValue != lRangeFilter->GetDark() )
        {
            lRangeFilter->SetDark( lValue );
            LoadHistogram();
        }
    }
    else if ( pScrollBar->GetSafeHwnd() == mLightSlider.GetSafeHwnd() )
    {
        PvRangeFilter *lRangeFilter = mImageFiltering->GetRangeFilter();

        int lValue = mLightSlider.GetPos();
        if ( lValue != lRangeFilter->GetLight() )
        {
            lRangeFilter->SetLight( lValue );
            LoadHistogram();
        }
    }
}


// =============================================================================
double FilteringDlg::FromGainSlider( CSliderCtrl *aSlider )
{
    double lPos = min( static_cast<double>( GAIN_TICK_COUNT - 1 ), aSlider->GetPos() );
    double lLog = log( GAIN_TICK_COUNT - lPos );
    double lValue = GAIN_MAX - lLog / cNormalizer * GAIN_MAX;

    return lValue;
}


// =============================================================================
void FilteringDlg::ToGainSlider( CSliderCtrl *aSlider, double &aValue )
{
    double lV1 = ( static_cast<double>( GAIN_MAX ) - aValue ) / static_cast<double>( GAIN_MAX ) * cNormalizer;
    double lV2 = static_cast<double>( GAIN_TICK_COUNT ) - exp( lV1 );

    int lMin, lMax;
    aSlider->GetRange( lMin, lMax );

    int lValue = max( lMin, min( lMax, static_cast<int>( lV2 + 0.5 ) ) );

    aSlider->SetPos( lValue );
}


// =============================================================================
BOOL FilteringDlg::PreTranslateMessage(MSG *pMsg)
{
    // Flip up and down on horizontal scrollbars, see bug 1729
    HWND lFocus = GetFocus()->GetSafeHwnd();
    if ( ( lFocus == mRGSlider.GetSafeHwnd() ) ||
         ( lFocus == mGGSlider.GetSafeHwnd() ) ||
         ( lFocus == mBGSlider.GetSafeHwnd() ) ||
         ( lFocus == mROSlider.GetSafeHwnd() ) ||
         ( lFocus == mGOSlider.GetSafeHwnd() ) ||
         ( lFocus == mBOSlider.GetSafeHwnd() ) )
    {
        if ( ( pMsg->message == WM_KEYDOWN ) || ( pMsg->message == WM_KEYUP ) )
        {
            switch ( pMsg->wParam )
            {
                case VK_UP:
                    pMsg->wParam = VK_DOWN;
                    break;

                case VK_DOWN:
                    pMsg->wParam = VK_UP;
                    break;

                case VK_NEXT:
                    pMsg->wParam = VK_PRIOR;
                    break;

                case VK_PRIOR:
                    pMsg->wParam = VK_NEXT;
                    break;
            }
        }
    }

    return CDialog::PreTranslateMessage( pMsg );
}


// =============================================================================
void FilteringDlg::LoadHistogram()
{
    PvRangeFilter *lRangeFilter = mImageFiltering->GetRangeFilter();

	mHistogramCheck.SetCheck( lRangeFilter->IsEnabled() ? BST_CHECKED : BST_UNCHECKED );
    mDarkSlider.SetPos( lRangeFilter->GetDark() );
    mLightSlider.SetPos( lRangeFilter->GetLight() );
}


// =============================================================================
void FilteringDlg::OnBnClickedHistogramenabled()
{
    EnableInterface();
    mImageFiltering->GetRangeFilter()->SetEnabled( mHistogramCheck.GetCheck() == BST_CHECKED );
}

    
// =============================================================================
void FilteringDlg::OnBnClickedAutoconfigure()
{
    mImageFiltering->GetRangeFilter()->AutoConfigure();
    LoadHistogram();
}


