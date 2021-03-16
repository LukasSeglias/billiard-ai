// *****************************************************************************
//
//     Copyright (c) 2016, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once


// Specialized CEdit to handle dec and hex
class CHexEdit : public CEdit
{
public:

    CHexEdit() {}
    virtual ~CHexEdit() {}

    void SetValue( UINT32 aValue, bool aHex );
    void SetValue( UINT64 aValue, bool aHex );

    UINT32 GetValueUInt32();
    UINT64 GetValueUInt64();

    void Cut();

protected:

    DECLARE_MESSAGE_MAP()
    afx_msg void OnChar( UINT aChar, UINT aRepCnt, UINT aFlags );
    afx_msg LRESULT OnPaste( WPARAM awParam, LPARAM alParam );
    afx_msg void OnKillFocus( CWnd* pcNewReceiver );

    void ValidateValue();

};
