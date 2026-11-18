// Startup.cpp
// 自定义最小入口：不依赖默认 CRT 启动代码（配合 /NODEFAULTLIB + 系统自带 msvcrt.dll）
#include "stdafx.h"

// 由 main.cpp 提供（C++ 链接，保持与原 WinMain 签名一致）
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd);

// 链接器 /ENTRY:WinMainCRTStartup 指向此处。
// msvcrt.dll 已通过导入表加载，其 DllMain(PROCESS_ATTACH) 已完成自身初始化，
// 因此 malloc/free/_snprintf 等可直接使用，无需手动 CRT_INIT。
// 本工程无全局 C++ 对象、无异常，故无需构造/展开逻辑。
extern "C" void WINAPI WinMainCRTStartup()
{
    int ret = WinMain(GetModuleHandleW(NULL), NULL, NULL, SW_SHOWDEFAULT);
    ExitProcess((UINT)ret);
}
