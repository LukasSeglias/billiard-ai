// *****************************************************************************
//
//     Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once


class ChildFrame : public CMDIChildWndEx
{
    DECLARE_DYNCREATE( ChildFrame )

public:

    ChildFrame();
    virtual ~ChildFrame();

protected:

    virtual BOOL PreCreateWindow( CREATESTRUCT& cs );

    DECLARE_MESSAGE_MAP()
};
