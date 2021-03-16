// *****************************************************************************
//
//     Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "BitmapButton.h"

#include <vector>
#include <map>

class NetCommandDoc;


class ProjectViewToolBar : public CMFCToolBar
{
    virtual void OnUpdateCmdUI( CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler )
    {
        CMFCToolBar::OnUpdateCmdUI( (CFrameWnd*) GetOwner(), bDisableIfNoHndler );
    }

    virtual BOOL AllowShowOnList() const { return FALSE; }
};


class ProjectPane : public CDockablePane
{
public:

    ProjectPane();
    virtual ~ProjectPane();

    void AdjustLayout();
    void OnChangeVisualStyle();

    void Update( std::vector<NetCommandDoc *> *aDocuments );
    void SelectDocument( NetCommandDoc *aDocument );

protected:

    afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
    afx_msg void OnSize( UINT nType, int cx, int cy );
    afx_msg void OnContextMenu( CWnd* pWnd, CPoint point );
    afx_msg void OnPaint();
    afx_msg void OnSetFocus( CWnd* pOldWnd );
    afx_msg void OnTvnSelchangedTree( NMHDR *pNMHDR, LRESULT *pResult );
    afx_msg void OnTvnDblclkTree( NMHDR *pNMHDR, LRESULT *pResult );
    afx_msg void OnAddDevice();
    afx_msg void OnRemoveDevice();
    afx_msg void OnUpdateRemoveDevice( CCmdUI *pCmdUI );
    DECLARE_MESSAGE_MAP()

    void EnableInterface();

    void FillProjectView();
    void DeleteChildren( HTREEITEM aNode );

    CTreeCtrl mFileView;
    CImageList mFileViewImages;
    ProjectViewToolBar mToolBar;

    std::map<NetCommandDoc *, HTREEITEM> mDocumentMap;

    HTREEITEM mDevices;
    HTREEITEM mSoftware;
    HTREEITEM mPeripherals;
    HTREEITEM mReceivers;
    HTREEITEM mTransceivers;
    HTREEITEM mTransmitters;

};

