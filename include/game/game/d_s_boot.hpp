#pragma once

// d_s_boot (DOL TU d_s_boot.cpp, .text 801C9EF4..801CA0F4): the boot scene dScBoot_c (its RTTI name; profile
// 0xA6). It draws through dSysGfx::sDrawCallback while it runs and forwards its steps to the boot sequence object
// of d_s_boot_static. Member names other than the fBase_c overrides are inferred.

#include <types.h>
#include <game/game/d_scene_base.hpp>

class dScBoot_c : public dScene_c {
public:
    virtual int create();    // 801C9F78
    virtual int preExecute(); // 801C9FBC
    virtual int execute();   // 801C9FFC
    virtual int doDelete();  // 801CA024
    virtual int draw();      // 801CA064
    virtual ~dScBoot_c();    // 801CA08C
    virtual int getKind();   // 801CA0EC: SCENE_KIND_BOOT

    static void drawCallback(); // 801C9F4C: the boot draw (dSysGfx::sDrawCallback)
}; // size 0x64
