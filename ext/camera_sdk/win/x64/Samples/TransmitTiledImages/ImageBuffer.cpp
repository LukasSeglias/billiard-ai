// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"

#include "ImageBuffer.h"

#include <PvImage.h>


ImageBuffer::ImageBuffer()
    : mBuffer( PvPayloadTypeImage )
    , mConverter( NULL )
    , mImage( NULL )
    , mWidth( 0 )
    , mHeight( 0 )
{
}

ImageBuffer::~ImageBuffer()
{
    Free();
}

void ImageBuffer::Free()
{
    if ( mImage != NULL )
    {
        mBuffer.Detach();

        delete mImage;
        mImage = NULL;
    }
}

bool ImageBuffer::Alloc( int aWidth, int aHeight )
{
    PvImage* lImage;
    PvResult lResult;

    mWidth = 0;
    mHeight = 0;

    if ( ( aWidth != mWidth ) || ( aHeight != mHeight ) )
    {
        Free();
    }

    try
    {
        mImage = new CImage;
        mImage->Create( aWidth, -aHeight, 24, 0 );

        SetZero();
    }
    catch ( ... )
    {
        mImage = NULL;
    }
    if ( !mImage )
    {
        return false;
    }

    int lPaddingX = mImage->GetPitch() - ( aWidth * 3 );

    // Now attach the PvBuffer to the memory allocated for the CImage
    mBuffer.SetID( ( int64_t ) this );
    lImage = mBuffer.GetImage();
    lResult = lImage->Attach( mImage->GetBits(), aWidth, aHeight, PvPixelRGB8, lPaddingX );
    if ( !lResult.IsOK() )
    {
        Free();
        return false;
    }

    mWidth = aWidth;
    mHeight = aHeight;

    return true;
}

bool ImageBuffer::Copy( PvBuffer* aBuffer )
{
    PvImage *lImage = aBuffer->GetImage();

    if ( ( lImage->GetWidth() != mWidth ) ||
         ( lImage->GetHeight() != mHeight ) )
    {
        Alloc( lImage->GetWidth(), lImage->GetHeight() );
    }

    // Take the incoming image and initialize the current buffer in GBR24 with it
    PvResult lResult = mConverter->Convert( aBuffer, &mBuffer, false );
    if ( !lResult.IsOK() )
    {
        return false;
    }

    return true;
}

void ImageBuffer::StretchTo( ImageBuffer* aBuffer, int aX, int aY, int aSizeX, int aSizeY )
{
    CRect lSource( 0, 0, mImage->GetWidth(), mImage->GetHeight() );
    CRect lDest( aX, aY, aX + aSizeX, aY + aSizeY );

    Stretch( mImage, lSource, aBuffer->mImage, lDest );
}

void ImageBuffer::CropTo( ImageBuffer* aBuffer, int aX, int aY, int aSizeX, int aSizeY )
{
    CPoint lSource( 0, 0 );
    CRect lDest( aX, aY, aX + aSizeX, aY + aSizeY );

    HDC lHDC = aBuffer->mImage->GetDC();

    BOOL lRetVal = mImage->BitBlt( lHDC, lDest, lSource );

    aBuffer->mImage->ReleaseDC();
}

void ImageBuffer::FitToWithRatio( ImageBuffer* aBuffer, int aX, int aY, int aSizeX, int aSizeY )
{
    int lWidth;
    int lHeight;
    double lTileAspectRatio;
    double lImageAspectRatio;
    
    lTileAspectRatio = ( double ) aSizeX / ( double ) aSizeY;
    lImageAspectRatio = ( double ) mWidth / ( double ) mHeight;

    if ( lImageAspectRatio > lTileAspectRatio )
    {
        lWidth = aSizeX;
        lHeight = ( int ) ( ( double ) aSizeX / lImageAspectRatio );
    }
    else
    {
        lWidth = ( int ) ( ( double ) aSizeY * lImageAspectRatio );
        lHeight = aSizeY;
    }

    CPoint lP( aX + ( ( aSizeX - lWidth ) / 2 ), aY + ( ( aSizeY - lHeight ) / 2 ) );
    CSize lS( min( aSizeX, lWidth ), min( aSizeY, lHeight ) );

    CRect lSource( 0, 0, mImage->GetWidth(), mImage->GetHeight() );
    CRect lDest( lP.x, lP.y, lP.x + lS.cx, lP.y + lS.cy );

    Stretch( mImage, lSource, aBuffer->mImage, lDest );
}

void ImageBuffer::SetZero()
{
    if ( mImage )
    {
        memset( mImage->GetBits(), 0x00, mImage->GetHeight() * mImage->GetPitch() );
    }
}

void ImageBuffer::Stretch( CImage *aSource, CRect &aSourceRect, CImage *aDestination, CRect &aDestinationRect )
{
    BITMAPINFO lBitmapInfo;
    memset( &lBitmapInfo, 0x00, sizeof( lBitmapInfo ) );
    lBitmapInfo.bmiHeader.biSize = sizeof( lBitmapInfo.bmiHeader );
    lBitmapInfo.bmiHeader.biWidth = aSource->GetWidth();
    lBitmapInfo.bmiHeader.biHeight = -(LONG)aSource->GetHeight();
    lBitmapInfo.bmiHeader.biPlanes = 1;
    lBitmapInfo.bmiHeader.biBitCount = 24;
    lBitmapInfo.bmiHeader.biCompression = BI_RGB;

    HDC lHDC = aDestination->GetDC();

    SetStretchBltMode( lHDC, COLORONCOLOR );
    StretchDIBits( lHDC, 
        aDestinationRect.left, aDestinationRect.top, aDestinationRect.Width(), aDestinationRect.Height(), 
        aSourceRect.left, aSourceRect.top, aSourceRect.Width(), aSourceRect.Height(), 
        aSource->GetBits(), 
        &lBitmapInfo, DIB_RGB_COLORS, SRCCOPY );

    aDestination->ReleaseDC();
}

