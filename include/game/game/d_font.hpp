#pragma once

// The game's main font (TU d_font.cpp, .text 800A75B0..800A773C; see notes on the split: 800A7368..800A75B0 is
// the dFltg list, another TU). The namespace is from the file name, font_c and the function names are
// inferred (no RTTI: font_c has no vtable of its own, its ArchiveFont is a member).

#include <types.h>
#include <nw4r/ut/ut_ArchiveFont.h>
#include <game/game/d_dvd.hpp>

namespace dFont {

#define FONT_HEAP_SIZE 0x280000

// The .brfna, its loader and the glyph buffer (in dHeap::fontHeap_p).
struct font_c {
    /* 0x00 */ nw4r::ut::ArchiveFont mFont;
    /* 0x3C */ dDvd::loader_c mLoader;
    /* 0x50 */ void *mpBuffer;
    /* 0x54 */ u8 _54[0x10]; // unused here
}; // size 0x64

// Loads "/Font/Anml_HummingStd_B_28_I4.brfna" (through dHeap::saveHeap_p, freed again) and builds the font
// in a buffer from dHeap::fontHeap_p. TRUE once loaded, FALSE while the archive is still loading.
BOOL load(); // 800A75B0 (from the boot scene, d_s_boot_static)
u32 getHeapSize(); // 800A7678: size of the font heap (FONT_HEAP_SIZE)

extern font_c g_mainFont; // 8058B6FC

} // namespace dFont
