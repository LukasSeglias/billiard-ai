// PlcAndGevEventsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PlcAndGevEvents.h"
#include "PlcAndGevEventsDlg.h"
#include "PvDeviceInfoGEV.h"

#include <process.h> 

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CPlcAndGevEventsDlg dialog
CPlcAndGevEventsDlg::CPlcAndGevEventsDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CPlcAndGevEventsDlg::IDD, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPlcAndGevEventsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPlcAndGevEventsDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    //}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_BUTTON1, &CPlcAndGevEventsDlg::OnBnClickedButton1)
    ON_BN_CLICKED(IDC_BUTTON2, &CPlcAndGevEventsDlg::OnBnClickedButton2)
    ON_BN_CLICKED(IDC_BUTTON3, &CPlcAndGevEventsDlg::OnBnClickedButton3)
    ON_BN_CLICKED(IDC_BUTTON4, &CPlcAndGevEventsDlg::OnBnClickedButton4)
    ON_BN_CLICKED(IDC_BUTTON5, &CPlcAndGevEventsDlg::OnBnClickedButton5)
    ON_BN_CLICKED(IDOK, &CPlcAndGevEventsDlg::OnBnClickedOk)
    ON_WM_TIMER()
END_MESSAGE_MAP()

HANDLE      hProcessThread;
bool        g_bAcquisition;
int         g_iNbOfFrameGrabbed;
int         g_iNbOfGrabTimeout;

void Processfunc(void *arg)
{
    Str_shared*  strTemp = (Str_shared *)arg;
    PvPipeline* pvPipeline =strTemp->pvPipeline;
    PvDisplayWnd* pvDisplayWnd =strTemp->pvDisplayWnd;
    
    PvResult pvResult;
    PvResult pvResult2;

   // Time variables used for limiting display rate
    DWORD lPrevious = 0;
    DWORD lCurrent  = 0; 
    long lDelta ;
    static int m =0;

    g_iNbOfFrameGrabbed =0;
    g_iNbOfGrabTimeout  =0;
    while(g_bAcquisition )
    {
        PvBuffer *lBuffer = NULL;
        PvResult  lOperationResult;
        
        //Grab timeout here is determined by the DefaultImageTimeout
        pvResult = pvPipeline->RetrieveNextBuffer( & lBuffer, 0xFFFFFFFF, &lOperationResult );
        if ( pvResult.IsOK() )
        { 
            if (lOperationResult.IsOK())
            {
                //
                // We now have a valid buffer. This is where you would typically process the buffer.
                // -----------------------------------------------------------------------------------------
                // ...

                // limit refresh rate to ~ 60 fps max
                lCurrent = ::GetTickCount();
                lDelta = ( lCurrent - lPrevious ) - ( 1000 / 60 );
            
                if ( lDelta > 0 )
                {
                    pvDisplayWnd->Display( *lBuffer );
                    lPrevious = ::GetTickCount();
                }
                g_iNbOfFrameGrabbed++;
            }
            pvPipeline->ReleaseBuffer( lBuffer );
        }
        else
        {
            g_iNbOfGrabTimeout++;
        }
    }
    hProcessThread = NULL;
    _endthread();

}

// CPlcAndGevEventsDlg message handlers

BOOL CPlcAndGevEventsDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);         // Set big icon
    SetIcon(m_hIcon, FALSE);        // Set small icon

    // TODO: Add extra initialization here
    mbInited =false;
    mbTimerOn =false;
    return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPlcAndGevEventsDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialog::OnPaint();
    }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPlcAndGevEventsDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CPlcAndGevEventsDlg::OnBnClickedButton1()
{  // "Select/Connect" button

    PvGenEnum* lPvGenEnum;
    PvGenInteger *lPvGenInteger;
 
    if (mbInited)
        Disconnect();

    // ********************************* //
    // Step 1: Select the device         //
    // ********************************* //
    PvDeviceFinderWnd pvDeviceFinderWnd;
    if ( !pvDeviceFinderWnd.ShowModal().IsOK() )
    {
        return;
    }
    const PvDeviceInfo* lDeviceInfo = pvDeviceFinderWnd.GetSelected();
    if ( lDeviceInfo == NULL )
    {
        return;
    }

    BeginWaitCursor();
    
    // ******************************** //
    // Step 2: Connect to the device    //
    // ******************************** //
    PvResult lResult = mDevice.Connect( lDeviceInfo, PvAccessControl );
    if (!lResult.IsOK())
    {
        return;
    }
    
    // 
    // Set long Heartbeat Timeout for the debugging purpose
    // If the program is stopped at the breakpoint too long then the
    // heartbeat may expired
    //
#ifdef _DEBUG
    mDevice.GetParameters()->SetIntegerValue( "GevHeartbeatTimeout", 60000 );
#endif
    // Perform automatic packet size negotiation
    lResult = mDevice.NegotiatePacketSize( 0, 1476 );
    if ( !lResult.IsOK() )
    {
        ::Sleep( 2500 );
    }

    // ************************************************* //
    // Step 3: Open and set stream destination    //
    // ************************************************* //
    const PvDeviceInfoGEV* lDeviceInfoGEV = dynamic_cast<const PvDeviceInfoGEV*>( lDeviceInfo);
    if ( lDeviceInfoGEV != NULL )
    {
        lResult = mStream.Open( lDeviceInfoGEV->GetIPAddress() );
    }
    
    if ( !lResult.IsOK() || lDeviceInfoGEV == NULL )
    {
        AfxMessageBox(_T("Could not open stream"));
        return;
    }
    mDevice.SetStreamDestination( mStream.GetLocalIPAddress(), mStream.GetLocalPort() );
    margParam.pvStream = &mStream;

    // Set the grab timeout value, default value 1000ms
    mStream.GetParameters()->SetIntegerValue("RequestTimeout", 1000 );

    // ************************************************* //
    // Step 4: Get image size information                //
    // ************************************************* //
    int64_t value;
    lPvGenInteger =mDevice.GetParameters()->GetInteger( "Width"  );
    lPvGenInteger->GetValue(value);
     mWidth = (int)value;
    lPvGenInteger->RegisterEventSink(this);
    
    lPvGenInteger = mDevice.GetParameters()->GetInteger( "Height" );
    lPvGenInteger->GetValue(value);
    mHeight = (int)value;
    lPvGenInteger->RegisterEventSink(this);

    int64_t i64PixelFormat;
    lPvGenEnum = mDevice.GetParameters()->GetEnum("PixelFormat" );
    lPvGenEnum->GetValue(i64PixelFormat);
    mpvPixelFormat = (int)i64PixelFormat;
    lPvGenEnum->RegisterEventSink(this);

    // Retrieve Pixel Size Byte per pixel (Bpp) information
    miPixelSize  =1;
    miPixelSize  =((mpvPixelFormat & 0x00FF0000) >> 16)/8;

    // ****************************************************************************    //
    // Step 5: Prepare the PvPipeline                                              //
    // a) PvPipeline internally creates buffers with default value 640x480, 8bit.      //
    // b) At receiving of each image frame if the image leader tells the image size    //
    //    is larger than the buffer size then PvPipeline reallocates the proper size   //
    //    buffer. If the image size is smaller than the buffer size there is no buffer //
    //    resize.                                                                      //
    // ************************************************* //
    mPipeline = new PvPipeline( & mStream );
    mPipeline->SetBufferSize( mWidth*mHeight*miPixelSize ); 
    mPipeline->SetBufferCount( 8 );
    mPipeline->Start();
    margParam.pvPipeline = mPipeline;

    // Step 6: Prepare for Display
    CRect lDisplayRect;
    GetDlgItem( IDC_DISPLAY )->GetClientRect( &lDisplayRect );
    GetDlgItem( IDC_DISPLAY )->ClientToScreen(&lDisplayRect );
    ScreenToClient( &lDisplayRect );
    mDisplayWnd.Create( GetSafeHwnd(), 10000 );
    mDisplayWnd.SetPosition( lDisplayRect.left, lDisplayRect.top, lDisplayRect.Width(), lDisplayRect.Height() );
    lResult =mDisplayWnd.SetBackgroundColor( 0x00, 0x00, 0x20);
    margParam.pvDisplayWnd =&mDisplayWnd;

    GetDlgItem(IDC_BUTTON1)->EnableWindow(true);
    GetDlgItem(IDC_BUTTON2)->EnableWindow(true);
    GetDlgItem(IDC_BUTTON3)->EnableWindow(true);
    GetDlgItem(IDC_BUTTON4)->EnableWindow(false);
    GetDlgItem(IDC_BUTTON5)->EnableWindow(true);

    EndWaitCursor();

    g_bAcquisition         = false;
    mbCallbackregistered   = false;
    mDeviceWnd             = NULL;
    mbInited               = true;

    if (!mbTimerOn)
    {
        SetTimer(1, 2000, 0);
        mbTimerOn = true;
    }

 }

void CPlcAndGevEventsDlg::OnBnClickedButton2()
{   // "Acquisition Start/Stop" button
    // Confirm if the device is connected
    if ( !mDevice.IsConnected() )
    {
        CString str;
        str.Format(_T("Device appears to be disconnected."));  
        SetDlgItemText(IDC_STATIC, str); 
        return;
    }

    if (!g_bAcquisition)
    {
        mDevice.GetParameters()->SetEnumValue( "AcquisitionMode" ,  "Continuous" );

        // Reset the stream
        mStream.GetParameters()->ExecuteCommand( "Reset" );
        
        PvResult lResult;
        lResult = mDevice.GetParameters()->SetIntegerValue( "TLParamsLocked", 1 );
        if ( !lResult.IsOK() )
        {
            CString str;
            str.Format(_T("Unable to change TLParamsLocked to 1.."));  
            SetDlgItemText(IDC_STATIC, str); 
        }

        // Start the stream
        lResult = mDevice.GetParameters()->ExecuteCommand( "AcquisitionStart" );
        if ( !lResult.IsOK() )
        {   
            CString str;
            str.Format(_T("Unable to execute AcquisitionStart command"));  
            SetDlgItemText(IDC_STATIC, str); 
        }

        // Start the image processing thread
        g_bAcquisition =true;

#ifdef USE_FVAL
    // ************************************//
    // Clear the counter using PCL_Ctrl1   //
    // Reset the callback counter
    // ************************************//
    mDevice.GetParameters()->SetBooleanValue( "PLC_ctrl1", false );
    Sleep(10);
    mDevice.GetParameters()->SetBooleanValue( "PLC_ctrl1", true);
    miNbOfCallback =0;
    
#endif
    
        hProcessThread = (HANDLE)_beginthread(Processfunc, 0, (void *)&margParam);
        GetDlgItem(IDC_BUTTON1)->EnableWindow(false);
        GetDlgItem(IDC_BUTTON3)->EnableWindow(false);
        GetDlgItem(IDC_BUTTON5)->EnableWindow(false);
        SetDlgItemText(IDC_BUTTON2, _T("Acquisition Stop"));
    }
    else
    {
        //Stop Acquisition
        g_bAcquisition =false;

        mDevice.GetParameters()->ExecuteCommand( "AcquisitionStop"  );
        
        // TLParamsLocked - Optionnal
        mDevice.GetParameters()->SetIntegerValue( "TLParamsLocked", 0 );
        Sleep(50);
        mStream.AbortQueuedBuffers();
        GetDlgItem(IDC_BUTTON1)->EnableWindow(true);
        GetDlgItem(IDC_BUTTON3)->EnableWindow(true);
        GetDlgItem(IDC_BUTTON5)->EnableWindow(true);    
        SetDlgItemText(IDC_BUTTON2, _T("Acquisition Start"));
    }
}

void CPlcAndGevEventsDlg::OnBnClickedButton3()
{   // "Setup PLC" button

    PvGenEnum*      lPvGenEnum;
    PvResult        lResult;

    // ********************************************* //
    // Setup PLC_I0 as PLC_A4 (FVAL) or PLC_ctrl0    //
    // ********************************************* //
    lPvGenEnum = mDevice.GetParameters()->GetEnum( "PLC_I0"  );
    if (lPvGenEnum !=NULL)
    {
#ifdef USE_FVAL
        lResult = lPvGenEnum->SetValue("PLC_A4");
#else
        lResult =lPvGenEnum->SetValue("PLC_ctrl0");
#endif
        if (!lResult.IsOK())
        {
            AfxMessageBox(_T("PLC_I0, SetValue() failed"));
            return;
        }
    }
    else
    {
        AfxMessageBox(_T("Could not access parameter PLC_I0"));
        return;
    }

    // ********************************************** //
    // Setup PLC_I4 as PLC_ctrl1                      //
    // Note: This is the default setting              //
    //       Set it here in case it has been changed  //
    // ********************************************** //
    lResult = mDevice.GetParameters()->SetEnumValue( "PLC_I4", "PLC_ctrl1" );
    if (!lResult.IsOK())
        AfxMessageBox(_T("Could not access parameter PLC_I4"));
        

    // *********************************************** //
    // Route PLC_I0 to Q7  for PLC_Interrupt_FIFO_Q7   //
    // *********************************************** //
    mDevice.GetParameters()->SetEnumValue ("PLC_Q7_Variable0", "PLC_I0" );
    mDevice.GetParameters()->SetEnumValue ("PLC_Q7_Operator0", "Or"     );
    mDevice.GetParameters()->SetEnumValue ("PLC_Q7_Variable1", "Zero"   );
    mDevice.GetParameters()->SetEnumValue ("PLC_Q7_Operator1", "Or"     );
    mDevice.GetParameters()->SetEnumValue ("PLC_Q7_Variable2", "Zero"   );
    mDevice.GetParameters()->SetEnumValue ("PLC_Q7_Operator2", "Or"     );
    mDevice.GetParameters()->SetEnumValue ("PLC_Q7_Variable3", "Zero"   );


    //**************************************************************//
    // Route PLC_I0 to Q17 which is the counter increament event    //
    //**************************************************************//
    mDevice.GetParameters()->SetEnumValue ("PLC_Q17_Variable0", "PLC_I0" );
    mDevice.GetParameters()->SetEnumValue ("PLC_Q17_Operator0", "Or"     );
    mDevice.GetParameters()->SetEnumValue ("PLC_Q17_Variable1", "Zero"   );
    mDevice.GetParameters()->SetEnumValue ("PLC_Q17_Operator1", "Or"     );
    mDevice.GetParameters()->SetEnumValue ("PLC_Q17_Variable2", "Zero"   );
    mDevice.GetParameters()->SetEnumValue ("PLC_Q17_Operator2", "Or"     );
    mDevice.GetParameters()->SetEnumValue ("PLC_Q17_Variable3", "Zero"   );


    // **************************************************************//
    // Route the PLC_I4 to Q3 which is the counter clear event    //
    // **************************************************************//
    mDevice.GetParameters()->SetEnumValue ("PLC_Q3_Variable0", "PLC_I4" );
    mDevice.GetParameters()->SetEnumValue ("PLC_Q3_Operator0", "Or"     );
    mDevice.GetParameters()->SetEnumValue ("PLC_Q3_Variable1", "Zero"   );
    mDevice.GetParameters()->SetEnumValue ("PLC_Q3_Operator1", "Or"     );
    mDevice.GetParameters()->SetEnumValue ("PLC_Q3_Variable2", "Zero"   );
    mDevice.GetParameters()->SetEnumValue ("PLC_Q3_Operator2", "Or"     );
    mDevice.GetParameters()->SetEnumValue ("PLC_Q3_Variable3", "Zero"   );
    
    //**************************************** //
    // Enable PLC_Interrupt_FIFO_Q7            //
    // *************************************** //

    // Step 1): Select the EventSelector as Q7
    mDevice.GetParameters()->SetEnumValue( "EventSelector", "PLC_Interrupt_FIFO0_Q7" );
    
    // Step 2): Enable Q7 Interrupt
    mDevice.GetParameters()->SetEnumValue( "EventNotification", "GigEVisionEvent");
    
    //*********************************************************** //
    // Register an PvDeviceEvent sink for accessing the 
    //  PvDeviceEventSink::OnEvent() callback:
    //*********************************************************** //
    if (! mbCallbackregistered)
    {
        mDevice.RegisterEventSink(this);
        mbCallbackregistered =true;
    }

    //*********************************************************** //
    // Setup Counter to monitor the Q17             //
    //*********************************************************** //
    mDevice.GetParameters()->SetEnumValue( "CounterSelector", "Counter1" );
    mDevice.GetParameters()->SetEnumValue( "CounterEventSource", "PLC_Q17_RisingEdge");
    mDevice.GetParameters()->SetEnumValue( "CounterDecrementEventSource", "Off");
    mDevice.GetParameters()->SetEnumValue( "CounterResetSource", "PLC_Q3");
    mDevice.GetParameters()->SetEnumValue( "CounterResetActivation", "RisingEdge");

    // ************************************//
    // Clear the counter using PCL_ctrl1   //
    // ************************************//
    mDevice.GetParameters()->SetBooleanValue( "PLC_ctrl1", false );
    Sleep(10);
    mDevice.GetParameters()->SetBooleanValue( "PLC_ctrl1", true);
    miNbOfCallback =0;
    miCurCounterValue = 0;

#ifndef USE_FVAL
    GetDlgItem(IDC_BUTTON4)->EnableWindow(true);
#endif
    
}

void CPlcAndGevEventsDlg::OnBnClickedButton4()
{   // "Generate 1 pulse" button

    // ************************************//
    // Generate 1 pulse                    //
    // ************************************//
    mDevice.GetParameters()->SetBooleanValue( "PLC_ctrl0", false );
    Sleep(10);
    mDevice.GetParameters()->SetBooleanValue( "PLC_ctrl0", true);
}

void CPlcAndGevEventsDlg::OnBnClickedButton5()
{   // "Device control" button

    if ( !mDevice.IsConnected() )
    {
        return;
    }
    
    if (mDeviceWnd !=NULL)
    {   
        if ( mDeviceWnd->GetHandle() != 0 )
            return;
        else
            mDeviceWnd->Close();
    }

    mDeviceWnd  = new PvGenBrowserWnd;
    mDeviceWnd ->SetTitle( PvString( "Device control parameters" ) );
    mDeviceWnd ->SetGenParameterArray( mDevice.GetParameters() );
    mDeviceWnd->SetVisibility(PvGenVisibilityGuru);
    mDeviceWnd ->ShowModeless(GetSafeHwnd() );
}

void CPlcAndGevEventsDlg::OnParameterUpdate( PvGenParameter *aParameter )
{
    PvString lName;
    bool bBufferResize =false;

    if ( !aParameter->GetName( lName ).IsOK() )
    {
        ASSERT( 0 ); // Totally unexpected  
        return;
    }

    if (  lName == "Width"  )
    {   
        int64_t value;
        mDevice.GetParameters()->GetIntegerValue( "Width", value );
        if (mWidth != (int)value)
        {
            mWidth = (int)value;
            bBufferResize  =true;
        }

    }
    else if (  lName == "Height" )
    {   
        int64_t value;
        mDevice.GetParameters()->GetIntegerValue( "Height", value );
        if ( mHeight != (int)value)
        {   
            mHeight = (int)value;
            bBufferResize  =true;
        }
    }
    else if (  lName == "PixelFormat" )
    {
        int64_t i64PixelFormat;
        mDevice.GetParameters()->GetEnumValue( "PixelFormat", i64PixelFormat );
        if (mpvPixelFormat != (int)i64PixelFormat)
        {
            mpvPixelFormat = (int)i64PixelFormat;
            miPixelSize =1;
            miPixelSize =((mpvPixelFormat & 0x00FF0000) >> 16)/8;
            bBufferResize  =true;
        }
    }
    else
    {
        CString str;
        str.Format(_T("An unregistered parameter update: %s updated"), lName.GetAscii());  
        ::AfxMessageBox(str);
    }

    if (bBufferResize)
    {
        if ( mPipeline->IsStarted())
        {
            mPipeline->Stop();
        }

        mPipeline->SetBufferSize( mWidth*mHeight*miPixelSize ); 
        mPipeline->Start();
    }

}

void CPlcAndGevEventsDlg::OnEvent( PvDevice *aDevice, uint16_t aEventID, uint16_t aChannel, uint64_t aBlockID, 
    uint64_t aTimestamp, const void *aData, uint32_t aDataLength )
{
    if (aEventID ==36869)
        miNbOfCallback++;
    else
    {
        CString str;
        str.Format(_T("Unexpected EventID %d, Channel %d, BlockID %d, Timestamp %d"), aEventID, aChannel, aBlockID, aTimestamp);
        AfxMessageBox(str);
    }
}

void CPlcAndGevEventsDlg::OnBnClickedOk()
{
    if (! mbInited)
    {
        OnOK();
        return;
    }
    
    Disconnect();
    OnOK();
}


void CPlcAndGevEventsDlg::Disconnect()
{
    if ( !mDevice.IsConnected() )
    {
        return;
    }

    if ( mDeviceWnd != NULL )
    {
        if ( mDeviceWnd->GetHandle() != 0 )
        {
            mDeviceWnd->Close();
        }
    }

    if ( g_bAcquisition )
    {
        OnBnClickedButton2();
    }

    //Unregister the PvDeviceEventSink
    if ( mbCallbackregistered )
    {
        mDevice.UnregisterEventSink(this);
    }

    if ( mbTimerOn )
    {
        KillTimer(1);
        mbTimerOn =false;
    }

    // Unregister image parameters                     
    PvGenInteger* lPvGenInteger;
    lPvGenInteger = mDevice.GetParameters()->GetInteger( "Width" );
    lPvGenInteger->UnregisterEventSink(this);
    lPvGenInteger = mDevice.GetParameters()->GetInteger( "Height" );
    lPvGenInteger->UnregisterEventSink(this);
    PvGenEnum* lPvGenEnum;
    lPvGenEnum = mDevice.GetParameters()->GetEnum("PixelFormat" );
    lPvGenEnum->UnregisterEventSink(this);
    
    mDevice.Disconnect();   
    
    if ( mPipeline != NULL )
    {
        if ( mPipeline->IsStarted() ) 
        {
            mPipeline->Stop();
        }

        delete mPipeline;
        mPipeline = NULL;
    }
   
    mStream.Close();
}

void CPlcAndGevEventsDlg::OnTimer(UINT_PTR nIDEvent)
{
    // TODO: Add your message handler code here and/or call default
    CString str;
    if (mbCallbackregistered)
    {
        int64_t value;
        mDevice.GetParameters()->GetIntegerValue( "CounterValue", value);
        miCurCounterValue =(int) value;
        if (g_bAcquisition)
            str.Format(_T("Grabbed frames =%i, Grab timeout =%i, PLC interrupt callback =%i, Counter value =%i"), g_iNbOfFrameGrabbed, g_iNbOfGrabTimeout, miNbOfCallback, miCurCounterValue);  
        else
            str.Format(_T("Received PLC interrupt callback =%i, Counter value =%i"), miNbOfCallback, miCurCounterValue);  
    
        SetDlgItemText(IDC_STATIC, str); 
    }
    else
    {
        if (g_bAcquisition)
            str.Format(_T("Grabbed frames =%i, Grab timeout =%i"), g_iNbOfFrameGrabbed, g_iNbOfGrabTimeout);  

        SetDlgItemText(IDC_STATIC, str); 
    }
    __super::OnTimer(nIDEvent);
}
