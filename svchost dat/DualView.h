// DualView.h
#pragma once
#include "stdafx.h"

typedef struct _DUAL_VIEW {
    HANDLE hSection;      // 节对象
    PVOID  pWriteView;    // 可写视图基址（RW）
    PVOID  pExecView;     // 可执行视图基址（RX）
    SIZE_T viewSize;      // 映射大小
} DUAL_VIEW, *PDUAL_VIEW;

// 创建双视图：writeView 用于写入 PE 数据，execView 用于执行
BOOL DualViewCreate(SIZE_T size, DUAL_VIEW* out);

// 销毁双视图（先解映射，再关节句柄）
void DualViewDestroy(DUAL_VIEW* dv);

// 便捷宏：把 writeView 的偏移转换成 execView 的偏移
#define DV_WRITE(dv, off)  ((PVOID)((BYTE*)(dv)->pWriteView + (off)))
#define DV_EXEC(dv, off)   ((PVOID)((BYTE*)(dv)->pExecView  + (off)))