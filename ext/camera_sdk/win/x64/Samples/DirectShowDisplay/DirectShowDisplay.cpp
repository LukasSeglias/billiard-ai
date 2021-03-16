// *****************************************************************************
//
//    Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

//
// Shows how to use the eBUS SDK DirectShow source filter to receive images
// from a GigE Vision or USB3 Vision device which is also being controlled.
//
// The images are simply sent to the Microsoft Video Mixing Renderer 9.
//
// It is possible to control the eBUS SDK DirectShow source filter using 
// its IPvDSSource COM interface.
//
// This sample focuses mainly on using the eBUS SDK DirectShow Source filter and
// assumes good knowledge of DirectShow and COM.
//

#include <PvSampleUtils.h>

#include <dshow.h>

#include <initguid.h>
#include <PvDSSourceUIDs.h>
#include <IPvDSSource.h>


PV_INIT_SIGNAL_HANDLER();

#define SAFE_RELEASE( a ) \
    if ( a != NULL ) \
    { \
        a->Release(); \
        a = NULL; \
    }


// Forward declarations
HRESULT GetFirstPin( IBaseFilter *aFilter, IPin **aPin );
BOOL CreateDisplayWindow();
void UpdateDisplayPosition();
LRESULT CALLBACK WndProc( HWND aHwnd, UINT aMsg, WPARAM wParam, LPARAM lParam );

// Globals
HWND gWindowHandle = NULL;
IVideoWindow *gVideoWindow = NULL;


//
// Main function
//
int main()
{
    PV_SAMPLE_INIT();

    PvString lConnectionID;
    if ( !PvSelectDevice( &lConnectionID ) )
    {
        return -1;
    }

    // Initialize the COM library
    ::CoInitialize( NULL );

    // Create the filter graph manager
    std::cout << "Creating filter graph manager" << std::endl;
    IGraphBuilder *lGraph = NULL;
    HRESULT lResult = ::CoCreateInstance( 
        CLSID_FilterGraph, 
        NULL, 
        CLSCTX_INPROC_SERVER, 
        IID_IGraphBuilder, 
        (void **)&lGraph );
    if ( FAILED( lResult ) )
    {
        // ...
    }

    // Create eBUS SDK PvDSSource filter
    std::cout << "Creating eBUS SDK PvDSSource filter" << std::endl;
    IBaseFilter *lSourceFilter = NULL;
    lResult = ::CoCreateInstance( 
        CLSID_PvDSSource, 
        NULL, 
        CLSCTX_INPROC_SERVER, 
        IID_IBaseFilter, 
        (void **)&lSourceFilter);
    if ( FAILED( lResult ) )
    {
        // ...
    }

    // Get interface used to control the eBUS SDK PvDSSource filter from source filter
    IPvDSSource *lDSSource = NULL;
    lResult = lSourceFilter->QueryInterface( IID_IPvDSSource, (void **)&lDSSource );
    if ( FAILED( lResult ) )
    {
        // ...
    }

    // Configure filter. 
    //
    // It is important to have width, height and interlacing configured *BEFORE* connecting the filters.
    // Connecting the filter to the GigE Vision or USB3 Vision device automatically updates the filter 
    // width, height and interlacing configuration.
    //
    std::cout << "Configuring and connecting eBUS SDK PvDSSource filter to device" << std::endl;
    BSTR lDeviceID = ::SysAllocStringLen( lConnectionID.GetUnicode(), lConnectionID.GetLength() );
    lDSSource->put_DeviceID( lDeviceID );
    ::SysFreeString( lDeviceID );
    lDSSource->put_DropThreshold( 4 ); // The filter will drop frames if more than 4 frames are available to limit latency
    lDSSource->put_Role( 0 ); // Controller AND data receiver
    lDSSource->put_DiagnosticEnabled( TRUE ); // Diagnostics will be displayed on the images
    lResult = lDSSource->ConnectIfNeeded();
    if ( FAILED( lResult ) )
    {
        // ...
    }

    // Create display window
    std::cout << "Creating display window" << std::endl;
    BOOL lSuccess = CreateDisplayWindow();
    if ( !lSuccess )
    {
        // ...
    }

    // Create video renderer
    std::cout << "Creating video renderer filter" << std::endl;
    IBaseFilter *lVideoFilter = NULL;
    lResult = ::CoCreateInstance( 
        CLSID_VideoMixingRenderer9,
        NULL, 
        CLSCTX_INPROC_SERVER, 
        IID_IBaseFilter, 
        (void **)&lVideoFilter );

    // Add source and display filters to the graph
    std::cout << "Adding filters to graph" << std::endl;
    lResult = lGraph->AddFilter( lSourceFilter, L"Source" );
    if ( FAILED( lResult ) )
    {
        // ...
    }
    lResult = lGraph->AddFilter( lVideoFilter, L"Display" );
    if ( FAILED( lResult ) )
    {
        // ...
    }

    // Get pins of out filters: easy, we want the 1st pin of both filters.
    std::cout << "Retrieving filter pins" << std::endl;
    IPin *lSourcePin = NULL;
    lResult = GetFirstPin( lSourceFilter, &lSourcePin );
    if ( FAILED( lResult ) )
    {
        // ...
    }
    IPin *lDisplayPin = NULL;
    lResult = GetFirstPin( lVideoFilter, &lDisplayPin );
    if ( FAILED( lResult ) )
    {
        // ...
    }

    // Connect the pins. The graph will instantiate required intermediate filters, if any.
    std::cout << "Connecting filter pins" << std::endl;
    lResult = lGraph->Connect( lSourcePin, lDisplayPin );
    if ( FAILED( lResult ) )
    {
        // ...
    }

    // Get the video window interface from the graph
    gVideoWindow = NULL;
    lResult = lGraph->QueryInterface( IID_IVideoWindow, (void **)&gVideoWindow );
    if ( FAILED( lResult ) )
    {
        // ...
    }

    // Attach display to our window
    gVideoWindow->put_Owner( (OAHWND)gWindowHandle );
    gVideoWindow->put_MessageDrain( (OAHWND)gWindowHandle );
    gVideoWindow->put_WindowStyle( WS_CHILD | WS_CLIPSIBLINGS );
    UpdateDisplayPosition();

    // Get the media control interface from the graph
    std::cout << "Retrieving media control interface from graph" << std::endl;
    IMediaControl *lMediaControl = NULL;
    lResult = lGraph->QueryInterface( IID_IMediaControl, (void **)&lMediaControl );
    if ( FAILED( lResult ) )
    {
        // ...
    }

    // Start graph.
    std::cout << "Starting the graph" << std::endl;
    lResult = lMediaControl->Run();
    if ( FAILED( lResult ) )
    {
        // ...
    }

    // Process display window messages, until it is closed.
    std::cout << "Processing window messages until window terminates" << std::endl;
    MSG lMsg;
    while ( ::GetMessage( &lMsg, NULL, 0, 0 ) > 0 )
    {
        TranslateMessage( &lMsg );
        DispatchMessage( &lMsg );
    }

    // Stop graph.
    std::cout << "Stopping the graph" << std::endl;
    lResult = lMediaControl->Stop();
    if ( FAILED( lResult ) )
    {
        // ...
    }

    // Release acquired interfaces
    SAFE_RELEASE( lMediaControl );
    SAFE_RELEASE( gVideoWindow );
    SAFE_RELEASE( lDisplayPin );
    SAFE_RELEASE( lSourcePin );
    SAFE_RELEASE( lVideoFilter );
    SAFE_RELEASE( lDSSource );
    SAFE_RELEASE( lSourceFilter );
    SAFE_RELEASE( lGraph );

    // Release COM library
    ::CoUninitialize();

    PV_SAMPLE_TERMINATE();

    return 0;
}


// Returns the first pin of a filter
HRESULT GetFirstPin( IBaseFilter *aFilter, IPin **aPin )
{
    // Get pin enumeration interface from the filter
    IEnumPins *lEP = NULL;
    HRESULT lResult = aFilter->EnumPins( &lEP );
    if ( FAILED( lResult ) )
    {
        return lResult;
    }

    // Retrieve the first pin of the filter
    ULONG lFetched = 0;
    lResult = lEP->Next( 1, aPin, &lFetched );
    if ( FAILED( lResult ) )
    {
        return lResult;
    }

    // Release enumerator. Caller is responsible of releasing the pin.
    lEP->Release();

    return NOERROR;
}


// Creates a window where the display will draw
BOOL CreateDisplayWindow()
{
    const wchar_t lClassName[] = L"DisplayWindowClass";

    WNDCLASSEX lWindowClass;
    lWindowClass.cbSize = sizeof(WNDCLASSEX);
    lWindowClass.style = 0;
    lWindowClass.lpfnWndProc = WndProc;
    lWindowClass.cbClsExtra = 0;
    lWindowClass.cbWndExtra = 0;
    lWindowClass.hInstance = ::AfxGetInstanceHandle();
    lWindowClass.hIcon = ::LoadIcon( NULL, IDI_APPLICATION );
    lWindowClass.hCursor = ::LoadCursor( NULL, IDC_ARROW );
    lWindowClass.hbrBackground = (HBRUSH)( COLOR_WINDOW + 1 );
    lWindowClass.lpszMenuName = NULL;
    lWindowClass.lpszClassName = lClassName;
    lWindowClass.hIconSm = ::LoadIcon( NULL, IDI_APPLICATION );

    if( !::RegisterClassEx( &lWindowClass ) )
    {
        return FALSE;
    }

    gWindowHandle = ::CreateWindowEx(
        WS_EX_CLIENTEDGE,
        lClassName,
        L"DirectShowDisplay",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1024, 768,
        NULL, NULL, ::AfxGetInstanceHandle(), NULL );
    if ( gWindowHandle == NULL )
    {
        return FALSE;
    }

    ::ShowWindow( gWindowHandle, SW_SHOW );
    ::UpdateWindow( gWindowHandle );

    return TRUE;
}


// Updates the display position
void UpdateDisplayPosition()
{
    RECT lRect;
    ::GetClientRect( gWindowHandle, &lRect );

    if ( gVideoWindow != NULL )
    {
        gVideoWindow->SetWindowPosition( 0, 0, lRect.right, lRect.bottom );
    }
}


// Display window message handler
LRESULT CALLBACK WndProc( HWND aHwnd, UINT aMsg, WPARAM wParam, LPARAM lParam )
{
    switch( aMsg )
    {
    case WM_CLOSE:
        DestroyWindow( aHwnd );
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_LBUTTONDBLCLK :
        if ( gVideoWindow != NULL )
        {
            LONG lCurrent = 0;
            gVideoWindow->get_FullScreenMode( &lCurrent );
            gVideoWindow->put_FullScreenMode( ( lCurrent == OATRUE ) ? OAFALSE : OATRUE );
        }
        break;

    case WM_SIZE:
        UpdateDisplayPosition();
        break;

    default:
        return DefWindowProc( aHwnd , aMsg, wParam, lParam );
    }

    return 0;
}


