// *****************************************************************************
//
// Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include <PvSampleUtils.h>
#include <PvDeviceFinderWnd.h>
#include <PvTransmitterGEV.h>
#include <PvVersion.h>
#include <PvDeviceGEV.h>

#include <sstream>

#include "VideoSource.h"

VideoSource::VideoSource( const char *aConnectionID )
    : mConnectionID( aConnectionID )
    , mDevice( NULL )
    , mStream( NULL )
    , mPipeline( NULL )
{
    Gdiplus::GdiplusStartupInput lGdiplusStartupInput;
    Gdiplus::GdiplusStartup( &mGdiplusToken, &lGdiplusStartupInput, NULL );
}

VideoSource::~VideoSource()
{
    if ( mDevice != NULL )
    {
        PvDevice::Free( mDevice );
        mDevice = NULL;
    }

    if ( mStream != NULL )
    {
        PvStream::Free( mStream );
        mStream = NULL;
    }

	PVDELETE( mPipeline );

    // Free bitmaps.
    gBMap::iterator lIt = mMap.begin();
    while ( lIt != mMap.end() )
    {
        Gdiplus::Bitmap *lImage = ( lIt++ )->second;
        delete lImage;
        lImage = NULL;
    }

    Gdiplus::GdiplusShutdown( mGdiplusToken );
}

bool VideoSource::Connect()
{
    PvResult lResult = PvResult::Code::OK;

    if ( mConnectionID.GetLength() == 0 )
    {
        // The connection ID wasn't provided by the user; open device finder
        PvDeviceInfoType lType;
        if ( !PvSelectDevice( &mConnectionID, &lType ) )
        {
            cout << "No device selected." << endl;
            return false;
        }
    }

    // Create and connect device
    mDevice = PvDevice::CreateAndConnect( mConnectionID, &lResult );
    if ( !lResult.IsOK() )
    {
        cout << "Unable to connect to device." << endl;
        return false;
    }

    // Create and open stream.
    mStream = PvStream::CreateAndOpen( mConnectionID, &lResult );
    if ( !lResult.IsOK() )
    {
        cout << "Unable to open stream." << endl;
        return false;
    }

	// Create pipeline
	mPipeline = new PvPipeline( mStream );

    return true;
}

bool VideoSource::StartAcquisition()
{
    bool lRetVal = true;
    PvResult lResult = PvResult::Code::OK;

    try
    {
        // Start pipeline.
        lResult = mPipeline->Start();
        if ( lResult.IsOK() )
        {
			PvDeviceGEV *lDeviceGEV = dynamic_cast<PvDeviceGEV *>( mDevice );
			PvStreamGEV *lStreamGEV = dynamic_cast<PvStreamGEV *>( mStream );
			if ( lDeviceGEV != NULL && lStreamGEV != NULL )
			{
				// Set streaming destination.
				lResult = lDeviceGEV->SetStreamDestination( lStreamGEV->GetLocalIPAddress(), lStreamGEV->GetLocalPort() );
			}

            if ( lResult.IsOK() )
            {
                // Enables stream.
                mDevice->StreamEnable();
                // Start acquisition.
                lResult = mDevice->GetParameters()->ExecuteCommand( "AcquisitionStart" );
            }
        }
        if ( !lResult.IsOK() )
        {
            cout << "Failed to start acquisition process." <<  endl;
            cout << "    Error: " << lResult.GetDescription() << endl;
            lRetVal = false;
        }
    }
    catch ( ... )
    {
        cout << "Failed to start acquisition process. Unexpected error." << endl;
        lRetVal = false;
    }

    return lRetVal;
}

bool VideoSource::StopAcquisition()
{
    PvResult lResult = PvResult::Code::OK;
    bool lRetVal = true;

    try
    {
        // Stop acquisition.
        lResult = mDevice->GetParameters()->ExecuteCommand( "AcquisitionStop" );
        if ( lResult.IsOK() )
        {
            // Stream disable.
            mDevice->StreamDisable();
            // Stop pipeline.
            lResult = mPipeline->Stop();
        }

        if ( mDevice != NULL )
        {
            PvDevice::Free( mDevice );
            mDevice = NULL;
        }

        if ( mStream != NULL )
        {
            PvStream::Free( mStream );
            mStream = NULL;
        }

		PVDELETE( mPipeline );

        if ( !lResult.IsOK() )
        {
            cout << "Failed to stop acquisition process." <<  endl;
            cout << "    Error: " << lResult.GetDescription() << endl;
            lRetVal = false;
        }
    }
    catch ( ... )
    {
        cout << "Failed to stop acquisition process. Unexpected error." << endl;
        lRetVal = false;
    }

    return lRetVal;
}

bool VideoSource::Disconnect()
{
	bool lRetVal = true;

	if ( mStream != NULL )
	{
		PvResult lResult = PvResult::Code::OK;

		try
		{
			if ( mStream->IsOpen() )
			{
				// Close stream.
				lResult = mStream->Close();
				if ( ( mDevice != NULL ) && lResult.IsOK() )
				{
					if ( mDevice->IsConnected() )
					{
						// Disconnect the device.
						lResult = mDevice->Disconnect();
					}
				}
			}
			if ( !lResult.IsOK() )
			{
				cout << "Failed to disconnect the device." <<  endl;
				cout << "    Error: " << lResult.GetDescription() << endl;
				lRetVal = false;
			}
		}
		catch ( ... )
		{
			cout << "Failed to disconnect the device. Unexpected error." << endl;
			lRetVal = false;
		}
	}

    return lRetVal;
}

bool VideoSource::FillBuffer( PvBuffer *aBuffer, PvTransmitterGEV *aTransmitter )
{
    // Get input buffer from pipeline (stream in).
    PvBuffer *lInBuffer = NULL;
    PvResult lOperationResult;

    // Retrieve next buffer
    PvResult lResult = mPipeline->RetrieveNextBuffer( &lInBuffer, 1000, &lOperationResult );
    if ( !lResult.IsOK() )
    {
        return false;
    }

    if ( lOperationResult.IsOK() )
    {
        // Get image by ID (one for every buffer, attached for the buffer).
        Gdiplus::Bitmap *lImage = GetImageForBuffer( aBuffer );

        if ( lImage == NULL )
        {
            return false;
        }

        uint32_t lWidth = GetWidth();
        uint32_t lHeight = GetHeight();

        // Lock bitmap data.
        Gdiplus::BitmapData lBitmapData;
        Gdiplus::Rect lRect(0, 0, lWidth, lHeight );
        lImage->LockBits( &lRect, Gdiplus::ImageLockModeRead, lImage->GetPixelFormat(), &lBitmapData );

        // Attach image to output buffer with conversion-happy pixel type.
        aBuffer->GetImage()->Attach( (byte*)lBitmapData.Scan0, 
            lWidth, lHeight, PvPixelBGR8 );

        // Copy in buffer into image attached to out buffer using eBUS SDK pixel converters.
        mConverter.Convert( lInBuffer, aBuffer, false );
        lImage->UnlockBits( &lBitmapData );

        // Draw (transform part of the sample) on image attached to out buffer.
        Draw( aTransmitter, lImage );
    }

    // Release buffer back to pipeline.
    mPipeline->ReleaseBuffer( lInBuffer );

    return lOperationResult.IsOK();
}

uint32_t VideoSource::GetWidth()
{
    int64_t lWidth;
    mDevice->GetParameters()->GetIntegerValue( "Width", lWidth );

    return static_cast<uint32_t>( lWidth );
}

uint32_t VideoSource::GetHeight()
{
    int64_t lHeight;
    mDevice->GetParameters()->GetIntegerValue( "Height", lHeight );

    return static_cast<uint32_t>( lHeight );
}

PvPixelType VideoSource::GetPixelFormat()
{
    int64_t lFormat;
    mDevice->GetParameters()->GetEnumValue( "PixelFormat", lFormat );

    return static_cast<PvPixelType>( lFormat );
}

Gdiplus::Bitmap *VideoSource::GetImageForBuffer( PvBuffer *aBuffer )
{
    uint32_t lID = static_cast<uint32_t>( aBuffer->GetID() );
    Gdiplus::Bitmap *lImage = NULL;

    uint32_t lWidth = GetWidth();
    uint32_t lHeight = GetHeight();
    uint32_t lImageSize = aBuffer->GetImage()->GetImageSize();
    uint32_t lBufAcqSize = aBuffer->GetAcquiredSize();

    lImageSize = lWidth * lHeight;

    if ( mMap.find( lID ) == mMap.end() )
    {
        // No buffer found at map position.
    }
    else
    {
        lImage = mMap[ lID ];
        if ( lImage != NULL )
        {
            if ( ( lImage->GetWidth() != lWidth ) ||
                ( lImage->GetHeight() != lHeight ) )
            {
                // Detach from PvBuffer.
                aBuffer->Detach();

                // Image found, but not the right size.
                delete lImage;
                lImage = NULL;
            }
        }
    }

    if ( lImage == NULL )
    {
        // Create a new image of the right size.
        lImage  = new Gdiplus::Bitmap( lWidth, lHeight, PixelFormat24bppRGB );
        mMap[ lID ] = lImage;
    }

    return lImage;
}

void VideoSource::Draw( PvTransmitterGEV *aTransmitter, Gdiplus::Bitmap *aImage )
{
    if ( aImage == NULL )
    {
        return;
    }

    // Create a Graphics object that is associated with the image.
    Gdiplus::Graphics *lImageGraphics = Gdiplus::Graphics::FromImage( aImage );
    if ( lImageGraphics == NULL )
    {
        return;
    }

    HDC lDC = lImageGraphics->GetHDC();
    Gdiplus::Graphics lGraphics( lDC );

    Gdiplus::FontFamily lFontFamily( L"Arial" );
    Gdiplus::Font lFont( &lFontFamily, 24, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel );
    Gdiplus::PointF lPointF( 25.0F, 25.0F );
    Gdiplus::SolidBrush lSolidBrush( Gdiplus::Color( 0x00, 0xFF, 0xFF ) );

    stringstream lSS;

    lSS << "TransmitProcessedImage" << endl;
    lSS << PRODUCT_NAME << " " << NVERSION_STRING << endl << endl;

    lSS << fixed << setprecision( 1 );
    lSS << "Transmitted " << aTransmitter->GetBlocksTransmitted() << " blocks" << endl;

    lSS << fixed << setprecision( 1 );
    lSS << aTransmitter->GetAverageTransmissionRate() << " (" << aTransmitter->GetInstantaneousTransmissionRate() << ") FPS" << endl;

    lSS << fixed << setprecision( 1 );
    lSS << aTransmitter->GetAveragePayloadThroughput() / 1000000.0f <<
        " (" << aTransmitter->GetInstantaneousPayloadThroughput() / 1000000.0f << ") Mb/s" << endl;

    PvString lString( lSS.str().c_str() );
    lGraphics.DrawString( lString.GetUnicode(), lString.GetLength(), &lFont, lPointF, &lSolidBrush );

    lImageGraphics->ReleaseHDC( lDC );
    delete lImageGraphics;
}



