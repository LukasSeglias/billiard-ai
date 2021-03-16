// *****************************************************************************
//
// Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// -----------------------------------------------------------------------------
//
// Dialog used to interactively edit a Setup object.
// 
// *****************************************************************************

#include "stdafx.h"
#include "NetCommand.h"
#include "SetupDlg.h"

#include <PvDeviceInfoGEV.h>


BEGIN_MESSAGE_MAP(SetupDlg, CDialog)
    ON_BN_CLICKED(IDC_CTRLDATA, &OnBnClicked)
    ON_BN_CLICKED(IDC_CTRL, &OnBnClicked)
    ON_BN_CLICKED(IDC_DATA, &OnBnClicked)
    ON_BN_CLICKED(IDC_UNICASTAUTO, &OnBnClicked)
    ON_BN_CLICKED(IDC_UNICASTSPECIFIC, &OnBnClicked)
    ON_BN_CLICKED(IDC_UNICASTOTHER, &OnBnClicked)
    ON_BN_CLICKED(IDC_MULTICAST, &OnBnClicked)
END_MESSAGE_MAP()


// =============================================================================
SetupDlg::SetupDlg(CWnd* pParent /*=NULL*/)
    : CDialog(IDD_SETUP, pParent)
    , mEnabled( true )
    , mDeviceInfo( NULL )
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
}

// =============================================================================
BOOL SetupDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Role
    int lRole = IDC_CTRLDATA;
    switch ( mSetup.GetRole() )
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

    // Destination
    int lDestination = IDC_UNICASTAUTO;
    switch ( mSetup.GetDestination() )
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

    CString lStr;

    // Specific port
    lStr.Format( _T( "%i" ), mSetup.GetUnicastSpecificPort() );
    mUnicastSpecificPortEdit.SetWindowText( lStr );

    // IP
    IPStrToCtrl( mSetup.GetUnicastIP(), mUnicastIPCtrl );

    // Port
    lStr.Format( _T( "%i" ), mSetup.GetUnicastPort() );
    mUnicastPortEdit.SetWindowText( lStr );

    // Multicast IP
    IPStrToCtrl( mSetup.GetMulticastIP(), mMulticastIPCtrl );

    // Multicast port
    lStr.Format( _T( "%i" ), mSetup.GetMulticastPort() );
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
    if ( !mEnabled )
    {
        // Master disable, not touching the state of the UI
        mCtrlDataRadio.EnableWindow( false );
        mCtrlRadio.EnableWindow( false );
        mDataRadio.EnableWindow( false );
        mUnicastAutoRadio.EnableWindow( false );
        mUnicastSpecificRadio.EnableWindow( false );
        mUnicastOtherRadio.EnableWindow( false );
        mMulticastRadio.EnableWindow( false );
        mUnicastSpecificPortEdit.EnableWindow( false );
        mUnicastSpecificPortLabel.EnableWindow( false );
        mUnicastIPCtrl.EnableWindow( false );
        mUnicastPortEdit.EnableWindow( false );
        mUnicastIPLabel.EnableWindow( false );
        mUnicastPortLabel.EnableWindow( false );
        mMulticastIPCtrl.EnableWindow( false );
        mMulticastPortEdit.EnableWindow( false );
        mMulticastIPLabel.EnableWindow( false );
        mMulticastPortLabel.EnableWindow( false );

        return;
    }

    int lRole = GetCheckedRadioButton( IDC_CTRLDATA, IDC_DATA );
    int lDest = GetCheckedRadioButton( IDC_UNICASTAUTO, IDC_MULTICAST );
    
    bool lCanStream = true;
    if ( mDeviceInfo != NULL )
    {
        lCanStream = ( mDeviceInfo->GetClass() == PvDeviceClassTransmitter ) ||
            ( mDeviceInfo->GetClass() == PvDeviceClassTransceiver ) ||
            ( mDeviceInfo->GetClass() == PvDeviceClassUnknown );
    }

    mCtrlDataRadio.EnableWindow( lCanStream );
    mCtrlRadio.EnableWindow( true );
    mDataRadio.EnableWindow( lCanStream );

    // Make sure we check the first enabled option for role (if needed)
    bool lValid = false;
    lValid |= ( lRole == IDC_CTRLDATA ) && mCtrlDataRadio.IsWindowEnabled();
    lValid |= ( lRole == IDC_CTRL ) && mCtrlRadio.IsWindowEnabled();
    lValid |= ( lRole == IDC_DATA ) && mDataRadio.IsWindowEnabled();
    if ( !lValid )
    {
        if ( mCtrlDataRadio.IsWindowEnabled() )
        {
            CheckRadioButton( IDC_CTRLDATA, IDC_DATA, IDC_CTRLDATA );
        }
        else if ( mCtrlRadio.IsWindowEnabled() )
        {
            CheckRadioButton( IDC_CTRLDATA, IDC_DATA, IDC_CTRL );
        }
        else if ( mDataRadio.IsWindowEnabled() )
        {
            CheckRadioButton( IDC_CTRLDATA, IDC_DATA, IDC_DATA );
        }

        // ...refresh!
        lRole = GetCheckedRadioButton( IDC_CTRLDATA, IDC_DATA );
    }

    bool lCtrlData = ( lRole == IDC_CTRLDATA );
    bool lCtrl = ( lRole == IDC_CTRL );
    bool lData = ( lRole == IDC_DATA );

    // Stream radio buttons
    mUnicastAutoRadio.EnableWindow( ( lCtrlData || lData ) && mEnabled );
    mUnicastSpecificRadio.EnableWindow( ( lCtrlData || lData ) && mEnabled );
    mUnicastOtherRadio.EnableWindow( lCtrl && mEnabled );
    mMulticastRadio.EnableWindow( ( lCtrl || lData || lCtrlData ) && mEnabled );

    // Make sure we check the first enabled option for destination (if needed)
    lValid = false;
    lValid |= ( lDest == IDC_UNICASTAUTO ) && mUnicastAutoRadio.IsWindowEnabled();
    lValid |= ( lDest == IDC_UNICASTSPECIFIC ) && mUnicastSpecificRadio.IsWindowEnabled();
    lValid |= ( lDest == IDC_UNICASTOTHER ) && mUnicastOtherRadio.IsWindowEnabled();
    lValid |= ( lDest == IDC_MULTICAST ) && mMulticastRadio.IsWindowEnabled();
    if ( !lValid )
    {
        if ( mUnicastAutoRadio.IsWindowEnabled() )
        {
            CheckRadioButton( IDC_UNICASTAUTO, IDC_MULTICAST, IDC_UNICASTAUTO );
        }
        else if ( mUnicastSpecificRadio.IsWindowEnabled() )
        {
            CheckRadioButton( IDC_UNICASTAUTO, IDC_MULTICAST, IDC_UNICASTSPECIFIC );
        }
        else if ( mUnicastOtherRadio.IsWindowEnabled() )
        {
            CheckRadioButton( IDC_UNICASTAUTO, IDC_MULTICAST, IDC_UNICASTOTHER  );
        }
        else if ( mMulticastRadio.IsWindowEnabled() )
        {
            CheckRadioButton( IDC_UNICASTAUTO, IDC_MULTICAST, IDC_MULTICAST );
        }

        // ...refresh!
        lDest = GetCheckedRadioButton( IDC_UNICASTAUTO, IDC_MULTICAST );
    }

    // Unicast specific port
    mUnicastSpecificPortEdit.EnableWindow( ( lDest == IDC_UNICASTSPECIFIC ) && mUnicastSpecificRadio.IsWindowEnabled() && mEnabled );
    mUnicastSpecificPortLabel.EnableWindow( mUnicastSpecificPortEdit.IsWindowEnabled() );

    // Unicast other IP/port
    mUnicastIPCtrl.EnableWindow( ( lDest == IDC_UNICASTOTHER ) && mUnicastOtherRadio.IsWindowEnabled() && mEnabled );
    mUnicastPortEdit.EnableWindow( mUnicastIPCtrl.IsWindowEnabled() );
    mUnicastIPLabel.EnableWindow( mUnicastIPCtrl.IsWindowEnabled() );
    mUnicastPortLabel.EnableWindow( mUnicastIPCtrl.IsWindowEnabled() );

    // Multicast IP/port
    mMulticastIPCtrl.EnableWindow( ( lDest == IDC_MULTICAST ) && mMulticastRadio.IsWindowEnabled() && mEnabled );
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
    if ( mEnabled )
    {
        mSetup.SetRole( Setup::RoleInvalid );
        mSetup.SetDestination( Setup::DestinationInvalid );
        
        int lRole = GetCheckedRadioButton( IDC_CTRLDATA, IDC_DATA );
        switch ( lRole )
        {
        case IDC_CTRLDATA:
            mSetup.SetRole( Setup::RoleCtrlData );
            break;

        case IDC_CTRL:
            mSetup.SetRole( Setup::RoleCtrl );
            break;

        case IDC_DATA:
            mSetup.SetRole( Setup::RoleData );
            break;

        default:
            ASSERT( 0 );
        }

        int lDest = GetCheckedRadioButton( IDC_UNICASTAUTO, IDC_MULTICAST );
        switch ( lDest )
        {
            case IDC_UNICASTAUTO:
                mSetup.SetDestination( Setup::DestinationUnicastAuto );
                break;

            case IDC_UNICASTSPECIFIC:
                mSetup.SetDestination( Setup::DestinationUnicastSpecific );
                break;

            case IDC_UNICASTOTHER:
                mSetup.SetDestination( Setup::DestinationUnicastOther );
                break;

            case IDC_MULTICAST:
                mSetup.SetDestination( Setup::DestinationMulticast );
                break;

            default:
                ASSERT( 0 );
        }

        BYTE lIP[ 4 ];
        int lPort = 0;
        CString lStr;

        mUnicastSpecificPortEdit.GetWindowText( lStr );
        swscanf_s( (LPCTSTR)lStr, _T( "%i" ), &lPort );
        mSetup.SetUnicastSpecificPort( static_cast<unsigned short>( lPort ) );

        mUnicastIPCtrl.GetAddress( lIP[ 0 ], lIP[ 1 ], lIP[ 2 ], lIP[ 3 ] );
        lStr.Format( _T( "%i.%i.%i.%i" ), lIP[ 0 ], lIP[ 1 ], lIP[ 2 ], lIP[ 3 ] );
        mSetup.SetUnicastIP( lStr );

        mUnicastPortEdit.GetWindowText( lStr );
        swscanf_s( (LPCTSTR)lStr, _T( "%i" ), &lPort );
        mSetup.SetUnicastPort( static_cast<unsigned short>( lPort ) );

        mMulticastIPCtrl.GetAddress( lIP[ 0 ], lIP[ 1 ], lIP[ 2 ], lIP[ 3 ] );
        lStr.Format( _T( "%i.%i.%i.%i" ), lIP[ 0 ], lIP[ 1 ], lIP[ 2 ], lIP[ 3 ] );
        mSetup.SetMulticastIP( lStr );

        mMulticastPortEdit.GetWindowText( lStr );
        swscanf_s( (LPCTSTR)lStr, _T( "%i" ), &lPort );
        mSetup.SetMulticastPort( static_cast<unsigned short>( lPort ) );
    }

    CDialog::OnOK();
}

// =============================================================================
void SetupDlg::OnCancel()
{
    CDialog::OnCancel();
}


