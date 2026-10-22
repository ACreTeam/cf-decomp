#pragma once

#include <types.h>

#include <nw4r/g3d.h>
#include <revolution/GX.h>

namespace EGG {

// GX state management (lib/egg/gfx/eggStateGX.cpp). CF's version is older than ogws': the GX cache
// is only tracked through flag bits and the projection/viewport/scissor wrappers do not compare.
class StateGX {
public:
    static void initialize(unsigned short width, unsigned short height, GXColor clearColor, GXPixelFmt pixFmt);

    static void frameInit();

    static void resetGX();
    static void resetVtx();
    static void resetColorChannel();
    static void resetIndirect();
    static void resetTexture();
    static void resetTexGen();
    static void resetTev();
    static void resetPE();
    static void resetGXCache();

    static void setPixelFormatGX(GXPixelFmt pixFormat, GXZFmt16 zFmt16);

    static void GXSetProjectionv_(const f32 *proj);
    static void GXSetViewport_(f32 left, f32 top, f32 width, f32 height, f32 nearZ, f32 farZ);
    static void GXSetScissor_(u32 left, u32 top, u32 width, u32 height);
    static void GXSetScissorBoxOffset_(int x, int y);

    static u16 getEfbWidth() { return s_widthFb; }
    static u16 getEfbHeight() { return s_heightEfb; }

    enum {
        FLAG_0x01 = 1 << 0,
        FLAG_ENABLE_TMEM = 1 << 4,
        FLAG_ENABLE_CACHE = 1 << 5,
    };

    enum {
        CACHE_PROJECTION = 1 << 0,
        CACHE_VIEWPORT = 1 << 1,
        CACHE_SCISSOR = 1 << 2,
        CACHE_SCISSOR_BOX = 1 << 3,
    };

    struct GXCache {
        u32 flags;
        int scissorX;
        int scissorY;
        bool colorUpdate;
        bool alphaUpdate;
        bool dither;
        bool dstAlphaEnable;
        u8 dstAlpha;
    };

    static u16 s_flag;
    static GXCache s_cacheGX;

    static u16 s_widthFb;
    static u16 s_heightEfb;
    static GXPixelFmt s_pixFormat;
    static GXZFmt16 s_zFmt16;
    static GXPixelFmt s_pixFormatCurrent;
    static GXZFmt16 s_zFmt16Current;
    static GXColor s_clearEfb;
    static u16 s_cacheFlags;
    static int s_field_CE4;
    static nw4r::g3d::tmem::TMemLayout s_tmem_layout;
};

} // namespace EGG
