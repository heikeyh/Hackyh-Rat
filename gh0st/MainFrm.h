// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__E257EA63_31AA_4BD6_BAD4_81DF13E78D5A__INCLUDED_)
#define AFX_MAINFRM_H__E257EA63_31AA_4BD6_BAD4_81DF13E78D5A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TrayIcon.h"

class CMainFrame : public CXTPFrameWnd
{

protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:
	UINT nCount;
	int nXP,nWin7,nWin8,n2012,nWin10,nWin11,n2016,n2019,n2022,n2025,nOther;
	void UpdateOS(int m_XP,int m_Win7,int m_Win8,int m_2012,int m_Win10,int m_Win11,int m_2016,int m_2019,int m_2022,int m_2025,int m_Other);
	void ShowNumber();
	static void ProcessReceiveComplete(ClientContext *pContext);
	static void ProcessReceive(ClientContext *pContext);
	void Activate(UINT nPort, UINT nMaxConnections);
	static void CALLBACK NotifyProc(LPVOID lpParam, ClientContext* pContext, UINT nCode);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CXTPStatusBar  m_wndStatusBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnExit();
	afx_msg void OnSetting();
	afx_msg void OnBuild();
	//}}AFX_MSG
	afx_msg void OnCustomize();
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnDockingPaneNotify(WPARAM wParam, LPARAM lParam);
private:
	CTrayIcon m_TrayIcon;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__E257EA63_31AA_4BD6_BAD4_81DF13E78D5A__INCLUDED_)
