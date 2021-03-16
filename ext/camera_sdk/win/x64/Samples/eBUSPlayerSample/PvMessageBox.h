// *****************************************************************************
//
//     Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once


inline CString PvTrimFilename( const PvString &aString )
{
    CString lString = aString.GetUnicode();

    int lIndex = lString.Find( _T( ".cpp:" ) );
    if ( lIndex > 0 )
    {
        lIndex = lString.Find( _T( " " ) );
        int lCount = lString.GetLength() - ( lIndex + 1 );
        if ( ( lIndex > 0 ) && ( lCount > 0 ) )
        {
            return lString.Right( lCount );
        }
    }

    return lString;
}

inline void PvMessageBox( CWnd *aWnd, PvResult &aResult )
{
    CString lError;
    lError.Format( _T( "%s\r\n\r\n%s\r\n" ), 
        aResult.GetCodeString().GetUnicode(), 
        (LPCTSTR)PvTrimFilename( aResult.GetDescription() ) );

    ::MessageBox( aWnd->GetSafeHwnd(), lError, _T( "Error" ), MB_ICONERROR | MB_OK );
}


