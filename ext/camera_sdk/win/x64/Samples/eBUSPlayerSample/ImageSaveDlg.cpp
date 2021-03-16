// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"
#include "ImageSaveDlg.h"


IMPLEMENT_DYNAMIC(ImageSaveDlg, CDialog)

BEGIN_MESSAGE_MAP(ImageSaveDlg, CDialog)
    ON_BN_CLICKED(IDOK, &ImageSaveDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &ImageSaveDlg::OnBnClickedCancel)
    ON_BN_CLICKED(IDC_ONEOUTOF_RADIO, &ImageSaveDlg::OnBnClickedOneoutofRadio)
    ON_BN_CLICKED(IDC_MAXRATE_RADIO, &ImageSaveDlg::OnBnClickedMaxrateRadio)
    ON_BN_CLICKED(IDC_AVERAGETHROUGHPUT_RADIO, &ImageSaveDlg::OnBnClickedAveragethroughputRadio)
    ON_BN_CLICKED(IDC_NOTHROTTLE_RADIO, &ImageSaveDlg::OnBnClickedNothrottleRadio)
    ON_BN_CLICKED(IDC_LOCATION_BUTTON, &ImageSaveDlg::OnBnClickedLocationButton)
    ON_BN_CLICKED(IDC_IMAGESAVEENABLE, &ImageSaveDlg::OnBnClickedImagesaveenable)
    ON_CBN_SELCHANGE(IDC_FORMAT, &ImageSaveDlg::OnCbnSelchangeFormat)
END_MESSAGE_MAP()


///
/// \brief Constructor
///

ImageSaveDlg::ImageSaveDlg( ImageSaving *aImageSaving, CWnd* pParent /*=NULL*/ )
    : CDialog( ImageSaveDlg::IDD, pParent )
    , mImageSaving( aImageSaving )
{
}


///
/// \brief Destructor
///

ImageSaveDlg::~ImageSaveDlg()
{
}


///
/// \brief Standard dialog DoDataExchange
///

void ImageSaveDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_ONEOUTOF, mOneOutOfEdit);
    DDX_Control(pDX, IDC_MAXRATE, mMaxRateEdit);
    DDX_Control(pDX, IDC_AVERAGETHROUGHPUT, mAverageThroughputEdit);
    DDX_Control(pDX, IDC_LOCATION, mSavePathEdit);
    DDX_Control(pDX, IDC_IMAGESAVEENABLE, mSaveEnabledCheck);
    DDX_Control(pDX, IDC_LOCATIONGROUP, mLocationGroup);
    DDX_Control(pDX, IDC_LOCATION_BUTTON, mLocationButton);
    DDX_Control(pDX, IDC_THROTTLINGGROUP, mThrottlingGroup);
    DDX_Control(pDX, IDC_ONEOUTOF_RADIO, mOneOutOfRadio);
    DDX_Control(pDX, IDC_MAXRATE_RADIO, mMaxRateRadio);
    DDX_Control(pDX, IDC_AVERAGETHROUGHPUT_RADIO, mAverageThroughputRadio);
    DDX_Control(pDX, IDC_NOTHROTTLE_RADIO, mNoThrottleRadio);
    DDX_Control(pDX, IDC_CAPTUREDIMAGESLABEL, mCapturedImagesLabel);
    DDX_Control(pDX, IDC_MSLABEL, mMsLabel);
    DDX_Control(pDX, IDC_AVERAGELABEL, mAverageLabel);
    DDX_Control(pDX, IDC_FORMAT, mFormatCombo);
    DDX_Control(pDX, IDC_AVGBITRATE, mAvgBitrateEdit);
}


///
/// \brief OK button click event handler
///

void ImageSaveDlg::OnBnClickedOk()
{   
    if ( FromDialog() )
	{
        mImageSaving->StopMp4();
		CDialog::OnOK();
	}    
}


///
/// \brief Cancel button click event handler
///

void ImageSaveDlg::OnBnClickedCancel()
{
    CDialog::OnCancel();
}


///
/// \brief Init dialog event handler
///

BOOL ImageSaveDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    mFormatCombo.AddString( _T( "Device independent bitmap (BMP)" ) );
    mFormatCombo.AddString( _T( "Tagged Image File Format (TIFF)" ) );
    mFormatCombo.AddString( _T( "Raw binary data" ) );

    if ( mImageSaving->IsMp4Supported() )
    {
        mFormatCombo.AddString( _T( "H.264 video in MP4 container" ) );
    }

    ToDialog();

    return TRUE;
}


///
/// \brief One-out-of radio button click event handler
///

void ImageSaveDlg::OnBnClickedOneoutofRadio()
{
    EnableInterface();
}


///
/// \brief Max rate radio button event handler
///

void ImageSaveDlg::OnBnClickedMaxrateRadio()
{
    EnableInterface();
}


///
/// \brief Average throughput radio button event handler
///

void ImageSaveDlg::OnBnClickedAveragethroughputRadio()
{
    EnableInterface();
}


///
/// \brief No throttling radio button event handler
///

void ImageSaveDlg::OnBnClickedNothrottleRadio()
{
    EnableInterface();
}


///
/// \brief Location button event handler. Shows a directory selection dialog.
///

void ImageSaveDlg::OnBnClickedLocationButton()
{
    BROWSEINFO bi = { 0 };
    bi.lpszTitle = _T( "Save images to folder" );
    bi.hwndOwner = GetSafeHwnd();
    bi.ulFlags = BIF_USENEWUI;
    wchar_t lPath[512];

    LPITEMIDLIST pidl = SHBrowseForFolder ( &bi );
    if ( pidl != 0 )
    {
        // get the name of the folder
        if ( SHGetPathFromIDList ( pidl, lPath ) )
        {
            mSavePathEdit.SetWindowTextW( lPath );
        }

        // free memory used
        IMalloc *imalloc = 0;
        if ( SUCCEEDED( SHGetMalloc ( &imalloc )) )
        {
            imalloc->Free ( pidl );
            imalloc->Release ( );
        }
    }
}


///
/// \brief Event handler
///

void ImageSaveDlg::OnBnClickedImagesaveenable()
{
    EnableInterface();
} 


///
/// \brief Sets the enabled state of the dialog
///

void ImageSaveDlg::EnableInterface()
{
    BOOL lEnabled = mSaveEnabledCheck.GetCheck() == BST_CHECKED;

    mSavePathEdit.EnableWindow( lEnabled );
    mLocationGroup.EnableWindow( lEnabled );
    mLocationButton.EnableWindow( lEnabled );
    mThrottlingGroup.EnableWindow( lEnabled );
    mOneOutOfRadio.EnableWindow( lEnabled );
    mMaxRateRadio.EnableWindow( lEnabled );
    mAverageThroughputRadio.EnableWindow( lEnabled );
    mNoThrottleRadio.EnableWindow( lEnabled );
    mCapturedImagesLabel.EnableWindow( lEnabled );
    mMsLabel.EnableWindow( lEnabled );
    mAverageLabel.EnableWindow( lEnabled );
    mFormatCombo.EnableWindow( lEnabled );

    int lChecked = GetCheckedRadioButton( IDC_ONEOUTOF_RADIO, IDC_NOTHROTTLE_RADIO );

    mOneOutOfEdit.EnableWindow( lEnabled && ( lChecked == IDC_ONEOUTOF_RADIO ) );
    mMaxRateEdit.EnableWindow( lEnabled && ( lChecked == IDC_MAXRATE_RADIO ) );
    mAverageThroughputEdit.EnableWindow( lEnabled && ( lChecked == IDC_AVERAGETHROUGHPUT_RADIO ) );

    mAvgBitrateEdit.EnableWindow( lEnabled && ( mFormatCombo.GetCurSel() == 3 ) );
}


///
/// \brief Loads data on the dialog
///

void ImageSaveDlg::ToDialog()
{
    switch ( mImageSaving->GetFormat() )
    {
    default:
        ASSERT( 0 );

    case ImageSaving::FormatBmp:
        mFormatCombo.SetCurSel( 0 );
        break;

    case ImageSaving::FormatTiff:
        mFormatCombo.SetCurSel( 1 );
        break;

    case ImageSaving::FormatRaw:
        mFormatCombo.SetCurSel( 2 );
        break;

    case ImageSaving::FormatMp4:
        mFormatCombo.SetCurSel( 3 );
        break;
    }

    wchar_t lOneOutOf[ 256 ];
    _itow_s( mImageSaving->GetOneOutOf(), lOneOutOf, 256, 10 );
    mOneOutOfEdit.SetWindowTextW( lOneOutOf );
    wchar_t lMaxRate[ 256 ];
    _itow_s( mImageSaving->GetMaxRate(), lMaxRate, 256, 10 );
    mMaxRateEdit.SetWindowTextW( lMaxRate );
    wchar_t lAverageThroughput[ 256 ];
    _itow_s( mImageSaving->GetAverageThroughput(), lAverageThroughput, 256, 10 );
    mAverageThroughputEdit.SetWindowTextW( lAverageThroughput );
    switch ( mImageSaving->GetThrottling() )
    {
    default:
        ASSERT( 0 );

    case ImageSaving::ThrottleOneOutOf:
        CheckRadioButton( IDC_ONEOUTOF_RADIO, IDC_NOTHROTTLE_RADIO, IDC_ONEOUTOF_RADIO );
        mOneOutOfEdit.EnableWindow( TRUE );
        break;

    case ImageSaving::ThrottleMaxRate:
        CheckRadioButton( IDC_ONEOUTOF_RADIO, IDC_NOTHROTTLE_RADIO, IDC_MAXRATE_RADIO );
        mMaxRateEdit.EnableWindow( TRUE );
        break;

    case ImageSaving::ThrottleAverageThroughput:
        CheckRadioButton( IDC_ONEOUTOF_RADIO, IDC_NOTHROTTLE_RADIO, IDC_AVERAGETHROUGHPUT_RADIO );
        mAverageThroughputEdit.EnableWindow( TRUE );
        break;

    case ImageSaving::ThrottleNone:
        CheckRadioButton( IDC_ONEOUTOF_RADIO, IDC_NOTHROTTLE_RADIO, IDC_NOTHROTTLE_RADIO );
        break;
    }

    mSavePathEdit.SetWindowTextW( mImageSaving->GetPath().GetUnicode() );
    mSaveEnabledCheck.SetCheck( mImageSaving->GetEnabled() );

    wchar_t lAvgBitrate[ 256 ];
    _itow_s( mImageSaving->GetAvgBitrate(), lAvgBitrate, 256, 10 );
    mAvgBitrateEdit.SetWindowTextW( lAvgBitrate );

    EnableInterface();
}


///
/// \brief Saves data from the dialog
///

bool ImageSaveDlg::FromDialog()
{
	bool lSuccess = true;

    CString lStrOneOutOf;
    mOneOutOfEdit.GetWindowTextW( lStrOneOutOf );
    mImageSaving->SetOneOutOf( _wtoi( lStrOneOutOf.GetBuffer() ) );
    
    CString lStrAverageThroughput;
    mAverageThroughputEdit.GetWindowTextW( lStrAverageThroughput );
    mImageSaving->SetAverageThroughput( _wtoi( lStrAverageThroughput.GetBuffer() ) );

    CString lStrMaxRate;
    mMaxRateEdit.GetWindowTextW( lStrMaxRate );
    mImageSaving->SetMaxRate( _wtoi( lStrMaxRate.GetBuffer() ) );

    switch( GetCheckedRadioButton( IDC_ONEOUTOF_RADIO, IDC_NOTHROTTLE_RADIO ) )
    {
    case IDC_ONEOUTOF_RADIO:
        mImageSaving->SetThrottling( ImageSaving::ThrottleOneOutOf );
        break;

    case IDC_MAXRATE_RADIO:
        mImageSaving->SetThrottling( ImageSaving::ThrottleMaxRate );
        break;

    case IDC_AVERAGETHROUGHPUT_RADIO:
        mImageSaving->SetThrottling( ImageSaving::ThrottleAverageThroughput );
        break;

    case IDC_NOTHROTTLE_RADIO:
        mImageSaving->SetThrottling( ImageSaving::ThrottleNone );
        break;

    default:
        ASSERT(FALSE);
        break;
    }

    CString lPath;
    mSavePathEdit.GetWindowTextW( lPath );
    mImageSaving->SetPath( (LPCTSTR)lPath );

    if ( mSaveEnabledCheck.GetCheck() == BST_CHECKED )
    {
        lSuccess = ValidateImageFolder();
        mImageSaving->SetEnabled( lSuccess );           
      
    }
    else
    {
        mImageSaving->SetEnabled( false );
    }

    switch ( mFormatCombo.GetCurSel() )
    {
    default:
        ASSERT( 0 );

    case 0:
        mImageSaving->SetFormat( ImageSaving::FormatBmp );
        break;

    case 1:
        mImageSaving->SetFormat( ImageSaving::FormatTiff );
        break;

    case 2:
        mImageSaving->SetFormat( ImageSaving::FormatRaw );
        break;

    case 3:
        mImageSaving->SetFormat( ImageSaving::FormatMp4 );
        break;
    }

    CString lAvgBitrate;
    mAvgBitrateEdit.GetWindowTextW( lAvgBitrate );
    mImageSaving->SetAvgBitrate( _wtoi( lAvgBitrate.GetBuffer() ) );

    return lSuccess;
}


///
/// \brief Validates whether the image folder exists or not
///

bool ImageSaveDlg::ValidateImageFolder()
{
    CString lSavePath;
	mSavePathEdit.GetWindowText( lSavePath );
    if ( FolderPathIsEmpty( lSavePath ) )
    {
        return false;
    }
    
    return FolderExists( lSavePath );   
}

bool ImageSaveDlg::FolderPathIsEmpty( const CString &lSavePath )
{
    if ( lSavePath.IsEmpty() )
    {
        AfxMessageBox( L"Location to save images is empty.  Please insert a valid path.", MB_ICONERROR );
        return true;
    }

    return false;
}


///
/// \brief Returns true if a fodler exists
///

bool ImageSaveDlg::FolderExists( const CString lSavePath )
{
    if ( !PathFileExists( lSavePath ) )
    {
        CString lErrorMsg;
        lErrorMsg.Format( L"The path \"%s\" does not exist.  Please insert a valid path.", (LPCTSTR)lSavePath );
        AfxMessageBox( lErrorMsg, MB_ICONERROR);
        return false;

    }

    return true;
}


///
/// \brief Enables the UI when the format combo box changes
///

void ImageSaveDlg::OnCbnSelchangeFormat()
{
    EnableInterface();
}

