// ServersManagement.h: interface for the ServersManagement class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVERSMANAGEMENT_H__B3C32C81_D091_4EBC_A61C_DB5EF4453B77__INCLUDED_)
#define AFX_SERVERSMANAGEMENT_H__B3C32C81_D091_4EBC_A61C_DB5EF4453B77__INCLUDED_

#include "Registry.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


struct ServersMSG{                  //描述服务的消息
        BYTE Sername[MAX_PATH/4];   //服务名
		BYTE Serdes[MAX_PATH/2];    //显示的服务
		BYTE serdis[MAX_PATH];      //服务描述
	    BYTE Serpath[MAX_PATH];     //服务路径
		BOOL  isrun;                //是否在运行  1停止,2运行,3暂停,4其他 
        BOOL   fanshi;              //启动方式     1自动,2禁止,3引导,4手动,5系统,6未知
		BOOL  sys;                  //登陆方式       1 本地系统 2  网络系统
};

class ServersManagement  
{
public:
	bool DelSer(char* name);
	bool StopSrv(char* name);
	bool StartSrv(char *name);
	bool SetOpt(char* name,BOOL how);
	BYTE* GetServices(DWORD& count);
	ServersManagement();
	BOOL DelSrv(char* ServerName);
	virtual ~ServersManagement();

protected:
	void GetStartType(ServersMSG& sermsg);
};

#endif // !defined(AFX_SERVERSMANAGEMENT_H__B3C32C81_D091_4EBC_A61C_DB5EF4453B77__INCLUDED_)
