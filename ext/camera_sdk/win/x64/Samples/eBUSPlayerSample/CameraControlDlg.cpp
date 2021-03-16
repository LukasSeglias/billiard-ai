// *****************************************************************************
//
//     Copyright (c) 2016, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"
#include "CameraControlDlg.h"
#include "PvMessageBox.h"


IMPLEMENT_DYNAMIC(CameraControlDlg, CDialog)

BEGIN_MESSAGE_MAP(CameraControlDlg, CDialog)
    ON_BN_CLICKED(IDOK, &CameraControlDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &CameraControlDlg::OnBnClickedCancel)
    ON_BN_CLICKED(IDC_PCFRADIO, &CameraControlDlg::OnBnClickedRadio)
    ON_BN_CLICKED(IDC_CLPRADIO, &CameraControlDlg::OnBnClickedRadio)
    ON_BN_CLICKED(IDC_GENCPRADIO, &CameraControlDlg::OnBnClickedRadio)
    ON_BN_CLICKED(IDC_MANUALRADIO, &CameraControlDlg::OnBnClickedRadio)
    ON_BN_CLICKED(IDC_PCFPATHBUTTON, &CameraControlDlg::OnBnClickedPCFPathButton)
    ON_EN_CHANGE(IDC_PCFPATHEDIT, &CameraControlDlg::OnEditChangePathEdit)
    ON_CBN_SELCHANGE(IDC_CLPTEMPLATECOMBO, &CameraControlDlg::OnCbnSelchangeCLPTemplate)
END_MESSAGE_MAP()


///
/// \brief Constructor
///

CameraControlDlg::CameraControlDlg( PvCameraBridge *aCameraBridge, CWnd* pParent, bool aPCFVisible )
    : CDialog( CameraControlDlg::IDD, pParent )
    , mCameraBridge( aCameraBridge )
    , mDontShowAgain( false )
    , mPCFVisible( aPCFVisible )
{
}


///
/// \brief Destructor
///

CameraControlDlg::~CameraControlDlg()
{
}


///
/// \brief Standard dialog DoDataExchange
///

void CameraControlDlg::DoDataExchange( CDataExchange* pDX )
{
    CDialog::DoDataExchange( pDX );
    DDX_Control( pDX, IDC_PCFRADIO, mPCFRadio );
    DDX_Control( pDX, IDC_PCFPATHLABEL, mPCFPathLabel );
    DDX_Control( pDX, IDC_PCFPATHEDIT, mPCFPathEdit );
    DDX_Control( pDX, IDC_PCFPATHBUTTON, mPCFPathButton );
    DDX_Control( pDX, IDC_CLPRADIO, mCLPRadio );
    DDX_Control( pDX, IDC_CLPTEMPLATELABEL, mCLPTemplateLabel );
    DDX_Control( pDX, IDC_CLPTEMPLATECOMBO, mCLPTemplateCombo );
    DDX_Control( pDX, IDC_GENCPRADIO, mGenCPRadio );
    DDX_Control( pDX, IDC_MANUALRADIO, mManualRadio );
    DDX_Control( pDX, IDC_MANUALDESCRIPTION, mManualDescription );
    DDX_Control( pDX, IDC_DONTSHOWAGAINCHECK, mDontShowAgainCheckBox );
    DDX_Control( pDX, IDOK, mOKButton );
    DDX_Control( pDX, IDCANCEL, mCancelButton );
    DDX_Control( pDX, IDC_NOPOCLRADIO, mNoPoCLRadio );
    DDX_Control( pDX, IDC_POCLRADIO, mPoCLRadio );
}


///
/// \brief OK button click event handler
///

void CameraControlDlg::OnBnClickedOk()
{   

    // Update the PoCL status
    int lPoCLChecked = GetCheckedRadioButton( IDC_POCLRADIO, IDC_NOPOCLRADIO );
    PvCameraBridge::SetPoCLEnabled( mCameraBridge->GetDevice(), lPoCLChecked == IDC_POCLRADIO );

    // Configure the Camera Link option used to control the camera
    int lOptionChecked = GetCheckedRadioButton( IDC_CLPRADIO, IDC_MANUALRADIO );
    switch ( lOptionChecked )
    {
    case IDC_PCFRADIO:
        if ( !ConfigurePCF() )
        {
            return;
        }
        break;

    case IDC_CLPRADIO:
        if ( !ConfigureCLP() )
        {
            return;
        }
        break;

    case IDC_GENCPRADIO:
        if ( !ConfigureGenCP() )
        {
            return;
        }
        break;

    default:
        break;
    }

    mDontShowAgain = ( mDontShowAgainCheckBox.GetCheck() == BST_CHECKED );
    CDialog::OnOK();
}


///
/// \brief Cancel button click event handler
///

void CameraControlDlg::OnBnClickedCancel()
{
    mDontShowAgain = mDontShowAgainCheckBox.GetCheck() == BST_CHECKED;
    CDialog::OnCancel();
}


///
/// \brief Init dialog event handler
///

BOOL CameraControlDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Select default bridge type
    CheckRadioButton( IDC_CLPRADIO, IDC_MANUALRADIO, IDC_MANUALRADIO );
    
    // Set PoCL as configured on the device
    if ( PvCameraBridge::IsPoCLEnabled( mCameraBridge->GetDevice() ) )
    {
        CheckRadioButton( IDC_POCLRADIO, IDC_NOPOCLRADIO, IDC_POCLRADIO );
    }
    else
    {
        CheckRadioButton( IDC_POCLRADIO, IDC_NOPOCLRADIO, IDC_NOPOCLRADIO );
    }

    // Get CLProtocol options
    mTemplates.Clear();
    PvResult lResult = PvCameraBridge::GetCLProtocolTemplates( mTemplates );
    if ( lResult.IsOK() )
    {
        mCLPTemplateCombo.ResetContent();

        // Load options in combo box
        PvString *lTemplate = mTemplates.GetFirst();
        while ( lTemplate != NULL )
        {
            // Make the raw ID pretty for the UI
            CString lString = lTemplate->GetUnicode();
            for ( int i = 0; i < 2; i++ )
            {
                int lIndex = lString.Find( _T( "#" ) );
                if ( lIndex > 0 )
                {
                    // Bad schema, bad devices should be discarded at CLPort level. 
                    // Just keep XML version.
                    lString = lString.Right( lString.GetLength() - ( lIndex + 1 ) );
                }
            }
            lString.Replace( _T( "#" ), _T( " " ) );

            mCLPTemplateCombo.AddString( lString );
            lTemplate = mTemplates.GetNext();
        }

        // Select default
        if ( mCLPTemplateCombo.GetCount() > 0 )
        {
            mCLPTemplateCombo.SetCurSel( 0 );
        }
    }

    EnableInterface();

    return TRUE;
}


///
/// \brief Sets the enabled state of the dialog
///

void CameraControlDlg::EnableInterface()
{
    int lChecked = GetCheckedRadioButton( IDC_CLPRADIO, IDC_MANUALRADIO );

    bool lPCFEnabled = ( lChecked == IDC_PCFRADIO );
    bool lCLPEnabled = ( lChecked == IDC_CLPRADIO );
    bool lGenCPEnabled = ( lChecked == IDC_GENCPRADIO );
    bool lManualEnabled = ( lChecked == IDC_MANUALRADIO );

    bool lValid = true;

    // PCF is valid if we have some path
    if ( lPCFEnabled )
    {
        CString lText;
        mPCFPathEdit.GetWindowText( lText );
        lValid = lText.GetLength() > 0;
    }

    // CLP is valid is a template is selected
    if ( lCLPEnabled )
    {
        lValid = mCLPTemplateCombo.GetCurSel() >= 0;
    }

    // PCF option is configurable
    mPCFRadio.ShowWindow( mPCFVisible ? SW_SHOW : SW_HIDE );
    mPCFRadio.EnableWindow( mPCFVisible );

    // PCF specific
    mPCFPathLabel.ShowWindow( mPCFVisible ? SW_SHOW : SW_HIDE );
    mPCFPathLabel.EnableWindow( lPCFEnabled );
    mPCFPathEdit.ShowWindow( mPCFVisible ? SW_SHOW : SW_HIDE );
    mPCFPathEdit.EnableWindow( lPCFEnabled );
    mPCFPathButton.ShowWindow( mPCFVisible ? SW_SHOW : SW_HIDE );
    mPCFPathButton.EnableWindow( lPCFEnabled );

    // CLP specific
    mCLPTemplateLabel.EnableWindow( lCLPEnabled );
    mCLPTemplateCombo.EnableWindow( lCLPEnabled );

    // OK button
    mOKButton.EnableWindow( lValid );

    // Manual mode specific
    mManualDescription.EnableWindow( lManualEnabled );

    // PoCL
    mPoCLRadio.EnableWindow( ( mCameraBridge != NULL ) && PvCameraBridge::IsPoCLSupported( mCameraBridge->GetDevice() ) );
}


///
/// \brief Radio button click handler
///

void CameraControlDlg::OnBnClickedRadio()
{
    EnableInterface();
}


///
/// \brief PCF path button: select PCF file location
///

void CameraControlDlg::OnBnClickedPCFPathButton()
{
    CFileDialog lFileDlg( TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, L"Pleora Camera Files (*.yml)|*.yml|All files (*.*)|*.*||", this );
    lFileDlg.m_ofn.lpstrTitle = L"Select Pleora Camera File";
    if ( lFileDlg.DoModal() == IDOK )
    {
        mPCFPathEdit.SetWindowText( lFileDlg.GetPathName() );
    }
}


///
/// \brief Configures the camera bridge with a Pleora Camera File
///

bool CameraControlDlg::ConfigurePCF()
{
    CString lPath;
    mPCFPathEdit.GetWindowText( lPath );
    if ( lPath.IsEmpty() )
    {
        return false;
    }

    PvResult lResult;
    {
        CWaitCursor lCursor;
        lResult = mCameraBridge->ConnectPleoraCameraFile( (LPCTSTR)lPath );
    }

    if ( !lResult.IsOK() )
    {
        PvMessageBox( this, lResult );
        return false;
    }

    return true;
}


///
/// \brief Configures the camera bridge for CL Protocol
///

bool CameraControlDlg::ConfigureCLP()
{
    int lSel = mCLPTemplateCombo.GetCurSel();
    PvString *lTemplate = mTemplates.GetItem( lSel );
    if ( lTemplate == NULL )
    {
        return false;
    }

    PvResult lResult;
    {
        CWaitCursor lCursor;
        lResult = mCameraBridge->ConnectCLProtocol( *lTemplate );
    }

    if ( !lResult.IsOK() )
    {
        PvMessageBox( this, lResult );
        return false;
    }

    return true;
}


///
/// \brief Configures the camera bridge for GenCP
///

bool CameraControlDlg::ConfigureGenCP()
{
    PvResult lResult;
    {
        CWaitCursor lCursor;
        lResult = mCameraBridge->ConnectGenCP();
    }

    if ( !lResult.IsOK() )
    {
        PvMessageBox( this, lResult );
        return false;
    }

    return true;
}


///
/// \brief Pleora Camera File path text box content changed handler
///

void CameraControlDlg::OnEditChangePathEdit()
{
    EnableInterface();
}


///
/// \brief CLProtocol template combo box selection chanaged handler
///

void CameraControlDlg::OnCbnSelchangeCLPTemplate()
{
    EnableInterface();
}


