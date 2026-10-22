#include <MetroTRK/flush_cache.h>

// clang-format off
asm void TRK_flush_cache(u32 addr, u32 length) {
    nofralloc

    lis r5, 0xFFFF
    ori r5, r5, 0xFFF1
    and r5, r5, r3
    subf r3, r5, r3
    add r4, r4, r3

rept:
    dcbst 0, r5
    dcbf 0, r5
    sync
    icbi 0, r5
    addic r5, r5, 0x8
    subic. r4, r4, 0x8
    bge rept
    isync
    blr
}
// clang-format on
