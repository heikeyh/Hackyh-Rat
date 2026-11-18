// stdafx.h
#pragma once

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
// 不使用 UCRT 头里把 _snprintf 等内联成 __stdio_common_vsprintf 的版本，
// 改为外部函数声明，由系统自带 msvcrt.dll 提供（配合自定义 msvcrt_x64.lib）。
// 该方案仅用于 x64（/NODEFAULTLIB + msvcrt_x64.lib）；x86 走默认 UCRT 链接，
// 必须保留头内联，否则会产生外部符号 __snprintf，而 UCRT 导入库不提供该短名。
#ifdef _WIN64
#define _NO_CRT_STDIO_INLINE
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <winternl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 体积优化：可行时内联常用内存/字符串函数，减少对 CRT 的外部调用
// （大尺寸/动态尺寸时编译器仍会调用外部实现，由 msvcrt.dll 提供）
#pragma intrinsic(memset, memcpy, memmove, strlen, memcmp)

// SECTION_ALL_ACCESS 在某些 SDK 里没定义，兜底
#ifndef SECTION_ALL_ACCESS
#define SECTION_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | SECTION_QUERY | \
    SECTION_MAP_WRITE | SECTION_MAP_READ | SECTION_MAP_EXECUTE | SECTION_EXTEND_SIZE)
#endif

// NT_SUCCESS
#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif

// ViewUnmap 常量（winternl.h 里通常有，兜底）
#ifndef ViewUnmap
#define ViewUnmap 2
#endif
