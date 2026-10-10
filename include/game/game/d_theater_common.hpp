#pragma once

// The theater's shared state (TU d_theater_common.cpp, .text 8016CCCC..8016CEF8, not decompiled).
// Class name from the RTTI ("dTheater::common_c", vtable 804F43C8); member names are inferred.
// One global instance (.bss 80600874), constructed by the TU's static initializer. Only what the talk
// TUs use is declared.

#include <types.h>

namespace dTheater {

class common_c {
public:
    virtual ~common_c(); // 8016CD24

    /* 0x4 */ u8 mProgram; // today's program index + 1 (0: none)
    /* 0x5 */ u8 _5[4];
    /* 0x9 */ u8 _9;
    /* 0xA */ u8 mOpen;    // nonzero while a show is on
}; // size 0xC

} // namespace dTheater

extern "C" {
extern dTheater::common_c lbl_80600874; // the theater state (.bss)
BOOL fn_8016CD8C(const dTheater::common_c *theater); // 8016CD8C: mProgram is one of Frillard's (1..29, flag 0 in 8047B130)
}
