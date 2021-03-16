// *****************************************************************************
//
//     Copyright (c) 2016, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"
#include "afxdialogex.h"

#include "PersistenceInfoDlg.h"


#define DEVICE_TITLE ( L"Session Configuration Changed" )
#define DEVICE_MESSAGE \
    ( L"You have changed the configuration of your device, " \
      L"communication parameters or stream parameters." )

#define PREFERENCES_TITLE ( L"Application Preferences Changed" )
#define PREFERENCES_MESSAGE \
    ( L"You have changed application preferences for either application setup, image filtering, " \
      L"image saving, event monitor, GenApi browsers, buffer or display options." )


IMPLEMENT_DYNAMIC( PersistenceInfoDlg, CDialogEx )

BEGIN_MESSAGE_MAP(PersistenceInfoDlg, CDialogEx)
END_MESSAGE_MAP()


///
/// \brief Constructor
///

PersistenceInfoDlg::PersistenceInfoDlg( CWnd* aParent )
    : CDialogEx( PersistenceInfoDlg::IDD, aParent )
    , mDontShowAgain( false )
    , mMessage( DEVICE_MESSAGE )
    , mTitle( DEVICE_TITLE )
{
}


///
/// \brief Destructor.
///

PersistenceInfoDlg::~PersistenceInfoDlg()
{
}


///
/// \brief Syncs UI vs data.
///

void PersistenceInfoDlg::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange( pDX );
    DDX_Control( pDX, IDC_DONTSHOWAGAIN, mDontShowAgainCheckBox );
    DDX_Control( pDX, IDC_MESSAGE, mMessageStatic );
}


///
/// \brief Responds to the WM_INITDIALOG message.
///

BOOL PersistenceInfoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

    SetWindowText( mTitle );
    mMessageStatic.SetWindowText( mMessage );

	return TRUE;
}


///
/// \brief Dialog OK handler
///

void PersistenceInfoDlg::OnOK()
{
    mDontShowAgain = mDontShowAgainCheckBox.GetCheck() == BST_CHECKED;
	CDialogEx::OnOK();
}


///
/// \brief Dialog Cancel handler
///

void PersistenceInfoDlg::OnCancel()
{
}


///
/// \brief Sets the dialog to display the warning in the device's config context
///

void PersistenceInfoDlg::SetDeviceContext()
{
    mMessage = DEVICE_MESSAGE;
    mTitle = DEVICE_TITLE;
}


///
/// \brief Sets the dialog to display the warning in the preference's context
///

void PersistenceInfoDlg::SetPreferencesContext()
{
    mMessage = PREFERENCES_MESSAGE;
    mTitle = PREFERENCES_TITLE;
}

