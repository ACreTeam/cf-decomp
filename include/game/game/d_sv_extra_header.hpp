#pragma once

// d_sv_extra_header (DOL TU d_sv_extra_header.cpp, .text 80143E78..80143ECC): the check block at the start of the
// save's extra block (dSaveExtra_c + 0, d_sv_extra): a CRC32 over +0x04..+0x20. No RTTI: names inferred.

#include <types.h>

namespace dSvExtraHeader {

struct Header_c {
    void updateChecksum(); // 80143E78
    u32 calcChecksum();    // 80143EA8
    void init();           // 80143EC0: clears mNetEnabled

    /* 0x00 */ u32 mChecksum;
    /* 0x04 */ u8 mNetEnabled; // dSaveExtra_c::mNetEnabled
    /* 0x05 */ u8 _05[0x1B];
}; // size 0x20

} // namespace dSvExtraHeader
