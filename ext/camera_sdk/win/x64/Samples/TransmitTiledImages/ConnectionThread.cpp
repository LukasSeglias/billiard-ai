// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"

#include "ConnectionThread.h"
#include "Constants.h"

#include <PvNetworkAdapter.h>
#include <PvDeviceInfoU3V.h>
#include <PvStreamU3V.h>


ConnectionThread::ConnectionThread( Setup *aSetup, const PvDeviceInfo *aDeviceInfo,
	PvDevice *aDevice, PvStream *aStream, CWnd* aParent )
	: mSetup( aSetup )
	, mDeviceInfo( aDeviceInfo )
	, mDevice( aDevice )
	, mStream( aStream )
	, mProgressDialog( NULL )
{
	mProgressDialog = new ProgressDlg( this, aParent );
}

ConnectionThread::~ConnectionThread()
{
	PVDELETE( mProgressDialog );
}

PvResult ConnectionThread::Connect()
{
	mProgressDialog->DoModal();

	return mResult;
}

DWORD ConnectionThread::Function()
{
	try
	{
		// Connect the device if any device required
		if ( mSetup->GetRole() == ROLE_CTRLDATA )
		{
			mResult = ConnectDevice();
			if ( !mResult.IsOK() )
			{
				return 0;
			}
		}

		// Open stream - and retry if it fails
		mResult = OpenStream();
		if ( !mResult.IsOK() )
		{
			return 0;
		}
		
		PvDeviceGEV *lDeviceGEV = dynamic_cast<PvDeviceGEV *>( mDevice );
		PvStreamGEV *lStreamGEV = dynamic_cast<PvStreamGEV *>( mStream );

		if ( ( mSetup->GetRole() == ROLE_CTRLDATA )
          && ( lDeviceGEV != NULL )
          && ( lStreamGEV != NULL ) )
		{
			// Now we need to make the link between the thread and the stream
			switch ( mSetup->GetDestination() )
			{
				case DESTINATION_UNICAST_AUTO:

				case DESTINATION_UNICAST_SPECIFIC:
					lDeviceGEV->SetStreamDestination( lStreamGEV->GetLocalIPAddress(), lStreamGEV->GetLocalPort() );
					break;

				case DESTINATION_MULTICAST:
					lDeviceGEV->SetStreamDestination( mSetup->GetIPAddress().GetBuffer(), mSetup->GetPort() );
					break;
			}
		}
	}
	catch ( ... )
	{
		mResult = PvResult( PvResult::Code::ABORTED, "Unexpected error" );
	}
	return 0;
}

PvResult ConnectionThread::ConnectDevice()
{
	PvResult lResult;

	mProgressDialog->SetStatus( _T( "Building GenICam interface..." ) );
	lResult = mDevice->Connect( mDeviceInfo );
	if ( !lResult.IsOK() )
	{
		return lResult.GetCode();
	}

	bool lEnabledValue = true;
	PvGenBoolean *lEnabled = dynamic_cast<PvGenBoolean *>( mDevice->GetCommunicationParameters()->Get( "AutoNegotiation" ) );
	if ( lEnabled != NULL )
	{
		lEnabled->GetValue( lEnabledValue );
	}

	int64_t lUserPacketSizeValue = 1476;
	PvGenInteger *lUserPacketSize = dynamic_cast<PvGenInteger *>( mDevice->GetCommunicationParameters()->Get( "DefaultPacketSize" ) );
	if ( ( lUserPacketSize != NULL ) && lUserPacketSize->IsReadable() )
	{
		lUserPacketSize->GetValue( lUserPacketSizeValue );
	}
    
	PvDeviceGEV *lDeviceGEV = dynamic_cast<PvDeviceGEV *>( mDevice );
	if ( ( lDeviceGEV != NULL ) && lEnabledValue )
	{
		// Perform automatic packet size negociation
		mProgressDialog->SetStatus( _T( "Optimizing streaming packet size..." ) );
		lResult = lDeviceGEV->NegotiatePacketSize( 0, 1476 );

		if ( !lResult.IsOK() )
		{
			mProgressDialog->SetStatus( _T( "WARNING: streaming packet size optimization failure, using 1476 bytes!" ) );
			::Sleep( 3000 );
		}
	}
	else if ( lDeviceGEV != NULL )
	{
		bool lManualPacketSizeSuccess = false;

		// Start by figuring out if we can use GenICam to set the packet size
		bool lUseGenICam = false;
		PvGenInteger *lPacketSize = dynamic_cast<PvGenInteger *>( mDevice->GetCommunicationParameters()->Get( "GevSCPSPacketSize" ) );
		if ( lPacketSize != NULL )
		{
			bool lWritable = false;
			PvResult lResult = lPacketSize->IsWritable( lWritable );
			if ( lResult.IsOK() && lWritable )
			{
				lUseGenICam = true;
			}
		}

		if ( lUseGenICam )
		{
			// Setting packet size through the GenICam interface
			PvResult lResult = lPacketSize->SetValue( lUserPacketSizeValue );
			if ( lResult.IsOK() )
			{
				lManualPacketSizeSuccess = true;
			}
			else
			{
				// Last resort default...
				lPacketSize->SetValue( 576 );
				lManualPacketSizeSuccess = false;
			}
		}
		else
		{
			// Direct write, for GenICam challenged devices...
			PvResult lResult = lDeviceGEV->WriteRegister( 0x0D04, (uint32_t) lUserPacketSizeValue );
			if ( lResult.IsOK() )
			{
				lManualPacketSizeSuccess = true;
			}
			else
			{
				// Last resort default...
				lDeviceGEV->WriteRegister( 0x0D04, 576 );
				lManualPacketSizeSuccess = false;
			}
		}

		CString lNewStr;
		if ( lManualPacketSizeSuccess )
		{
			lNewStr.Format(
				_T( "A packet of size of %i bytes was configured for streaming. You may experience issues " )
				_T( "if your system configuration cannot support this packet size." ),
				lUserPacketSizeValue );
		}
		else
		{
			lNewStr.Format( _T( "WARNING: could not set streaming packet size to %i bytes, using %i bytes!" ),
				lUserPacketSizeValue, 576 );
		}
		mProgressDialog->SetStatus( lNewStr );
		::Sleep( 3000 );
	}

	return PvResult::Code::OK;
}

PvResult ConnectionThread::OpenStream()
{
	PvResult lResult;

	mProgressDialog->SetStatus( _T( "Opening eBUS stream to device..." ) );

	if ( mDeviceInfo->GetType() == PvDeviceInfoTypeGEV )
	{
		const PvDeviceInfoGEV *lDeviceInfoGEV = static_cast<const PvDeviceInfoGEV *>( mDeviceInfo );
		PvStreamGEV *lStreamGEV = static_cast<PvStreamGEV *>( mStream );
		const PvNetworkAdapter *lNetworkAdapter = static_cast<const PvNetworkAdapter *>( mDeviceInfo->GetInterface() );

		switch ( mSetup->GetDestination() )
		{
			case DESTINATION_UNICAST_AUTO:
				lResult = lStreamGEV->Open( lDeviceInfoGEV->GetIPAddress() );
				break;

			case DESTINATION_UNICAST_SPECIFIC:
				lResult = lStreamGEV->Open( lDeviceInfoGEV->GetIPAddress(), mSetup->GetPort() );
				break;

			case DESTINATION_MULTICAST:
				lResult = lStreamGEV->Open( lDeviceInfoGEV->GetIPAddress(), mSetup->GetIPAddress().GetBuffer(), mSetup->GetPort() );
				break;

			default:
				ASSERT( FALSE );
				break;
		}
	}
	else if ( mDeviceInfo->GetType() == PvDeviceInfoTypeU3V )
	{
		const PvDeviceInfoU3V *lDeviceInfoU3V = static_cast<const PvDeviceInfoU3V *>( mDeviceInfo );
		PvStreamU3V *lStreamU3V = static_cast<PvStreamU3V *>( mStream );
		lResult = lStreamU3V->Open( lDeviceInfoU3V );
	}
    else
    {
        lResult = PvResult::Code::GENERIC_ERROR;
    }

	return lResult;
}
