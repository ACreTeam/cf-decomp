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

class dDesign_c {
public:
    dDesign_c(); // 8010F124

    dDesignTex_c mTexture[ORG_DESIGN_TEX_NUM] ALIGN(32);
    u16 mPalette[ORG_DESIGN_PALETTE_COUNT] ALIGN(32);
    dPersonalID_c mCreator;
    wchar_t mName[ORG_DESIGN_NAME_LEN+1];
    u8 mType;
    u8 mPaletteIdx;
    u8 _870;
    u8 _871;
};
