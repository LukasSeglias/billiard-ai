// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#ifndef __AFXWIN_H__
    #error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// SimpleGUIApplicationApp:
// See SimpleGUIApplication.cpp for the implementation of this class
//

class SimpleGUIApplicationApp : public CWinApp
{
public:
    SimpleGUIApplicationApp();

// Overrides
    public:
    virtual BOOL InitInstance();

// Implementation

    DECLARE_MESSAGE_MAP()
};

extern SimpleGUIApplicationApp theApp;