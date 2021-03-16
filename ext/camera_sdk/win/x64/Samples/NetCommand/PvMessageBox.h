// *****************************************************************************
//
//     Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once


inline void PvMessageBox( CWnd *aWnd, PvResult &aResult )
{
    CString lError;
    lError.Format( _T( "%s\r\n\r\n%s\r\n" ), 
        aResult.GetCodeString().GetUnicode(), 
        aResult.GetDescription().GetUnicode() );

    ::MessageBox( aWnd->GetSafeHwnd(), lError, _T( "Error" ), MB_ICONERROR | MB_OK );
}


