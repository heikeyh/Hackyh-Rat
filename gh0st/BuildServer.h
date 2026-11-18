#if !defined(AFX_BUILDSERVER_H__C125C3E7_59DD_4873_8E2E_CD783EDF2831__INCLUDED_)
#define AFX_BUILDSERVER_H__C125C3E7_59DD_4873_8E2E_CD783EDF2831__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BuildServer.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CBuildServer dialog

class CBuildServer : public CDialog
{
// Construction
public:
	CBuildServer(CWnd* pParent = NULL);   // standard constructor
	static	DWORD WINAPI TestMaster(LPVOID lparam);
	void AntiJinShan(CString Path);

// Dialog Data
	//{{AFX_DATA(CBuildServer)
	enum { IDD = IDD_BUILD_SERVER };
	CString	m_ServiceName;
	CString	m_ServiceDisplayName;
	CString	m_ServiceDescription;
	CString	m_remote_port;
	CString	m_remote_host;
	CString	m_down_url;
	CString	m_group_name;
	BOOL	m_bDelete;
	BOOL	m_bReg;

	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBuildServer)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBuildServer)
	afx_msg void OnTestMaster();
	afx_msg void OnEnableHttp();
	virtual BOOL OnInitDialog();
	afx_msg void OnSuiji();       //随机服务需要的
	afx_msg void OnBuild();
	afx_msg void OnExit();
	afx_msg void OnGetinfo();
	afx_msg void OnButtonTestDown();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void CompressUpx(CString inpath);
public:
	afx_msg void OnEnChangeEditUrl();
	afx_msg void OnEnChangeServiceName2();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BUILDSERVER_H__C125C3E7_59DD_4873_8E2E_CD783EDF2831__INCLUDED_)
