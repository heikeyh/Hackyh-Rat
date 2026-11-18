// ChangeGroup.cpp : implementation file
//

#include "stdafx.h"
#include "gh0st.h"
#include "ChangeGroup.h"
#include "gh0stView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CGh0stView *g_pConnectView;
/////////////////////////////////////////////////////////////////////////////
// CChangeGroup dialog


CChangeGroup::CChangeGroup(CWnd* pParent /*=NULL*/)
	: CDialog(CChangeGroup::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChangeGroup)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CChangeGroup::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChangeGroup)
	DDX_Control(pDX, IDC_COMBO_GROUP, m_combo_group);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChangeGroup, CDialog)
	//{{AFX_MSG_MAP(CChangeGroup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChangeGroup message handlers

BOOL CChangeGroup::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	CString strGroupName;
	int nGroup = g_pConnectView->m_wndReport.GetRecords()->GetCount();
	for (int i = 0; i < nGroup; i++)
	{	
		m_combo_group.AddString(g_pConnectView->m_wndReport.GetRecords()->GetAt(i)->GetItem(0)->GetTooltip());
	}
	m_combo_group.SetCurSel(0);
	//	strGroupName.Format("%d", nGroup);
	//	AfxMessageBox(strGroupName);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CChangeGroup::OnOK() 
{
	// TODO: Add extra validation here
	((CEdit*)GetDlgItem(IDC_COMBO_GROUP))->GetWindowText(strGroup);
	if (strGroup == "" )
	{
		AfxMessageBox("输入不能为空!");
		return;
	}
	CDialog::OnOK();
}

BOOL CChangeGroup::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
			return true;
		if (pMsg->wParam == VK_RETURN)
		{
			OnOK();
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}
