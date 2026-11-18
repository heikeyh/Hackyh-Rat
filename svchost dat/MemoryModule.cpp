// MemoryModule.cpp
#include "stdafx.h"
#include "ApiHash.h"
#include "DualView.h"
#include "MemoryModule.h"

// =====================

#ifndef IMAGE_SIZEOF_BASE_RELOCATION
#define IMAGE_SIZEOF_BASE_RELOCATION (sizeof(IMAGE_BASE_RELOCATION))
#endif

#ifdef _WIN64
#define POINTER_TYPE ULONGLONG
#else
#define POINTER_TYPE DWORD
#endif

typedef BOOL (WINAPI *DllEntryProc)(HINSTANCE, DWORD, LPVOID);

typedef struct {
    PIMAGE_NT_HEADERS headers;
    DUAL_VIEW         dv;          // ★ 双视图
    HMODULE          *modules;
    int               numModules;
    int               initialized;
    // Cached before the PE header is erased, so GetProcAddress / FreeLibrary
    // no longer depend on the in-image header after MemoryEraseHeaders().
    DllEntryProc      pDllEntry;   // entry point address (for ATTACH/DETACH)
    DWORD             exportRva;   // export directory RVA
    DWORD             exportSize;  // export directory size
} MEMORYMODULE, *PMEMORYMODULE;

#define GET_HEADER_DICTIONARY(module, idx) &(module)->headers->OptionalHeader.DataDirectory[idx]

// 自带大小写不敏感比较，避免依赖 CRT 的 _stricmp（仅 ASCII 导出名，足够）
static int my_stricmp(const char* a, const char* b)
{
    while (*a && *b) {
        unsigned char ca = (unsigned char)*a, cb = (unsigned char)*b;
        if (ca >= 'A' && ca <= 'Z') ca += 32;
        if (cb >= 'A' && cb <= 'Z') cb += 32;
        if (ca != cb) return (int)ca - (int)cb;
        a++; b++;
    }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

// ---------- 复制节区：写入可写视图 ----------
static void CopySectionsDualView(const unsigned char *data, PIMAGE_NT_HEADERS old_headers, PMEMORYMODULE module)
{
    int i, size;
    unsigned char *writeBase = (unsigned char *)module->dv.pWriteView;

    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(module->headers);
    for (i = 0; i < module->headers->FileHeader.NumberOfSections; i++, section++) {
        if (section->SizeOfRawData == 0) {
            size = old_headers->OptionalHeader.SectionAlignment;
            if (size > 0) {
                unsigned char *dest = writeBase + section->VirtualAddress;
                memset(dest, 0, size);
                section->Misc.PhysicalAddress = (POINTER_TYPE)dest;
            }
            continue;
        }
        unsigned char *dest = writeBase + section->VirtualAddress;
        memcpy(dest, data + section->PointerToRawData, section->SizeOfRawData);
        section->Misc.PhysicalAddress = (POINTER_TYPE)dest;
    }
}

// ---------- 重定位：基于 execView ----------
static void PerformBaseRelocation(PMEMORYMODULE module, SIZE_T delta)
{
    DWORD i;
    // 双视图：重定位修补写可写视图（同一节对象，自动同步到执行视图）
    unsigned char *codeBase = (unsigned char *)module->dv.pWriteView;

    PIMAGE_DATA_DIRECTORY directory = GET_HEADER_DICTIONARY(module, IMAGE_DIRECTORY_ENTRY_BASERELOC);
    if (directory->Size > 0) {
        PIMAGE_BASE_RELOCATION relocation = (PIMAGE_BASE_RELOCATION)(codeBase + directory->VirtualAddress);
        for (; relocation->VirtualAddress > 0; ) {
            unsigned char *dest = codeBase + relocation->VirtualAddress;
            unsigned short *relInfo = (unsigned short *)((unsigned char *)relocation + IMAGE_SIZEOF_BASE_RELOCATION);
            for (i = 0; i < ((relocation->SizeOfBlock - IMAGE_SIZEOF_BASE_RELOCATION) / 2); i++, relInfo++) {
                DWORD *patchAddrHL;
#ifdef _WIN64
                ULONGLONG *patchAddr64;
#endif
                int type, offset;
                type = *relInfo >> 12;
                offset = *relInfo & 0xfff;
                switch (type) {
                case IMAGE_REL_BASED_ABSOLUTE: break;
                case IMAGE_REL_BASED_HIGHLOW:
                    patchAddrHL = (DWORD *)(dest + offset);
                    *patchAddrHL += (DWORD)delta;
                    break;
#ifdef _WIN64
                case IMAGE_REL_BASED_DIR64:
                    patchAddr64 = (ULONGLONG *)(dest + offset);
                    *patchAddr64 += delta;
                    break;
#endif
                default: break;
                }
            }
            relocation = (PIMAGE_BASE_RELOCATION)(((char *)relocation) + relocation->SizeOfBlock);
        }
    }
}

// ---------- 导入表：基于 execView ----------
static int BuildImportTable(PMEMORYMODULE module)
{
    int result = 1;
    // 双视图：导入表修补写可写视图（同一节对象，自动同步到执行视图）
    unsigned char *codeBase = (unsigned char *)module->dv.pWriteView;

    PIMAGE_DATA_DIRECTORY directory = GET_HEADER_DICTIONARY(module, IMAGE_DIRECTORY_ENTRY_IMPORT);
    if (directory->Size > 0) {
        PIMAGE_IMPORT_DESCRIPTOR importDesc = (PIMAGE_IMPORT_DESCRIPTOR)(codeBase + directory->VirtualAddress);
        int maxImports = 256;  // hard cap against a malformed import directory
        for (; maxImports-- > 0 && !pIsBadReadPtr(importDesc, sizeof(IMAGE_IMPORT_DESCRIPTOR)) && importDesc->Name; importDesc++) {
            POINTER_TYPE *thunkRef;
            FARPROC *funcRef;
            HMODULE handle = NULL;
            // 用 LdrLoadDll 加载模块（避免 LoadLibraryA 明文）
            {
                const char* dllName = (const char*)(codeBase + importDesc->Name);
                WCHAR wname[260] = {0};
                for (int k = 0; dllName[k] && k < 259; k++) wname[k] = (WCHAR)dllName[k];
                UNICODE_STRING us;
                us.Buffer = wname;
                us.Length = (USHORT)(wcslen(wname) * sizeof(WCHAR));
                us.MaximumLength = us.Length + sizeof(WCHAR);
                HANDLE h = NULL;
                NTSTATUS st = pLdrLoadDll(NULL, NULL, &us, &h);
                handle = NT_SUCCESS(st) ? (HMODULE)h : NULL;
            }
            if (handle == NULL) { result = 0; break; }

            module->modules = (HMODULE *)realloc(module->modules, (module->numModules + 1) * sizeof(HMODULE));
            if (module->modules == NULL) { result = 0; break; }
            module->modules[module->numModules++] = handle;

            if (importDesc->OriginalFirstThunk) {
                thunkRef = (POINTER_TYPE *)(codeBase + importDesc->OriginalFirstThunk);
                funcRef = (FARPROC *)(codeBase + importDesc->FirstThunk);
            } else {
                thunkRef = (POINTER_TYPE *)(codeBase + importDesc->FirstThunk);
                funcRef = (FARPROC *)(codeBase + importDesc->FirstThunk);
            }
            for (; *thunkRef; thunkRef++, funcRef++) {
                if (IMAGE_SNAP_BY_ORDINAL(*thunkRef)) {
                    *funcRef = (FARPROC)GetProcByOrdinal(handle, (WORD)IMAGE_ORDINAL(*thunkRef));
                } else {
                    PIMAGE_IMPORT_BY_NAME thunkData = (PIMAGE_IMPORT_BY_NAME)(codeBase + (*thunkRef));
                    *funcRef = (FARPROC)GetProcByHash(handle, HashApiString((const char*)&thunkData->Name));
                }
                if (*funcRef == 0) { result = 0; break; }
            }
            if (!result) break;
        }
    }
    return result;
}

// ---------- 主加载函数 ----------
HMEMORYMODULE MemoryLoadLibrary(const void *data)
{
    if (!pNtCreateSection) InitNtdllApis();

    PMEMORYMODULE result = NULL;
    PIMAGE_DOS_HEADER dos_header;
    PIMAGE_NT_HEADERS old_header;
    SIZE_T locationDelta;
    BOOL successfull;

    dos_header = (PIMAGE_DOS_HEADER)data;
    if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) return NULL;

    old_header = (PIMAGE_NT_HEADERS)&((const unsigned char *)(data))[dos_header->e_lfanew];
    if (old_header->Signature != IMAGE_NT_SIGNATURE) return NULL;

    SIZE_T imageSize = old_header->OptionalHeader.SizeOfImage;

    // ★ 用双视图替代 VirtualAlloc(RW)
    DUAL_VIEW dv;
    if (!DualViewCreate(imageSize, &dv)) return NULL;

    result = (PMEMORYMODULE)pHeapAlloc(pGetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(MEMORYMODULE));
    if (!result) { DualViewDestroy(&dv); return NULL; }
    result->dv = dv;
    result->numModules = 0;
    result->modules = NULL;
    result->initialized = 0;
    result->pDllEntry = NULL;
    result->exportRva = 0;
    result->exportSize = 0;

    // 头信息：写在可写视图开头
    unsigned char *writeBase = (unsigned char *)dv.pWriteView;
        memcpy(writeBase, dos_header, dos_header->e_lfanew + old_header->OptionalHeader.SizeOfHeaders);
        result->headers = (PIMAGE_NT_HEADERS)&((const unsigned char *)(writeBase))[dos_header->e_lfanew];

    // 更新 ImageBase 为可执行视图地址
    result->headers->OptionalHeader.ImageBase = (POINTER_TYPE)dv.pExecView;

        CopySectionsDualView((const unsigned char *)data, old_header, result);

    // 重定位：execView 与原始 ImageBase 的差
    locationDelta = (SIZE_T)((BYTE*)dv.pExecView - old_header->OptionalHeader.ImageBase);
        if (locationDelta != 0) {
        PerformBaseRelocation(result, locationDelta);
    }
    if (!BuildImportTable(result)) goto error;
    // ★ 数据节（IMAGE_SCN_MEM_WRITE）在 execView 中设为可写：
    //   execView 默认是 RX，而 DLL 的 .data/.bss 需要写权限（DllMain 常写全局变量）。
    {
        // execView 初始为 XWC（装载全程已无 RWX）。按节收紧（NtProtect 只能减少权限）：
        //   代码节(X,!W) -> PAGE_EXECUTE_READ（RX）；
        //   数据节(W,!X) -> PAGE_WRITECOPY（WC，无执行位；DLL 写全局变量时系统自动复制为
        //                  私有 RW 页，无需也无法直接授予 RW，因初始保护不含"直接可写"）；
        //   只读节       -> PAGE_READONLY（R）；
        //   W+X 节       -> RX（绝不授予 RWX）。
        // Cache export directory + entry point BEFORE the header is made
        // read-only / erased, so neither GetProcAddress nor FreeLibrary needs
        // the in-image header afterwards.
        {
            PIMAGE_DATA_DIRECTORY ed =
                &result->headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
            result->exportRva = ed->VirtualAddress;
            result->exportSize = ed->Size;
        }
        if (result->headers->OptionalHeader.AddressOfEntryPoint != 0) {
            result->pDllEntry = (DllEntryProc)((BYTE*)dv.pExecView +
                result->headers->OptionalHeader.AddressOfEntryPoint);
        }

        PIMAGE_SECTION_HEADER sec = IMAGE_FIRST_SECTION(old_header);
        WORD nsec = old_header->FileHeader.NumberOfSections;

        // PE 头首页收紧为 WRITECOPY（无 X）：保留可写能力供 EraseHeaders 清零，
        // 清零后再收紧为只读。
        {
            PVOID hb = dv.pExecView; SIZE_T hs = 0x1000; ULONG ho = 0;
            pNtProtectVirtualMemory(GetCurrentProcess(), &hb, &hs, PAGE_WRITECOPY, &ho);
        }
        for (WORD i = 0; i < nsec; i++) {
            DWORD sz = sec[i].Misc.VirtualSize;
            if (!sz) sz = sec[i].SizeOfRawData;
            if (!sz) continue;
            BOOL isX = (sec[i].Characteristics & IMAGE_SCN_MEM_EXECUTE) != 0;
            BOOL isW = (sec[i].Characteristics & IMAGE_SCN_MEM_WRITE) != 0;
            ULONG want;
            if (isX && !isW)       want = PAGE_EXECUTE_READ;   // code  -> RX
            else if (isW && !isX)  want = PAGE_WRITECOPY;      // data  -> COW (no X)
            else if (isW && isX)   want = PAGE_EXECUTE_READ;   // never grant RWX
            else                   want = PAGE_READONLY;      // rodata-> R
            PVOID pbase = (BYTE*)dv.pExecView + sec[i].VirtualAddress;
            SIZE_T psz = sz;
            ULONG oldp = 0;
            pNtProtectVirtualMemory(GetCurrentProcess(), &pbase, &psz, want, &oldp);
        }
    }

    if (result->pDllEntry != 0) {
        successfull = (*result->pDllEntry)((HINSTANCE)dv.pExecView, DLL_PROCESS_ATTACH, 0);
        if (!successfull) goto error;
        result->initialized = 1;
    }

    return (HMEMORYMODULE)result;

error:MemoryFreeLibrary(result);
    return NULL;
}

FARPROC MemoryGetProcAddress(HMEMORYMODULE module, const char *name)
{
    PMEMORYMODULE m = (PMEMORYMODULE)module;
    unsigned char *codeBase = (unsigned char *)m->dv.pExecView;
    int idx = -1;
    DWORD i, *nameRef;
    WORD *ordinal;
    PIMAGE_EXPORT_DIRECTORY exports;
    if (m->exportRva == 0 || m->exportSize == 0) return NULL;

    exports = (PIMAGE_EXPORT_DIRECTORY)(codeBase + m->exportRva);
    if (exports->NumberOfNames == 0 || exports->NumberOfFunctions == 0) return NULL;

    nameRef = (DWORD *)(codeBase + exports->AddressOfNames);
    ordinal = (WORD *)(codeBase + exports->AddressOfNameOrdinals);
    for (i = 0; i < exports->NumberOfNames; i++, nameRef++, ordinal++) {
        if (my_stricmp(name, (const char *)(codeBase + (*nameRef))) == 0) {
            idx = *ordinal; break;
        }
    }
    if (idx == -1 || (DWORD)idx > exports->NumberOfFunctions) return NULL;
    return (FARPROC)(codeBase + (*(DWORD *)(codeBase + exports->AddressOfFunctions + (idx * 4))));
}

void MemoryEraseHeaders(HMEMORYMODULE mod)
{
    PMEMORYMODULE m = (PMEMORYMODULE)mod;
    if (!m) return;

    // Header size is read from m->headers (which lives in the write view) while
    // that view is still mapped.
    DWORD hdrSize = m->headers->OptionalHeader.SizeOfHeaders;
    if (hdrSize == 0) hdrSize = 0x1000;

    // Zero the shared header pages via the RW write view first (clears the
    // section backing store; the exec-view header page still maps the shared
    // page and therefore reads as zero, without needing its own COW fault).
    memset(m->dv.pWriteView, 0, hdrSize);

    // The write view is no longer needed after loading; unmap it to remove the
    // RW view (and its PE header). The exec view keeps the running image.
    if (pNtUnmapViewOfSection) {
        pNtUnmapViewOfSection(GetCurrentProcess(), m->dv.pWriteView);
        m->dv.pWriteView = NULL;
    }
    m->headers = NULL;   // pointed into the now-unmapped write view

    // Tighten the exec-view header page (WC -> R, a reduction). It reads zero.
    PVOID base = m->dv.pExecView;
    SIZE_T region = 0x1000;
    ULONG oldp = 0;
    pNtProtectVirtualMemory(GetCurrentProcess(), &base, &region, PAGE_READONLY, &oldp);
}

void MemoryFreeLibrary(HMEMORYMODULE mod)
{
    PMEMORYMODULE module = (PMEMORYMODULE)mod;
    if (!module) return;

    if (module->initialized != 0 && module->pDllEntry != 0) {
        (*module->pDllEntry)((HINSTANCE)module->dv.pExecView, DLL_PROCESS_DETACH, 0);
        module->initialized = 0;
    }
    if (module->modules != NULL) {
        for (int i = 0; i < module->numModules; i++) {
            if (module->modules[i] != INVALID_HANDLE_VALUE)
                pFreeLibrary(module->modules[i]);
        }
        free(module->modules);
    }
    DualViewDestroy(&module->dv);
    pHeapFree(pGetProcessHeap(), 0, module);
}