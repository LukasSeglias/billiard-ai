// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// -----------------------------------------------------------------------------
// This sample illustrates how to use the eBUS SDK to receive, transform and 
// transmit images. More specifically, it is an MFC GUI application that 
// receives video from up to four different eBUS Transmitters, tiles the images 
// together, displays a preview of the tiled output and finally transmits the 
// output to a given destination. This sample is designed more for use as an 
// application than as illustrative source code. As a result, it is recommended 
// that application developers begin by reading and understanding a simpler 
// example such as TransmitTestPatternSample before attempting to dissect 
// the source code of TransmitTiledImages.
// 
// Basic instructions for using the sample:
//    1. Start TransmitTiledImages.
//    2. Click the plus (+) button next to the source selection combo box.
//    3. Select a eBUS Transmitter from the list of available devices (should be 
//       on the same subnet as the network interface you wish to receive on).
//    4. Leave the default option to connect as a controller and data receiver.
//    5. Repeat steps 2-4 to connect to up to four devices.
//    6. Optionally configure each device by right clicking on it and selecting 
//       "Properties..." in the ensuing context menu.
//    7. Optionally modify the tiling and video output parameters.
//    8. Start your receiver (such as a vDisplay or eBUSPlayer) and listen for 
//       data from the transmitter.
//    9. Click Start in the application. You should see a tiled input appearing 
//       in the application that should be transmitting to your the specified 
//       address.
// *****************************************************************************

#include "stdafx.h"
#include "TransmitTiledImagesApp.h"
#include "TransmitTiledImagesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTransmitTiledImagesApp

BEGIN_MESSAGE_MAP(CTransmitTiledImagesApp, CWinApp)
    ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CTransmitTiledImagesApp construction

CTransmitTiledImagesApp::CTransmitTiledImagesApp()
{
    // TODO: add construction code here,
    // Place all significant initialization in InitInstance
}


// The one and only CTransmitTiledImagesApp object

CTransmitTiledImagesApp theApp;


// CTransmitTiledImagesApp initialization

BOOL CTransmitTiledImagesApp::InitInstance()
{
    // InitCommonControlsEx() is required on Windows XP if an application
    // manifest specifies use of ComCtl32.dll version 6 or later to enable
    // visual styles.  Otherwise, any window creation will fail.
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    // Set this to include all the common control classes you want to use
    // in your application.
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    CWinApp::InitInstance();

    // Standard initialization
    // If you are not using these features and wish to reduce the size
    // of your final executable, you should remove from the following
    // the specific initialization routines you do not need
    // Change the registry key under which our settings are stored
    // TODO: You should modify this string to be something appropriate
    // such as the name of your company or organization
    SetRegistryKey(_T("Local AppWizard-Generated Applications"));

    CTransmitTiledImagesDlg dlg;
    m_pMainWnd = &dlg;
    INT_PTR nResponse = dlg.DoModal();
    if (nResponse == IDOK)
    {
        // TODO: Place code here to handle when the dialog is
        //  dismissed with OK
    }
    else if (nResponse == IDCANCEL)
    {
        // TODO: Place code here to handle when the dialog is
        //  dismissed with Cancel
    }

    // Since the dialog has been closed, return FALSE so that we exit the
    //  application, rather than start the application's message pump.
    return FALSE;
}
