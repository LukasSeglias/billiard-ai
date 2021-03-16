// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"
#include "eBUSPlayerApp.h"
#include "eBUSPlayerFactory.h"
#include "SplashDlg.h"

#include <PvLogger.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


BEGIN_MESSAGE_MAP( eBUSPlayerApp, CWinApp )
END_MESSAGE_MAP()


///
/// \brief Constructor
///

eBUSPlayerApp::eBUSPlayerApp()
{
}


//
// The one and only eBUSPlayerApp object
//

eBUSPlayerApp theApp;



///
/// \brief App initialization
///

BOOL eBUSPlayerApp::InitInstance()
{
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof( InitCtrls );

    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx( &InitCtrls );

    CCommandLineInfo lCommandLine;
    ParseCommandLine( lCommandLine );
    CWinApp::InitInstance();

    AfxEnableControlContainer();

    SetRegistryKey( _T( "Pleora Technologies Inc" ) );

    eBUSPlayerFactory lFactory;
    eBUSPlayerDlg *lDlg = lFactory.CreateDlg( lCommandLine.m_strFileName );

    SplashDlg lSplash( lDlg->GetAppName() );
    lSplash.DoModal();

    m_pMainWnd = lDlg;
    lDlg->DoModal();
    PVDELETE( lDlg );

    return FALSE;
}

