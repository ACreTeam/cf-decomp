#pragma once

// The npc lists (TU d_npc_list.cpp, not decompiled).

#include <types.h>

class dDemoActor_c;

extern "C" {
u32 fn_800F97FC();                  // 800F97FC: number of entries of list A (lbl_805CE9E0)
u32 fn_800F980C();                  // 800F980C: number of entries of list B (lbl_805CEA20)
dDemoActor_c *fn_800F9850(u32 idx); // 800F9850: entry of list A
dDemoActor_c *fn_800F98BC(u32 idx); // 800F98BC: entry of list B
}
