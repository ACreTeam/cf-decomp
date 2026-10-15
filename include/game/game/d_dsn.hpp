#pragma once

#include <types.h>
#include <game/game/d_land.hpp>
#include <game/game/d_personal_id.hpp>

#define ORG_DESIGN_WIDTH 32
#define ORG_DESIGN_HEIGHT 32
#define ORG_DESIGN_4BPP_SIZE ((ORG_DESIGN_WIDTH*ORG_DESIGN_HEIGHT)/2)

#define ORG_DESIGN_PALETTE_COUNT 16
#define ORG_DESIGN_PALETTE_SIZE (ORG_DESIGN_PALETTE_COUNT*sizeof(u16))

#define ORG_DESIGN_NAME_LEN 16

enum STYLE_LOOK {
    STYLE_UNSET,
    STYLE_CUTE,
    STYLE_COOL,
    STYLE_SUBTLE,
    STYLE_GAUDY,
    STYLE_STRANGE,
    STYLE_FUNKY,
    STYLE_REFINED,
    STYLE_FRESH,
    STYLE_STYLISH,
    STYLE_STRIKING,

    STYLE_NUM
};

enum {
    ORG_DESIGN_TEX_FRONT,
    ORG_DESIGN_TEX_BACK,
    ORG_DESIGN_TEX_SLEEVE_0,
    ORG_DESIGN_TEX_SLEEVE_1,

    ORG_DESIGN_TEX_NUM
};

struct dDesignTex_c {
    u8 mData[ORG_DESIGN_4BPP_SIZE] ALIGN(32);
};

namespace dItem {
struct BITM;
}

#define ORG_DESIGN_PALETTE_NUM 16

// An original design (pattern). Source: src/dol/game/d_dsn.cpp
// (.text 8010F124..8010FB6C). Names are inferred unless noted.
class dDesign_c {
public:
    dDesign_c(); // 8010F124

    void clear();                                  // 8010F154 (Ghidra: ClearPattern)
    void initBlank();                              // 8010F1C4: cleared, texture 0xFF, palette 0
    void initDefault();                            // 8010F210: item 0x9D3
    BOOL setFromDlItem(void *dlItem);              // 8010F254: from a downloaded item block
    BOOL setFromItem(int item);                    // 8010F374
    BOOL setLandFromTown();                        // 8010F3DC: creator's town = this save's town
    BOOL loadTexture(int item);                    // 8010F5A0
    BOOL loadTextureA(u32 idx);                    // 8010F65C: item 0x9CC + (idx & 7)
    BOOL loadTextureB(u32 idx);                    // 8010F668: item 0x9D4 + (idx & 7)
    BOOL loadTextureC();                           // 8010F674: item 0x9DC
    BOOL loadTextureD(u32 idx);                    // 8010F67C: item 0x9DD + (idx & 3)
    BOOL isSame(const dDesign_c *other) const;     // 8010F688
    void *getTexture();                            // 8010F7B0
    void setTexture(const void *src, BOOL flush);  // 8010F7B4
    static const u16 *getPaletteData(int idx);     // 8010F804
    u16 *getPalette();                             // 8010F850
    void setPalette(int idx, BOOL flush);          // 8010F858
    void setName(const wchar_t *name);             // 8010F8C0
    int getStyle() const;                           // 8010F8FC
    void setStyle(u32 style);                       // 8010F914
    BOOL setFromBITM(const dItem::BITM *bitm);           // 8010F930

    const dPersonalID_c *getCreator() const { return &mCreator; }

    BOOL operator==(const dDesign_c& other) const {
        return (mIsPro != 0) == (other.mIsPro != 0) && getStyle() == other.getStyle() &&
                mPaletteIdx == other.mPaletteIdx && memcmp(mName, other.mName, sizeof(mName)) == 0;
    }

    BOOL operator!=(const dDesign_c& other) const {
        return !(*this == other);
    }

    static u16 sDefaultPalette[ORG_DESIGN_PALETTE_COUNT]; // 804EE360

    /* 0x000 */ dDesignTex_c mTexture[ORG_DESIGN_TEX_NUM] ALIGN(32);
    /* 0x800 */ u16 mPalette[ORG_DESIGN_PALETTE_COUNT] ALIGN(32);
    /* 0x820 */ dPersonalID_c mCreator;
    /* 0x84C */ wchar_t mName[ORG_DESIGN_NAME_LEN+1];
    /* 0x86E */ u8 mStyle;      // < STYLE_NUM
    /* 0x86F */ u8 mPaletteIdx; // < ORG_DESIGN_PALETTE_NUM
    /* 0x870 */ u8 mIsPro;      // BITM::m_proDesign, set by setFromBITM
    /* 0x871 */ u8 _871;
}; // size 0x880 (alignment 32)
