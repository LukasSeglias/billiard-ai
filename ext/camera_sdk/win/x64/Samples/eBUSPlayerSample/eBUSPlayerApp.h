// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "resource.h"


class eBUSPlayerApp : public CWinApp
{
public:

    eBUSPlayerApp();

    virtual BOOL InitInstance();

    DECLARE_MESSAGE_MAP()

};


extern eBUSPlayerApp theApp;


