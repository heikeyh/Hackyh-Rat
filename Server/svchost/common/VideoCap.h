// VideoCap.h: interface for the CVideoCap class.
//
// DirectShow 采集实现 (参考 借鉴视频查看 项目): ISampleGrabber + SetMediaType(RGB24),
// 由系统颜色转换器将任意摄像头输出格式(YUY2/MJPG等)转换为 RGB24, 保证主控端可绘制。
//////////////////////////////////////////////////////////////////////
#if !defined(AFX_VIDEOCAP_H__5D857DCE_D889_45B0_8A98_33E1DF64CDE3__INCLUDED_)
#define AFX_VIDEOCAP_H__5D857DCE_D889_45B0_8A98_33E1DF64CDE3__INCLUDED_
#include <windows.h>
#include <vfw.h>
#include <dshow.h>
#include "qedit.h"	// ISampleGrabber
#include <amvideo.h>
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "ole32.lib")
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CVideoCap;

// SampleGrabber 帧回调接收器 (在 DirectShow 流线程中回调)
class CWebCamGrabberCB : public ISampleGrabberCB
{
public:
	CVideoCap *m_pCap;
	CWebCamGrabberCB() : m_pCap(NULL) {}
	STDMETHODIMP_(ULONG) AddRef() { return 2; }
	STDMETHODIMP_(ULONG) Release() { return 1; }
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
	{
		if (riid == IID_ISampleGrabberCB || riid == IID_IUnknown)
		{
			*ppv = (void *)(ISampleGrabberCB *)this;
			return NOERROR;
		}
		return E_NOINTERFACE;
	}
	STDMETHODIMP SampleCB(double dblSampleTime, IMediaSample *pSample);
	STDMETHODIMP BufferCB(double dblSampleTime, BYTE *pBuffer, long lBufferSize) { return 0; }
};

class CVideoCap
{
public:
	HANDLE	m_hCaptureEvent;
	LPBITMAPINFO m_lpbmi;
	static bool IsWebCam();
	bool Initialize(int nWidth = 0, int nHeight = 0);
	LPBYTE GetDIB();
	CVideoCap();
	virtual ~CVideoCap();
	void OnFrame(BYTE *pBuffer, long lBufferSize);	// 帧回调线程调用
private:
	LPBYTE m_lpDIB;			// 回调写入的当前帧
	LPBYTE m_lpDIBRead;		// GetDIB 读取缓冲 (双缓冲避免竞态)
	CRITICAL_SECTION m_cs;
	bool m_bStarted;
	// DirectShow
	IGraphBuilder *m_pGB;
	ICaptureGraphBuilder2 *m_pCapture;
	IBaseFilter *m_pBF;
	IMediaControl *m_pMC;
	ISampleGrabber *m_pGrabber;
	IBaseFilter *m_pNullRenderer;
	CWebCamGrabberCB m_cb;
	static bool m_bIsConnected;
	bool BindFilter(int deviceId, IBaseFilter **pFilter);
	void StopGraph();
};

#endif // !defined(AFX_VIDEOCAP_H__5D857DCE_D889_45B0_8A98_33E1DF64CDE3__INCLUDED_)