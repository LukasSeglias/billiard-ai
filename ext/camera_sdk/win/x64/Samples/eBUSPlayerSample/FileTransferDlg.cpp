// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"
#include "FileTransferDlg.h"
#include "PvMessageBox.h"

#include <PvGenFile.h>


IMPLEMENT_DYNAMIC(FileTransferDlg, CDialog)

BEGIN_MESSAGE_MAP(FileTransferDlg, CDialog)
    ON_CBN_SELCHANGE(IDC_FILECOMBO, &FileTransferDlg::OnCbnSelchangeFile)
    ON_BN_CLICKED(IDC_UPLOADBUTTON, &FileTransferDlg::OnBnClickedUpload)
    ON_BN_CLICKED(IDC_DOWNLOADBUTTON, &FileTransferDlg::OnBnClickedDownload)
    ON_WM_TIMER()
END_MESSAGE_MAP()


///
/// \brief Constructor
///

FileTransferDlg::FileTransferDlg( PvDevice *aDevice, CWnd* pParent /*=NULL*/ )
    : CDialog(FileTransferDlg::IDD, pParent)
    , mDevice( aDevice )
    , mParameters( aDevice->GetParameters() )
    , mFile( NULL )
    , mThreadHandle( 0 )
    , mTimer( 0 )
    , mWrite( true )
{
    mDevice->RegisterEventSink( this );
}


///
/// \brief Destructor
///

FileTransferDlg::~FileTransferDlg()
{
    if ( mDevice != NULL )
    {
        mDevice->UnregisterEventSink( this );
    }

    if ( mFile != NULL)
    {
        delete mFile;
        mFile = NULL;
    }
}


///
/// \brief Data exchange
///

void FileTransferDlg::DoDataExchange(CDataExchange* pDX)
{
    DDX_Control(pDX, IDC_GROUP1, mGroup1);
    DDX_Control(pDX, IDC_GROUP2, mGroup2);
    DDX_Control(pDX, IDC_FILECOMBO, mFileCombo);
    DDX_Control(pDX, IDC_DOWNLOADBUTTON, mDownloadButton);
    DDX_Control(pDX, IDC_UPLOADBUTTON, mUploadButton);
    DDX_Control(pDX, IDC_PROGRESS, mProgress);
    DDX_Control(pDX, IDC_PROGRESSLABEL, mProgressLabel);
    DDX_Control(pDX, IDCANCEL, mCancelButton);
    CDialog::DoDataExchange(pDX);
}


///
/// \brief Dialog initialization
///

BOOL FileTransferDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    LoadFileCombo();
    EnableInterface();

    // We always bring the progress back on a 0-100 scale
    mProgress.SetRange( 0, 100 );

    return TRUE;
}


///
/// \brief Sets the enabled state of the UI
///

void FileTransferDlg::EnableInterface()
{
    bool lTransfering = ( mFile != NULL );
    bool lFileEnabled = ( !lTransfering );

    mGroup1.EnableWindow( lFileEnabled );
    mFileCombo.EnableWindow( lFileEnabled );

    bool lUploadEnabled = false;
    bool lDownloadEnabled = false;

    if ( !lTransfering )
    {
        if ( mFileCombo.GetCurSel() >= 0 )
        {
            CString lFile;
            mFileCombo.GetWindowText( lFile );

            lUploadEnabled = PvGenFile::IsWritable( mParameters, (LPCTSTR)lFile );
            lDownloadEnabled = PvGenFile::IsReadable( mParameters, (LPCTSTR)lFile );
        }
    }

    mGroup2.EnableWindow( lTransfering || lUploadEnabled || lDownloadEnabled );
    mDownloadButton.EnableWindow( lDownloadEnabled );
    mUploadButton.EnableWindow( lUploadEnabled );

    mProgress.EnableWindow( lTransfering );
    mProgressLabel.EnableWindow( lTransfering );
    mCancelButton.EnableWindow( !lTransfering );
}


///
/// \brief Loads all device file names in the file combo box
///

void FileTransferDlg::LoadFileCombo()
{
    PvStringList lFiles;
    PvGenFile::GetFiles( mParameters, lFiles );

    mFileCombo.ResetContent();
    PvString *lString = lFiles.GetFirst();
    while ( lString != NULL )
    {
        mFileCombo.AddString( *lString );
        lString = lFiles.GetNext();
    }
}


///
/// \brief Selected file changed combo handler
///

void FileTransferDlg::OnCbnSelchangeFile()
{
    EnableInterface();
}


///
/// \brief Upload button clicked handler
///

void FileTransferDlg::OnBnClickedUpload()
{
    if ( mFile != NULL )
    {
        return;
    }

    // Get device file
    mFileCombo.GetWindowText( mDeviceFile );

    // Get host file
    CFileDialog lDlg( TRUE, NULL, mDeviceFile, 
        OFN_HIDEREADONLY, 
        _T( "All files (*.*)||" ), this );
    if ( lDlg.DoModal() != IDOK )
    {
        return;
    }
    mHostFile = lDlg.GetPathName();

    // Read file
    mFile = new PvGenFile;
    PvResult lResult = mFile->Open( mParameters, (LPCTSTR)mDeviceFile, PvGenOpenModeWrite );
    if ( !lResult.IsOK() )
    {
        delete mFile;
        mFile = NULL;

        PvMessageBox( this, lResult );
        return;
    }

    mWrite = true;
    Transfer();
}


///
/// \brief Download button clicked handler
///

void FileTransferDlg::OnBnClickedDownload()
{
    if ( mFile != NULL )
    {
        return;
    }

    // Get device file
    mFileCombo.GetWindowText( mDeviceFile );

    // Get host file
    CFileDialog lDlg( FALSE, NULL, mDeviceFile, 
        OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
        _T( "All files (*.*)||" ), this );
    if ( lDlg.DoModal() != IDOK )
    {
        return;
    }
    mHostFile = lDlg.GetPathName();

    // Create and open GenICam file object
    mFile = new PvGenFile;
    PvResult lResult = mFile->Open( mParameters, (LPCTSTR)mDeviceFile, PvGenOpenModeRead );
    if ( !lResult.IsOK() )
    {
        delete mFile;
        mFile = NULL;

        PvMessageBox( this, lResult );
        return;
    }

    mWrite = false;
    Transfer();
}


///
/// \brief Starts the thread that will perform the file transfer and and timer that will keep the UI updated on progress
///

void FileTransferDlg::Transfer()
{
    // Create thread
    DWORD lID = 0;
    mThreadHandle = ::CreateThread(
        NULL,               // Security attributes
        0,                  // Stack size, 0 is default
        Link,               // Start address
        this,               // Parameter
        0,                  // Creation flags
        &lID );             // Thread ID

    // Start timer - that will poll for the transfer status
    mTimer = SetTimer( 1, 50, NULL );

    EnableInterface();
}


///
/// \brief Thread performing the update - while the main UI thread updates the progress bar
///

unsigned long FileTransferDlg::Link( void *aParam )
{
    FileTransferDlg *lThis = reinterpret_cast<FileTransferDlg *>( aParam );
    if ( lThis->mWrite )
    {
        // Write the file to the device
        lThis->mResult = lThis->mFile->WriteFrom( (LPCTSTR)lThis->mHostFile );
    }
    else
    {
        // Read the file from the device
        lThis->mResult = lThis->mFile->ReadTo( (LPCTSTR)lThis->mHostFile );
    }

    return lThis->mResult.IsOK() ? 0 : 1;
}


///
/// \brief Timer event handler
///

void FileTransferDlg::OnTimer( UINT_PTR nIDEvent )
{
    if ( nIDEvent == 1 )
    {
        int64_t lCompleted = 0, lTotal = 0;
        mFile->GetProgress( lCompleted, lTotal );

        double lDCompleted = static_cast<double>( lCompleted) / static_cast<double>( lTotal ) * 100.0f;
        short lProgress = min( 100, static_cast<short>( lDCompleted + 0.5 ) );

        mProgress.SetPos( lProgress );

        CString lStr;
        lStr.Format( _T( "%lld/%lld bytes transferred..."), lCompleted, lTotal );
        mProgressLabel.SetWindowText( lStr );

        // If the thread is done, complete
        if ( ::WaitForSingleObject( mThreadHandle, 0 ) == WAIT_OBJECT_0 )
        {
            if ( mTimer != 0 )
            {
                KillTimer( 1 );
                mTimer = 0;
            }

            ASSERT( mFile != NULL );
            if ( mFile != NULL )
            {
                mFile->Close();

                delete mFile;
                mFile = NULL;
            }

            if ( !mResult.IsOK() )
            {
                PvMessageBox( this, mResult );
            }

            mProgress.SetPos( 100 );
            MessageBox( _T( "Transfer successfully completed." ), _T( "eBUS Player" ), MB_OK | MB_ICONINFORMATION );

            EndDialog( IDOK );
        }
    }

    CDialog::OnTimer( nIDEvent );
}


///
/// \brief On destroy handler
///

void FileTransferDlg::OnDestroy()
{
    CDialog::OnDestroy();

    if ( mThreadHandle != 0 )
    {
        ::WaitForSingleObject( mThreadHandle, INFINITE );
        mThreadHandle = 0;
    }

    if ( mTimer != 0 )
    {
        KillTimer( 1 );
        mTimer = 0;
    }
}


///
/// \brief OnOK handler. Block if transferring.
///

void FileTransferDlg::OnOK()
{
    if ( mFile != NULL )
    {
        return;
    }

    CDialog::OnOK();
}


///
/// \brief OnCancel handler. Block if transferring.
///

void FileTransferDlg::OnCancel()
{
    if ( mFile != NULL )
    {
        return;
    }

    CDialog::OnCancel();
}


///
/// \brief Callback that the device has been disconnected
///

void FileTransferDlg::OnLinkDisconnected( PvDevice *aDevice )
{
    // Only explicitly close the dialog here if the transfer has not yet started
    if ( mFile == NULL )
    {
        mDevice = NULL;
        EndDialog( IDCANCEL );
    }
}

