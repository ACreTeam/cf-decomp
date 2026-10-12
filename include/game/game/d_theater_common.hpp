#pragma once

// The theater's shared state (TU d_theater_common.cpp, .text 8016CCCC..8016CEF8).
// Class name from the RTTI ("dTheater::common_c", vtable 804F43C8); member and function names are
// inferred. One global instance (.bss 80600874), constructed by the TU's static initializer.

#include <types.h>

namespace dTheater {

class common_c {
public:
    common_c();                             // 8016CCCC
    virtual ~common_c();                    // 8016CD24

    void clear();                           // 8016CD64
    void setProgram(u8 program);            // 8016CD84
    BOOL isKyosyoProgram() const;           // 8016CD8C: mProgram is one of Frillard's (1..29, flag 0 in 8047B130)
    static u8 drawSisyoProgram();           // 8016CDC4: a random Dr. Shrunk program + 1
    void setFlag9(u8 on);                   // 8016CE80: setting one of _9 / mOpen clears the other
    void setOpen(u8 on);                    // 8016CE98: (dtk merged it into 8016CE80)

    /* 0x4 */ u8 mProgram; // today's program index + 1 (0: none)
    /* 0x5 */ u8 _5[4];
    /* 0x9 */ u8 _9;
    /* 0xA */ u8 mOpen;    // nonzero while a show is on
}; // size 0xC

extern common_c g_common; // 80600874: the theater state (.bss)

} // namespace dTheater
