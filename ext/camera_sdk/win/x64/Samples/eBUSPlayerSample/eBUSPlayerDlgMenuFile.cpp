// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"
#include "eBUSPlayerDlg.h"


// =============================================================================
void eBUSPlayerDlg::InitFileMenu( CMenu* pMenuItem, UINT aID, int aIndex )
{
    switch ( aID )
    {
    case ID_FILE_SAVEAS:
    case ID_FILE_SAVE:
        pMenuItem->EnableMenuItem( aIndex, MF_BYPOSITION | 
            ( ( ( mPlayer->IsDeviceConnected() || mPlayer->IsStreamOpened() ) && !mPlayer->IsInRecovery() && !mPlayer->IsStreaming() ) ? MF_ENABLED : MF_GRAYED ) );
        break;

    case ID_FILE_LOAD:
        pMenuItem->EnableMenuItem( aIndex, MF_BYPOSITION | 
            ( ( ( mCommunicationWnd.GetHandle() == 0 ) && 
            ( ( mDeviceWnd == NULL ) || ( mDeviceWnd->GetHandle() == 0 ) ) && 
            ( mStreamParametersWnd.GetHandle() == 0 ) && 
            ( !mPlayer->IsInRecovery() && !mPlayer->IsStreaming() ) ) ? MF_ENABLED : MF_GRAYED ) );
        break;

    case ID_FILE_RECENT1:
    case ID_FILE_RECENT2:
    case ID_FILE_RECENT3:
    case ID_FILE_RECENT4:
    case ID_FILE_RECENT5:
    case ID_FILE_RECENT6:
    case ID_FILE_RECENT7:
    case ID_FILE_RECENT8:
    case ID_FILE_RECENT9:
    case ID_FILE_RECENT10:
        pMenuItem->EnableMenuItem( aIndex, MF_BYPOSITION | 
            ( ( !mPlayer->IsInRecovery() && !mPlayer->IsStreaming() ) ? MF_ENABLED : MF_GRAYED ) );
        break;
    }
}


// =============================================================================
void eBUSPlayerDlg::OnFileLoad()
{
    if ( mPlayer->IsStreaming() )
    {
        return;
    }

    CFileDialog lFileDlg( TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T( "eBUS Player Configuration Files (*.pvcfg)|*.pvcfg|All files (*.*)|*.*||" ), this);

    lFileDlg.m_ofn.lpstrTitle = L"Open eBUS SDK Project";

    if ( lFileDlg.DoModal() == IDOK)
    {
        mFileName = lFileDlg.GetPathName();
        OpenConfig( mFileName );  
    }
}


// =============================================================================
void eBUSPlayerDlg::OnFileSave()
{
    if ( mPlayer->IsStreaming() )
    {
        return;
    }

    if ( mFileName.IsEmpty() )
    {
        OnFileSaveAs();
        return;
    }

    SaveConfig( mFileName, true );

    ReportMRU( mFileName );
}


// =============================================================================
void eBUSPlayerDlg::OnFileExit()
{
    Disconnect();
    EndDialog( IDOK );
}


// =============================================================================
void eBUSPlayerDlg::OnRecent( UINT nID )
{
    unsigned int lIndex = nID - ID_FILE_RECENT1;
    std::list<CString>::iterator lIt;
    lIt = mRecentList.begin();
    unsigned int i = 0;
    while ( lIt != mRecentList.end() && i <= lIndex )
    {
        if ( i == lIndex )
        {
            mFileName = *lIt;
        }

        lIt++;
        i++;
    }

    OpenConfig( mFileName );
}


// =============================================================================
void eBUSPlayerDlg::OnFileSaveAs()
{
    if ( mPlayer->IsStreaming() )
    {
        return;
    }

    CFileDialog lFileDlg( FALSE, _T( ".pvcfg" ), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
        _T( "eBUS Player Configuration Files (*.pvcfg)|*.pvcfg|All files (*.*)|*.*||" ), this);
    lFileDlg.m_ofn.lpstrTitle = L"Save eBUS SDK Project";
    if ( lFileDlg.DoModal() == IDOK)
    {
        mFileName = lFileDlg.GetPathName(); // This is your selected file name with path

        OnFileSave();
    }
}


// =============================================================================
void eBUSPlayerDlg::ReportMRU(CString aFileName)
{
    //typedef CStringT< TCHAR, StrTraitATL< TCHAR > > CAtlString;
    CString resToken;
    int curPos= 0;
    CString lName;
    resToken= aFileName.Tokenize(_T("\\"),curPos);
    while (resToken != "")
    {
        lName = resToken;
        resToken= aFileName.Tokenize(_T("\\"),curPos);
    };
    SetWindowText( lName + _T(" - " ) + GetAppName() );

    // Try to find the item in our current list

    std::list<CString>::iterator lIt;
    std::list<CString>::iterator lToRemove;

    lIt = mRecentList.begin();
    bool lRemove = false;
    while (lIt != mRecentList.end())
    {
        if (*lIt == aFileName)
        {
            // If match, remove it, will be added as top latter
            lToRemove = lIt;
            lRemove = true;
        }
        lIt++;
    }

    if ( lRemove )
    {
        mRecentList.erase(lToRemove);
    }

    // Whether we removed already existing or not, insert at top
    mRecentList.push_front(aFileName);

    // trim the end (we don't want more than 10)
    if ( mRecentList.size() > 10 )
    {
        lIt = mRecentList.begin();
        std::list<CString>::iterator lLast;
        while (lIt != mRecentList.end())
        {
            lLast = lIt;
            lIt++;
        }
        // erase the back, not the front
        mRecentList.erase( lLast );
    }

    // Save current MRU to registry
    SaveMRUToRegistry();

    UpdateMRUMenu();
}


// =============================================================================
void eBUSPlayerDlg::SaveMRUToRegistry()
{
    // iterate through the list

    std::list<CString>::iterator lIt;

    lIt = mRecentList.begin();

    CWinApp* pApp = AfxGetApp();

    UINT lCount = 0;
    while( lIt != mRecentList.end() ) 
    {
        wchar_t lNumber[4];
        swprintf_s(lNumber, 4, L"%d", lCount);

        wchar_t lToRegistry[MAX_PATH];
        swprintf_s(lToRegistry, MAX_PATH, L"%s", lIt->GetBuffer());
        BOOL lRet = pApp->WriteProfileString(_T("MRU"), lNumber, lToRegistry);
        lIt++;
        lCount++;
    }

}


// =============================================================================
void eBUSPlayerDlg::LoadMRUFromRegistry()
{

    CString lData;
    DWORD lRet = 1;
    UINT lCount = 0;

    CWinApp* pApp = AfxGetApp();
    while( lData.Compare(_T("default")) != 0 )
    {
        wchar_t lNumber[4];
        swprintf_s(lNumber, 4, _T("%d"), lCount);
        lData = pApp->GetProfileString( _T("MRU"), lNumber, _T("default") );
        if ( lData.Compare(_T("default")) != 0 )
        {
            mRecentList.push_back( lData ); 
        }
        lCount++;
    }
}


// =============================================================================
void eBUSPlayerDlg::UpdateMRUMenu()
{
    if ( mMRUMenu->GetSafeHmenu() == NULL )
    {
        mMRUMenu->CreatePopupMenu();

        CMenu* lMainMenu = GetMenu();
        CMenu* lSubMenu = lMainMenu->GetSubMenu( 0 );
        lSubMenu->InsertMenu( 1, MF_BYPOSITION | MF_POPUP, (UINT_PTR)mMRUMenu->m_hMenu, L"Open Recent" );
    }
    
    if ( !mRecentList.empty() )
    {
        std::list<CString>::iterator lIt;
        lIt = mRecentList.begin();
        unsigned int i = 0;
        while( lIt != mRecentList.end() )
        {
            mMRUMenu->DeleteMenu( ID_FILE_RECENT1 + i, MF_BYCOMMAND );

            wchar_t lRecentString[ MAX_PATH ] = { 0 };
            swprintf_s( lRecentString, MAX_PATH, L"%d - %s", i+1, lIt->GetBuffer() );

            mMRUMenu->AppendMenu( MF_STRING | MF_ENABLED, ID_FILE_RECENT1 + i, lRecentString );
            i++;
            lIt++;
        }
    }
}


// =============================================================================
void eBUSPlayerDlg::OnAcceleratorSave()
{
    OnFileSave();
}


// =============================================================================
void eBUSPlayerDlg::OnAcceleratorOpen()
{
    OnFileLoad();
}


