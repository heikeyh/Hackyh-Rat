// Info.h - shared configuration structure (must match controller BuildServer.cpp
// and controlled-side svchost.cpp field layout/size)
#pragma once
#include <windows.h>

struct Info
{
    char Address[100];
    char Port[10];
    char Mark[50];
    char SerName[100];
    char Serdisplay[128];
    char Serdesc[256];
    char strGroup[256];
    BOOL bDelete;
    BOOL bReg;
    char URL[256];
    char SerFile[MAX_PATH];
};
