// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "afxwin.h"
#include "afxcmn.h"

#include <PvVirtualDeviceGEV.h>

#include "Constants.h"
#include "CurrentBuffersTable.h"
#include "StreamThread.h"
#include "TransmitterThread.h"
#include "IPAddress.h"
#include "BitmapButton.h"


///
/// \class CTransmitTiledImagesDlg
///
/// \brief Main dialog of the sample.
///
class CTransmitTiledImagesDlg : public CDialog
{
// Construction
public:
    CTransmitTiledImagesDlg(CWnd* pParent = NULL);  // standard constructor
    virtual ~CTransmitTiledImagesDlg();

// Dialog Data
    enum { IDD = IDD_TILINGSAMPLEMFC_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
    HICON m_hIcon;

    virtual void OnOK() {}
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()

public:

    /// Handlers
    afx_msg void OnBnClickedButtonConnect11();
    afx_msg void OnBnClickedButtonOptions11();
    afx_msg void OnBnClickedButtonConnect21();
    afx_msg void OnBnClickedButtonOptions21();
    afx_msg void OnBnClickedButtonConnect12();
    afx_msg void OnBnClickedButtonOptions12();
    afx_msg void OnBnClickedButtonConnect22();
    afx_msg void OnBnClickedButtonOptions22();
    afx_msg void OnBnClickedBtnStart();
    afx_msg void OnBnClickedBtnStop();
    afx_msg void OnCbnSelchangeComboIfAddress();
    afx_msg void OnTimer( UINT_PTR nIDEvent );
    afx_msg void OnEnKillfocusEditFps();
    afx_msg void OnEnKillfocusEditWidth();
    afx_msg void OnEnKillfocusEditHeight();
    afx_msg void OnDestroy();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnGetMinMaxInfo( MINMAXINFO *lpMMI );
    afx_msg void OnFileOpen();
    afx_msg void OnFileSave();
    afx_msg void OnFileSaveAs();
    afx_msg void OnFileExit();
    afx_msg void OnHelpAbout();
    afx_msg void OnAcceleratorOpen();
    afx_msg void OnAcceleratorSave();
    afx_msg void OnAcceleratorSaveAs();
    afx_msg void OnAcceleratorExit();
    afx_msg void OnCommunicationParameters();
    afx_msg void OnDeviceParameters();
    afx_msg void OnStreamParameters();

    ///
    /// \brief A stream thread has lost its connection
    /// 
    /// \param wp Row index of the stream
    /// \param lp Column index of the stream
    ///
    afx_msg LRESULT OnLinkDisconnected( WPARAM wp, LPARAM lp );

    ///
    /// \brief A stream thread buffer re-allocation fail at run-time
    /// 
    /// \param wp Row index of the stream
    /// \param lp Column index of the stream
    ///
    afx_msg LRESULT OnReAllocationFail( WPARAM wp, LPARAM lp );
    
    ///
    /// \brief The transmitter memory re-allocation fail at run-time
    /// 
    /// \param wp Not used
    /// \param lp Not used
    ///
    afx_msg LRESULT OnConversionFail( WPARAM wp, LPARAM lp );

private:

    // GUI elements
    CButton mButtonConnect[ MAX_TILES_ROW ][ MAX_TILES_COLUMN ];
    CButton mButtonOptions[ MAX_TILES_ROW ][ MAX_TILES_COLUMN ];
    CStatic mStaticConnection[ MAX_TILES_ROW ][ MAX_TILES_COLUMN ];
    BitmapButton mButtonStart;
    BitmapButton mButtonStop;
    IPAddress mEditAddress;
    CEdit mEditPort;
    CEdit mEditFps;
    CEdit mEditPacketSize;
    CEdit mStatusBar;
    CEdit mEditWidth;
    CEdit mEditHeight;
    CComboBox mComboIfAddress;
    CComboBox mTilingCombo;

private:
    ///
    /// \brief Update the stats in the bar
    ///
    void UpdateStats();

    ///
    /// Enable / Disable the GUI element to set the proper state
    ///
    /// \param aRunning New state of the GUI, true for running
    ///
    void SetGUIInRunningState( BOOL aRunning );

    ///
    /// \brief Process to the connection / disconnection handling of a source
    ///
    /// \param aRow Row index of the source
    /// \param aColumn Column index of the source
    ///
    void ProcessConnection( int aRow, int aColumn );

    ///
    /// \brief Show the configuration dlg of a source
    ///
    /// \param aRow Row index of the source
    /// \param aColumn Column index of the source
    ///
    void ProcessOptions( int aRow, int aColumn );

    ///
    /// \brief Connect a source
    ///
    /// \param aRow Row index of the source
    /// \param aColumn Column index of the source
    ///  
    void Connect( int aRow, int aColumn );

    ///
    /// \brief Disconnect a source
    ///
    /// \param aRow Row index of the source
    /// \param aColumn Column index of the source
    ///  
    void Disconnect( int aRow, int aColumn );

    ///
    /// \brief Check is any source is connected
    ///
    /// \return true if at least one source is connected
    ///
    bool IsConnected();

    /// 
    /// \brief Start the streaming
    ///
    void Start();

    ///
    /// \brief Stop the streaming
    ///
    void Stop();

    ///
    /// \brief Save the project to a file
    /// if filename empty, the filename is asked to user
    ///
    void SaveAs( CString& aFileName );

    ///
    /// \brief Open a project from a file
    ///
    void Open();

    ///
    ///  \brief Set the internal transmitter configuration to the GUI
    ///
    /// \param aTransmitterConfig Data
    ///
    void SetTransmitterConfig( TransmitterConfig& aTransmitterConfig );

    ///
    ///  \brief Get the internal transmitter configuration from the GUI
    ///
    /// \param aTransmitterConfig Data
    ///
    void GetTransmitterConfig( TransmitterConfig& aTransmitterConfig );

    ///
    /// \brief Format the display for a stream
    ///
    /// \param aStreamThread The stream to based our display on
    /// \param aDisplay The formatted string
    ///
    void FormatDisplay( StreamThread* aStreamThread, CString& aDisplay );

private:

    // Transmitter virtual device used to answer the query of finders
    PvVirtualDeviceGEV mVirtualDeviceGEV;

    // eBUS SDK display used to display the image 
    PvDisplayWnd mDisplay;

    // Shared table to store the last buffer of each stream and manage 
    // the memory properly
    CurrentBuffersTable mCurrentBuffersTable;

    // List of image stream threads
    StreamThread mStreamThreads[ MAX_TILES_ROW ][ MAX_TILES_COLUMN ];
    StreamThread *mActiveStreamThread;

    // Transmitter thread
    TransmitterThread mTransmitterThread;  

    // Internal timer used to update the stats
    UINT_PTR mTimer;

    // Keep track of the client rectangle of the application
    CRect mClientRectangle;

    // Minimum Dialog Size allowed
    int mMinSizeX;
    int mMinSizeY;

    // Menu accelerator table
    HACCEL mAccelerator; 

    // Keep the last file name to manage the Save menu
    CString mLastFileName;

    // The transmitter configuration
    TransmitterConfig mTransmitterConfig;

};
