#pragma once

// The npc lists (TU d_npc_list.cpp, not decompiled).

#include <types.h>

class dDemoActor_c;

extern "C" {
u32 fn_800F97FC();                  // 800F97FC: number of entries of list A (lbl_805CE9E0)
u32 fn_800F980C();                  // 800F980C: number of entries of list B (lbl_805CEA20)
u32 fn_800F981C(dDemoActor_c *actor, const u16 *key); // 800F981C: adds the actor to list A (dAcNpcNml_c::addToNpcList)
void fn_800F9834(dDemoActor_c *actor);                // 800F9834: removes it from list A (dAcNpcNml_c::removeFromNpcList)
dDemoActor_c *fn_800F9850(u32 idx); // 800F9850: entry of list A
u32 fn_800F9888(dDemoActor_c *actor, const u16 *key); // 800F9888: adds the actor to list B (dAcNpcSp_c::addToNpcList)
void fn_800F98A0(dDemoActor_c *actor);                // 800F98A0: removes it from list B (dAcNpcSp_c::removeFromNpcList)
dDemoActor_c *fn_800F98BC(u32 idx); // 800F98BC: entry of list B
}
