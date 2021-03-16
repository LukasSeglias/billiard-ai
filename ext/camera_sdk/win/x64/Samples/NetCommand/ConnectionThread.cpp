// *****************************************************************************
//
// Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// -----------------------------------------------------------------------------
//
// Thread responsible of connecting a device and opening a stream. Performed
// in a thread in order not to lock the UI while this is happening. This allows
// the UI to stay responsive.
// 
// *****************************************************************************

#include "stdafx.h"
#include "ConnectionThread.h"

#include <PvNetworkAdapter.h>


// ==========================================================================
ConnectionThread::ConnectionThread( Setup *aSetup, const PvDeviceInfoGEV *aDeviceInfo, PvDeviceGEV *aDevice, PvStreamGEV *aStream, CWnd* aParent )
        : mSetup( aSetup )
        , mDeviceInfo( aDeviceInfo )
        , mDevice( aDevice )
        , mStream( aStream )
{
    mDlg = new ProgressDlg( this, aParent );
}

// ==========================================================================
ConnectionThread::~ConnectionThread()
{
    if ( mDlg != NULL )
    {
        delete mDlg;
        mDlg = NULL;
    }
}

// ==========================================================================
PvResult ConnectionThread::Connect()
{
    ASSERT( mDlg != NULL );
    mDlg->DoModal();

    return mResult;
}

// ==========================================================================
DWORD ConnectionThread::Function()
{
    PvResult lResult = PvResult::Code::NOT_CONNECTED;

    const PvNetworkAdapter *lNetworkAdapter = dynamic_cast<const PvNetworkAdapter *>( mDeviceInfo->GetInterface() );

    try
    {
        if ( ( mSetup->GetRole() == Setup::RoleCtrlData ) ||
             ( mSetup->GetRole() == Setup::RoleCtrl ) )
        {
            // Connect device
            mDlg->SetStatus( _T( "Building GenICam interface..." ) );
            lResult = mDevice->Connect( mDeviceInfo, PvAccessControl );
            if ( !lResult.IsOK() )
            {
                mResult = lResult;
                return lResult.GetCode();
            }

            if ( mSetup->GetRole() == Setup::RoleCtrlData )
            {
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

                if ( lEnabledValue )
                {
                    // Perform automatic packet size negociation
                    mDlg->SetStatus( _T( "Optimizing streaming packet size..." ) );
                    lResult = mDevice->NegotiatePacketSize( 0, 1476 );
                    if ( !lResult.IsOK() )
                    {
                        mDlg->SetStatus( _T( "WARNING: streaming packet size optimization failure, using 1476 bytes!" ) );
                        ::Sleep( 3000 );
                    }
                }
                else
                {
                    bool lManualPacketSizeSuccess = false;

                    // Start by figuring out if we can use GenICam to set the packet size
                    bool lUseGenICam = false;
                    PvGenInteger *lPacketSize = dynamic_cast<PvGenInteger *>( mDevice->GetParameters()->Get( "GevSCPSPacketSize" ) );
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
                        PvResult lResult = mDevice->WriteRegister( 0x0D04, (uint32_t)lUserPacketSizeValue );
                        if ( lResult.IsOK() )
                        {
                            lManualPacketSizeSuccess = true;
                        }
                        else
                        {
                            // Last resort default...
                            mDevice->WriteRegister( 0x0D04, 576 );
                            lManualPacketSizeSuccess = false;
                        }
                    }
                    
                    CString lNewStr;
                    if ( lManualPacketSizeSuccess )
                    {
                        lNewStr.Format( 
                            _T( "A packet of size of %i bytes was configured for streaming. You may experience issues " )
                            _T( "if your system configuration cannot support this packet size." ), 
                            lUserPacketSizeValue);
                    }
                    else
                    {                        
                        lNewStr.Format( _T( "WARNING: could not set streaming packet size to %i bytes, using %i bytes!" ),
                                       lUserPacketSizeValue, 576 );
                    }
                    mDlg->SetStatus( lNewStr );
                    ::Sleep( 3000 );
                }
            }
        }

        // Open stream
        if ( ( mSetup->GetRole() == Setup::RoleCtrlData ) ||
             ( mSetup->GetRole() == Setup::RoleData ) )
        {
            mDlg->SetStatus( _T( "Opening eBUS stream to device..." ) );

            // Open stream
            if ( mSetup->GetDestination() == Setup::DestinationUnicastAuto )
            {
                lResult = mStream->Open( mDeviceInfo->GetIPAddress() );
            }
            else if ( mSetup->GetDestination() == Setup::DestinationUnicastSpecific )
            {
                lResult = mStream->Open( mDeviceInfo->GetIPAddress(), mSetup->GetUnicastSpecificPort() );
            }
            else if ( mSetup->GetDestination() == Setup::DestinationMulticast )
            {
                lResult = mStream->Open( mDeviceInfo->GetIPAddress(), (LPCTSTR)mSetup->GetMulticastIP(), mSetup->GetMulticastPort() );
            }
            else
            {
                ASSERT( 0 );
            }

            if ( !lResult.IsOK() )
            {
                mResult = lResult;
                return lResult.GetCode();
            }
        }

        // Now that the stream is opened, set the destination on the device
        if ( ( mSetup->GetRole() == Setup::RoleCtrlData ) ||
             ( mSetup->GetRole() == Setup::RoleCtrl ) )
        {
            if ( ( mSetup->GetDestination() == Setup::DestinationUnicastAuto ) ||
                 ( mSetup->GetDestination() == Setup::DestinationUnicastSpecific ) )
            {
                mDevice->SetStreamDestination( 
                    mStream->GetLocalIPAddress(), 
                    mStream->GetLocalPort() );
            }
            else if ( mSetup->GetDestination() == Setup::DestinationUnicastOther )
            {
                // Bug 2132: only set destination if different than 0.0.0.0:0
                if ( ( mSetup->GetUnicastIP() != "0.0.0.0" ) &&
                     ( mSetup->GetUnicastPort() != 0 ) )
                {
                    mDevice->SetStreamDestination( 
                        (LPCTSTR)mSetup->GetUnicastIP(), 
                        mSetup->GetUnicastPort() );
                }
            }
            else if ( mSetup->GetDestination() == Setup::DestinationMulticast )
            {
                mDevice->SetStreamDestination( 
                    (LPCTSTR)mSetup->GetMulticastIP(), 
                    mSetup->GetMulticastPort() );
            }
        }
    }
    catch ( ... )
    {
        lResult = PvResult( PvResult::Code::ABORTED, "Unexpected error" );
    }

    mResult = lResult;
    return lResult.GetCode();
}

