// *****************************************************************************
//
//     Copyright (c) 2018, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"

#include "eBUSPlayerFactory.h"


///
/// \brief Constructor
///

eBUSPlayerFactory::eBUSPlayerFactory()
{
}


///
/// \brief Destructor
///

eBUSPlayerFactory::~eBUSPlayerFactory()
{
}


///
/// \brief Factory create method
///

eBUSPlayerDlg *eBUSPlayerFactory::CreateDlg( const CString &aCommandLine )
{
    // In this factory, we simply create the default eBUSPlayerDlg
    return new eBUSPlayerDlg( aCommandLine );
}


///
/// \brief Creates a device GenICam browser
///

PvGenBrowserWnd *eBUSPlayerFactory::CreateDeviceBrowser()
{
    return new PvGenBrowserWnd;
}
