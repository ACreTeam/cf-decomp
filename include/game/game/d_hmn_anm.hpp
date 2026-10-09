#pragma once

// Human/npc body animation tables (TU d_hmn_anm.cpp, not decompiled). Body chr animation ids are
// < 0x1BC (0x1BC = none).

#include <types.h>
#include <nw4r/g3d.h>
#include <nw4r/math.h>

extern "C" {
nw4r::g3d::ResAnmChr fn_800B63EC(int anmId); // 800B63EC: body animation resource (null if none)
u32 fn_800B64DC(int toolType);               // 800B64DC: animation id of a tool type (0x1BC if >= 14)
BOOL fn_800B6558(int anmId);                 // 800B6558: per-animation flag (lbl_8046FB78): curve the root node
void fn_800B6580(nw4r::math::MTX34 *out, const nw4r::math::MTX34 *in, f32 baseY); // 800B6580: round-world curve
int fn_800B66C8(u32 anmId);                   // 800B66C8: default play mode of a body animation (>= 0x1BC -> 4)
}
