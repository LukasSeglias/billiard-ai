// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"
#include "BufferOptionsDlg.h"
#include "Player.h"

#include "PvMessageBox.h"


IMPLEMENT_DYNAMIC(BufferOptionsDlg, CDialog)

BEGIN_MESSAGE_MAP(BufferOptionsDlg, CDialog)
    ON_BN_CLICKED(IDOK, &BufferOptionsDlg::OnBnClickedOk)
END_MESSAGE_MAP()


///
/// \brief Constructor.
///

BufferOptionsDlg::BufferOptionsDlg( Player *aPlayer, CWnd* pParent )
    : CDialog( BufferOptionsDlg::IDD, pParent)
    , mPlayer( aPlayer )
{

}


///
/// \brief Destructor.
///

BufferOptionsDlg::~BufferOptionsDlg()
{
}


///
/// \brief Dialog objects binding
///

void BufferOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
    DDX_Control(pDX, IDC_BUFFERCOUNT, mBufferCountEdit);
    DDX_Control(pDX, IDC_BUFFERSIZE, mBufferSizeEdit);
    DDX_Control(pDX, IDC_AUTORESIZECHECK, mAutoResizeCheck);
    CDialog::DoDataExchange(pDX);
}


///
/// \brief OnOK handler
///

void BufferOptionsDlg::OnBnClickedOk()
{
    BufferOptions *lBufferOptions = mPlayer->GetBufferOptions();

    uint32_t lCount = 0;
    if ( !EditToUInt32( mBufferCountEdit, lCount ) )
    {
        // Parsing error
        MessageBox( _T( "Invalid buffer count value." ), _T( "Error" ), MB_ICONERROR | MB_OK );
        return;
    }
    if ( lCount <= 0 )
    {
        MessageBox( _T( "Buffer count cannot be zero." ), _T( "Error" ), MB_ICONERROR | MB_OK );
        return;
    }
    lBufferOptions->SetBufferCount( lCount );

    uint32_t lDefaultBufferSize = 0;
    if ( !EditToUInt32( mBufferSizeEdit, lDefaultBufferSize ) )
    {
        // Parsing error
        MessageBox( _T( "Invalid default buffer size value." ), _T( "Error" ), MB_ICONERROR | MB_OK );
        return;
    }
    if ( lDefaultBufferSize <= 0 )
    {
        MessageBox( _T( "Default buffer size cannot be zero." ), _T( "Error" ), MB_ICONERROR | MB_OK );
        return;
    }
    lBufferOptions->SetBufferSize( lDefaultBufferSize );

    bool lAutoResize = true;
    if ( mAutoResizeCheck.GetCheck() != BST_CHECKED )
    {
        lAutoResize = false;
    }
    lBufferOptions->SetAutoResize( lAutoResize );

    if ( !mPlayer->DoBufferOptionsRequireApply() )
    {
        // Nothing changed, nothing to do...
        OnOK();
        return;
    }

    // If *A LOT* of memory is about to be allocated, ask the user first
    if ( lBufferOptions->ShowWarning() )
    {
        CString lMessage;
        lMessage.Format( _T( "%.1f GB of memory will be required by the acquisition pipeline. Are you sure you want to continue?" ),
            lBufferOptions->GetRequiredMemoryInGB() );

        int lReturn = MessageBox( lMessage, _T( "Warning" ), MB_ICONWARNING | MB_YESNO );
        if ( lReturn != IDYES )
        {
            return;
        }
    }

    if ( mPlayer->IsStreamOpened() )
    {
        PvResult lResult = mPlayer->ApplyBufferOptions();
        if ( !lResult.IsOK() )
        {
            PvMessageBox( this, lResult );
        }
    }

    OnOK();
}


///
/// \brief Converts a string to integer
///

bool BufferOptionsDlg::StrToUInt32( const CString &aString, uint32_t &aValue )
{
    aValue = 0;
    int lScanCount = swscanf_s( (LPCTSTR)aString, _T( "%d" ), &aValue );
    return ( lScanCount == 1 );
}


///
/// \brief Reads an integer from a CEdit
///

bool BufferOptionsDlg::EditToUInt32( CEdit &aEdit, uint32_t &aValue )
{
    CString lString;
    aEdit.GetWindowText( lString );

    return StrToUInt32( lString, aValue );
}


///
/// \brief Dialog initialization
///

BOOL BufferOptionsDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    BufferOptions *lBufferOptions = mPlayer->GetBufferOptions();

    GetDlgItem( IDC_HELP1 )->SetWindowText( lBufferOptions->GetHelpBufferCount().GetUnicode() );
    GetDlgItem( IDC_HELP2 )->SetWindowText( lBufferOptions->GetHelpBufferSize().GetUnicode() );
    GetDlgItem( IDC_HELP3 )->SetWindowText( lBufferOptions->GetHelpAutoResizing().GetUnicode() );

    CString lBufferCountStr;
    lBufferCountStr.Format( _T( "%i" ), lBufferOptions->GetBufferCount() );
    mBufferCountEdit.SetWindowText( lBufferCountStr );

    CString lBufferSizeStr;
    lBufferSizeStr.Format( _T( "%i" ), lBufferOptions->GetBufferSize() );
    mBufferSizeEdit.SetWindowText( lBufferSizeStr );

    mAutoResizeCheck.SetCheck( lBufferOptions->GetAutoResize() ? BST_CHECKED : BST_UNCHECKED );

    return TRUE;
}




