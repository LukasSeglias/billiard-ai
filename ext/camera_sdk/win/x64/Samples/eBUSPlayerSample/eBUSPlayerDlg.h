// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __EBUSPLAYERDLG_H__
#define __EBUSPLAYERDLG_H__


#include "resource.h"

#include "ActionCommandDlg.h"
#include "BitmapButton.h"
#include "EventMonitorDlg.h"
#include "FilteringDlg.h"
#include "ImageSaveDlg.h"
#include "RegisterInterfaceDlg.h"

#include <Player.h>
#include <IProgress.h>
#include <IPlayerController.h>

#include <PvGenBrowserWnd.h>
#include <PvDisplayWnd.h>
#include <PvSerialTerminalWnd.h>

#include <list>
#include <vector>

#include "afxcmn.h"


#define WM_IMAGEDISPLAYED ( WM_USER + 0x4431 )

#define MIN_HEIGHT ( 547 )
#define MIN_FG_HEIGHT ( 578 )

#define DONTSHOWNODRIVERWARNING ( _T( "DontShowNoDriverWarning" ) )
#define DONTSHOWCAMERABRIDGE ( _T( "DontShowCameraBridge" ) )
#define PCFAVAILABLE ( _T( "PleoraCameraFiles" ) )


class PvGenStateStack;


class eBUSPlayerDlg : public CDialog, IPlayerController
{
public:

    eBUSPlayerDlg( CString aFileName, CWnd *pParent = NULL );
    virtual ~eBUSPlayerDlg();

    enum { IDD = IDD_EBUSPLAYER };

    void StartStreaming();
    void StopStreaming();

    static bool IsMultiSourceTransmitter( PvDevice *aDevice );
    static DWORD ReadRegistryDWORD( const CString &aName );

    virtual CString GetAppName() const { return _T( "eBUS Player" ); }

protected:

    HICON m_hIcon;

    enum StatusColor { SCDefault, SCRed, SCYellow };

    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnGetMinMaxInfo( MINMAXINFO *lpMMI );
    afx_msg void OnBnClickedDeviceButton();
    afx_msg void OnBnClickedLinkButton();
    afx_msg void OnBnClickedStreamparamsButton();
    afx_msg void OnBnClickedConnectButton();
    afx_msg void OnBnClickedDisconnectButton();
    afx_msg void OnClose();
    LRESULT OnImageDisplayed( WPARAM wParam, LPARAM lParam );
    afx_msg void OnBnClickedStart();
    afx_msg void OnBnClickedStop();
    afx_msg void OnCbnSelchangeMode();
    afx_msg void OnFileLoad();
    afx_msg void OnFileSave();
    afx_msg void OnFileExit();
    afx_msg void OnToolsEventMonitor();
    afx_msg void OnHelpAbout();
    afx_msg void OnInitMenu(CMenu* pMenu);
    afx_msg void OnMove(int x, int y);
    afx_msg void OnRecent(UINT nID);
    afx_msg void OnFileSaveAs();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg void OnAcceleratorSave();
    afx_msg void OnAcceleratorOpen();
    afx_msg void OnToolsSetup();
    afx_msg void OnToolsImagefiltering();
    afx_msg void OnToolsSerialCommunicationBridge();
    afx_msg void OnToolsCameraBridge();
    afx_msg void OnToolsActionCommand();
    HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnToolsSaveimages();
    afx_msg void OnToolsSavecurrentimage();
    afx_msg void OnToolsSerialCommunication();
    afx_msg void OnDestroy();
    afx_msg void OnTimer( UINT_PTR nIDEvent );
    afx_msg void OnRegisterInterface();
    afx_msg void OnDisplayDisabled();
    afx_msg void OnDisplay30FPS();
    afx_msg void OnDisplay60FPS();
    afx_msg void OnToolsSavepreferences();
    afx_msg void OnToolsRestoredefaultpreferences();
    afx_msg void OnToolsBufferoptions();
	afx_msg void OnToolsResetStreamingStatistics();
    afx_msg void OnToolsDisplaypartialimages();
    afx_msg void OnToolsDisplaychunkdata();
    afx_msg void OnToolsSaveXml();
    afx_msg void OnToolsDeinterlacingDisabled();
    afx_msg void OnToolsDeinterlacingWeavingHalf();
    afx_msg void OnToolsDeinterlacingWeavingFull();
    afx_msg void OnToolsDeinterlacingBlending();
    afx_msg void OnToolsDeinterlacingLineDoubling();
    afx_msg void OnToolsDefaultGEVCommunicationParameters();
    afx_msg void OntoolsDefaultU3VCommunicationParameters();
    afx_msg void OnCbnSelchangeComboSource();
    afx_msg void OnToolsFileTransfer();
    virtual void OnOK() {}
    virtual void OnCancel() {}
    LRESULT OnLinkDisconnected( WPARAM wParam, LPARAM lParam );
    LRESULT OnLinkReconnected( WPARAM wParam, LPARAM lParam );
    LRESULT OnAcquisitionStateChanged( WPARAM wParam, LPARAM lParam );
    LRESULT OnUpdateSource( WPARAM wParam, LPARAM lParam );
    LRESULT OnUpdateSources( WPARAM wParam, LPARAM lParam );
    LRESULT OnUpdateAcquisitionMode( WPARAM wParam, LPARAM lParam );
    LRESULT OnUpdateAcquisitionModes( WPARAM wParam, LPARAM lParam );
    DECLARE_MESSAGE_MAP()

    void EnableInterface();
    void EnableTreeBrowsers( BOOL aEnabled );
    void EnableControls( BOOL aEnabled );
    void UpdateDeviceAttributes();
    void CreateEventMonitor();

    virtual void Connect( const PvDeviceInfo *aDI );
    virtual void Disconnect( BOOL aLinkDisconnected = false );
    
    void PersistenceWarning();

    void OpenConfig( const CString &aFileName );
    void OpenBrowserOptions( PvConfigurationReader *aReader );
    void SaveConfig( const CString &aLocation, bool aSaveConnectedDevice );
    void SaveBrowserOptions( PvConfigurationWriter *aWriter );

    bool HaveBrowserOptionsChanged();
    void ResetBrowserOptionsChanged();
    void ResetChangedSession();
    void ResetChangedPreferences();

    void ShowGenWnd( PvGenBrowserWnd *aWnd, PvGenParameterArray *aParams, const CString &aTitle );
    void CloseWnd( PvWnd *aWnd );

    void InitFileMenu( CMenu *aMenuItem, UINT aID, int aIndex );
    void InitToolsMenu( CMenu *aMenuItem, UINT aID, int aIndex );
    virtual void InitOtherMenus( CMenu *aMenuItem, UINT aID, int aIndex ) {}

    void ReportMRU(CString aFileName);
    void SaveMRUToRegistry();
    void LoadMRUFromRegistry();
    void UpdateMRUMenu();

    void SetStatusColor( StatusColor aColor );

    void StartTimer();
    void StopTimer();

    IProgress *CreateProgressDialog();
    intptr_t PostMsg( uint32_t aMessage, uintptr_t wParam = 0, intptr_t lParam = 0 ) { return CWnd::PostMessage( aMessage, wParam, lParam ); }
    intptr_t SendMsg( uint32_t aMessage, uintptr_t wParam = 0, intptr_t lParam = 0 ) { return CWnd::SendMessage( aMessage, wParam, lParam ); }
    intptr_t SendMsgIfPossible( uint32_t aMessage, uintptr_t wParam = 0, intptr_t lParam = 0 );

    CString GetStickyPath();
    CString GetDefaultPath();

    // Methods having different implementation depending on the eBUS Player variant
    void VerifySecret();

    std::list<CString> mRecentList;
    CString mFileName;
    CMenu* mMRUMenu;

    CRect mCrt;
    BOOL mNeedInit;

    CComboBox mModeCombo;
    BitmapButton mPlayButton;
    BitmapButton mStopButton;
    CEdit mStatusTextBox;

    CEdit mIPEdit;
    CEdit mMACEdit;
    CEdit mGUIDEdit;
    CEdit mVendorEdit;
    CEdit mModelEdit;
    CEdit mNameEdit;

    CComboBox mSourceCombo;

    CButton mCommunicationButton;
    CButton mDeviceButton;
    CButton mCameraButton;
    CButton mStreamButton;

    HACCEL mAccel; // accelerator table

    StatusColor mStatusColor;

    DWORD mYellowColor;
    CBrush mYellowBrush;

    DWORD mRedColor;
    CBrush mRedBrush;

    UINT_PTR mTimer;

    Player *mPlayer;

    PvGenBrowserWnd *mDeviceWnd;
    PvGenBrowserWnd mCommunicationWnd;
    PvGenBrowserWnd mStreamParametersWnd;
    PvGenBrowserWnd mDefaultCommU3VParametersWnd;
    PvGenBrowserWnd mDefaultCommGEVParametersWnd;
    PvSerialTerminalWnd mSerialTerminalWnd;
    ActionCommandDlg *mActionCommandDlg;

    EventMonitorDlg *mEventMonitorDlg;
    FilteringDlg *mFilteringDlg;
    RegisterInterfaceDlg mRegisterInterfaceDlg;

    IPvDisplayAdapter mDisplay;
    PvString mBrowserOptionsBaseline;

};


#endif // __EBUSPLAYERDLG_H__

