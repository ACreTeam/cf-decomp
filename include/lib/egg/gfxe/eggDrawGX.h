#pragma once

#include <types.h>
#include <lib/egg/core/eggHeap.h>
#include <nw4r/g3d.h>
#include <nw4r/math.h>
#include <revolution/GX.h>

namespace EGG {

// DrawGX (lib/egg/gfx/eggDrawGX.cpp 80445F80..804475A0). Function names follow Skyward Sword's EGG (same
// shapes); the enum value names are placeholders.
namespace DrawGX {
    enum ColorChannel { COLORCHAN_0, COLORCHAN_1 };
    enum TexGen { TEXGEN_0, TEXGEN_1 };
    enum ZMode { ZMODE_0, ZMODE_1, ZMODE_2 };
    enum Blend { BLEND_0, BLEND_1, BLEND_2, BLEND_3, BLEND_4, BLEND_5, BLEND_6, BLEND_7, BLEND_8, BLEND_9, BLEND_10 };
    enum DL { DL_16 = 16 };

    void Initialize(EGG::Heap *);

    void BeginDrawScreen(bool, bool, bool);                     // 804463F8
    void SetMat_ColorChannel(ColorChannel chan);                // 804464D4
    void SetMat_TexGen(TexGen texGen);                          // 804465B0
    void SetMat_Ind();                                          // 80446630: no indirect stages
    void SetMat_PE(ZMode zMode, Blend blend);                   // 8044679C
    void LoadTexture(nw4r::g3d::ResTex tex, GXTexMapID map);    // 80446800
    void SetBlendMode(Blend blend);                             // 80446858
    void DrawDL(DL dl, const nw4r::math::MTX34 &mtx);           // 80447450
    void DrawDL(DL dl, const nw4r::math::MTX34 &mtx, GXColor color); // 804474B8
}

} // namespace EGG
