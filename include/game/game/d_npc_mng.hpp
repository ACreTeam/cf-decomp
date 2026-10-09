#pragma once

// The npc manager (TU d_npc_mng.cpp, not decompiled). Only what d_a_npc uses.

#include <types.h>

namespace EGG {
class FrmHeap;
}

extern "C" {
extern EGG::FrmHeap *lbl_8074EABC; // the npc manager heap (801A658C); parent of dAcNpc_c::m_heap_p (no split yet)
}
