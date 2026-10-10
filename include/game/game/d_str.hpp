#pragma once

// Town structures (DOL TU d_str.cpp, not decompiled). Only what other TUs use; names are inferred.

#include <types.h>

class dStrWork_c; // 805FBF10 (layout unknown)

extern "C" {
void fn_80168B90();                  // 80168B90: (d_reset soft reset)
dStrWork_c *fn_801683C0();           // 801683C0: &805FBF10
void fn_80166E08(dStrWork_c *work);  // 80166E08: clears it (d_reset soft reset)
int *fn_801699CC();                  // 801699CC: a lazily initialised int (8074E870)
}
