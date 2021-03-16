// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"
#include "eBUSPlayerDlg.h"

#include "AboutBox.h"



// =============================================================================
void eBUSPlayerDlg::OnInitMenu( CMenu* pMenu )
{
    CDialog::OnInitMenu(pMenu);

    int lCountI = static_cast<int>( pMenu->GetMenuItemCount() );
    for ( int i = 0; i < lCountI; i++ )
    {
        UINT lID = pMenu->GetMenuItemID( i );
        if ( lID == -1 )
        {
            CMenu *lSubMenu = pMenu->GetSubMenu( i );
            int lCountJ = static_cast<int>( lSubMenu->GetMenuItemCount() );
            for ( int j = 0; j < lCountJ; j++ )
            {  
                UINT lID = lSubMenu->GetMenuItemID( j );
                if ( lID != -1 )
                {
                    InitFileMenu( lSubMenu, lID, j );
                    InitToolsMenu( lSubMenu, lID, j );
                    InitOtherMenus( lSubMenu, lID, j );
                }
                else
                {
                    CMenu *lSubMenu2 = lSubMenu->GetSubMenu( j );
                    int lCountK = static_cast<int>( lSubMenu2->GetMenuItemCount() );
                    for ( int k = 0; k < lCountK; k++ )
                    {  
                        UINT lID = lSubMenu2->GetMenuItemID( k );
                        if ( lID != -1 )
                        {
                            InitFileMenu( lSubMenu2, lID, k );
                            InitToolsMenu( lSubMenu2, lID, k );
                        }
                    }
                }
            }
        }
    }
}


// =============================================================================
void eBUSPlayerDlg::OnHelpAbout()
{
    AboutBox lDlg;
    lDlg.DoModal();
}


