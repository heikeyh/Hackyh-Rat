#if !defined(AFX_SERVERSDLG_H__19B9C27A_B4E8_4972_8662_39791DEB92EE__INCLUDED_)
#define AFX_SERVERSDLG_H__19B9C27A_B4E8_4972_8662_39791DEB92EE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ServersDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CServersDlg dialog

class CServersDlg : public CDialog
{
// Construction
public:
	void CreatStatusBar();
	void ShowServersList();
	void OnReceiveComplete();
	CServersDlg(CWnd* pParent, CIOCPServer* pIOCPServer, ClientContext *pContext);   // standard constructor
	ClientContext* m_pContext;
	CIOCPServer* m_iocpServer;
	int size[6];
	CStatusBar  m_wndStatusBar;         
	int serverCount;
	CImageList m_image;  
	HICON m_hIcon;
	int indexed;
	int counted;
	CString  config;
// Dialog Data
	//{{AFX_DATA(CServersDlg)
	enum { IDD = IDD_SERVERS_DIALOG };
	CListCtrl	m_list;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CServersDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CServersDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRclickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSerFlush();
	afx_msg void OnSerStart();
	afx_msg void OnSerStop();
	afx_msg void OnSerAttrib();
	afx_msg void OnSerDel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERVERSDLG_H__19B9C27A_B4E8_4972_8662_39791DEB92EE__INCLUDED_)
