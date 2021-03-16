// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"
#include "Resource.h"
#include "SetupDlg.h"

#include <PvDeviceInfo.h>


BEGIN_MESSAGE_MAP(SetupDlg, CDialog)
    ON_BN_CLICKED(IDC_UNICASTAUTO, &OnBnClicked)
    ON_BN_CLICKED(IDC_UNICASTSPECIFIC, &OnBnClicked)
    ON_BN_CLICKED(IDC_MULTICAST, &OnBnClicked)
END_MESSAGE_MAP()


SetupDlg::SetupDlg( Setup* aSetup, PvDeviceInfoType aSetupType, CWnd* pParent /*=NULL*/)
    : CDialog(IDD_SETUP, pParent)
    , mSetup( aSetup )
    , mSetupType( aSetupType )
{
}

SetupDlg::~SetupDlg()
{
}

void SetupDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CTRLDATA, mCtrlDataRadio);
    DDX_Control(pDX, IDC_DATA, mDataRadio);
    DDX_Control(pDX, IDC_DESTINATION_GROUP, mDestinationGroup);
    DDX_Control(pDX, IDC_UNICASTAUTO, mUnicastAutoRadio);
    DDX_Control(pDX, IDC_UNICASTSPECIFIC, mUnicastSpecificRadio);
    DDX_Control(pDX, IDC_SPECIFICPORT, mUnicastSpecificPortEdit);
    DDX_Control(pDX, IDC_MULTICAST, mMulticastRadio);
    DDX_Control(pDX, IDC_MULTICASTIP, mMulticastIPCtrl);
    DDX_Control(pDX, IDC_MULTICASTPORT, mMulticastPortEdit);
    DDX_Control(pDX, IDC_SPECIFICPORTLABEL, mSpecificPortLabel);
    DDX_Control(pDX, IDC_MULTICASTIPLABEL, mMulticastIPLabel);
    DDX_Control(pDX, IDC_MULTICASTPORTLABEL, mMulticastPortLabel);
}

BOOL SetupDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    int lRoleID;
    int lDestinationID;
    CString lTemp;

    // Role
    lRoleID = IDC_CTRLDATA;
    switch ( mSetup->GetRole() )
    {
    case ROLE_CTRLDATA:
        lRoleID = IDC_CTRLDATA;
        break;
    case ROLE_DATA:
        lRoleID = IDC_DATA;
        break;
    }

    CheckRadioButton( IDC_CTRLDATA, IDC_DATA, lRoleID );
    
    // Destination
    if ( mSetupType == PvDeviceInfoTypeGEV )
    {
        lDestinationID = IDC_UNICASTAUTO;
        switch ( mSetup->GetDestination() )
        {
            case DESTINATION_UNICAST_AUTO:
                lDestinationID = IDC_UNICASTAUTO;

                mUnicastSpecificPortEdit.SetWindowText( _T( "" ) );
                mMulticastIPCtrl.SetAddress( 0, 0, 0, 0 );
                mMulticastPortEdit.SetWindowText( _T( "" ) );
                break;

            case DESTINATION_UNICAST_SPECIFIC:
                lDestinationID = IDC_UNICASTSPECIFIC;

                lTemp.Format( _T( "%d" ), mSetup->GetPort() );
                mUnicastSpecificPortEdit.SetWindowText( lTemp );
                mMulticastIPCtrl.SetAddress( 0, 0, 0, 0 );
                mMulticastPortEdit.SetWindowText( _T( "" ) );
                break;

            case DESTINATION_MULTICAST:
                lDestinationID = IDC_MULTICAST;

                mUnicastSpecificPortEdit.SetWindowText( _T( "" ) );
                mMulticastIPCtrl.SetAsText( mSetup->GetIPAddress() );
                lTemp.Format( _T( "%d" ), mSetup->GetPort() );
                mMulticastPortEdit.SetWindowText( lTemp );
                break;
        }
        
        CheckRadioButton( IDC_UNICASTAUTO, IDC_MULTICAST, lDestinationID );
    }

    EnableInterface();

    return TRUE;
}

void SetupDlg::EnableInterface()
{
    bool lIsGEV = ( mSetupType == PvDeviceInfoTypeGEV );
    if ( lIsGEV )
    {
        int lDestinationID = GetCheckedRadioButton( IDC_UNICASTAUTO, IDC_MULTICAST );
        switch ( lDestinationID )
        {
            case IDC_UNICASTAUTO:
                mUnicastSpecificPortEdit.EnableWindow( false );
                mMulticastIPCtrl.EnableWindow( false );
                mMulticastPortEdit.EnableWindow( false );
                break;

            case IDC_UNICASTSPECIFIC:
                mUnicastSpecificPortEdit.EnableWindow( true );
                mMulticastIPCtrl.EnableWindow( false );
                mMulticastPortEdit.EnableWindow( false );
                break;

            case IDC_MULTICAST:
                mUnicastSpecificPortEdit.EnableWindow( false );
                mMulticastIPCtrl.EnableWindow( true );
                mMulticastPortEdit.EnableWindow( true );
                break;

            default:
                ASSERT( 0 );
        }
    }
    else
    {
        mUnicastAutoRadio.EnableWindow( false );
        mUnicastSpecificRadio.EnableWindow( false );
        mUnicastSpecificPortEdit.EnableWindow( false );
        mMulticastRadio.EnableWindow( false );
        mMulticastIPCtrl.EnableWindow( false );
        mMulticastPortEdit.EnableWindow( false );
        mDestinationGroup.EnableWindow( false );
        mSpecificPortLabel.EnableWindow( false );
        mMulticastIPLabel.EnableWindow( false );
        mMulticastPortLabel.EnableWindow( false );
    }
}

void SetupDlg::OnBnClicked()
{
    EnableInterface();
}

void SetupDlg::OnOK()
{
    int lPort = 0;
    CString lStr;
    int lRoleID;
    int lDestinationID;

    // Invalidate the results
    mSetup->Reset();

    lRoleID = GetCheckedRadioButton( IDC_CTRLDATA, IDC_DATA );
    switch ( lRoleID )
    {
    case IDC_CTRLDATA:
        mSetup->SetRole( ROLE_CTRLDATA );
        break;
    case IDC_DATA:
        mSetup->SetRole( ROLE_DATA );
        break;
    default:
        ASSERT( 0 );
    }

    lDestinationID = GetCheckedRadioButton( IDC_UNICASTAUTO, IDC_MULTICAST );
    switch ( lDestinationID )
    {
        case IDC_UNICASTAUTO:
            mSetup->SetDestination( DESTINATION_UNICAST_AUTO );
            break;
        case IDC_UNICASTSPECIFIC:
            mSetup->SetDestination( DESTINATION_UNICAST_SPECIFIC );

            mUnicastSpecificPortEdit.GetWindowText( lStr );
            swscanf_s( ( LPCTSTR ) lStr, _T( "%i" ), &lPort );
            mSetup->SetPort( static_cast<unsigned short>( lPort ) );
            break;
        case IDC_MULTICAST:
            mSetup->SetDestination( DESTINATION_MULTICAST );

            mMulticastIPCtrl.GetAsText( lStr );
            mSetup->SetIPAddress( lStr ); 
            
            mMulticastPortEdit.GetWindowText( lStr );
            swscanf_s( ( LPCTSTR ) lStr, _T( "%i" ), &lPort );
            mSetup->SetPort( static_cast<unsigned short>( lPort ) );
            break;

        default:
            ASSERT( mSetupType != PvDeviceInfoTypeGEV );
    }

    CDialog::OnOK();
}

void SetupDlg::OnCancel()
{
    CDialog::OnCancel();
}
