// MemoryModule.h
#pragma once
#include "stdafx.h"

typedef void *HMEMORYMODULE;

#ifdef __cplusplus
extern "C" {
#endif

HMEMORYMODULE MemoryLoadLibrary(const void *data);
FARPROC       MemoryGetProcAddress(HMEMORYMODULE, const char *name);
void          MemoryEraseHeaders(HMEMORYMODULE);
void          MemoryFreeLibrary(HMEMORYMODULE);

#ifdef __cplusplus
}
#endif