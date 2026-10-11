// The save's extra block check header (dSvExtraHeader::Header_c). .text 80143E78..80143ECC.
// See include/game/game/d_sv_extra_header.hpp.
#include <game/game/d_sv_extra_header.hpp>
#include <game/sLib/s_crc.hpp>

namespace dSvExtraHeader {

// 80143E78
void Header_c::updateChecksum() {
    mChecksum = calcChecksum();
}

// 80143EA8
u32 Header_c::calcChecksum() {
    const u8 *base = (const u8 *)this;
    return sCrc::calcCRC32(base + sizeof(mChecksum), sizeof(Header_c) - sizeof(mChecksum) - ((const u8 *)&mChecksum - base),
                           -1, -1);
}

// 80143EC0
void Header_c::init() {
    mNetEnabled = FALSE;
}

} // namespace dSvExtraHeader
