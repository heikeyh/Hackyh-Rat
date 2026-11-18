// ConfigBlock.cpp - encrypted configuration block storage and runtime decryption
#include "stdafx.h"
#include "ConfigBlock.h"

// Position-dependent XOR key; must match the controller-side encryptor.
#define CFG_KEY 0x5A
static BYTE CfgKeyAt(DWORD i) { return (BYTE)(CFG_KEY + (i & 0x3F)); }

// Static instance: the controller locates it in the template via CFG_TAG, writes
// blobLen + encrypted blob, then overwrites tag.
// tag[8] holds the encrypted marker (no plaintext ASCII); see CFG_TAG_ENC.
// volatile is REQUIRED: the controller patches this structure in the binary after
// build. Without volatile the compiler sees blobLen==0 and no in-image writes,
// folds DecryptConfig to constant FALSE and the linker removes g_Cfg (and the tag).
static volatile CfgBlob g_Cfg = { { 0x19,0x1D,0x1B,0x05,0x0A,0x1E,0x27,0x50 }, 0, { 0 } };

BOOL DecryptConfig(Info* outInfo)
{
    if (!outInfo) return FALSE;
    DWORD n = g_Cfg.blobLen;
    if (n < sizeof(Info) || n > CFG_CAP) return FALSE;

    BYTE* tmp = (BYTE*)malloc(n);
    if (!tmp) return FALSE;

    for (DWORD i = 0; i < n; i++)
        tmp[i] = g_Cfg.blob[i] ^ CfgKeyAt(i);

    memcpy(outInfo, tmp, sizeof(Info));
    free(tmp);
    return TRUE;
}
