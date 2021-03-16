// *****************************************************************************
//
//     Copyright (c) 2014, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"

#include "eBUSPlayerApp.h"
#include "HistogramCtrl.h"

#include <PvRangeFilter.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


BEGIN_MESSAGE_MAP(HistogramCtrl, CDialog)
	ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_TIMER()
END_MESSAGE_MAP()


///
/// \brief Constructor
///

HistogramCtrl::HistogramCtrl( PvRangeFilter *aRangeFilter, CWnd* pParent /* =NULL */ )
	: CDialog( HistogramCtrl::IDD, pParent )
	, mRangeFilter( aRangeFilter )
    , mTimer( 0 )
{
}


///
/// \brief Destructor
///

HistogramCtrl::~HistogramCtrl()
{
}


///
/// \brief MFC data exchance
///

void HistogramCtrl::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


///
/// \brief CDialog OnInit handler
///

BOOL HistogramCtrl::OnInitDialog()
{
	CDialog::OnInitDialog();
    if ( mTimer == 0 )
    {
        mTimer = SetTimer( 1, 100, NULL );
    }

    return TRUE;  // return TRUE  unless you set the focus to a control
}


///
/// \brief Paint handler
///

void HistogramCtrl::OnPaint()
{
	CDC *lDC = GetDC();
    
	CRect lCR;
	GetClientRect( &lCR );

    if ( !mRangeFilter->IsEnabled() )
    {
        lDC->FillSolidRect( lCR, GetSysColor( COLOR_BACKGROUND ) );
        ReleaseDC( lDC );
    }

	// Background
	lDC->FillSolidRect( lCR, 0 );

	// Get histogram, max value
    uint32_t lMax = 0;
    uint32_t lHistogram[ 256 ] = { 0 };
    mRangeFilter->GetHistogram( lHistogram, 256, lMax );
    if ( lMax == 0 )
    {
        lMax = 1; // Prevents the division by 0 for an empty histogram
    }
	lMax = static_cast<uint32_t>( static_cast<double>( lMax ) * 1.2 + 0.5 ); // Adds a gap at the top of the histogram
    
	// Draw histogram
    const uint32_t *lPtr = lHistogram;
	for ( int i = 0; i < lCR.right; i++ )
	{
        // RGB
        int lIndex = ( i * 255 ) / lCR.right;
        int lHeight = lPtr[ lIndex ] * lCR.bottom / lMax;
        bool lInRange = ( lIndex >= mRangeFilter->GetDark() ) && ( lIndex <= mRangeFilter->GetLight() );
        int lValue = lInRange ? ( lIndex / 2 + 0x80 ) : 0x40;
        lDC->FillSolidRect( i, lCR.Height() - lHeight, 1, lHeight, RGB( lValue, lValue, lValue ) );
    }

    // Draw threshold bars
    if ( mRangeFilter->GetDark() != 0 )
    {
        int lX = mRangeFilter->GetDark() * lCR.right / 255;
        lDC->FillSolidRect( lX, lCR.top, 1, lCR.bottom, RGB( 0x00, 0xFF, 0x00 ) );
    }

    if ( mRangeFilter->GetLight() != 255 )
    {
        int lX = mRangeFilter->GetLight() * lCR.right / 255;
        lDC->FillSolidRect( lX, lCR.top, 1, lCR.bottom, RGB( 0x00, 0xFF, 0x00 ) );
    }

	ReleaseDC( lDC );

    CDialog::OnPaint();
}


///
/// \brief Disable erase background - we handle it all in the pain handler
///

BOOL HistogramCtrl::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}


///
/// \brief Timer - just refresh the histogram
///

void HistogramCtrl::OnTimer( UINT_PTR nIDEvent )
{
    Invalidate( FALSE );
}

