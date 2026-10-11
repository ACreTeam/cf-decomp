#pragma once

// Fonts (DOL TU d_font.cpp, not decompiled). Only what other TUs use.

#include <types.h>
#include <nw4r/ut/ut_ResFont.h>

extern "C" {
u32 fn_800A7678(); // 800A7678: size of the font heap (0x280000)
}

// 8058B6FC: the main font, "Font/Anml_HummingStd_B_28_I4.brfnta" (loaded by 800A75B0). It appears to be an
// nw4r::ut::ArchiveFont (.brfnta; 8022DB60 takes buffer + size + data), which has no header here yet.
extern nw4r::ut::ResFont g_mainFont;
