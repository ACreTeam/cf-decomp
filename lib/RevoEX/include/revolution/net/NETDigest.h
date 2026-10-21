#ifndef REVOLUTION_NET_NETDIGEST_H
#define REVOLUTION_NET_NETDIGEST_H

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// layout from the DWARF of RVLDWC 2.1.3 (Tokyo Friend Park II)
typedef struct NETMD5Context {
    union {
        struct {
            u32 a;
            u32 b;
            u32 c;
            u32 d;
        };
        u32 state[4];
    };
    u64 length;
    union {
        u32 buffer32[16];
        u8 buffer8[64];
    };
} NETMD5Context; // size 0x58

void NETMD5Init(NETMD5Context* context);
void NETMD5Update(NETMD5Context* context, const void* input, u32 length);
void NETMD5GetDigest(NETMD5Context* context, void* digest);

static inline void NETCalcMD5(void* digest, const void* input, u32 length) {
    NETMD5Context context;

    NETMD5Init(&context);
    NETMD5Update(&context, input, length);
    NETMD5GetDigest(&context, digest);
}

#ifdef __cplusplus
}
#endif

#endif
