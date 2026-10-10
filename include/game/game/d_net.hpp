#pragma once

// Network session helpers (TU d_net.cpp, not decompiled). Several .cpp files still declare these
// locally; d_sv_runtime.hpp keeps its own (bool) declarations of fn_800DCEDC / fn_800DCF30.

#include <types.h>

extern "C" {
bool fn_800DCEDC();                            // 800DCEDC: an online session is active (returns a u8 flag)
u32 fn_800DCF30();                             // 800DCF30: number of members
int fn_800DCF58();                             // 800DCF58: this console's member index (0..3)
BOOL fn_800DD960();                            // 800DD960: this console is the host
void fn_800DD5F8(int id, void *data, int arg); // 800DD5F8: send shared record id
}
