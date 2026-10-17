#pragma once

// d_insect_fieldNP (REL): only what other modules call so far. Not decompiled
// yet; names unknown.

#include <types.h>

class dPlayActorMng_c;
class mVec3_c;

// .text 1C74: called by d_fgobj_managerNP when a fallen honeycomb has landed.
// Unless the REL's flag at .bss 0x14 is set (fn_116_16524), it creates insect
// type 0xC (the bees) at pos through fn_116_B28 (with the manager's +0xB4 as
// the last argument). Its first argument is the DOL pointer g_playActorMng
// (8074E838), which d_insect_fieldNP also writes. The owning class and so the
// mangled name are not known until that REL is decompiled.
extern "C" void fn_116_1C74(dPlayActorMng_c *mng, const mVec3_c *pos);
