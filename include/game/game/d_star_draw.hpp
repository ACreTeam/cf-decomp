#pragma once

// The night sky's star field (RTTI dStarDraw_c), part of the sky actor (dSky_c + 0xA4). Its code is
// in the DOL (dol/game/d_star_draw.cpp), not decompiled yet; only what d_sky uses is declared.

#include <game/mLib/m_3d.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/game/d_dvd.hpp>

class dStarDraw_c : public m3d::proc_c {
public:
    struct star_c {
        star_c() : m0(0.0f), m4(0), m8(0) {}
        ~star_c() {}

        /* 0x0 */ f32 m0;
        /* 0x4 */ int m4;
        /* 0x8 */ u16 m8;
    }; // size 0xC
    // Fix this later
    dStarDraw_c()
        : m008(0), m0AC(0.0f), m0B0(0.0f), m128(0), m12C(0), m130(0), m134(-1), m138(-1), m13C(-1),
          m140(0) {}
    ~dStarDraw_c() {}

    virtual void drawXlu(); // 80164D14 (DOL; its vtable is emitted there)

    /* 0x008 */ int m008;
    /* 0x00C */ u8 _00C[0x4A - 0xC];
    /* 0x04A */ u8 mAlpha;
    /* 0x04C */ star_c mStars[8];
    /* 0x0AC */ f32 m0AC;
    /* 0x0B0 */ f32 m0B0;
    /* 0x0B4 */ mAllocator_c mAllocator;
    /* 0x0D0 */ dDvd::brresBank_c mRes;
    /* 0x128 */ int m128;
    /* 0x12C */ u8 m12C;
    /* 0x130 */ int m130;
    /* 0x134 */ int m134;
    /* 0x138 */ int m138;
    /* 0x13C */ int m13C;
    /* 0x140 */ u8 m140;
}; // size 0x144

// Not decompiled yet (C linkage keeps the target names).
extern "C" {
bool fn_801640DC(dStarDraw_c *draw, EGG::Heap *heap); // 801640DC: load and create
bool fn_8016418C(dStarDraw_c *draw);                  // 8016418C: unload
void fn_80164278(dStarDraw_c *draw, const nw4r::math::VEC3 *pos, const s16 &angle); // 80164278: calc
}
