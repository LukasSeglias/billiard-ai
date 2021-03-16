// *****************************************************************************
//
// Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// -----------------------------------------------------------------------------
//
// The document of the MDI application. Contains a device, stream and everything
// required to hold them together. There is one document for each entity in
// the NetCommand projet.
// 
// *****************************************************************************


#include "stdafx.h"
#include "NetCommand.h"

#include "NetCommandDoc.h"
#include "ConnectionThread.h"
#include "PvMessageBox.h"
#include "ThreadDisplay.h"
#include "Messages.h"
#include "SetupDlg.h"

#include <PvDeviceFinderWnd.h>
#include <PvConfigurationReader.h>
#include <PvConfigurationWriter.h>


#define BUFFER_SIZE ( 1024 )


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Default PvDevice. Never connected, only used to hold default communication parameters
extern PvDeviceGEV sDefaultDevice;


IMPLEMENT_DYNCREATE(NetCommandDoc, CDocument)

BEGIN_MESSAGE_MAP(NetCommandDoc, CDocument)
END_MESSAGE_MAP()


// ==========================================================================
NetCommandDoc::NetCommandDoc()
    : mPipeline( &mStream )
    , mThreadDisplay( NULL )
    , mRole( RoleInvalid )
    , mMainWnd( NULL )
{
    // We save this pointer, as we cannot retrieve it from PvDevice callbacks
    // comming from another thread context
    mMainWnd = ::AfxGetMainWnd();
    ASSERT( mMainWnd != NULL );
}

// ==========================================================================
NetCommandDoc::~NetCommandDoc()
{
    if ( mDevice.IsConnected() || mStream.IsOpen() )
    {
        OnCloseDocument();
    }
}

// ==========================================================================
BOOL NetCommandDoc::OnNewDocument()
{
    if ( !CDocument::OnNewDocument() )
    {
        return FALSE;
    }

    PvDeviceFinderWnd lFinder;
    lFinder.SetTitle( _T( "Device Selection" ) );

    PvResult lResult = lFinder.ShowModal();
    const PvDeviceInfoGEV *lDeviceInfoGEV = dynamic_cast<const PvDeviceInfoGEV *>( lFinder.GetSelected() );
    if ( !lResult.IsOK() || ( lFinder.GetSelected() == NULL ) )
    {
        return FALSE;
    }

    if ( lDeviceInfoGEV == NULL )
    {
        MessageBox( mMainWnd->GetSafeHwnd(), _T( "This application only supports GigE Vision devices." ), _T( "NetCommand" ), MB_ICONERROR | MB_OK );
        return FALSE;
    }

    SetupDlg lDlg;
    lDlg.SetSetup( &mSetup );
    lDlg.SetDevice( lDeviceInfoGEV );
    if ( lDlg.DoModal() != IDOK )
    {
        return FALSE;
    }

    mSetup = *( lDlg.GetSetup() );

    CWaitCursor lCursor;
    Connect( lDeviceInfoGEV );

    ::AfxGetMainWnd()->SendMessage( WM_NEWDOCUMENT, reinterpret_cast<WPARAM>( this ) );

    m_bAutoDelete = FALSE;

    return TRUE;
}

// ==========================================================================
void NetCommandDoc::OnCloseDocument()
{
    CWaitCursor lCursor;

    ::AfxGetMainWnd()->SendMessage( WM_DOCUMENTCLOSING, reinterpret_cast<WPARAM>( this ) );
    Disconnect();

    CDocument::OnCloseDocument();
}

// ==========================================================================
void NetCommandDoc::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        // TODO: add storing code here
    }
    else
    {
        // TODO: add loading code here
    }
}

// ==========================================================================
void NetCommandDoc::Connect( const PvDeviceInfoGEV *aDI )
{
    ASSERT( aDI != NULL );
    if ( aDI == NULL )  
    {
        return;
    }

    // Just in case we came here still connected...
    Disconnect();

    // Propagate default com parameters to our PvDevice
    SetComParameters();

    // Device connection, packet size negociation and stream opening is performed in a separate 
    // thread while we display a progress dialog to the user
    PvResult lResult;
    if ( aDI != NULL )
    {
        ConnectionThread lConnectionThread( &mSetup, aDI, &mDevice, &mStream, NULL );
        lResult = lConnectionThread.Connect();
    }

    if ( !lResult.IsOK() )
    {
        PvMessageBox( NULL, lResult );
        Disconnect();

        return;
    }

    mIPAddress = aDI->GetIPAddress().GetUnicode();
    mMACAddress = aDI->GetMACAddress().GetUnicode();
    mManufacturer = aDI->GetManufacturerInfo().GetUnicode();
    mModel = aDI->GetModelName().GetUnicode();
    mName = aDI->GetUserDefinedName().GetUnicode();
    mDescription = aDI->GetDisplayID().GetUnicode();

    if ( mDevice.IsConnected() )
    {
        // Register PvDevice callbacks
        mDevice.RegisterEventSink( this );

        switch ( aDI->GetClass() )
        {
        default:
            ASSERT( 0 );

        case PvDeviceClassTransmitter:
            mRole = RoleDeviceTransmitter;
            break;

        case PvDeviceClassReceiver:
            mRole = RoleDeviceReceiver;
            break;

        case PvDeviceClassTransceiver:
            mRole = RoleDeviceTransceiver;
            break;

        case PvDeviceClassPeripheral:
            mRole = RoleDevicePeripheral;
            break;
        }
    }
    else if ( mStream.IsOpen() )
    {
        mRole = RoleSoftwareReceiver;
    }

    SetTitle( mDescription );

    if ( mStream.IsOpen() )
    {
        // Ready image reception
        StartStreaming();
    }
}

// ==========================================================================
void NetCommandDoc::Disconnect()
{
    // If streaming, stop streaming
    if ( mStream.IsOpen() )
    {
        StopStreaming();
        mStream.Close();
    }

    if ( mDevice.IsConnected() )
    {
        mDevice.UnregisterEventSink( this );
        mDevice.Disconnect();
    }
}

// ==========================================================================
void NetCommandDoc::StartStreaming()
{
    CSingleLock lLock( &mStartStreamingMutex );
    VERIFY( lLock.Lock() );
    if ( mThreadDisplay == NULL )
    {
        // Create display thread
        mThreadDisplay = new ThreadDisplay;

        // Start threads
        mThreadDisplay->Start( &mPipeline, mDevice.GetParameters() );
        mThreadDisplay->SetPriority( THREAD_PRIORITY_ABOVE_NORMAL );
    }

    if ( !mPipeline.IsStarted() )
    {
        mPipeline.Start();
    }
}

// ==========================================================================
void NetCommandDoc::StopStreaming()
{
    CSingleLock lLock( &mStartStreamingMutex );
    VERIFY( lLock.Lock() );

    // Stop stream thread
    if ( mPipeline.IsStarted() )
    {
        mPipeline.Stop();
    }

    if ( mThreadDisplay != NULL )
    {
        mThreadDisplay->Stop( true );

        delete mThreadDisplay;
        mThreadDisplay = NULL;
    }
}

// ==========================================================================
void NetCommandDoc::SetDisplay( PvDisplayWnd *aDisplay )
{
    CSingleLock lLock( &mStartStreamingMutex );
    VERIFY( lLock.Lock() );
    if ( mThreadDisplay != NULL )
    {
        mThreadDisplay->SetDisplay( aDisplay );
    }
}

// ==========================================================================
void NetCommandDoc::StartAcquisition()
{
    CSingleLock lLock( &mStartAcquisitionMutex );
    VERIFY( lLock.Lock() );
    PvGenEnum *lMode = dynamic_cast<PvGenEnum *>( mDevice.GetParameters()->Get( "AcquisitionMode" ) );
    PvGenCommand *lStart = dynamic_cast<PvGenCommand *>( mDevice.GetParameters()->Get( "AcquisitionStart" ) );
    PvGenInteger *lTLParamsLocked = dynamic_cast<PvGenInteger *>( mDevice.GetParameters()->Get( "TLParamsLocked" ) );
    PvGenCommand *lResetStats = dynamic_cast<PvGenCommand *>( mStream.GetParameters()->Get( "Reset" ) );

    if ( mStream.IsOpen() )
    {
        uint32_t lPayloadSize = mDevice.GetPayloadSize();
        if ( lPayloadSize > 0 )
        {
            mPipeline.SetBufferSize( lPayloadSize );
        }

        mPipeline.Reset();
    }

    lResetStats->Execute();
    if ( mThreadDisplay != NULL )
    {
        mThreadDisplay->ResetStatistics();
    }

    PvResult lResult = PvResult::Code::NOT_INITIALIZED;

    PvString lStr;
    lMode->GetValue( lStr );
    CString lModeStr = lStr.GetUnicode();
    if ( lModeStr.Find( _T( "Continuous" ) ) >= 0 )
    {
        // We are streaming, lock the TL parameters
        if ( lTLParamsLocked != NULL ) 
        {
            lResult = lTLParamsLocked->SetValue( 1 );
        }

        lStart->Execute();
    }
    else if ( ( lModeStr.Find( _T( "Multi" ) ) >= 0 ) || 
              ( lModeStr.Find( _T( "Single" ) ) >= 0 ) )
    {
        // We are streaming, lock the TL parameters
        if ( lTLParamsLocked != NULL ) 
        {
            lTLParamsLocked->SetValue( 1 );
        }

        lResult = lStart->Execute();

        // We are done streaming, unlock the TL parameters
        if ( lTLParamsLocked != NULL ) 
        {
            lTLParamsLocked->SetValue( 0 );
        }
    }
}

// ==========================================================================
void NetCommandDoc::StopAcquisition()
{
    CSingleLock lLock( &mStartAcquisitionMutex );
    VERIFY( lLock.Lock() );

    PvResult lResult;

    // Execute AcquisitionStop command, if it exists
    PvGenCommand *lStop = dynamic_cast<PvGenCommand *>( mDevice.GetParameters()->Get( "AcquisitionStop" ) );
    if ( lStop != NULL )
    {
        lResult = lStop->Execute();
    }

    // TLParamsLocked, if it exists
    PvGenInteger *lTLParamsLocked = dynamic_cast<PvGenInteger *>( mDevice.GetParameters()->Get( "TLParamsLocked" ) );
    if ( lTLParamsLocked != NULL )
    {
        lResult = lTLParamsLocked->SetValue( 0 );
    }
}

// ==========================================================================
void NetCommandDoc::SetComParameters()
{
    // Get temp path
    WCHAR lTempPath[ BUFFER_SIZE ] = { 0 };
    if ( ::GetTempPath( BUFFER_SIZE, lTempPath ) == 0 )
    {
        TRACE0( "Failed to retrieve temp path\n" );
        return;
    }

    // Get temp file name
    WCHAR lTempFile[ BUFFER_SIZE ] = { 0 };
    if ( ::GetTempFileName( lTempPath, _T( "" ), 0, lTempFile ) == 0 )
    {
        TRACE0( "Failed to create temp file name\n" );
        return;
    }

    // Write default com parameters to file
    PvConfigurationWriter lWriter;
    VERIFY( lWriter.Store( sDefaultDevice.GetCommunicationParameters() ).IsOK() );
    if ( !lWriter.Save( lTempFile ).IsOK() )
    {
        TRACE0( "Unable to save com parameters to temporary file\n" );
        return;
    }

    // Retrieve file content in doc's PvDevice com parameters
    PvConfigurationReader lReader;
    VERIFY( lReader.Load( lTempFile ).IsOK() );
    if ( !lReader.Restore( 0, mDevice.GetCommunicationParameters() ).IsOK() )
    {
        TRACE0( "Unable to retrieve com parameters from temporary file\n" );
        return;
    }

    // Delete temp file
    if ( !::DeleteFile( lTempFile ) )
    {
        TRACE0( "Error deleting temp file\n" );
    }
}

// ==========================================================================
void NetCommandDoc::OnLinkDisconnected( PvDeviceGEV *aDevice )
{
    ASSERT( mMainWnd != NULL );
    mMainWnd->PostMessage( WM_LINKDISCONNECTED, reinterpret_cast<WPARAM>( this ) );
}

