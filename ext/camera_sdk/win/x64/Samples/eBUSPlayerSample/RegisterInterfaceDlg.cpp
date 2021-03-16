// *****************************************************************************
//
//     Copyright (c) 2009, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"
#include "RegisterInterfaceDlg.h"


#define DEFAULTSTR ( _T( "00000000" ) )


BEGIN_MESSAGE_MAP(RegisterInterfaceDlg, CDialog)
    ON_BN_CLICKED(IDC_READBUTTON, &RegisterInterfaceDlg::OnReadButtonClicked)
    ON_BN_CLICKED(IDC_WRITEBUTTON, &RegisterInterfaceDlg::OnWriteButtonClicked)
END_MESSAGE_MAP()


// =============================================================================
RegisterInterfaceDlg::RegisterInterfaceDlg( CWnd* pParent /*=NULL*/ )
    : CDialog( IDD_REGISTERINTERFACE, pParent )
    , mDevice( NULL )
{
}


// =============================================================================
RegisterInterfaceDlg::~RegisterInterfaceDlg()
{
}


// =============================================================================
void RegisterInterfaceDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_READADDRESS, mReadAddressEdit);
    DDX_Control(pDX, IDC_READVALUE, mReadValueEdit);
    DDX_Control(pDX, IDC_READRESULT, mReadResultEdit);
    DDX_Control(pDX, IDC_WRITEADDRESS, mWriteAddressEdit);
    DDX_Control(pDX, IDC_WRITEVALUE, mWriteValueEdit);
    DDX_Control(pDX, IDC_WRITERESULT, mWriteResultEdit);
}


// =============================================================================
BOOL RegisterInterfaceDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    mReadAddressEdit.SetWindowText( DEFAULTSTR );
    mReadValueEdit.SetWindowText( DEFAULTSTR );
    mReadValueEdit.SetWindowText( _T( "" ) );

    mWriteAddressEdit.SetWindowText( DEFAULTSTR );
    mWriteValueEdit.SetWindowText( DEFAULTSTR );
    mWriteValueEdit.SetWindowText( _T( "" ) );

    EnableInterface();

    return TRUE;
}


// =============================================================================
void RegisterInterfaceDlg::EnableInterface()
{
}


// =============================================================================
void RegisterInterfaceDlg::OnOK()
{
    CDialog::OnOK();
}


// =============================================================================
void RegisterInterfaceDlg::OnCancel()
{
    CDialog::OnCancel();
}


// =============================================================================
void RegisterInterfaceDlg::OnReadButtonClicked()
{
    uint32_t lAddress;
    if ( ReadValue( mReadAddressEdit, lAddress ) )
    {
        uint32_t lValue = 0;
        PvResult lResult = mDevice->ReadRegister( lAddress, lValue );
        if ( lResult.IsOK() )
        {
            WriteValue( lValue, mReadValueEdit );
        }
        else
        {
            mReadValueEdit.SetWindowText( _T( "" ) );
        }

        PvString lString = lResult.GetCodeString();
        mReadResultEdit.SetWindowText( lString.GetUnicode() );
    }
    else
    {
        MessageBox( _T( "Could not parse read address." ), _T( "Error" ), MB_OK | MB_ICONERROR );
    }
}


// =============================================================================
void RegisterInterfaceDlg::OnWriteButtonClicked()
{
    uint32_t lAddress;
    if ( ReadValue( mWriteAddressEdit, lAddress ) )
    {
        uint32_t lValue;     
        if ( ReadValue( mWriteValueEdit, lValue ) )
        {
            PvResult lResult = mDevice->WriteRegister( lAddress, lValue );

            PvString lString = lResult.GetCodeString();
            mWriteResultEdit.SetWindowText( lString.GetUnicode() );
        }
        else
        {
            MessageBox( _T( "Could not parse write value." ), _T( "Error" ), MB_OK | MB_ICONERROR );
        }
    }
    else
    {
        MessageBox( _T( "Could not parse write address." ), _T( "Error" ), MB_OK | MB_ICONERROR );
    }
}


// =============================================================================
bool RegisterInterfaceDlg::ReadValue( CEdit &aEdit, uint32_t &aValue )
{
    CString lStr;
    aEdit.GetWindowText( lStr );

    // Remove extra spaces, make uppercase
    lStr = lStr.Trim();
    lStr = lStr.MakeUpper();

    // Remove leading 0x
    if ( lStr.GetLength() > 2 )
    {
        if ( lStr.Left( 2 ) == _T( "0X" ) )
        {
            lStr = lStr.Right( lStr.GetLength() - 2 );
        }
    }

    // Read as hex
    int lCount = swscanf_s( (LPCTSTR)lStr, _T( "%X" ), &aValue );

    return ( lCount == 1 );
}


// =============================================================================
void RegisterInterfaceDlg::WriteValue( uint32_t aValue, CEdit &aEdit )
{
    CString lStr;
    lStr.Format( _T( "%08X" ), aValue );

    aEdit.SetWindowText( lStr );
}


