// *****************************************************************************
//
//     Copyright (c) 2008, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"
#include "SetupDlg.h"


BEGIN_MESSAGE_MAP(SetupDlg, CDialog)
    ON_BN_CLICKED(IDC_CTRLDATA, &SetupDlg::OnBnClicked)
    ON_BN_CLICKED(IDC_CTRL, &SetupDlg::OnBnClicked)
    ON_BN_CLICKED(IDC_DATA, &SetupDlg::OnBnClicked)
    ON_BN_CLICKED(IDC_UNICASTAUTO, &SetupDlg::OnBnClicked)
    ON_BN_CLICKED(IDC_UNICASTSPECIFIC, &SetupDlg::OnBnClicked)
    ON_BN_CLICKED(IDC_UNICASTOTHER, &SetupDlg::OnBnClicked)
    ON_BN_CLICKED(IDC_MULTICAST, &SetupDlg::OnBnClicked)
END_MESSAGE_MAP()


// =============================================================================
SetupDlg::SetupDlg( Setup *aSetup, CWnd* pParent /*=NULL*/ )
    : CDialog( IDD_SETUP, pParent )
    , mSetup( aSetup )
{
}


// =============================================================================
SetupDlg::~SetupDlg()
{
}


// =============================================================================
void SetupDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CTRLDATA, mCtrlDataRadio);
    DDX_Control(pDX, IDC_CTRL, mCtrlRadio);
    DDX_Control(pDX, IDC_DATA, mDataRadio);
    DDX_Control(pDX, IDC_UNICASTAUTO, mUnicastAutoRadio);
    DDX_Control(pDX, IDC_UNICASTSPECIFIC, mUnicastSpecificRadio);
    DDX_Control(pDX, IDC_UNICASTOTHER, mUnicastOtherRadio);
    DDX_Control(pDX, IDC_SPECIFICPORT, mUnicastSpecificPortEdit);
    DDX_Control(pDX, IDC_SPECIFICPORTLABEL, mUnicastSpecificPortLabel);
    DDX_Control(pDX, IDC_IP, mUnicastIPCtrl);
    DDX_Control(pDX, IDC_PORT, mUnicastPortEdit);
    DDX_Control(pDX, IDC_IPLABEL, mUnicastIPLabel);
    DDX_Control(pDX, IDC_PORTLABEL, mUnicastPortLabel);
    DDX_Control(pDX, IDC_MULTICAST, mMulticastRadio);
    DDX_Control(pDX, IDC_MULTICASTIP, mMulticastIPCtrl);
    DDX_Control(pDX, IDC_MULTICASTPORT, mMulticastPortEdit);
    DDX_Control(pDX, IDC_MULTICASTIPLABEL, mMulticastIPLabel);
    DDX_Control(pDX, IDC_MULTICASTPORTLABEL, mMulticastPortLabel);
    DDX_Control(pDX, IDC_CHANNEL_EDIT, mChannelEdit);
    DDX_Control(pDX, IDC_CHANNEL_LABEL, mChannelLabel);
}


// =============================================================================
BOOL SetupDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    SetCurrentRole( mSetup->GetRole() );
    SetCurrentDestination( mSetup->GetDestination() );

    CString lStr;

    // Specific port
    lStr.Format( _T( "%i" ), mSetup->GetUnicastSpecificPort() );
    mUnicastSpecificPortEdit.SetWindowText( lStr );

    // Default channel
    lStr.Format( _T( "%i" ), mSetup->GetDefaultChannel() );
    mChannelEdit.SetWindowText( lStr );

    // IP
    IPStrToCtrl( mSetup->GetUnicastIP().GetUnicode(), mUnicastIPCtrl );

    // Port
    lStr.Format( _T( "%i" ), mSetup->GetUnicastPort() );
    mUnicastPortEdit.SetWindowText( lStr );

    // Multicast IP
    IPStrToCtrl( mSetup->GetMulticastIP().GetUnicode(), mMulticastIPCtrl );

    // Multicast port
    lStr.Format( _T( "%i" ), mSetup->GetMulticastPort() );
    mMulticastPortEdit.SetWindowText( lStr );

    EnableInterface();

    return TRUE;
}


// =============================================================================
void SetupDlg::IPStrToCtrl( const CString &aIPStr, CIPAddressCtrl &aCtrl )
{
    int lIP[ 4 ];
    int lCount = swscanf_s( aIPStr, _T( "%i.%i.%i.%i" ), lIP, lIP + 1, lIP + 2, lIP + 3 );
    ASSERT( lCount == 4 );
    if ( lCount == 4 )
    {
        BYTE lIPb[ 4 ];
        for ( int i = 0; i < 4; i++ )
        {
            ASSERT( lIP[ i ] >= 0 );
            ASSERT( lIP[ i ] <= 255 );

            lIPb[ i ] = static_cast<BYTE>( lIP[ i ] );
        }

        aCtrl.SetAddress( lIPb[ 0 ], lIPb[ 1 ], lIPb[ 2 ], lIPb[ 3 ] );
    }
}


// =============================================================================
void SetupDlg::EnableInterface()
{
    Setup::Role lRole = GetCurrentRole();
    Setup::Destination lDestination = GetCurrentDestination();

    mSetup->UpdateEnabled( lRole, lDestination );

    // Refresh destination UI selection (it may have changed in UpdateEnagled)
    SetCurrentDestination( lDestination );

    mCtrlDataRadio.EnableWindow( mSetup->IsRoleCtrlDataEnabled() );
    mCtrlRadio.EnableWindow( mSetup->IsRoleCtrlEnabled() );
    mDataRadio.EnableWindow( mSetup->IsRoleDataEnabled() );
    mUnicastSpecificRadio.EnableWindow( mSetup->IsDestUnicastSpecificEnabled() );
    mUnicastAutoRadio.EnableWindow( mSetup->IsDestUnicastAutoEnabled() );
    mUnicastOtherRadio.EnableWindow( mSetup->IsDestUnicastOtherEnabled() );
    mMulticastRadio.EnableWindow( mSetup->IsDestMulticastEnabled() );
    mChannelEdit.EnableWindow( mSetup->IsChannelEnabled() );

    mChannelLabel.EnableWindow( mChannelEdit.IsWindowEnabled() );

    mUnicastSpecificPortEdit.EnableWindow( ( lDestination == Setup::DestinationUnicastSpecific ) && mUnicastSpecificRadio.IsWindowEnabled() );
    mUnicastSpecificPortLabel.EnableWindow( mUnicastSpecificPortEdit.IsWindowEnabled() );

    mUnicastIPCtrl.EnableWindow( ( lDestination == Setup::DestinationUnicastOther ) && mUnicastOtherRadio.IsWindowEnabled() );
    mUnicastPortEdit.EnableWindow( mUnicastIPCtrl.IsWindowEnabled() );
    mUnicastIPLabel.EnableWindow( mUnicastIPCtrl.IsWindowEnabled() );
    mUnicastPortLabel.EnableWindow( mUnicastIPCtrl.IsWindowEnabled() );

    mMulticastIPCtrl.EnableWindow( ( lDestination == Setup::DestinationMulticast ) && mMulticastRadio.IsWindowEnabled() );
    mMulticastPortEdit.EnableWindow( mMulticastIPCtrl.IsWindowEnabled() );
    mMulticastIPLabel.EnableWindow( mMulticastIPCtrl.IsWindowEnabled() );
    mMulticastPortLabel.EnableWindow( mMulticastIPCtrl.IsWindowEnabled() );
}


// =============================================================================
void SetupDlg::OnBnClicked()
{
    EnableInterface();
}


// =============================================================================
void SetupDlg::OnOK()
{
    if ( mSetup->IsEnabled() )
    {
        mSetup->SetRole( GetCurrentRole() );
        mSetup->SetDestination( GetCurrentDestination() );
        
        BYTE lIP[ 4 ];
        int lPort = 0;
        int lChannel = 0;
        int lCount = 0;
        CString lStr;

        mUnicastSpecificPortEdit.GetWindowText( lStr );
        swscanf_s( (LPCTSTR)lStr, _T( "%i" ), &lPort );
        mSetup->SetUnicastSpecificPort( static_cast<unsigned short>( lPort ) );

        mChannelEdit.GetWindowText( lStr );
        lCount = swscanf_s( (LPCTSTR)lStr, _T( "%i" ), &lChannel );
        if ( lCount != 1 )
        {
            MessageBox( _T( "Error parsing default channel." ), _T( "eBUS Player" ),
                MB_OK | MB_ICONINFORMATION );
            return;
        }
        if ( ( lChannel < 0 ) || ( lChannel > 511 ) )
        {
            MessageBox( _T( "Default channel out of range. It must be between 0 and 511 inclusively." ), _T( "eBUS Player" ),
                MB_OK | MB_ICONINFORMATION );
            return;
        }
        mSetup->SetDefaultChannel( static_cast<unsigned short>( lChannel ) );

        mUnicastIPCtrl.GetAddress( lIP[ 0 ], lIP[ 1 ], lIP[ 2 ], lIP[ 3 ] );
        mSetup->SetUnicastIP( Setup::IPFromBytes( lIP[ 0 ], lIP[ 1 ], lIP[ 2 ], lIP[ 3 ] ) );

        mUnicastPortEdit.GetWindowText( lStr );
        lCount = swscanf_s( (LPCTSTR)lStr, _T( "%i" ), &lPort );
        if ( lCount != 1 )
        {
            MessageBox( _T( "Error parsing unicast port." ), _T( "eBUS Player" ),
                MB_OK | MB_ICONINFORMATION );
            return;
        }
        mSetup->SetUnicastPort( static_cast<unsigned short>( lPort ) );

        mMulticastIPCtrl.GetAddress( lIP[ 0 ], lIP[ 1 ], lIP[ 2 ], lIP[ 3 ] );
        mSetup->SetMulticastIP( Setup::IPFromBytes( lIP[ 0 ], lIP[ 1 ], lIP[ 2 ], lIP[ 3 ] ) );

        mMulticastPortEdit.GetWindowText( lStr );
        lCount = swscanf_s( (LPCTSTR)lStr, _T( "%i" ), &lPort );
        if ( lCount != 1 )
        {
            MessageBox( _T( "Error parsing multicast port." ), _T( "eBUS Player" ),
                MB_OK | MB_ICONINFORMATION );
            return;
        }
        mSetup->SetMulticastPort( static_cast<unsigned short>( lPort ) );
    }

    CDialog::OnOK();
}


// =============================================================================
void SetupDlg::OnCancel()
{
    CDialog::OnCancel();
}


// =============================================================================
Setup::Role SetupDlg::GetCurrentRole()
{
    int lRole = GetCheckedRadioButton( IDC_CTRLDATA, IDC_DATA );
    switch ( lRole )
    {
    case IDC_CTRLDATA:
        return Setup::RoleCtrlData;

    case IDC_CTRL:
        return Setup::RoleCtrl;

    case IDC_DATA:
        return Setup::RoleData;

    default:
        ASSERT( 0 );
    }

    return Setup::RoleInvalid;
}


// =============================================================================
Setup::Destination SetupDlg::GetCurrentDestination()
{
    int lDest = GetCheckedRadioButton( IDC_UNICASTAUTO, IDC_MULTICAST );
    switch ( lDest )
    {
    case IDC_UNICASTAUTO:
        return Setup::DestinationUnicastAuto;

    case IDC_UNICASTSPECIFIC:
        return Setup::DestinationUnicastSpecific;

    case IDC_UNICASTOTHER:
        return Setup::DestinationUnicastOther;

    case IDC_MULTICAST:
        return Setup::DestinationMulticast;

    default:
        ASSERT( 0 );
    }

    return Setup::DestinationInvalid;
}


// =============================================================================
void SetupDlg::SetCurrentRole( Setup::Role aRole )
{
    // Role
    int lRole = IDC_CTRLDATA;
    switch ( aRole )
    {
    case Setup::RoleCtrlData:
        lRole = IDC_CTRLDATA;
        break;

    case Setup::RoleCtrl:
        lRole = IDC_CTRL;
        break;

    case Setup::RoleData:
        lRole = IDC_DATA;
        break;

    default:
        ASSERT( 0 );
    }

    CheckRadioButton( IDC_CTRLDATA, IDC_DATA, lRole );
}


// =============================================================================
void SetupDlg::SetCurrentDestination( Setup::Destination aDestination )
{
    // Destination
    int lDestination = IDC_UNICASTAUTO;
    switch ( aDestination )
    {
    case Setup::DestinationUnicastAuto:
        lDestination = IDC_UNICASTAUTO;
        break;

    case Setup::DestinationUnicastSpecific:
        lDestination = IDC_UNICASTSPECIFIC;
        break;

    case Setup::DestinationUnicastOther:
        lDestination = IDC_UNICASTOTHER;
        break;

    case Setup::DestinationMulticast:
        lDestination = IDC_MULTICAST;
        break;

    default:
        ASSERT( 0 );
    }

    CheckRadioButton( IDC_UNICASTAUTO, IDC_MULTICAST, lDestination );
}

