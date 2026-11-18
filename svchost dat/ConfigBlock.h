// ConfigBlock.h - encrypted, random-length configuration block
#pragma once
#include "stdafx.h"
#include "Info.h"

// Fixed capacity of the encrypted blob; the valid length is random per build.
// Controller writes sizeof(Info) + random padding (0..128); 1600 covers the
// worst case (1424 + 128 = 1552) with headroom. Must match LOCAL_CFG_CAP.
#define CFG_CAP 1600

// 8-byte marker the controller searches in the template. It exists only in the
// template and is overwritten with random bytes after the build, so the final
// payload carries no such marker.
// It is stored XOR-encrypted with the same position-dependent key, so the
// template contains no plaintext ASCII marker. The controller computes the same
// encrypted pattern and searches for it.
#define CFG_TAG_PLAIN "CFGXTAG1"
static const unsigned char CFG_TAG_ENC[8] =
    { 0x19,0x1D,0x1B,0x05,0x0A,0x1E,0x27,0x50 };

#pragma pack(push, 1)
struct CfgBlob {
    char  tag[8];          // template marker (runtime-unused, overwritten)
    DWORD blobLen;         // encrypted length = sizeof(Info) + random padding
    BYTE  blob[CFG_CAP];   // encrypted bytes
};
#pragma pack(pop)

// Decrypt g_Cfg and copy the first sizeof(Info) into outInfo.
BOOL DecryptConfig(Info* outInfo);
