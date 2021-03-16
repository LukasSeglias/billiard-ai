// *****************************************************************************
//
//     Copyright (c) 2016, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"
#include "HexEdit.h"

#include <PvString.h>


BEGIN_MESSAGE_MAP(CHexEdit, CEdit)    
    ON_WM_CHAR()
    ON_MESSAGE(WM_PASTE, OnPaste)
    ON_WM_KILLFOCUS()
END_MESSAGE_MAP()


///
/// \brief Sets a value, 32-bit
///

void CHexEdit::SetValue( UINT32 aValue, bool aHex )
{
    CString lTemp;
    if( aHex )
    {
        lTemp.Format( L"0x%08X", aValue );
    }
    else
    {
        lTemp.Format( L"%u", aValue );
    }
    SetWindowTextW( lTemp );
}


///
/// \brief Sets a value, 64-bit
///

void CHexEdit::SetValue( UINT64 aValue, bool aHex )
{
    CString lTemp;
    if( aHex )
    {
        lTemp.Format( L"%016llX", aValue );
    }
    else
    {
        lTemp.Format( L"%llu", aValue );
    }
    SetWindowTextW( lTemp );
}


///
/// \brief Gets a 32-bit value
///

UINT32 CHexEdit::GetValueUInt32()
{
    uint32_t lValue = 0;
    CString lText;
    bool lHex = false;

    GetWindowText( lText );

    for( int i = 0; i < lText.GetLength(); i++ )
    {
        if( wcschr( L"AaBbCcDdEeFfxX", lText[ i ] ) != NULL )
        {
            lHex = true;
            break;
        }
    }

    if( lHex )
    {
        swscanf_s( lText, L"%x", &lValue ); 
    }
    else
    {
        swscanf_s( lText, L"%u", &lValue ); 
    }

    return lValue;
}


///
/// \brief Gets a 64-bit value
///

UINT64 CHexEdit::GetValueUInt64()
{
    UINT64 lValue = 0;
    CString lText;
    bool lHex = false;

    GetWindowText( lText );

    for( int i = 0; i < lText.GetLength(); i++ )
    {
        if( wcschr( L"AaBbCcDdEeFfxX", lText[ i ] ) != NULL )
        {
            lHex = true;
            break;
        }
    }

    if( lHex )
    {
        swscanf_s( lText, L"%llx", &lValue ); 
    }
    else
    {
        swscanf_s( lText, L"%llu", &lValue ); 
    }

    return lValue;
}


///
/// \brief Validates the value
///

void CHexEdit::ValidateValue()
{
    // Just validate that we do not need a 0x...
    int lStartChar;
    int lEndChar;
    CString lText;

    GetWindowText( lText );
    GetSel( lStartChar, lEndChar );

    for( int i = 0; i < lText.GetLength(); i++ )
    {
        if( wcschr( L"AaBbCcDdEeFf", lText[ i ] ) != NULL )
        {
            if( lText.Find( L"0x" ) < 0 
                && lText.Find( L"0X" ) < 0)
            {
                if( lText[ 0 ] == L'x' || lText[ 0 ] == L'X' )
                {
                    lText.Insert( 0, L"0" );
                    lStartChar++;
                }
                else
                {
                    lText.Insert( 0, L"0x" );
                    lStartChar += 2;
                }

                SetWindowText( lText );
                SetSel( lStartChar, lStartChar );
                return;
            }
        }
    }
}


///
/// \brief Focus-lost handler
///

void CHexEdit::OnKillFocus( CWnd* pcNewReceiver ) 
{
    ValidateValue();

    CEdit::OnKillFocus( pcNewReceiver );
}


///
/// \brief Clipboard cut operation
///

void CHexEdit::Cut()
{
    CEdit::Cut();

    ValidateValue();
}


///
/// \brief Clipboard past operation
///

LRESULT CHexEdit::OnPaste( WPARAM awParam, LPARAM alParam )
{
    int lStartChar;
    int lEndChar;
    CString lText;

    UNUSED_ALWAYS( awParam );
    UNUSED_ALWAYS( alParam );

    GetWindowText( lText );
    GetSel( lStartChar, lEndChar );

    if( lStartChar < 2 
        && ( lText.Find( L"0x" ) == 0 
        || lText.Find( L"0X" ) == 0 ) )
    {
        return 0;
    }

    // Remove the selected item
    if( lEndChar - lStartChar )
    {
        lText.Delete( lStartChar, lEndChar - lStartChar );
    }

    // Now paste the element from the clipboard
    CString lClipboardText;
    if( ::IsClipboardFormatAvailable( CF_UNICODETEXT ) )
    {
        // open the clipboard to get clipboard text
        if( ::OpenClipboard( m_hWnd ) )
        {
            HANDLE lClipBrdData = NULL;
            if( ( lClipBrdData = ::GetClipboardData( CF_UNICODETEXT ) ) != NULL )
            {
                LPWSTR lpClipBrdText = ( LPWSTR )::GlobalLock( lClipBrdData );
                if (lpClipBrdText)
                {
                    lClipboardText = lpClipBrdText;
                    ::GlobalUnlock( lClipBrdData );
                }
            }
            VERIFY( ::CloseClipboard() );
        }
    }
    else if( ::IsClipboardFormatAvailable( CF_TEXT ) )
    {
        // open the clipboard to get clipboard text
        if( ::OpenClipboard( m_hWnd ) )
        {
            HANDLE lClipBrdData = NULL;
            if( ( lClipBrdData = ::GetClipboardData( CF_TEXT ) ) != NULL )
            {
                LPCSTR lpClipBrdText = ( LPCSTR )::GlobalLock( lClipBrdData );
                if (lpClipBrdText)
                {
                    PvString lTextToConvert( lpClipBrdText );
                    lClipboardText = lTextToConvert.GetUnicode();
                    ::GlobalUnlock( lClipBrdData );
                }
            }
            VERIFY( ::CloseClipboard() );
        }
    }
    
    int i = 0;
    if( lClipboardText.Find( L"0x" ) == 0 
        || lClipboardText.Find( L"0X" ) == 0 )
    {
        i += 2;
    }
    for( ; i < lClipboardText.GetLength(); i++ )
    {
        if( wcschr( L"0123456789AaBbCcDdEeFf", lClipboardText[ i ] ) != NULL )
        {
            lText.Insert( lStartChar, lClipboardText[ i ] );
            lStartChar++;
        }
    }

    if( wcschr( L"AaBbCcDdEeFf", lClipboardText[ i ] ) != NULL )
    {
        if( lText.Find( L"0x" ) < 0 
            && lText.Find( L"0X" ) < 0)
        {
            if( lText[ 0 ] == L'x' || lText[ 0 ] == L'X' )
            {
                lText.Insert( 0, L"0" );
                lStartChar++;
            }
            else
            {
                lText.Insert( 0, L"0x" );
                lStartChar += 2;
            }
        }
    }
    SetWindowText( lText );
    SetSel( lStartChar, lStartChar );
    
    return 0;
}


///
/// \brief OnChar handler, only allow valid characters
///

void CHexEdit::OnChar(UINT aChar, UINT aRepCnt, UINT aFlags )
{    
    CString lText;
    
    if( GetKeyState( VK_CONTROL ) & 0x80000000 ) 
    {
        switch( aChar ) 
        {
            case 0x03:         
                Copy();
                return;
            case 0x16:
                Paste();
                return;
            case 0x18:
                Cut();
                return;
            case 0x1a:
                Undo();
                return;
        }
    }
    else
    {
        switch( aChar )
        {
            case _T('\b'):   
            case 10:   
            case 13:      
                CEdit::OnChar( aChar, aRepCnt, aFlags );
                break;
            default:

                if( wcschr( L"AaBbCcDdEeFfhHxX", aChar ) != NULL )
                {
                    CString lText;
                    GetWindowText( lText );
                    if( lText.Find( L"0x" ) < 0 
                        && lText.Find( L"0X" ) < 0)
                    {
                        int lStartChar;
                        int lEndChar;

                        GetSel( lStartChar, lEndChar );
                        SetWindowText( L"0x" + lText );
                        SetSel( lEndChar + 2, lEndChar + 2 );
                    }
                }

                if( wcschr( L"0123456789AaBbCcDdEeFf", aChar ) != NULL )
                {
                    CEdit::OnChar( aChar, aRepCnt, aFlags );
                }
                break;
        }
    }
}

