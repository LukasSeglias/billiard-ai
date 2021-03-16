// *****************************************************************************
//
//     Copyright (c) 2008, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"

#include "BitmapButton.h"


BEGIN_MESSAGE_MAP(BitmapButton, CButton)
END_MESSAGE_MAP()


// =============================================================================
BitmapButton::BitmapButton()
{
}


// =============================================================================
BitmapButton::~BitmapButton()
{
}


// =============================================================================
void BitmapButton::DrawItem( LPDRAWITEMSTRUCT lpDIS )
{
    CDC lDC;
    lDC.Attach( lpDIS->hDC );

    CRect lRect( &lpDIS->rcItem );

    // Clean background
    lDC.FillSolidRect( lRect, lDC.GetBkColor() );

    // Load them
    HTHEME lTheme = ::OpenThemeData( GetSafeHwnd(), _T( "BUTTON" ) );
    
    // Set state for theme background
    int lState = 0;
    if ( ( lpDIS->itemState & ODS_SELECTED ) != 0 )
    {
        lState = PBS_PRESSED;
    }
    else if ( ( lpDIS->itemState & ODS_DISABLED ) != 0 )
    {
        lState = PBS_DISABLED;
    }
    else
    {
        lState = PBS_NORMAL;
    }

    // Draw theme background
    ::DrawThemeBackground( lTheme, lpDIS->hDC, BP_PUSHBUTTON, lState, &lpDIS->rcItem, NULL ); 

    // We are done with the theme, close it
    ::CloseThemeData( lTheme );

    CString lText;
    GetWindowText( lText );

    // Process dynamic layout
    CSize lTextSize = lDC.GetTextExtent( lText );
    int lSpace = 3;
    int lTotalX = max( mBitmapSize.cx, lTextSize.cx );
    int lTotalY = mBitmapSize.cy + lSpace + lTextSize.cy;
    int lBitmapX = ( lRect.Width() / 2 ) - ( mBitmapSize.cx / 2 );
    int lBitmapY = ( lRect.Height() / 2 ) - ( lTotalY / 2 );

    if ( ( lpDIS->itemState & ODS_DISABLED ) == 0 )
    {
        // Enabled - use TransparentBlt
        CDC lBitmapDC;
        lBitmapDC.CreateCompatibleDC( &lDC );
        CSize lSize = lBitmapDC.GetWindowExt();
        CBitmap *lOldBitmap = lBitmapDC.SelectObject( &mBitmap );
        lDC.TransparentBlt( 
            lBitmapX, lBitmapY, mBitmapSize.cx, mBitmapSize.cy, 
            &lBitmapDC, 
            0, 0, mBitmapSize.cx, mBitmapSize.cy, 
            RGB( 0xFF, 0xFF, 0xFF ) );
    }
    else
    {
        // Disabled - use DrawState. White will be converted to background color
        lDC.DrawState( CPoint( lBitmapX, lBitmapY ), mBitmapSize, mBitmap, DSS_DISABLED );
    }

    // Compute text position
    int lTextX = ( lRect.Width() / 2 ) - ( lTextSize.cx / 2 );
    int lTextY = ( lRect.Height() / 2 ) + lTotalY / 2 - lTextSize.cy;

    // Draw text - transparent, taking in account enabled/disabled state
    CRect lTextRect( lTextX, lTextY, lTextX + lTextSize.cx, lTextY + lTextSize.cy );
    lDC.SetBkMode( TRANSPARENT );
    lDC.SetTextColor( ( ( lpDIS->itemState & ODS_DISABLED ) != 0 ) ?
        ::GetSysColor( COLOR_GRAYTEXT ) :
        ::GetSysColor( COLOR_BTNTEXT ) );
    lDC.DrawText( lText, &lTextRect, DT_LEFT | DT_TOP );
}


// =============================================================================
void BitmapButton::SetBitmap( int aResourceID )
{
    VERIFY( mBitmap.LoadBitmap( aResourceID ) );

    BITMAP lBm;
    mBitmap.GetBitmap( &lBm );
    mBitmapSize.cx = lBm.bmWidth;
    mBitmapSize.cy = lBm.bmHeight;
}


