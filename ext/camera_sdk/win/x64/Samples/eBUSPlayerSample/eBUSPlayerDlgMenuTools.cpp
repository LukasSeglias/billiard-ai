// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"
#include "eBUSPlayerDlg.h"
#include "FileTransferDlg.h"
#include "BufferOptionsDlg.h"

#include <PvGenBrowserWnd.h>
#include <PvGenFile.h>


// =============================================================================
void eBUSPlayerDlg::InitToolsMenu( CMenu* pMenuItem, UINT aID, int aIndex )
{
    switch ( aID )
    {
    case ID_TOOLS_SAVEPREFERENCES:
    case ID_TOOLS_RESTOREDEFAULTPREFERENCES:
        pMenuItem->EnableMenuItem( aIndex, MF_BYPOSITION | 
            ( ( !mPlayer->IsInRecovery() ) ? MF_ENABLED : MF_GRAYED ) );
        break;

    case ID_TOOLS_EVENTMONITOR:
        pMenuItem->EnableMenuItem( aIndex, MF_BYPOSITION | MF_ENABLED );
        pMenuItem->CheckMenuItem( aIndex, MF_BYPOSITION |
            ( ( ( mEventMonitorDlg != NULL ) && mEventMonitorDlg->IsWindowVisible() ) ? MF_CHECKED : MF_UNCHECKED ) );
        break;

    case ID_TOOLS_DISPLAYPARTIALIMAGES:
        pMenuItem->EnableMenuItem( aIndex, MF_BYPOSITION | 
            ( ( mPlayer->IsStreamStarted() ) ? MF_ENABLED : MF_GRAYED ) );
        pMenuItem->CheckMenuItem( aIndex, MF_BYPOSITION | 
            ( mPlayer->GetDisplayThread()->GetKeepPartialImagesEnabled() ? MF_CHECKED : MF_UNCHECKED ) );
        break;

    case ID_TOOLS_DISPLAYCHUNKDATA:
        pMenuItem->EnableMenuItem( aIndex, MF_BYPOSITION | 
            ( ( mPlayer->IsStreamStarted() ) ? MF_ENABLED : MF_GRAYED ) );
        pMenuItem->CheckMenuItem( aIndex, MF_BYPOSITION | 
            ( mPlayer->GetDisplayThread()->GetDisplayChunkDataEnabled() ? MF_CHECKED : MF_UNCHECKED ) );
        break;

    case ID_TOOLS_SAVEIMAGES:
        pMenuItem->EnableMenuItem( aIndex, MF_BYPOSITION | ( ( mPlayer->IsStreamStarted() ) ? MF_ENABLED : 
            MF_GRAYED ) );
        break;

    case ID_TOOLS_SAVECURRENTIMAGE:
        pMenuItem->EnableMenuItem( aIndex, MF_BYPOSITION | 
            ( ( mPlayer->IsStreamStarted() && !mPlayer->GetImageSaving()->IsFormatVideo() ) ? MF_ENABLED : MF_GRAYED ) );
        break;

    case ID_TOOLS_IMAGEFILTERING:
        pMenuItem->EnableMenuItem( aIndex, MF_BYPOSITION | 
            ( ( mPlayer->IsStreamOpened() ) ? MF_ENABLED : MF_GRAYED ) );
        pMenuItem->CheckMenuItem( aIndex, MF_BYPOSITION |
            ( ( mFilteringDlg->IsWindowVisible() ) ? MF_CHECKED : MF_UNCHECKED ) );
        break;

    case ID_TOOLS_DEVICESERIALCOMMUNICATION:
        pMenuItem->EnableMenuItem( aIndex, MF_BYPOSITION |
            ( ( mPlayer->IsSerialSupported() && !mPlayer->IsInRecovery() ) ? MF_ENABLED : MF_GRAYED ) );
        pMenuItem->CheckMenuItem( aIndex, MF_BYPOSITION |
            ( ( mSerialTerminalWnd.GetHandle() != 0 ) ? MF_CHECKED : MF_UNCHECKED ) );
        break;

    case ID_TOOLS_SAVE_XML:
        pMenuItem->EnableMenuItem( aIndex, MF_BYPOSITION |
            ( ( mPlayer->IsDeviceConnected() && !mPlayer->IsInRecovery() ) ? MF_ENABLED : MF_GRAYED ) );
        break;

    case ID_TOOLS_DEFAULTGEVCOMMUNICATIONPARAMETERS:
        pMenuItem->EnableMenuItem( aIndex, MF_BYPOSITION |
            ( ( mPlayer->IsSetupRoleController() && !mPlayer->IsDeviceConnected() ) ? MF_ENABLED : MF_GRAYED ) );
        break;

    case ID_TOOLS_DEFAULTU3VCOMMUNICATIONPARAMETERS:
        pMenuItem->EnableMenuItem( aIndex, MF_BYPOSITION |
            ( ( mPlayer->IsSetupRoleController() && !mPlayer->IsDeviceConnected() ) ? MF_ENABLED : MF_GRAYED ) );
        break;

#ifdef SERIALBRIDGE
    case ID_TOOLS_CAMERABRIDGE:
        pMenuItem->EnableMenuItem( aIndex, MF_BYPOSITION |
            ( ( ( mPlayer->GetCameraBridgeManager() != NULL ) && !mPlayer->IsInRecovery() ) ? MF_ENABLED : MF_GRAYED ) );
        pMenuItem->CheckMenuItem( aIndex, MF_BYPOSITION |
            ( ( ( mPlayer->GetCameraBridgeManager() != NULL ) && ( mPlayer->GetCameraBridgeManager()->GetHandle() != 0 ) ) ? MF_CHECKED : MF_UNCHECKED ) );
        break;
    case ID_TOOLS_SERIALCOMMUNICATIONBRIDGE:
        pMenuItem->EnableMenuItem( aIndex, MF_BYPOSITION | 
            ( ( ( mPlayer->GetSerialBridgeManager() != NULL ) && !mPlayer->IsInRecovery() ) ? MF_ENABLED : MF_GRAYED ) );
        pMenuItem->CheckMenuItem( aIndex, MF_BYPOSITION |
            ( ( ( mPlayer->GetSerialBridgeManager() != NULL ) && ( mPlayer->GetSerialBridgeManager()->GetHandle() != 0 ) ) ? MF_CHECKED : MF_UNCHECKED ) );
        break;
#else
    case ID_TOOLS_CAMERABRIDGE:
    case ID_TOOLS_SERIALCOMMUNICATIONBRIDGE:
        pMenuItem->EnableMenuItem( aIndex, MF_BYPOSITION | MF_DISABLED );
        break;
#endif // SERIALBRIDGE

    case ID_TOOLS_ACTIONCOMMAND:
        pMenuItem->CheckMenuItem( aIndex, MF_BYPOSITION |
            ( ( ( mActionCommandDlg != NULL ) && ( mActionCommandDlg->GetSafeHwnd() != 0 ) ) ? MF_CHECKED : MF_UNCHECKED ) );
        break;

    case ID_DISPLAYIMAGES_DISABLED:
        pMenuItem->EnableMenuItem( aIndex, MF_BYPOSITION | 
            ( ( mPlayer->IsStreamStarted() ) ? MF_ENABLED : MF_GRAYED ) );
        pMenuItem->CheckMenuItem( aIndex, MF_BYPOSITION |
            ( ( mPlayer->GetDisplayThread()->GetTargetFPS() == 0 ) ? MF_CHECKED : MF_UNCHECKED ) );
        break;

    case ID_DISPLAYIMAGES_30FPS:
        pMenuItem->EnableMenuItem( aIndex, MF_BYPOSITION | 
            ( ( mPlayer->IsStreamStarted() ) ? MF_ENABLED : MF_GRAYED ) );
        pMenuItem->CheckMenuItem( aIndex, MF_BYPOSITION |
            ( ( mPlayer->GetDisplayThread()->GetTargetFPS() == 30 ) ? MF_CHECKED : MF_UNCHECKED ) );
        break;

    case ID_DISPLAYIMAGES_60FPS:
        pMenuItem->EnableMenuItem( aIndex, MF_BYPOSITION | 
            ( ( mPlayer->IsStreamStarted() ) ? MF_ENABLED : MF_GRAYED ) );
        pMenuItem->CheckMenuItem( aIndex, MF_BYPOSITION |
            ( ( mPlayer->GetDisplayThread()->GetTargetFPS() == 60 ) ? MF_CHECKED : MF_UNCHECKED ) );
        break;

    case ID_TOOLS_DEINTERLACING_DISABLED:
        pMenuItem->EnableMenuItem( aIndex, MF_BYPOSITION | 
            ( ( mPlayer->IsStreamStarted() ) ? MF_ENABLED : MF_GRAYED ) );
        pMenuItem->CheckMenuItem( aIndex, MF_BYPOSITION |
            ( ( mPlayer->GetDisplayThread()->GetDeinterlacing() == PvDeinterlacingDisabled ) ? MF_CHECKED : MF_UNCHECKED ) );
        break;

    case ID_TOOLS_DEINTERLACING_WEAVE_HALF:
        pMenuItem->EnableMenuItem( aIndex, MF_BYPOSITION | 
            ( ( mPlayer->IsStreamStarted() ) ? MF_ENABLED : MF_GRAYED ) );
        pMenuItem->CheckMenuItem( aIndex, MF_BYPOSITION |
            ( ( mPlayer->GetDisplayThread()->GetDeinterlacing() == PvDeinterlacingWeavingHalf ) ? MF_CHECKED : MF_UNCHECKED ) );
        break;

    case ID_TOOLS_DEINTERLACING_WEAVE_FULL:
        pMenuItem->EnableMenuItem( aIndex, MF_BYPOSITION | 
            ( ( mPlayer->IsStreamStarted() ) ? MF_ENABLED : MF_GRAYED ) );
        pMenuItem->CheckMenuItem( aIndex, MF_BYPOSITION |
            ( ( mPlayer->GetDisplayThread()->GetDeinterlacing() == PvDeinterlacingWeavingFull ) ? MF_CHECKED : MF_UNCHECKED ) );
        break;

    case ID_TOOLS_DEINTERLACING_BLENDING:
        pMenuItem->EnableMenuItem( aIndex, MF_BYPOSITION | 
            ( ( mPlayer->IsStreamStarted() ) ? MF_ENABLED : MF_GRAYED ) );
        pMenuItem->CheckMenuItem( aIndex, MF_BYPOSITION |
            ( ( mPlayer->GetDisplayThread()->GetDeinterlacing() == PvDeinterlacingBlending ) ? MF_CHECKED : MF_UNCHECKED ) );
        break;

    case ID_TOOLS_DEINTERLACING_LINEDOUBLING:
        pMenuItem->EnableMenuItem( aIndex, MF_BYPOSITION | 
            ( ( mPlayer->IsStreamStarted() ) ? MF_ENABLED : MF_GRAYED ) );
        pMenuItem->CheckMenuItem( aIndex, MF_BYPOSITION |
            ( ( mPlayer->GetDisplayThread()->GetDeinterlacing() == PvDeinterlacingLineDoubling ) ? MF_CHECKED : MF_UNCHECKED ) );
        break;

	case ID_TOOLS_RESET_STREAMING_STATISTICS:
        pMenuItem->EnableMenuItem( aIndex, MF_BYPOSITION | 
            ( ( mPlayer->IsStreamStarted() ) ? MF_ENABLED : MF_GRAYED ) );
		break;

    case ID_TOOLS_FILETRANSFER:
        pMenuItem->EnableMenuItem( aIndex, MF_BYPOSITION |
            ( mPlayer->IsDeviceConnected() && PvGenFile::IsSupported( mPlayer->GetDeviceParameters() ) ? MF_ENABLED : MF_GRAYED ) );
        break;
    }
}


// =============================================================================
void eBUSPlayerDlg::OnToolsEventMonitor()
{
    if ( mEventMonitorDlg == NULL )
    {
        CreateEventMonitor();
    }

    if ( ( mEventMonitorDlg == NULL ) || ( mEventMonitorDlg->GetSafeHwnd() == 0 ) )
    {
        return;
    }

    if ( mEventMonitorDlg->IsWindowVisible() )
    {
        mEventMonitorDlg->ShowWindow( SW_HIDE );
    }
    else
    {
        mEventMonitorDlg->ShowWindow( SW_SHOW );
        mEventMonitorDlg->BringWindowToTop();
        mEventMonitorDlg->SetFocus();
    }
}


// =============================================================================
void eBUSPlayerDlg::OnToolsSerialCommunicationBridge()
{
    if ( !mPlayer->IsDeviceConnected() )
    {
        return;
    }

#ifdef SERIALBRIDGE
    if ( mPlayer->GetSerialBridgeManager()->GetHandle() != 0 )
    {
        mPlayer->GetSerialBridgeManager()->Close();
    }
    else
    {
        mPlayer->GetSerialBridgeManager()->ShowModeless( GetSafeHwnd() );
    }
#endif // SERIALBRIDGE
}


// =============================================================================
void eBUSPlayerDlg::OnToolsCameraBridge()
{
    if ( !mPlayer->IsDeviceConnected() )
    {
        return;
    }

#ifdef SERIALBRIDGE
    if ( mPlayer->GetCameraBridgeManager()->GetHandle() != 0 )
    {
        mPlayer->GetCameraBridgeManager()->Close();
    }
    else
    {
        // Read from Registry if the PCF option show be displayed
        bool lPCFAvailable = AfxGetApp()->GetProfileInt( _T( "" ), PCFAVAILABLE, 0 ) != 0;

        // Make PCF visible (or not)
        PvCameraBridgeManagerWnd *lWnd = mPlayer->GetCameraBridgeManager();
        if ( lPCFAvailable )
        {
            lWnd->ShowPCF();
        }
        else
        {
            lWnd->HidePCF();
        }

        // Show camera bridge manager
        lWnd->ShowModeless( GetSafeHwnd() );
    }
#endif
}


// =============================================================================
void eBUSPlayerDlg::OnToolsActionCommand()
{
    if ( mActionCommandDlg == NULL )
    {
        mActionCommandDlg = new ActionCommandDlg();
    }

    if ( mActionCommandDlg->GetSafeHwnd() != 0 )
    {
        mActionCommandDlg->DestroyWindow();

        delete mActionCommandDlg;
        mActionCommandDlg = NULL;
        return;
    }

    bool lRetVal;
    CWnd lWndObject;
    CWnd *lWnd = CWnd::FromHandlePermanent( this->GetSafeHwnd() );
    if ( lWnd == NULL )
    {
        lWndObject.Attach( this->GetSafeHwnd() );
        lWnd = &lWndObject;
    }

    lRetVal = ( mActionCommandDlg->Create( ActionCommandDlg::IDD, lWnd ) != 0 ) ? true : false;

    if ( lWndObject.GetSafeHwnd() != 0 )
    {
        lWndObject.Detach();
    }

    if ( lRetVal )
    {
        mActionCommandDlg->ShowWindow( SW_SHOW );
    }
}


// =============================================================================
void eBUSPlayerDlg::OnToolsSaveimages()
{
    if ( !mPlayer->IsStreamStarted() )
    {
        return;
    }

    ImageSaveDlg lDlg( mPlayer->GetImageSaving() );
    lDlg.DoModal();
}


// =============================================================================
void eBUSPlayerDlg::OnToolsSavecurrentimage()
{
    ASSERT( mPlayer->IsStreamStarted() );
    if ( !mPlayer->IsStreamStarted() )
    {
        return;
    }

    mPlayer->SaveCurrentImage();
}


// =============================================================================
void eBUSPlayerDlg::OnToolsSerialCommunication()
{
    if ( !mPlayer->IsSerialSupported() )
    {
        return;
    }

    if ( mSerialTerminalWnd.GetHandle() != 0 )
    {
        mSerialTerminalWnd.Close();
    }
    else
    {
        mSerialTerminalWnd.SetDevice( mPlayer->GetPvDeviceAdapter() );
        mSerialTerminalWnd.ShowModeless( GetSafeHwnd() );
    }
}


// ==========================================================================
void eBUSPlayerDlg::OnDisplayDisabled()
{
    mPlayer->GetDisplayThread()->SetTargetFPS( 0 );
}


// ==========================================================================
void eBUSPlayerDlg::OnDisplay30FPS()
{
    mPlayer->GetDisplayThread()->SetTargetFPS( 30 );
}


// ==========================================================================
void eBUSPlayerDlg::OnDisplay60FPS()
{
    mPlayer->GetDisplayThread()->SetTargetFPS( 60 );
}


// =============================================================================
void eBUSPlayerDlg::OnToolsDeinterlacingDisabled()
{
    mPlayer->GetDisplayThread()->SetDeinterlacing( PvDeinterlacingDisabled );
}


// =============================================================================
void eBUSPlayerDlg::OnToolsDeinterlacingWeavingHalf()
{
    mPlayer->GetDisplayThread()->SetDeinterlacing( PvDeinterlacingWeavingHalf );
}


// =============================================================================
void eBUSPlayerDlg::OnToolsDeinterlacingWeavingFull()
{
    mPlayer->GetDisplayThread()->SetDeinterlacing( PvDeinterlacingWeavingFull );
}


// =============================================================================
void eBUSPlayerDlg::OnToolsDeinterlacingBlending()
{
    mPlayer->GetDisplayThread()->SetDeinterlacing( PvDeinterlacingBlending );
}


// =============================================================================
void eBUSPlayerDlg::OnToolsDeinterlacingLineDoubling()
{
    mPlayer->GetDisplayThread()->SetDeinterlacing( PvDeinterlacingLineDoubling );
}


// =============================================================================
void eBUSPlayerDlg::OnToolsDefaultGEVCommunicationParameters()
{
    mDefaultCommGEVParametersWnd.SetGenParameterArray( mPlayer->GetDefaultGEVCommunicationParameters() );
    mDefaultCommGEVParametersWnd.SetTitle( "Default GigE Vision Communication Parameters" );

    mDefaultCommGEVParametersWnd.ShowModal();
}


// =============================================================================
void eBUSPlayerDlg::OntoolsDefaultU3VCommunicationParameters()
{
    mDefaultCommU3VParametersWnd.SetGenParameterArray( mPlayer->GetDefaultU3VCommunicationParameters() );
    mDefaultCommU3VParametersWnd.SetTitle( "Default USB3 Vision Communication Parameters" );

    mDefaultCommU3VParametersWnd.ShowModal();
}


// =============================================================================
void eBUSPlayerDlg::OnToolsFileTransfer()
{
    FileTransferDlg lDlg( mPlayer->GetPvDevice(), this );
    lDlg.DoModal();
}


// =============================================================================
void eBUSPlayerDlg::OnToolsSavepreferences()
{
    SaveConfig( GetStickyPath(), false );
}


// =============================================================================
void eBUSPlayerDlg::OnToolsRestoredefaultpreferences()
{
    // load the default file
    OpenConfig( GetDefaultPath() );

    // save the default settings so they are persisted next time
    SaveConfig( GetStickyPath(), false );
}


// =============================================================================
void eBUSPlayerDlg::OnToolsBufferoptions()
{
    BufferOptionsDlg lDlg( mPlayer, this );
    lDlg.DoModal();
}


// =============================================================================
void eBUSPlayerDlg::OnToolsResetStreamingStatistics()
{
	mPlayer->ResetStreamingStatistics();
}


// =============================================================================
void eBUSPlayerDlg::OnToolsDisplaypartialimages()
{
    DisplayThread *lDT = mPlayer->GetDisplayThread();
    lDT->SetKeepPartialImagesEnabled( !lDT->GetKeepPartialImagesEnabled() );
}


// =============================================================================
void eBUSPlayerDlg::OnToolsDisplaychunkdata()
{
    DisplayThread *lDT = mPlayer->GetDisplayThread();
    lDT->SetDisplayChunkDataEnabled( !lDT->GetDisplayChunkDataEnabled() );
}


// =============================================================================
void eBUSPlayerDlg::OnToolsSaveXml()
{
    if ( !mPlayer->IsDeviceConnected() )
    {
        return;
    }

    CString lFilename = mPlayer->GetDeviceXMLDefaultName().GetUnicode();
    if ( lFilename.GetLength() <= 0 )
    {
        lFilename = _T( "Default.xml" );
    }

    CString lFilter;
    CString lExtension;
    bool lZip = lFilename.MakeLower().Find( _T( ".zip" ) ) > 0;
    if ( lZip )
    {
        lFilter = _T( "Zip Files (*.zip)|*.zip|All files (*.*)|*.*||" );
        lExtension = _T( ".zip" );
    }
    else
    {
        lFilter = _T( "GenICam XML Files (*.xml)|*.xml|All files (*.*)|*.*||" );
        lExtension = _T( ".xml" );
    }

    CFileDialog lFileDlg( FALSE, lExtension, lFilename, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, lFilter, this );
    lFileDlg.m_ofn.lpstrTitle = _T( "Save Device XML" );
    if ( !lFileDlg.DoModal() == IDOK )
    {
        return;
    }

    PvResult lResult = mPlayer->SaveDeviceXML( (LPCTSTR)lFileDlg.GetPathName() );
    if ( !lResult.IsOK() )
    {
        MessageBox( _T( "Unable to retrieve the XML file from the device." ), GetAppName(), MB_ICONERROR | MB_OK );
    }
}



