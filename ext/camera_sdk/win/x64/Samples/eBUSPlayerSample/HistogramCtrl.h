// *****************************************************************************
//
//     Copyright (c) 2014, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "afxwin.h"

class PvRangeFilter;


class HistogramCtrl : public CDialog
{
public:

	HistogramCtrl( PvRangeFilter *aRangeFilter, CWnd *pParent = NULL );
	virtual ~HistogramCtrl();

	enum { IDD = IDD_HISTOGRAMCTRL };

protected:

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
    afx_msg void OnTimer( UINT_PTR nIDEvent );
    DECLARE_MESSAGE_MAP()

private:

	PvRangeFilter *mRangeFilter;
    UINT_PTR mTimer;

};


