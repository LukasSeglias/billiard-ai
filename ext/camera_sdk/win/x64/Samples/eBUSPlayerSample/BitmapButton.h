// *****************************************************************************
//
//     Copyright (c) 2008, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once


class BitmapButton : public CButton
{
public:

    BitmapButton();
    virtual ~BitmapButton();

    void SetBitmap( int aResourceID );

protected :

    void DrawItem(LPDRAWITEMSTRUCT lpDIS);
    DECLARE_MESSAGE_MAP()

private:

    CBitmap mBitmap;
    CSize mBitmapSize;
};



