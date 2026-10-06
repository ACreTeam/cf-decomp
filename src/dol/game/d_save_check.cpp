// The save file's check block (dSaveCheck_c, save + 0). .text 8011524C..80115380.
#include <game/game/d_save_check.hpp>
#include <game/game/d_save_data.hpp>
#include <game/sLib/s_crc.hpp>

// Not split yet (C linkage keeps the target names).
extern "C" {
extern const u32 lbl_80750334; // 80750334 (.sdata2 of another TU)
}

// 8011524C
void dSaveCheck_c::updateChecksum() {
    mChecksum = calcChecksum();
}

// Inlined: isChecksumOK testing this result with an if keeps the compare branchy.
static inline BOOL checkCrc(dSaveCheck_c *check) {
    u32 checksum = check->calcChecksum();
    if (checksum == check->mChecksum) {
        return TRUE;
    }
    return FALSE;
}

// 8011527C
BOOL dSaveCheck_c::isChecksumOK() {
    if (checkCrc(this)) {
        return TRUE;
    }
    return FALSE;
}

// 801152C0
u32 dSaveCheck_c::calcChecksum() {
    const u8 *base = (const u8 *)this;
    return sCrc::calcCRC32(base + sizeof(mChecksum),
                           sizeof(dSaveCheck_c) - sizeof(mChecksum) - ((const u8 *)&mChecksum - base), -1, -1);
}

// 801152D8
void dSaveCheck_c::init() {
    mVersion = dSaveData_c::getVersion();
    mState = 0;
    if (lbl_80750334 == 1) {
        mFlags = 0;
    } else {
        mFlags = 1;
    }
}

// 8011532C
BOOL dSaveCheck_c::isVersionOK() {
    return dSaveData_c::getVersion() == mVersion;
}

// 8011536C
BOOL dSaveCheck_c::isState2() {
    return mState == 2;
}
