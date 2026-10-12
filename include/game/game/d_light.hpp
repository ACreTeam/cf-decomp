#pragma once

// d_light (DOL TU d_light.cpp, .text 800CCCD8..800CD590, not decompiled). Only what other TUs use.

#include <types.h>
#include <nw4r/ut.h>
#include <revolution/GX.h>

extern "C" {
BOOL fn_800CCCD8(); // 800CCCD8: resets the light settings (m3d::getLightSettingP); d_sys_gfx create
nw4r::ut::Color fn_800CD168(int idx); // 800CD168: an ambient color of the light settings (GXSetChanAmbColor)
GXLightID fn_800CD250();              // 800CD250: the current light mask (GXSetChanCtrl)
}
