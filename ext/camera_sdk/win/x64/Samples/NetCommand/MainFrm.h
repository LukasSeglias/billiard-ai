// *****************************************************************************
//
//     Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "ProjectPane.h"
#include "ControlPane.h"
#include "AttributesPane.h"
#include "PropertiesWnd.h"

#include <vector>


class MainFrame : public CMDIFrameWndEx
{
    DECLARE_DYNAMIC( MainFrame )

public:

    MainFrame();
    virtual ~MainFrame();

    virtual BOOL PreCreateWindow( CREATESTRUCT& cs );
    virtual BOOL LoadFrame( UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL );

protected:

    afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
    afx_msg void OnWindowManager();
    afx_msg void OnViewCustomize();
    afx_msg LRESULT OnToolbarCreateNew( WPARAM wp, LPARAM lp );
    afx_msg LRESULT OnNewDevice( WPARAM wp, LPARAM lp );
    afx_msg LRESULT OnRemoveDevice( WPARAM wp, LPARAM lp );
    afx_msg void OnViewProject();
    afx_msg void OnViewControls();
    afx_msg void OnViewAttributes();
    afx_msg void OnViewProperties();
    afx_msg LRESULT OnActiveViewChanged( WPARAM wp, LPARAM lp );
    afx_msg LRESULT OnNewDocument( WPARAM wp, LPARAM lp );
    afx_msg LRESULT OnDocumentClosing( WPARAM wp, LPARAM lp );
    afx_msg LRESULT OnDocumentOpenView( WPARAM wp, LPARAM lp );
    afx_msg LRESULT OnLinkDisconnected( WPARAM wp, LPARAM lp );
    afx_msg void OnClose();
    afx_msg void OnDefaultCommunicationParameters();
    DECLARE_MESSAGE_MAP()

    CMFCMenuBar mMenuBar;
    CMFCStatusBar mStatusBar;
    CMFCToolBarImages mUserImages;
    ProjectPane mProjectPane;
    ControlPane mControlPane;
    AttributesPane mAttributesPane;
    CPropertiesWnd mProperties;

    std::vector<NetCommandDoc *> mDocuments;

    BOOL CreateDockingWindows();
    void SetDockingWindowIcons( BOOL bHiColorIcons );

private:

};


