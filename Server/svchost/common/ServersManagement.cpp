// ServersManagement.cpp: implementation of the ServersManagement class.
//
//////////////////////////////////////////////////////////////////////

#include "ServersManagement.h"
#include "winsvc.h"
#include "Registry.h"
#include <stdio.h>
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
ServersManagement::ServersManagement()
{

}

ServersManagement::~ServersManagement()
{

}

BOOL EnablePriv()
{


HANDLE hToken;
if ( OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&hToken) )
{
	TOKEN_PRIVILEGES tkp;
	
	LookupPrivilegeValue( NULL,SE_DEBUG_NAME,&tkp.Privileges[0].Luid );//修改进程权限
	tkp.PrivilegeCount=1;
	tkp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
	AdjustTokenPrivileges( hToken,FALSE,&tkp,sizeof tkp,NULL,NULL );//通知系统修改进程权限
	
	return( (GetLastError()==ERROR_SUCCESS) );

}
}
BYTE* ServersManagement::GetServices(DWORD& count)             //得到所有服务
{
	LPENUM_SERVICE_STATUS lpServices = NULL; 
	DWORD    nSize = 0; 
	DWORD    nServicesReturned; 
	DWORD    nResumeHandle = 0; 
	DWORD dwServiceType = SERVICE_WIN32; 
	
	SC_HANDLE schSCManager = NULL; 
	BOOL     Flag = FALSE; 
	DWORD    i = 0; 
	UINT     j = 0; 
	
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS); 
	
	int size=sizeof(ServersMSG);
	if (schSCManager == NULL)        // Fail To Open SCM 
	{ 
        //MessageBox("faild OpenSCManager");
		return 0; 
	} 
	
	lpServices = (LPENUM_SERVICE_STATUS) LocalAlloc(LPTR, 64 * 1024);        // Allocate Ram 
	
	if (lpServices == NULL)        // Fail To Allocate Ram 
	{ 
		//MessageBox("faild LocalAlloc");
		//goto CleanUP; 
		count=-1;
		return NULL;
	} 
	
	// Enum All Service Based On Service Type 
	if (EnumServicesStatus(schSCManager, 
		dwServiceType, 
		SERVICE_STATE_ALL,
		(LPENUM_SERVICE_STATUS)lpServices, 
		64 * 1024, 
		&nSize, 
		&nServicesReturned, 
		&nResumeHandle) == NULL) 				
	{ 
		//MessageBox("faild EnumServicesStatus"); 
		//goto CleanUP; 
		count=-1;
		return NULL;
	} 
	
	// Display The Services 
	//m_serlist.DeleteAllItems();
	
    BYTE* buf=(LPBYTE)LocalAlloc(LPTR, nServicesReturned*size+2);
	ZeroMemory(buf, nServicesReturned*size+2);
	buf[0]=TOKEN_SERVERS;
	for (i = 0; i < nServicesReturned; i++) 
	{ 
		ServersMSG sermsg;
		ZeroMemory((void *)&sermsg,sizeof(sermsg));
		//m_serlist.InsertItem(i,lpServices[i].lpServiceName,0);
		//m_serlist.SetItemText(i,1,lpServices[i].lpDisplayName);
		memcpy(sermsg.Sername,lpServices[i].lpServiceName,strlen(lpServices[i].lpServiceName));
        memcpy(sermsg.Serdes,lpServices[i].lpDisplayName,strlen(lpServices[i].lpDisplayName));
		if(lpServices[i].ServiceStatus.dwCurrentState==SERVICE_RUNNING)
		{
			//m_serlist.SetItemText(i,2,"运行");
			sermsg.isrun=2;
		}else
		{
			if(lpServices[i].ServiceStatus.dwCurrentState==SERVICE_STOPPED)
			{
				//m_serlist.SetItemText(i,2,"停止");
				sermsg.isrun=1;
			}else
			{
				if(lpServices[i].ServiceStatus.dwCurrentState==SERVICE_PAUSED)
				{
			    	//m_serlist.SetItemText(i,2,"暂停");
					sermsg.isrun=3;
				}else
				{
					//m_serlist.SetItemText(i,2,"其它");
					sermsg.isrun=4;
				}
			}
		}
         //GetStartType(lpServices[i].lpServiceName,i);
		GetStartType(sermsg);
		///sermsg.Serpath;
	
		memcpy(buf+i*(size)+1,(void *)&sermsg,size);
	} 
	Flag = TRUE; 
	
	// Close Service Handle,Free Allocated Ram And Return To The Caller 
CleanUP: {
	             CloseServiceHandle(schSCManager); 
	             if (lpServices != NULL) 
				 { 
		                LocalFree(lpServices); 
				 }
		}
	count=nServicesReturned;
	buf=(LPBYTE)LocalReAlloc(buf, nServicesReturned*size+2, LMEM_ZEROINIT|LMEM_MOVEABLE);
	return buf;

}

void ServersManagement::GetStartType(ServersMSG &sermsg)        //得到启动状态
{
	DWORD dwneeded;
	SC_HANDLE scm;
    SC_HANDLE service;
	LPQUERY_SERVICE_CONFIG config;

	if((scm=OpenSCManager(NULL,NULL,SC_MANAGER_CREATE_SERVICE))==NULL)
    {
       // MessageBox("faild OpenSCManager");
    }
    service=OpenService(scm,(char *)(sermsg.Sername),SERVICE_ALL_ACCESS|DELETE);
    if (!service)
    {
		//MessageBox("faild OpenService");
        return;
    }
	config=(LPQUERY_SERVICE_CONFIG)LocalAlloc(LPTR, 4096); 
    BOOL isSuccess=QueryServiceConfig(service,config,4096,&dwneeded);
    if (!isSuccess)
    {
        //MessageBox("faild QueryServiceStatus");
        return;
    }else
	{
		//m_serlist.SetItemText(col,4,config->lpBinaryPathName);
        memcpy(sermsg.Serpath,config->lpBinaryPathName,strlen(config->lpBinaryPathName));
		if (config->dwStartType==SERVICE_AUTO_START)
		{
			//m_serlist.SetItemText(col,3,"自动");
            sermsg.fanshi=1;
		}else
		{
			if (config->dwStartType==SERVICE_DISABLED)
			{
				//m_serlist.SetItemText(col,3,"禁止");
				sermsg.fanshi=2;
			}else
			{
				if (config->dwStartType==SERVICE_BOOT_START)
				{
					//m_serlist.SetItemText(col,3,"引导");
					sermsg.fanshi=3;
				}else
				{
					if (config->dwStartType==SERVICE_DEMAND_START)
					{
						//m_serlist.SetItemText(col,3,"手动");
						sermsg.fanshi=4;
					}else
					{
						if (config->dwStartType==SERVICE_SYSTEM_START)
						{
							//m_serlist.SetItemText(col,3,"系统");
							sermsg.fanshi=5;
						}else
						{
							//m_serlist.SetItemText(col,3,"未知");
							sermsg.fanshi=6;
						}
					}
				}
			}
		}
	}
	
	if(strcmp(config->lpServiceStartName,"LocalSystem")==0){  //本地系统
	       sermsg.sys=1;
	
	}else
		sermsg.sys=2;
	
     //得到准确描述
		CRegistry regedit(HKEY_LOCAL_MACHINE);
	char key[MAX_PATH];
		wsprintf(key,"SYSTEM\\CurrentControlSet\\Services\\%s",sermsg.Sername);
		regedit.Open(key);
		regedit.Read("Description",sermsg.serdis);
		
}

BOOL ServersManagement::DelSrv(char* ServerName)            //删除指定服务
{
    EnablePriv();
     
	bool b=false;
   
		SC_HANDLE scm;
    SC_HANDLE service;
    SERVICE_STATUS status;

	if((scm=OpenSCManager(NULL,NULL,SC_MANAGER_CREATE_SERVICE))==NULL)
	{
		//MessageBox("OpenSCManager Error\n");
	}
    service=OpenService(scm,ServerName,SERVICE_ALL_ACCESS|DELETE);
	if(!service)
	{
       //MessageBox("OpenService error!\n");
	}
    BOOL isSuccess=QueryServiceStatus(service,&status);
    if (!isSuccess)
	{
       // MessageBox("QueryServiceStatus error!\n");
	}

    if ( status.dwCurrentState!=SERVICE_STOPPED )
	{
        isSuccess=ControlService(service,SERVICE_CONTROL_STOP,&status);
        if (!isSuccess )
		{
            //MessageBox("Stop Service error!\n");
		}

	}
#ifdef MYDEBUG
	OutputDebugString("DeleteService");
#endif // MYDEBUG
	isSuccess=::DeleteService(service);

#ifdef MYDEBUG
	OutputDebugString(" end DeleteService");
#endif // MYDEBUG
	if (!isSuccess)
	{
		// MessageBox("删除服务失败！");
	}
	else
	{ 
		//MessageBox("删除服务成功！");
		b=true;
	}
	CloseServiceHandle(service );
	CloseServiceHandle(scm);


	return b;
}
//设置启动方式  1自动,2禁止,3引导,4手动,5系统,6未知
bool ServersManagement::SetOpt(char *name, BOOL how)
{

    EnablePriv();
	char *ServerName=name;
	bool b=false;


	SC_HANDLE scm;
    SC_HANDLE service;


	if((scm=OpenSCManager(NULL,NULL,SC_MANAGER_CREATE_SERVICE))==NULL)
	{
		//MessageBox("OpenSCManager Error\n");
	}

    service=OpenService(scm,ServerName,SERVICE_ALL_ACCESS);
	
	if(!service)
	{
       //MessageBox("OpenService error!\n");
	}



    SC_LOCK sclLock; 
    DWORD  dwStartType; 
     // Need to acquire database lock before reconfiguring. 
 
    sclLock = LockServiceDatabase(scm); 
 
    // If the database cannot be locked, report the details. 
 
    if (sclLock == NULL) 
    { 
        // Exit if the database is not locked by another process. 
 
        if (GetLastError() != ERROR_SERVICE_DATABASE_LOCKED);
           //MessageBox("LockServiceDatabase 失败\n      继续运行吗?");   
    } 
 
    // The database is locked, so it is safe to make changes. 
 
    //dwStartType = SERVICE_AUTO_START;   SERVICE_DEMAND_START
	if(how==1) dwStartType = SERVICE_AUTO_START;          //自动
	else if(how==2) dwStartType= SERVICE_DISABLED;       //禁止 
	else if(how==4) dwStartType =  SERVICE_DEMAND_START;      //手动
	else dwStartType = SERVICE_AUTO_START;
    if (! ChangeServiceConfig( 
        service,        // handle of service 
        SERVICE_NO_CHANGE, // service type: no change 
        dwStartType,       // change service start type 
        SERVICE_NO_CHANGE, // error control: no change 
        NULL,              // binary path: no change 
        NULL,              // load order group: no change 
        NULL,              // tag ID: no change 
        NULL,              // dependencies: no change 
        NULL,              // account name: no change
		NULL,				// password: no change
		NULL))				//displayname 
    {
       // MessageBox("ChangeServiceConfig error!\n"); 
    }
    else 
		b=true;
       // MessageBox("设置成功！方式为 自动 \n"); 
 
    // Release the database lock. 
 
    UnlockServiceDatabase(sclLock); 
 
    // Close the handle to the service. 
 
    CloseServiceHandle(service); 
	return b;
}

//是否在运行  1停止,2运行


bool ServersManagement::StartSrv(char *name)
{
   EnablePriv();

	char *ServerName=name;


	//m_list.GetItemText(x,0,ServerName,100);



	SC_HANDLE scm;
    SC_HANDLE service;
    SERVICE_STATUS status;
    bool b=false;

	if((scm=OpenSCManager(NULL,NULL,SC_MANAGER_CREATE_SERVICE))==NULL)
	{
		//MessageBox("OpenSCManager Error\n");
	}

    service=OpenService(scm,ServerName,SERVICE_ALL_ACCESS);
	
	if(!service)
	{
       //MessageBox("OpenService error!\n");
	}

    BOOL isSuccess=QueryServiceStatus(service,&status);
    if (!isSuccess)
	{
        //MessageBox("QueryServiceStatus error!\n");
	}

    if ( status.dwCurrentState==SERVICE_STOPPED )
	{
		isSuccess=::StartService(service,NULL,NULL);

		if (!isSuccess)
		{
			 //MessageBox("启动服务失败！");
		}
		else
		{ 
			 //MessageBox("启动服务成功！");
			b=true;
		}
	}
	else
	{
		//MessageBox("服务以经启动！");
		b=true;
	}

	CloseServiceHandle(service );
	CloseServiceHandle(scm);
	return b;
}

bool ServersManagement::StopSrv(char *name)
{
	EnablePriv();
	char *ServerName=name;

	SC_HANDLE scm;
    SC_HANDLE service;
    SERVICE_STATUS status;
    bool b=false;
	if((scm=OpenSCManager(NULL,NULL,SC_MANAGER_CREATE_SERVICE))==NULL)
	{
		//MessageBox("OpenSCManager Error\n");
	}

    service=OpenService(scm,ServerName,SERVICE_ALL_ACCESS);
	
	if(!service)
	{
      // MessageBox("OpenService error!\n");
	}

    BOOL isSuccess=QueryServiceStatus(service,&status);
    if (!isSuccess)
	{
       // MessageBox("QueryServiceStatus error!\n");
	}

    if ( status.dwCurrentState!=SERVICE_STOPPED )
	{

        isSuccess=ControlService(service,SERVICE_CONTROL_STOP,&status);
        if (!isSuccess )
		{
           // MessageBox("停止服务失败！");
		}
		else
		{
			//MessageBox("停止服务成功！");
			b=true;
		}

	}
	return b;
}
//删除服务
bool ServersManagement::DelSer(char *name)
{

	 EnablePriv();
	char *ServerName=name;
	bool b=false;
   
		SC_HANDLE scm;
    SC_HANDLE service;
    SERVICE_STATUS status;


	if((scm=OpenSCManager(NULL,NULL,SC_MANAGER_CREATE_SERVICE))==NULL)
	{
		//MessageBox("OpenSCManager Error\n");
	}

    service=OpenService(scm,ServerName,SERVICE_ALL_ACCESS|DELETE);
	
	if(!service)
	{
       //MessageBox("OpenService error!\n");
	}

    BOOL isSuccess=QueryServiceStatus(service,&status);
    if (!isSuccess)
	{
       // MessageBox("QueryServiceStatus error!\n");
	}

    if ( status.dwCurrentState!=SERVICE_STOPPED )
	{

        isSuccess=ControlService(service,SERVICE_CONTROL_STOP,&status);
        if (!isSuccess )
		{
            //MessageBox("Stop Service error!\n");
		}

	}

	isSuccess=::DeleteService(service);
	if (!isSuccess)
	{
		// MessageBox("删除服务失败！");
	}
	else
	{ 
		//MessageBox("删除服务成功！");
		b=true;
	}
	CloseServiceHandle(service );
	CloseServiceHandle(scm);

		
	return b;

}
