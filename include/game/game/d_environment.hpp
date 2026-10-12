#pragma once

// d_environment (DOL TU d_environment.cpp, .text 80195290..8019A264, not decompiled): only the globals and calls
// that other TUs use (d_fireworks, d_thunder). Target names kept until the TU is decompiled.

#include <types.h>
#include <nw4r/math.h>
#include <revolution/GX.h>

extern "C" {
// 80199BBC: queues a glow of color and size at pos on the glow drawer.
BOOL fn_80199BBC(void *glow, const GXColor *color, const nw4r::math::VEC3 *pos, f32 size);

extern void *lbl_8074EA60; // 8074EA60: the glow drawer (NULL: none)
extern f32 lbl_8074EA1C;   // 8074EA1C..8074EA28: the flash light color (r, g, b, a as floats)
extern f32 lbl_8074EA20;
extern f32 lbl_8074EA24;
extern f32 lbl_8074EA28;
extern f32 lbl_8074C010;   // 8074C010: the flash light's fade speed
}
