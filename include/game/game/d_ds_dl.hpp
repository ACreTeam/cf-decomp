#pragma once

// DS download play (DOL TU d_ds_dl.cpp, not decompiled). Only what other TUs use; names are inferred.

#include <types.h>

class dDsDl_c; // 80584B94, 0x1EC bytes (layout unknown)

extern "C" {
extern dDsDl_c lbl_80584B94;    // the instance
BOOL fn_80084738(dDsDl_c *dl);  // 80084738: idle (d_reset waits for it before a reset / quitting)
}
