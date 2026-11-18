// LoginDlg.cpp : implementation file
//

#include "stdafx.h"
#include "gh0st.h"
#include "LoginDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CLoginDlg dialog





CLoginDlg::CLoginDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLoginDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLoginDlg)
	m_LoginUser = ((CGh0stApp *)AfxGetApp())->m_IniFile.GetString(_T("Login"), _T("User"), _T("请输入论坛uid"));
 //   m_LoginPass = ((CGh0stApp *)AfxGetApp())->m_IniFile.GetString(_T("Login"), _T("Pass"), _T(""));  不自动输入密码  、还有下面一个
	//}}AFX_DATA_INIT
}


void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLoginDlg)
	DDX_Text(pDX, IDC_LOGIN_USER, m_LoginUser);
	DDX_Text(pDX, IDC_LOGIN_PASS, m_LoginPass);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLoginDlg, CDialog)
	//{{AFX_MSG_MAP(CLoginDlg)
	ON_BN_CLICKED(IDC_LOGIN, OnLogin)
	ON_BN_CLICKED(IDC_EXIT_LOGIN, OnExitLogin)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLoginDlg message handlers
BOOL CLoginDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLoginDlg::OnLogin() 
{
	// 本地化：跳过本地验证服务器，直接进入主控端
	dwLoginCount = GetTickCount();
	OnExitLogin();
}

void CLoginDlg::OnExitLogin() 
{
	// TODO: Add your control notification handler code here
	HWND King = ::FindWindow(NULL,_T("Hackyh Rat 2026 V1.0 个人版-[https://www.hackyh.com]"));   //修改标题了 要修改这里
	::PostMessage(King,WM_CLOSE,1,0);
}


BOOL CLoginDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message   ==   WM_KEYDOWN)   
	{   
		switch(pMsg->wParam)   
		{   
		case   VK_RETURN://屏蔽回车        
			return   TRUE;   
		case   VK_ESCAPE://屏蔽Esc   
			return   TRUE;   
		}   
	}  	
	return CDialog::PreTranslateMessage(pMsg);
}
