#pragma once

#include <types.h>
#include <lib/egg/core/eggHeap.h>
#include <nw4r/g3d.h>
#include <nw4r/math.h>
#include <revolution/GX.h>

namespace EGG {

// DrawGX (lib/egg/gfx/eggDrawGX.cpp 80445F80..804475A0). Names and shapes follow Skyward Sword's EGG; only the
// functions that CF's DOL kept are declared. The enum value names are SS's.
class DrawGX {
public:
    DrawGX() {}

    struct DLData {
        u8 *mpList;
        u32 mLen;
    };

    struct ZModeConfig {
        bool mTest;
        GXCompare mCompare;
        bool mUpdate;
    };

    struct BlendModeConfig {
        GXBlendMode mBlendMode;
        GXBlendFactor mBlendFactorSrc;
        GXBlendFactor mBlendFactorDst;
        GXLogicOp mLogicOp;
    };

    enum DL {
        DL_0,
        DL_1,
        DL_2,
        DL_3,
        DL_4,
        DL_5,
        DL_6,
        DL_7,
        DL_8,
        DL_9,
        DL_10,
        DL_11,
        DL_12,
        DL_13,
        DL_14,
        DL_15,
        DL_16,
        DL_17,

        DL_MAX
    };

    enum VtxType {
        VTX_TYPE_0,
        VTX_TYPE_1,
        VTX_TYPE_2,
        VTX_TYPE_3,
        VTX_TYPE_4,
        VTX_TYPE_5,
        VTX_TYPE_6,
        VTX_TYPE_7,
        VTX_TYPE_8,
        VTX_TYPE_9,
        VTX_TYPE_10,
        VTX_TYPE_11,
        VTX_TYPE_12,
        VTX_TYPE_13
    };

    enum Blend {
        BLEND_0,
        BLEND_1,
        BLEND_2,
        BLEND_3,
        BLEND_4,
        BLEND_5,
        BLEND_6,
        BLEND_7,
        BLEND_8,
        BLEND_9,
        BLEND_10,
        BLEND_11,
        BLEND_12, // blending off

        BLEND_MAX
    };

    enum ZMode {
        ZMODE_0,
        ZMODE_1,
        ZMODE_2,
        ZMODE_3,

        ZMODE_MAX,
    };

    enum TevSetting {
        TEV_0, // vertex colour only
        TEV_1, // texture modulated by the vertex colour
    };

    enum TexGen {
        TEXGEN_0,
        TEXGEN_1,
    };

    enum ColorChannel {
        COLORCHAN_0,
        COLORCHAN_1,
    };

    static void Initialize(Heap *);                               // 80446348

    static GXTexMapID GetTexMapDefault();                         // stripped from CF's DOL
    static GXLightID GetLightMaskDefault();                       // 804463F0

    static void BeginDrawScreen(bool, bool, bool);                // 804463F8
    static void SetMat_ColorChannel(enum ColorChannel);           // 804464D4
    static void SetMat_TexGen(enum TexGen);                       // 804465B0
    static void SetMat_Ind();                                     // 80446630: no indirect stages
    static void SetMat_Tev(GXTevStageID, enum TevSetting);        // 80446638
    static void SetMat_PE(enum ZMode, enum Blend);                // 8044679C

    static void LoadTexture(nw4r::g3d::ResTex, GXTexMapID);       // 80446800

    static void SetVtxState(enum VtxType);                        // 80446858
    static void CreateDisplayList(Heap *);                        // 80446BF0
    static void DrawDL(enum DL, const nw4r::math::MTX34 &);       // 80447450
    static void DrawDL(enum DL, const nw4r::math::MTX34 &, GXColor); // 804474B8
    static void SetZMode(enum ZMode);                             // 80447558
    static void SetBlendMode(enum Blend);                         // 80447578

    static DLData s_DL[DL_MAX];

    static nw4r::math::MTX34 s_cameraMtx;

    static u32 s_flag;

    static const GXColor WHITE; // 807532C0 (sdata2)
    static const GXColor GREEN;
    static const GXColor RED;
    static const GXColor BLUE;
};

} // namespace EGG
