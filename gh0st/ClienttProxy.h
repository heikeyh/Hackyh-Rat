#if !defined(AFX_CLIENTTPROXY_H__E119033E_E631_4622_AFBD_1A40C91F67EC__INCLUDED_)
#define AFX_CLIENTTPROXY_H__E119033E_E631_4622_AFBD_1A40C91F67EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ClienttProxy.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CClienttProxy dialog

class CClienttProxy : public CDialog
{
// Construction
public:
	CClienttProxy(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CClienttProxy)
	enum { IDD = IDD_PROXY };
	BOOL	m_socks5;
	CString	m_proxy_user;
	CString	m_proxy_port;
	CString	m_proxy_pass;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClienttProxy)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CClienttProxy)
	afx_msg void OnSocks5();
	virtual BOOL OnInitDialog();
	afx_msg void OnExitProxy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CIOCPServer* m_iocpServer;	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLIENTTPROXY_H__E119033E_E631_4622_AFBD_1A40C91F67EC__INCLUDED_)
