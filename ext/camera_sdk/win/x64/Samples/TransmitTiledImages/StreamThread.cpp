// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"

#include "ConnectionThread.h"
#include "StreamThread.h"
#include "Constants.h"
#include "SetupDlg.h"
#include "StreamDeviceFinderWnd.h"
#include "PvDeviceInfoGEV.h"
#include "PvDeviceInfoU3V.h"
#include "PvDeviceU3V.h"
#include "PvStreamU3V.h"

StreamThread::StreamThread()
    : mDevice( NULL )
	, mStream( NULL )
	, mMainWnd( NULL ) 
    , mRow( -1 )
    , mColumn( -1 )
    , mCurrentBuffersTable( NULL )
    , mStart( NULL )
    , mStop( NULL )
    , mConnected( false )
    , mLastPayloadSize( 0 )
{
    memset( mBuffers, 0, sizeof( SmartBuffer* ) * RX_POOL_SIZE );
}

bool StreamThread::Initialize( CWnd* aMainWnd, int32_t aRow, int32_t aColumn, CurrentBuffersTable* aCurrentBuffersTable )
{
    mMainWnd = aMainWnd;
    mRow = aRow;
    mColumn = aColumn;
    mCurrentBuffersTable = aCurrentBuffersTable;

    return mReturnBufferQueues.Initialize( RX_POOL_SIZE + 1 );
}

StreamThread::~StreamThread()
{
    Disconnect();
}

PvResult StreamThread::Connect( const CString &aConnectionID, CWnd *aParent )
{
    StreamDeviceFinderWnd lFinder( aConnectionID );
    lFinder.SetTitle( _T( "Device Selection" ) );
    PvResult lResult = lFinder.ShowModal();
    if ( !lResult.IsOK() )
    {
        return PvResult::Code::CANCEL;
    }

    const PvDeviceInfo *lDeviceInfo = lFinder.GetSelected();
    SetupDlg lSetupDlg( &mSetup, lDeviceInfo->GetType() );
    if ( lSetupDlg.DoModal() != IDOK )
    {
        return PvResult::Code::CANCEL;
    }
	
    CWaitCursor lCursor;

    // Just in case we came here still connected...
    Disconnect();

	// Acquire new stream and device
	switch ( lDeviceInfo->GetType() )
	{
		case ( PvDeviceInfoTypeGEV ):
			mDevice = new PvDeviceGEV();
			mStream = new PvStreamGEV();
			break;

		case ( PvDeviceInfoTypeU3V ):
			mDevice = new PvDeviceU3V();
			mStream = new PvStreamU3V();
			break;

		default:
            ASSERT( FALSE );
			break;
	}

    // Device connection, packet size negotiation and stream opening is performed in a separate 
    // thread while we display a progress dialog to the user
    ConnectionThread lConnectionThread( &mSetup, lDeviceInfo, mDevice, mStream, NULL );
    lResult = lConnectionThread.Connect();
    if ( !lResult.IsOK() )
    {
        Disconnect();
        return lResult;
    }

	const PvDeviceInfoGEV *lDeviceInfoGEV = dynamic_cast<const PvDeviceInfoGEV *>( lDeviceInfo );
	const PvDeviceInfoU3V *lDeviceInfoU3V = dynamic_cast<const PvDeviceInfoU3V *>( lDeviceInfo );
	if ( lDeviceInfoGEV != NULL )
	{
		mIPAddress = lDeviceInfoGEV->GetIPAddress().GetUnicode();
		mMACAddress = lDeviceInfoGEV->GetMACAddress().GetUnicode();
		mSerialNumber.Empty();
	}
	else if ( lDeviceInfoU3V != NULL )
	{
		mSerialNumber = lDeviceInfoU3V->GetU3VSerialNumber().GetUnicode();
		mIPAddress.Empty();
		mMACAddress.Empty();
	}
    else
    {
        ASSERT( FALSE );
    }

    mModel = lDeviceInfo->GetModelName().GetUnicode();
    mUserDefinedName = lDeviceInfo->GetUserDefinedName().GetUnicode();

    if ( mDevice->IsConnected() )
    {
        // Register PvDevice callbacks
        mDevice->RegisterEventSink( this );
    }

    mConnected = true;
    return PvResult::Code::OK;
}

void StreamThread::Disconnect()
{
    mCommunicationBrowser.Close();
    mDeviceBrowser.Close();
    mStreamBrowser.Close();

	if ( mDevice != NULL )
	{
		mDevice->UnregisterEventSink( this );
	}

    Stop();

	if ( mDevice != NULL )
	{
		if ( mDevice->IsConnected() )
		{
			mDevice->Disconnect();
		}

        delete mDevice;
        mDevice = NULL;
	}

	if ( mStream != NULL )
	{
		if ( mStream->IsOpen() )
		{
			mStream->Close();
		}

        delete mStream;
        mStream = NULL;
	}

    // Clear the used buffers memory
    if ( mLastPayloadSize != 0 )
    {
        for( int i = 0; i < RX_POOL_SIZE; i++ )
        {
            delete mBuffers[ i ];
            mBuffers[ i ] = NULL;
        }
        mLastPayloadSize = 0;
    }

    mConnected = false;
    
    mMACAddress.Empty();
    mIPAddress.Empty();
	mSerialNumber.Empty();
    mModel.Empty();
    mUserDefinedName.Empty();
}

bool StreamThread::Configure( unsigned int& aWidth, unsigned int& aHeight )
{
    uint32_t lPayloadSize;
    PvResult lResult;
    PvBuffer lBuffer;
    PvGenParameterArray* lDeviceParams;
    PvGenCommand* lResetStats;

    // Get access to the parameters
    lDeviceParams = mDevice->GetParameters();
    mStart = dynamic_cast<PvGenCommand *>( lDeviceParams->Get( "AcquisitionStart" ) );
    mStop = dynamic_cast<PvGenCommand *>( lDeviceParams->Get( "AcquisitionStop" ) );
    lResetStats = dynamic_cast<PvGenCommand *>( mStream->GetParameters()->Get( "Reset" ) );

    // Read width and height
    int64_t lWidth = 0; lDeviceParams->GetIntegerValue( "Width", lWidth );
    int64_t lHeight = 0; lDeviceParams->GetIntegerValue( "Height", lHeight );
    aWidth = static_cast<unsigned int>( lWidth );
    aHeight = static_cast<unsigned int>( lHeight );

    // Reset the statistic
    lResetStats->Execute();

    if( mSetup.GetRole() == ROLE_DATA )
    {
        if( mLastPayloadSize == 0 )
        {
            // In data receiver only mode, we do not know what will be the image size 
            // For this reason, we will re-allocate the image size on the fly. This is
            // bad but the only way we can make it work
            lResult = CreateNewBuffers( DEFAULT_RX_BUFFER_ALLOCATION_SIZE );
            if ( !lResult )
            {
                return false;
            }

            // We need to cache the payload size for future starts...
            mLastPayloadSize = DEFAULT_RX_BUFFER_ALLOCATION_SIZE;
        }
    }
    else
    {
        // Get the current configured payload size
        lPayloadSize = mDevice->GetPayloadSize();
        if ( lPayloadSize == 0 )
        {
            // We cannot get the size, the buffer cannot be pre-allocated
            return false;
        }

        // Manage the size of the buffer allocated to receive the images
        if ( mLastPayloadSize == 0 )
        {
            // This is a case where no buffers were allocated
            lResult = CreateNewBuffers( lPayloadSize );
            if ( !lResult )
            {
                return false;
            }

            // We need to cache the payload size for future starts...
            mLastPayloadSize = lPayloadSize;
        }
        else if ( mLastPayloadSize != lPayloadSize )
        {
            // The buffer need to be resized case

            // Reallocate the memory of each of the buffer to be able to 
            // deal with the new payload size
            for ( int i = 0; i < RX_POOL_SIZE; i++ )
            {
                // Recalling Alloc will make a re-allocate in this case
                lResult = mBuffers[ i ]->Alloc( lPayloadSize );
                if ( !lResult.IsOK() )
                {
                    for ( int j = 0; j <= i; j++ )
                    {
                        delete mBuffers[ j ];
                        mBuffers[ j ] = NULL;
                    }

                    // Set mLastPayloadSize to 0 to ensure a clean re-allocation of the memory
                    mLastPayloadSize = 0;

                    return false;
                }
            }
            
            // We need to cache the payload size for future starts...
            mLastPayloadSize = lPayloadSize;
        }
    }

    // Queue the buffers into the stream
    for ( int i = 0; i < RX_POOL_SIZE; i++ )
    {
        mStream->QueueBuffer( ( PvBuffer* ) mBuffers[ i ] );            
    }

    // Now configure the camera to be able to start streaming
    return true;
}

bool StreamThread::Start()
{
	ASSERT( mDevice != NULL );

    bool lReturn = true;

    if( mSetup.GetRole() != ROLE_DATA )
    {
		mDevice->StreamEnable();

        // Start the internal thread to retrieve the data
        lReturn = Thread::Start();
        if( !lReturn )
        {
            return false;
        }

        // Start the stream
        if( mStart )
        {
            mStart->Execute();
        }
    }
    else
    {
        // Start the internal thread to retrieve the data
        lReturn = Thread::Start();
        if( !lReturn )
        {
            return false;
        }
    }

    return true;
}

bool StreamThread::Stop()
{
    PvBuffer *lBuffer = NULL;
    PvResult lOperationResult;

    if( ( mDevice != NULL ) && ( mSetup.GetRole() != ROLE_DATA ) )
    {
		if ( !mDevice->IsConnected() )
		{        
			return true;
		}

        // Stop the streaming on the device
        if ( mStop )
        {
            mStop->Execute();
        }

		mDevice->StreamDisable();
    }

    // Stop the thread retrieving the data
    Thread::Stop();

    // Abort the stream and dequeue all the buffers
	if ( mStream != NULL )
	{
		mStream->AbortQueuedBuffers();
		while ( mStream->RetrieveBuffer( &lBuffer, &lOperationResult, 0 ).IsOK() );
	}

    // We want to recover all the buffers, including the one
    // in the display snapshot...
    mCurrentBuffersTable->Reset( mRow, mColumn );
    // This is emptying the ready queue...
    while ( mReturnBufferQueues.Pop() );

    return true;
}

DWORD StreamThread::Function()
{
    PvBuffer *lBuffer = NULL;
    PvImage* lImage;
    SmartBuffer* lSmartBuffer = NULL;
    PvResult lOperationResult;
    PvResult lResult;

    while ( !IsStopping() )
    {
        lResult = mStream->RetrieveBuffer( &lBuffer, &lOperationResult, 50 );
        if ( lResult.IsOK() )
        {
            if ( lOperationResult.IsOK() )
            {               
                // We will keep this image as the last acquired.
                // The mCurrentBuffersTable will manage the return of the buffer into the stream
                mCurrentBuffersTable->Set( ( SmartBuffer* ) lBuffer, mRow, mColumn );
                lBuffer = NULL;
            }
            else if ( lOperationResult.GetCode() == PvResult::Code::BUFFER_TOO_SMALL )
            {
                // The buffer is too small to retrieve the image from the source
                // we will need to re-allocate the image to fit the 
                lImage = lBuffer->GetImage();
                if ( !lImage )
                {
                    mMainWnd->PostMessage( WM_REALLOCATIONFAIL, ( WPARAM ) mRow, ( LPARAM ) mColumn );
                    return 0;
                }

                mLastPayloadSize = lImage->GetRequiredSize();
                lResult = lBuffer->Alloc( mLastPayloadSize );
                if ( !lResult.IsOK() )
                {
                    mMainWnd->PostMessage( WM_REALLOCATIONFAIL, ( WPARAM ) mRow, ( LPARAM ) mColumn );
                    return 0;
                }
                mStream->QueueBuffer( lBuffer );
                lBuffer = NULL;
            }
            else
            {
                mStream->QueueBuffer( lBuffer );
                lBuffer = NULL;
            }
        }

        // Requeue any available buffer
        while ( lSmartBuffer = mReturnBufferQueues.Pop() )
        {
            mStream->QueueBuffer( ( PvBuffer* ) lSmartBuffer );
        }        
    }

    return 0;
}

bool StreamThread::CreateNewBuffers( unsigned int aPayloadSize )
{
    PvResult lResult;

    // First allocate all the buffers pool
    for( int i = 0; i < RX_POOL_SIZE; i++ )
    {
        mBuffers[ i ] = new SmartBuffer( &mReturnBufferQueues );
        if( !mBuffers[ i ] )
        {
            if ( i )
            {
                for( int j = 0; j < i; j++ )
                {
                    delete mBuffers[ j ];
                    mBuffers[ j ] = NULL;
                }
            }
            return false;
        }
    }

    // Now we can set the size of the buffers
    for( int i = 0; i < RX_POOL_SIZE; i++ )
    {
        lResult = mBuffers[ i ]->Alloc( aPayloadSize );
        if( !lResult.IsOK() )
        {
            for( int j = 0; j <= i; j++ )
            {
                delete mBuffers[ j ];
                mBuffers[ j ] = NULL;
            }
            return false;
        }
    }
    
    return true;
}


///
/// \brief displays the communication parameters browser
///

void StreamThread::ShowCommunication()
{
    CString lTemp;
    lTemp.Format( _T( "Source %d Communication Parameters" ), ( mRow + 1 ) * ( mColumn + 1 ) );

    mCommunicationBrowser.SetTitle( (LPCTSTR)lTemp );
    mCommunicationBrowser.SetGenParameterArray( mDevice->GetCommunicationParameters() );
    mCommunicationBrowser.ShowModeless( mMainWnd->GetSafeHwnd() );
}


///
/// \brief displays the device parameters browser
///

void StreamThread::ShowDevice()
{
    if ( !mDevice->IsConnected() )
    {
        return;
    }

    CString lTemp;
    lTemp.Format( _T( "Source %d Device Parameters" ), ( mRow + 1 ) * ( mColumn + 1 ) );

    mDeviceBrowser.SetTitle( (LPCTSTR)lTemp );
    mDeviceBrowser.SetGenParameterArray( mDevice->GetParameters() );
    mDeviceBrowser.ShowModeless( mMainWnd->GetSafeHwnd() );
}


///
/// \brief displays the stream parameters browser
///

void StreamThread::ShowStream()
{
    if ( !mStream->IsOpen() )
    {
        return;
    }

    CString lTemp;
    lTemp.Format( _T( "Source %d Stream Parameters" ), ( mRow + 1 ) * ( mColumn + 1 ) );

    mStreamBrowser.SetTitle( (LPCTSTR)lTemp );
    mStreamBrowser.SetGenParameterArray( mStream->GetParameters() );
    mStreamBrowser.ShowModeless( mMainWnd->GetSafeHwnd() );
}


void StreamThread::OnLinkDisconnected( PvDevice *aDevice )
{
    // Notify the main application that the connection is lost
    mMainWnd->PostMessage( WM_LINKDISCONNECTED, ( WPARAM ) mRow, ( LPARAM ) mColumn );
}

bool StreamThread::Store( PvConfigurationWriter& aWriter )
{
    CString lTemp;

    if( mConnected )
    {
        // Store the device
        if( mSetup.GetRole() == ROLE_CTRLDATA )
        {
            lTemp.Format( _T( "Device( %d, %d )" ), mRow, mColumn );
            aWriter.Store( mDevice, lTemp.GetBuffer() );
        }

        // Store the stream
        lTemp.Format( _T( "Stream( %d, %d )" ), mRow, mColumn );
        aWriter.Store( mStream, lTemp.GetBuffer() );

        // Store the setup information
        mSetup.Store( aWriter, mRow, mColumn );      
    }

    return true;
}

bool StreamThread::Restore( PvConfigurationReader& aReader )
{
    CString lTemp;

    if( mConnected )
    {
        Disconnect();
    }

    // Read the setup information first to understand how to deal with the 
    // stream and the device
    PvResult lRestored = mSetup.Restore( aReader, mRow, mColumn );
    if ( !lRestored.IsOK() )
    {
        return false;
    }
    
    if ( mSetup.GetDestination() != DESTINATION_INVALID )
    {
        // A destination mode is saved to every GEV device
        mDevice = new PvDeviceGEV();
        mStream = new PvStreamGEV();
    }
    else
    {
        // No destination mode: device MUST be U3V
        mDevice = new PvDeviceU3V();
        mStream = new PvStreamU3V();
    }

    switch( mSetup.GetRole() )
    {
		case ROLE_CTRLDATA:
			lTemp.Format( _T( "Device( %d, %d )" ), mRow, mColumn );
			if( !aReader.Restore( lTemp.GetBuffer(), mDevice ).IsOK() )
			{
				return false;
			}

		case ROLE_DATA:
			lTemp.Format( _T( "Stream( %d, %d )" ), mRow, mColumn );
			if( !aReader.Restore( lTemp.GetBuffer(), mStream ).IsOK() )
			{
				mDevice->Disconnect();
				return false;
			}
			break;

		default:
			return false;
			break;
    }
	
	PvStreamGEV *lStreamGEV = dynamic_cast<PvStreamGEV *>( mStream );
	PvDeviceGEV *lDeviceGEV = dynamic_cast<PvDeviceGEV *>( mDevice );

	if ( ( lStreamGEV != NULL ) && ( lDeviceGEV != NULL ) )
	{
		if ( mSetup.GetRole() == ROLE_CTRLDATA )
		{
			switch( mSetup.GetDestination() )
			{
				case DESTINATION_UNICAST_AUTO:

				case DESTINATION_UNICAST_SPECIFIC:
					lDeviceGEV->SetStreamDestination( lStreamGEV->GetLocalIPAddress(),  lStreamGEV->GetLocalPort() );
					break;

				case DESTINATION_MULTICAST:
					lDeviceGEV->SetStreamDestination( mSetup.GetIPAddress().GetBuffer(), mSetup.GetPort() );
					break;
			}
		}
	}

    // Now we will retrieve the device information to be able to display it
    PvGenParameterArray *lGenDevice = mDevice->GetParameters();

	if ( lDeviceGEV != NULL )
	{
		// IP
		PvGenInteger *lIPAddressParam = lGenDevice->GetInteger( "GevCurrentIPAddress" );
		int64_t lIPAddress = 0;
		lIPAddressParam->GetValue( lIPAddress );
		unsigned char *lIPPtr = reinterpret_cast<unsigned char *>( &lIPAddress );
		mIPAddress.Format( _T( "%i.%i.%i.%i" ), 
			lIPPtr[3], lIPPtr[2], lIPPtr[1], lIPPtr[0] );

		// MAC address
		PvGenInteger *lMACAddressParam = lGenDevice->GetInteger( "GevMACAddress" );
		int64_t lMACAddress;
		lMACAddressParam->GetValue( lMACAddress );
		unsigned char *lMACPtr = reinterpret_cast<unsigned char *>( &lMACAddress );
		mMACAddress.Format( _T( "%02X:%02X:%02X:%02X:%02X:%02X" ), 
			lMACPtr[5], lMACPtr[4], lMACPtr[3], lMACPtr[2], lMACPtr[1], lMACPtr[0] );
	}
	else if ( dynamic_cast<PvDeviceU3V *>( mDevice ) != NULL )
	{
		// Serial number
		PvGenString *lSerialNumberParam = lGenDevice->GetString( "DeviceSerialNumber" );
		PvString lSerialNumber;
		lSerialNumberParam->GetValue( lSerialNumber );
		mSerialNumber.Format( _T( "%d" ), lSerialNumber );
	}

    // Model name
    PvGenString *lModelName = lGenDevice->GetString( "DeviceModelName" );
    PvString lModelNameStr;
    lModelName->GetValue( lModelNameStr );
    mModel = lModelNameStr.GetUnicode();

    // Device name ( User ID )
    PvGenString *lNameParam = lGenDevice->GetString( "GevDeviceUserID" );
    if ( lNameParam != NULL )
    {
        PvString lStr;
        lNameParam->GetValue( lStr );
        mUserDefinedName = lStr.GetUnicode();
    }

    mConnected = true;
    return true;
}
