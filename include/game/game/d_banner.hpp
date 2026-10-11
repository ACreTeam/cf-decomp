#pragma once

// d_banner (DOL TU d_banner.cpp, .text 80065230..80065524): the save banner. dBanner_c (named by its RTTI) is
// the banner archive ("/Banner/banner.arc", a dDvd::arcBank_c) and the NANDBanner built from it (title and
// subtitle from sys_STRING/STR_Banner, banner.tpl, the 8 frames of icon.tpl), which d_nand writes as
// "banner.bin". The member and data names are inferred.

#include <types.h>
#include <game/game/d_dvd.hpp>
#include <lib/revolution/NAND.h>

class dBanner_c : public dDvd::arcBank_c {
public:
    dBanner_c() {}
    virtual ~dBanner_c() {}
    BOOL loadArc(void *heap); // 80065230: starts loading /Banner/banner.arc
    static u32 getHeapSize(); // 80065244: 0xF220
    void build();             // 80065250: builds mBanner from the archive

    NANDBanner* getBannerP() { return &mBanner; }
    NANDBanner& getBanner() { return mBanner; }
    const NANDBanner& getBanner() const { return mBanner; }

    u8* getIconP(u32 idx) { return mBanner.iconTexture[idx]; }
    void setIconSpeed(u32 idx, int speed) { NANDSetIconSpeed(&mBanner, idx, speed); }

    static const char *sFileName; // 80749B70: "banner.bin"
    static const u32 sFileSize;   // 80750330: sizeof(NANDBanner)
    static const u32 sFlags;      // 80750334: NAND_BANNER_FLAG_NOCOPY (d_save_check: dSaveCheck_c::mFlags 0 when set)

    /* 0x74 */ u8 _74[0x80 - 0x74];
    /* 0x80 */ NANDBanner mBanner;
} ALIGN(32); // size 0xF120

extern dBanner_c g_banner; // 805655C0
