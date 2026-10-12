#pragma once

// mLib performance / thread helpers (TU dol/mLib/m_perf.cpp, not decompiled). Only what is used so
// far (also declared locally in d_field_assessment, d_prc_mng, d_screenshot).

#include <types.h>
#include <lib/revolution/OS/OSThread.h>

namespace EGG {
class Heap;
}

extern "C" {
void fn_802B8D30(OSThread *thread, int arg); // 802B8D30: called after OSCreateThread with fn_8016CF08(idx)
void fn_802B8D90(OSThread *thread);          // 802B8D90: called once that thread has terminated
}

// The frame performance meter (empty in this build; d_scene_base's d_sys hooks call them).
extern "C" {
void fn_802B8DE8(EGG::Heap *heap); // 802B8DE8: create (empty)
void fn_802B8DEC();                // 802B8DEC: begin (empty)
void fn_802B8DF0();                // 802B8DF0: update (empty)
void fn_802B8DF4();                // 802B8DF4: end (empty)
BOOL fn_802B8DF8();                // 802B8DF8: TRUE
}
