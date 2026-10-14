#pragma once

// The boot scene's static part (DOL TU d_s_boot_static.cpp, not decompiled). Only what other TUs use. The boot
// sequence object (lbl_805EBDA0, here dScBootStatic_c; name inferred) runs the boot screens behind a color fader;
// dScBoot_c (d_s_boot) forwards its create / execute / draw / delete to it.

#include <types.h>

struct dScBootStatic_c;

namespace dSys {
extern bool sBootComplete; // 8074E890: owned by d_sys; set once the boot sequence is done
}

extern "C" {
dScBootStatic_c *fn_80108698();        // 80108698: the boot sequence object (lbl_805EBDA0)
BOOL fn_801086A4();                    // 801086A4: FALSE when lbl_805EBDA0 +0x333 and +0x19C are both set (d_wifi: idle / reset)
BOOL fn_8010875C(dScBootStatic_c *p);  // 8010875C: create (FALSE until its parts are created; makes the fader)
BOOL fn_80108840(dScBootStatic_c *p);  // 80108840: delete (FALSE until its parts are deleted; frees the fader)
void fn_80108934(dScBootStatic_c *p);  // 80108934: runs the boot phase (lbl_8074E6B0) while the scene waits
void fn_80108940(dScBootStatic_c *p);  // 80108940: execute
void fn_80108A74(dScBootStatic_c *p);  // 80108A74: draw
}
