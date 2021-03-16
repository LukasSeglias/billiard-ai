// *****************************************************************************
//
//     Copyright (c) 2018, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "eBUSPlayerDlg.h"


class eBUSPlayerFactory
{
public:

    eBUSPlayerFactory();
    virtual ~eBUSPlayerFactory();

    eBUSPlayerDlg *CreateDlg( const CString &aCommandLine );
    PvGenBrowserWnd *CreateDeviceBrowser();

};
