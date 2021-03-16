// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"
#include "ActionCommandDlg.h"


IMPLEMENT_DYNAMIC(ActionCommandDlg, CDialog)

BEGIN_MESSAGE_MAP(ActionCommandDlg, CDialog)
    ON_BN_CLICKED(IDC_CHECKSCHEDULED, &ActionCommandDlg::OnBnClickedCheckscheduled)
    ON_BN_CLICKED(IDC_BUTTONSEND, &ActionCommandDlg::OnBnClickedButtonsend)
    ON_CLBN_CHKCHANGE(IDC_LIST_INTERFACES, &ActionCommandDlg::OnClBnChkChange)
END_MESSAGE_MAP()


///
/// \brief Constructor
///

ActionCommandDlg::ActionCommandDlg( CWnd* pParent /*=NULL*/ )
    : CDialog( ActionCommandDlg::IDD, pParent )
{

}


///
/// \brief Destructor
///

ActionCommandDlg::~ActionCommandDlg()
{
}


///
/// \brief MFC's binding of UI elements to class members
///

void ActionCommandDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDITDEVICEKEY, mDeviceKeyEdit);
    DDX_Control(pDX, IDC_EDITGROUPKEY, mGroupKeyEdit);
    DDX_Control(pDX, IDC_EDITGROUPMASK, mGroupMaskEdit);
    DDX_Control(pDX, IDC_ACTIONTIME, mScheduledTime);
    DDX_Control(pDX, IDC_REQUESTACKNOWLEDGESCHECKBOX, mRequestAcknowledgesCheckBox);
    DDX_Control(pDX, IDC_CHECKSCHEDULED, mScheduledCheck);
    DDX_Control(pDX, IDC_STATIC_ACTIONTIME_ENABLED, mActionTimeLabel);
    DDX_Control(pDX, IDC_BUTTONSEND, mSendButton);
    DDX_Control(pDX, IDC_LISTACK, mAcknowledgeList);
    DDX_Control(pDX, IDC_LIST_INTERFACES, mInterfaceList);
}


///
/// \brief Dialog initialization
///

BOOL ActionCommandDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    CString lTemp;
    PvString lDescription;
    PvString lIPAddress;
    PvString lMACAddress;

    // Populate the interface lists
    mInterfaceList.ResetContent();
    mInterfaceList.SetCheckStyle( BS_AUTOCHECKBOX );
    for( uint32_t i = 0; i < mActionCommand.GetInterfaceCount(); i++ )
    {
        // Format the information about the interface for the display
        mActionCommand.GetInterfaceDescription( i, lDescription );
        mActionCommand.GetInterfaceIPAddress( i, lIPAddress );
        lTemp.Format( L"%s, %s", 
            lDescription.GetUnicode(), lIPAddress.GetUnicode() );

        // Now add the line to the control  
        mInterfaceList.AddString( lTemp );

        bool lEnabled = false;
        mActionCommand.GetInterfaceEnabled( i, lEnabled );
        mInterfaceList.SetCheck( i, lEnabled );
    }
    
    // Try to setup the default to all the default value from the class
    mDeviceKeyEdit.SetValue( mActionCommand.GetDeviceKey(), true );
    mGroupKeyEdit.SetValue( mActionCommand.GetGroupKey(), true );
    mGroupMaskEdit.SetValue( mActionCommand.GetGroupMask(), true );

    uint64_t lScheduledTime = mActionCommand.GetScheduledTime();
    mScheduledTime.SetValue( lScheduledTime, false );
    mScheduledTime.EnableWindow( lScheduledTime != 0ULL );
    mActionTimeLabel.EnableWindow( lScheduledTime != 0ULL );
    mScheduledCheck.SetCheck( lScheduledTime != 0ULL );
    
    // We default to requesting acknowledges
    mRequestAcknowledgesCheckBox.SetCheck( BST_CHECKED );

    EnableInterface();

    return TRUE;
}


///
/// \brief Scheduled check box click handler
///

void ActionCommandDlg::OnBnClickedCheckscheduled()
{
    EnableInterface();
}


///
/// \brief Send button handler
///

void ActionCommandDlg::OnBnClickedButtonsend()
{
    CWaitCursor lWaitCursor;

    mAcknowledgeList.ResetContent();

    // Now build configure the action command
    mActionCommand.SetDeviceKey( mDeviceKeyEdit.GetValueUInt32() );
    mActionCommand.SetGroupKey( mGroupKeyEdit.GetValueUInt32() );
    mActionCommand.SetGroupMask( mGroupMaskEdit.GetValueUInt32() );
    mActionCommand.SetScheduledTimeEnable( mScheduledCheck.GetCheck() ? true : false );
    mActionCommand.SetScheduledTime( mScheduledTime.GetValueUInt64() );

    // Update the selection of action command
    for( uint32_t i = 0; i < mActionCommand.GetInterfaceCount(); i++ )
    {
        mActionCommand.SetInterfaceEnabled( i, mInterfaceList.GetCheck( i ) ? true : false );
    }

    // Now send the command
    bool lRequestAcknowledges = ( mRequestAcknowledgesCheckBox.GetCheck() == BST_CHECKED ) ? true : false;
    PvResult lResult = mActionCommand.Send( 1000U, 0U, lRequestAcknowledges );
    if( lResult.IsOK() )
    {
        CString lTemp;
        PvActionAckStatusEnum lStatus;
        PvString lIPAddress;
        static const wchar_t* sActionAckText[] = {L"OK", L"Late", L"Overflow", L"No ref time" };

        for( uint32_t i = 0; i < mActionCommand.GetAcknowledgementCount(); i++ )
        {
            mActionCommand.GetAcknowledgementStatus( i, lStatus );
            mActionCommand.GetAcknowledgementIPAddress( i, lIPAddress );

            lTemp.Format( L"[ %s ] from %s", sActionAckText[ lStatus ], lIPAddress.GetUnicode() );
            mAcknowledgeList.AddString( lTemp );
        }
    }

}


///
/// \brief OK handler, do nothing
///

void ActionCommandDlg::OnOK()
{
    // Do nothing, just ensure not calling the parent
}


///
/// \brief Cancel handler, close the window
///

void ActionCommandDlg::OnCancel()
{
    DestroyWindow();
}


///
/// \brief Handler of the event that an interface has been checked or unchecked
///

void ActionCommandDlg::OnClBnChkChange()
{
    EnableInterface();
}


///
/// \brief Based on the UI context, make sure eveything is enabled or disabled as it should
///

void ActionCommandDlg::EnableInterface()
{
    // Enable scheduled text box only if scheduled action command option is checked
    BOOL lScheduledEnabled = ( mScheduledCheck.GetCheck() == BST_CHECKED );
    mScheduledTime.EnableWindow( lScheduledEnabled );
    mActionTimeLabel.EnableWindow( lScheduledEnabled );

    // Enable send button only if at least one interface is checked
    BOOL lSendEnabled = FALSE;
    for ( int i = 0; i < mInterfaceList.GetCount(); i++ )
    {
        if ( mInterfaceList.GetCheck( i ) == BST_CHECKED )
        {
            lSendEnabled = TRUE;
            break;
        }
    }
    mSendButton.EnableWindow( lSendEnabled );
}

