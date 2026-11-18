#if !defined(AFX_SENDMSG_H__F04AF202_C8A9_4705_91ED_A766C2C0E94B__INCLUDED_)
#define AFX_SENDMSG_H__F04AF202_C8A9_4705_91ED_A766C2C0E94B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SendMsg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSendMsg dialog

class CSendMsg : public CDialog
{
// Construction
public:
	CSendMsg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSendMsg)
	enum { IDD = IDD_MSG };
	CComboBox	m_msg;
	CString	m_strTitle;
	int		m_counts;
	CString	m_edit_msg;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSendMsg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSendMsg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSendMsg();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SENDMSG_H__F04AF202_C8A9_4705_91ED_A766C2C0E94B__INCLUDED_)
