// *****************************************************************************
//
// Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// -----------------------------------------------------------------------------
//
// Child frame, the frame that wraps the document view used in the MDI 
// interface.
// 
// *****************************************************************************

#include "stdafx.h"
#include "NetCommand.h"

#include "ChildFrm.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE( ChildFrame, CMDIChildWndEx )

BEGIN_MESSAGE_MAP( ChildFrame, CMDIChildWndEx )
END_MESSAGE_MAP()


// ==========================================================================
ChildFrame::ChildFrame()
{
}

// ==========================================================================
ChildFrame::~ChildFrame()
{
}

// ==========================================================================
BOOL ChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    // TODO: Modify the Window class or styles here by modifying the CREATESTRUCT cs
    if( !CMDIChildWndEx::PreCreateWindow( cs ) )
    {
        return FALSE;
    }

    return TRUE;
}

