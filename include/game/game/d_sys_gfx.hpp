#pragma once

// d_sys_gfx (DOL TU d_sys_gfx.cpp, .text 800AC1CC..800AC260, .sbss 8074E398): creates the graphics systems
// (m3d with its fog manager, d_light, m2d, the 2D resource heap, mFace) during d_sys's boot setup (fn_8016BA7C,
// after the heaps; the heap is the d_sys root heap at 8074E888), and the per-frame draw hook d_sys's frame
// (fn_8016C3C0) calls between the m3d reset and the faders. d_s_boot sets the hook to its boot draw and clears it
// when booting ends. No RTTI: names inferred (the TU was split as d_sys_heap; renamed).

#include <types.h>

namespace EGG {
class Heap;
}

namespace dSysGfx {

void create(EGG::Heap *heap); // 800AC1CC
void draw();                  // 800AC248: calls sDrawCallback if set

extern void (*sDrawCallback)(); // 8074E398

} // namespace dSysGfx
