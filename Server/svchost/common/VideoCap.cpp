// VideoCap.cpp: implementation of the CVideoCap class.
// DirectShow 采集 (参考 借鉴视频查看 项目): ISampleGrabber 强制 RGB24 输出,
// 系统颜色转换器(Color Converter)自动将摄像头原生格式转为 RGB24。
//////////////////////////////////////////////////////////////////////
#include "VideoCap.h"
//////////////////////////////////////////////////////////////////////
// AM_MEDIA_TYPE 释放 (新版 SDK 已移除 DeleteMediaType)
//////////////////////////////////////////////////////////////////////
static void FreeAMType(AM_MEDIA_TYPE *pmt)
{
	if (pmt->cbFormat != 0) { CoTaskMemFree(pmt->pbFormat); pmt->cbFormat = 0; pmt->pbFormat = NULL; }
	if (pmt->pUnk) { pmt->pUnk->Release(); pmt->pUnk = NULL; }
}
static void DeleteAMType(AM_MEDIA_TYPE *pmt)
{
	if (pmt) { FreeAMType(pmt); CoTaskMemFree(pmt); }
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
bool CVideoCap::m_bIsConnected = false;

CVideoCap::CVideoCap()
{
	// 事件: 帧回调后置位, GetDIB 等待
	m_hCaptureEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	m_lpbmi = NULL;
	m_lpDIB = NULL;
	m_lpDIBRead = NULL;
	m_pGB = NULL;
	m_pCapture = NULL;
	m_pBF = NULL;
	m_pMC = NULL;
	m_pGrabber = NULL;
	m_pNullRenderer = NULL;
	m_bStarted = false;
	InitializeCriticalSection(&m_cs);

	if (FAILED(CoInitialize(NULL)))
		return;
	m_cb.m_pCap = this;
}

CVideoCap::~CVideoCap()
{
	StopGraph();
	if (m_lpbmi)
		delete m_lpbmi;
	if (m_lpDIB)
		delete [] m_lpDIB;
	if (m_lpDIBRead)
		delete [] m_lpDIBRead;
	CloseHandle(m_hCaptureEvent);
	DeleteCriticalSection(&m_cs);
	CoUninitialize();
	m_bIsConnected = false;
}

// SampleGrabber 帧回调: DirectShow 流线程中调用
STDMETHODIMP CWebCamGrabberCB::SampleCB(double dblSampleTime, IMediaSample *pSample)
{
	if (m_pCap == NULL || pSample == NULL)
		return 0;
	BYTE *pBuffer = NULL;
	if (FAILED(pSample->GetPointer(&pBuffer)) || pBuffer == NULL)
		return 0;
	m_pCap->OnFrame(pBuffer, pSample->GetActualDataLength());
	return 0;
}

void CVideoCap::OnFrame(BYTE *pBuffer, long lBufferSize)
{
	EnterCriticalSection(&m_cs);
	if (m_lpDIB && m_lpbmi)
	{
		DWORD dwNeed = m_lpbmi->bmiHeader.biSizeImage;
		memcpy(m_lpDIB, pBuffer, (lBufferSize < (long)dwNeed) ? (size_t)lBufferSize : dwNeed);
	}
	LeaveCriticalSection(&m_cs);
	SetEvent(m_hCaptureEvent);
}

bool CVideoCap::IsWebCam()
{
	// 枚举 DirectShow 视频采集设备是否存在
	bool bRet = false;
	CoInitialize(NULL);
	ICreateDevEnum *pCreateDevEnum = NULL;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pCreateDevEnum);
	if (SUCCEEDED(hr))
	{
		IEnumMoniker *pEm = NULL;
		hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
		// 无设备时返回 S_FALSE 且 pEm=NULL，必须同时判 S_OK 与非空，否则解引用空指针崩溃
		if (hr == S_OK && pEm != NULL)
		{
			IMoniker *pM = NULL;
			ULONG cFetched = 0;
			if (pEm->Next(1, &pM, &cFetched) == S_OK)
			{
				bRet = true;
				pM->Release();
			}
			pEm->Release();
		}
		pCreateDevEnum->Release();
	}
	CoUninitialize();
	return bRet;
}

// 按设备序号绑定 DirectShow 视频采集设备过滤器
bool CVideoCap::BindFilter(int deviceId, IBaseFilter **pFilter)
{
	if (deviceId < 0 || pFilter == NULL)
		return false;

	ICreateDevEnum *pCreateDevEnum = NULL;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pCreateDevEnum);
	if (FAILED(hr))
		return false;

	IEnumMoniker *pEm = NULL;
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
	if (FAILED(hr) || pEm == NULL)
	{
		pCreateDevEnum->Release();
		return false;
	}

	IMoniker *pM = NULL;
	ULONG cFetched = 0;
	bool bRet = false;
	int index = 0;
	while (pEm->Next(1, &pM, &cFetched) == S_OK)
	{
		if (index == deviceId)
		{
			if (SUCCEEDED(pM->BindToObject(0, 0, IID_IBaseFilter, (void **)pFilter)))
				bRet = true;
			pM->Release();
			break;
		}
		pM->Release();
		index++;
	}
	pEm->Release();
	pCreateDevEnum->Release();
	return bRet;
}

void CVideoCap::StopGraph()
{
	if (m_pMC)
		m_pMC->Stop();
	if (m_pGrabber)
		m_pGrabber->SetCallback(NULL, 0);
	if (m_pMC)		{ m_pMC->Release(); m_pMC = NULL; }
	if (m_pNullRenderer)	{ m_pNullRenderer->Release(); m_pNullRenderer = NULL; }
	if (m_pGrabber)		{ m_pGrabber->Release(); m_pGrabber = NULL; }
	if (m_pBF)		{ m_pBF->Release(); m_pBF = NULL; }
	if (m_pCapture)		{ m_pCapture->Release(); m_pCapture = NULL; }
	if (m_pGB)		{ m_pGB->Release(); m_pGB = NULL; }
	m_bStarted = false;
}

bool CVideoCap::Initialize(int nWidth, int nHeight)
{
	if (m_bIsConnected)
		return false;
	if (!IsWebCam())
		return false;

	HRESULT hr;

	// 1. 建立 FilterGraph
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&m_pGB);
	if (FAILED(hr)) goto FAIL;
	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, IID_ICaptureGraphBuilder2, (void **)&m_pCapture);
	if (FAILED(hr)) goto FAIL;
	m_pCapture->SetFiltergraph(m_pGB);
	hr = m_pGB->QueryInterface(IID_IMediaControl, (void **)&m_pMC);
	if (FAILED(hr)) goto FAIL;

	// 2. 绑定第一个视频采集设备
	if (!BindFilter(0, &m_pBF))
		goto FAIL;
	hr = m_pGB->AddFilter(m_pBF, L"Capture Filter");
	if (FAILED(hr)) goto FAIL;

	// 3. SampleGrabber + NullRenderer
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_ISampleGrabber, (void **)&m_pGrabber);
	if (FAILED(hr)) goto FAIL;
	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void **)&m_pNullRenderer);
	if (FAILED(hr)) goto FAIL;
	hr = m_pGB->AddFilter(m_pNullRenderer, L"NullRenderer");
	if (FAILED(hr)) goto FAIL;

	IBaseFilter *pGrabBase = NULL;
	hr = m_pGrabber->QueryInterface(IID_IBaseFilter, (void **)&pGrabBase);
	if (FAILED(hr)) goto FAIL;
	hr = m_pGB->AddFilter(pGrabBase, L"Grabber");
	if (FAILED(hr)) { pGrabBase->Release(); goto FAIL; }

	// 4. 强制 RGB24: 系统颜色转换器自动将摄像头原生格式(YUY2/MJPG等)转为 RGB24
	AM_MEDIA_TYPE mt;
	ZeroMemory(&mt, sizeof(mt));
	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_RGB24;
	hr = m_pGrabber->SetMediaType(&mt);
	if (FAILED(hr)) { pGrabBase->Release(); goto FAIL; }

	// 5. 尝试设置采集分辨率 (失败则保持摄像头默认分辨率)
	IAMStreamConfig *pConfig = NULL;
	hr = m_pCapture->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Interleaved, m_pBF, IID_IAMStreamConfig, (void **)&pConfig);
	if (FAILED(hr))
		hr = m_pCapture->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, m_pBF, IID_IAMStreamConfig, (void **)&pConfig);
	if (SUCCEEDED(hr) && pConfig)
	{
		AM_MEDIA_TYPE *pmt = NULL;
		if (SUCCEEDED(pConfig->GetFormat(&pmt)) && pmt && pmt->pbFormat)
		{
			if (nWidth && nHeight)
			{
				VIDEOINFOHEADER *pvih = (VIDEOINFOHEADER *)pmt->pbFormat;
				pvih->bmiHeader.biWidth = nWidth;
				pvih->bmiHeader.biHeight = nHeight;
				pvih->AvgTimePerFrame = 10000000 / 10;	// 10 fps
				pConfig->SetFormat(pmt);	// 失败则忽略, 保持默认
			}
			DeleteAMType(pmt);
		}
		pConfig->Release();
	}

	// 6. 渲染链路 (预览优先, 回退捕获)
	hr = m_pCapture->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, m_pBF, pGrabBase, m_pNullRenderer);
	if (FAILED(hr))
		hr = m_pCapture->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, m_pBF, pGrabBase, m_pNullRenderer);
	pGrabBase->Release();
	if (FAILED(hr)) goto FAIL;

	// 7. 取实际连接格式 (RGB24 + 实际分辨率)
	AM_MEDIA_TYPE mtConnected;
	ZeroMemory(&mtConnected, sizeof(mtConnected));
	hr = m_pGrabber->GetConnectedMediaType(&mtConnected);
	if (FAILED(hr) || mtConnected.formattype != FORMAT_VideoInfo || mtConnected.pbFormat == NULL)
	{
		if (mtConnected.pbFormat) FreeAMType(&mtConnected);
		goto FAIL;
	}
	VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER *)mtConnected.pbFormat;
	m_lpbmi = new BITMAPINFO;
	memset(m_lpbmi, 0, sizeof(BITMAPINFO));
	m_lpbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_lpbmi->bmiHeader.biWidth = pVih->bmiHeader.biWidth;
	m_lpbmi->bmiHeader.biHeight = pVih->bmiHeader.biHeight;
	m_lpbmi->bmiHeader.biPlanes = 1;
	m_lpbmi->bmiHeader.biBitCount = 24;
	m_lpbmi->bmiHeader.biCompression = BI_RGB;
	m_lpbmi->bmiHeader.biSizeImage = (((m_lpbmi->bmiHeader.biWidth * m_lpbmi->bmiHeader.biBitCount + 31) & ~31) >> 3) * m_lpbmi->bmiHeader.biHeight;
	FreeAMType(&mtConnected);

	m_lpDIB = new BYTE[m_lpbmi->bmiHeader.biSizeImage];
	m_lpDIBRead = new BYTE[m_lpbmi->bmiHeader.biSizeImage];
	memset(m_lpDIB, 0, m_lpbmi->bmiHeader.biSizeImage);
	memset(m_lpDIBRead, 0, m_lpbmi->bmiHeader.biSizeImage);

	// 8. 启动采集
	m_pGrabber->SetBufferSamples(FALSE);
	m_pGrabber->SetOneShot(FALSE);
	m_pGrabber->SetCallback(&m_cb, 0);
	m_bStarted = true;
	hr = m_pMC->Run();
	if (FAILED(hr)) goto FAIL;

	m_bIsConnected = true;
	return true;

FAIL:
	StopGraph();
	return false;
}

LPBYTE CVideoCap::GetDIB()
{
	DWORD dwRet = WaitForSingleObject(m_hCaptureEvent, 3000);
	if (dwRet != WAIT_OBJECT_0)
		return m_lpDIBRead;	// 超时返回缓冲 (避免上层空指针), 内容可能为旧帧或空

	EnterCriticalSection(&m_cs);
	if (m_lpDIBRead && m_lpDIB && m_lpbmi)
		memcpy(m_lpDIBRead, m_lpDIB, m_lpbmi->bmiHeader.biSizeImage);
	LeaveCriticalSection(&m_cs);
	return m_lpDIBRead;
}