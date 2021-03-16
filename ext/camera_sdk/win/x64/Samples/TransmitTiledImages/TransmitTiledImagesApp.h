// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#ifndef __AFXWIN_H__
    #error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CTransmitTiledImagesApp:
// See TransmitTiledImages.cpp for the implementation of this class
//
class CTransmitTiledImagesApp : public CWinApp
{
public:
    CTransmitTiledImagesApp();

// Overrides
    public:
    virtual BOOL InitInstance();

// Implementation

    DECLARE_MESSAGE_MAP()
};

extern CTransmitTiledImagesApp theApp;