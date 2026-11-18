// DualView.cpp
#include "stdafx.h"
#include "ApiHash.h"
#include "DualView.h"

BOOL DualViewCreate(SIZE_T size, DUAL_VIEW* out)
{
        if (!out || size == 0) return FALSE;
    if (!pNtCreateSection) InitNtdllApis();
    if (!pNtCreateSection) return FALSE;

    memset(out, 0, sizeof(DUAL_VIEW));
    out->viewSize = size;

    // 1) 创建节对象
    LARGE_INTEGER maxSize;
    maxSize.QuadPart = size;

    HANDLE hSection = NULL;
        NTSTATUS st = pNtCreateSection(
        &hSection,
        SECTION_ALL_ACCESS,
        NULL,
        &maxSize,
        PAGE_EXECUTE_READWRITE,   // 节对象保护；视图可以覆盖
        SEC_COMMIT,               // 普通提交节
        NULL);
        if (!NT_SUCCESS(st) || !hSection) return FALSE;

    // 2) 映射可写视图 RW
    PVOID writeView = NULL;
    SIZE_T viewSize = size;
            st = pNtMapViewOfSection(
        hSection, GetCurrentProcess(),
        &writeView, 0, size, NULL, &viewSize,
        ViewUnmap, 0, PAGE_READWRITE);
        if (!NT_SUCCESS(st)) { pNtClose(hSection); return FALSE; }

    // 3) 映射可执行视图：初始即 PAGE_EXECUTE_WRITECOPY（XWC，正是 Windows 加载正常映像
    //    所用的保护），从映射起就不存在 RWX 窗口。PE 数据全部经 writeView(RW) 写入（同一节
    //    对象，自动同步到本视图）。MemoryModule 完成装载后按节收紧：代码节 XWC->RX；数据节
    //    XWC->PAGE_WRITECOPY（写时复制，无执行位，DLL 写全局变量时系统自动复制为私有 RW 页）；
    //    只读节 XWC->R。最终运行态无 RWX。
    PVOID execView = NULL;
    viewSize = size;
    st = pNtMapViewOfSection(
        hSection, GetCurrentProcess(),
        &execView, 0, size, NULL, &viewSize,
        ViewUnmap, 0, PAGE_EXECUTE_WRITECOPY);
        if (!NT_SUCCESS(st)) {
        pNtUnmapViewOfSection(GetCurrentProcess(), writeView);
        pNtClose(hSection);
        return FALSE;
    }

        out->hSection  = hSection;
    out->pWriteView = writeView;
    out->pExecView  = execView;
    return TRUE;
}

void DualViewDestroy(DUAL_VIEW* dv)
{
    if (!dv) return;
    if (dv->pWriteView && pNtUnmapViewOfSection)
        pNtUnmapViewOfSection(GetCurrentProcess(), dv->pWriteView);
    if (dv->pExecView && pNtUnmapViewOfSection)
        pNtUnmapViewOfSection(GetCurrentProcess(), dv->pExecView);
    if (dv->hSection && pNtClose)
        pNtClose(dv->hSection);
    memset(dv, 0, sizeof(DUAL_VIEW));
}