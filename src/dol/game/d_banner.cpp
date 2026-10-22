// The save banner (dBanner_c). .text 80065230..80065524, .ctors 80465634, .data 804A4FC8..804A50B0,
// .bss 805655A0..805746E0, .sdata 80749B70..80749B90, .sdata2 80750330..80750338. See include/game/game/d_banner.hpp.
#include <game/game/d_banner.hpp>
#include <game/game/d_string.hpp>
#include <lib/revolution/tpl.h>
#include <string.h>

#define BANNER_HEAP_SIZE 0xF220

const char *dBanner_c::sFileName = "banner.bin";
const u32 dBanner_c::sFileSize = sizeof(NANDBanner);
const u32 dBanner_c::sFlags = NAND_BANNER_FLAG_NOCOPY;

// 80065230
BOOL dBanner_c::loadArc(void *heap) {
    return load("/Banner/banner.arc", heap, 0);
}

// 80065244
u32 dBanner_c::getHeapSize() {
    return BANNER_HEAP_SIZE;
}

// 80065250
void dBanner_c::build() {
    dString::Word_c title;
    dString::Word_c subtitle;
    dString::loadString(&title, 1, "sys_STRING/STR_Banner");
    dString::loadString(&subtitle, 2, "sys_STRING/STR_Banner");
    NANDInitBanner(getBannerP(), sFlags,
                   reinterpret_cast<const u16 *>(title.getText()),
                   reinterpret_cast<const u16 *>(subtitle.getText()));

    u32 bannerSize = 0;
    TPLPalette *tpl = (TPLPalette *)getFile("banner.tpl", &bannerSize);
    TPLBind(tpl);
    TPLDescriptor *desc = TPLGet(tpl, 0);
    memcpy(getBanner().bannerTexture, desc->textureHeader->data, sizeof(mBanner.bannerTexture));

    u32 iconSize = 0;
    tpl = (TPLPalette *)getFile("icon.tpl", &iconSize);
    TPLBind(tpl);
    for (int i = 0; i < NAND_BANNER_ICON_MAX_FRAME; i++) {
        TPLDescriptor *iconDesc = TPLGet(tpl, i);
        memcpy(getIconP(i), iconDesc->textureHeader->data, sizeof(mBanner.iconTexture[0]));
        setIconSpeed(i, NAND_BANNER_ICON_ANIM_SPEED_NORMAL);
    }
    setIconSpeed(NAND_BANNER_ICON_MAX_FRAME, NAND_BANNER_ICON_ANIM_SPEED_END);
}

dBanner_c g_banner; // 805655C0
