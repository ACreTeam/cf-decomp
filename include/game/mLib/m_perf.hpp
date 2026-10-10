#pragma once

// mLib performance / thread helpers (TU dol/mLib/m_perf.cpp, not decompiled). Only what is used so
// far (also declared locally in d_field_assessment, d_prc_mng, d_screenshot).

#include <types.h>
#include <lib/revolution/OS/OSThread.h>

extern "C" {
void fn_802B8D30(OSThread *thread, int arg); // 802B8D30: called after OSCreateThread with fn_8016CF08(idx)
}
