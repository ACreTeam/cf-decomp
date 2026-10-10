#pragma once

// The scene base class dScene_c (its RTTI name; DOL TU d_scene_base.cpp, not decompiled). Only what
// other TUs use. Its statics keep their lbl_ names (NSMBW has dScene_c::m_nowScene / m_nextScene).

#include <types.h>
#include <game/game/d_base.hpp>

class dScene_c : public dBase_c {
public:
    // vtable 0x4C: the scene's kind; 0 for the boot / title scenes (dScBoot_c returns 0), 2 for an
    // online scene (postCreate then calls fn_800DDE80).
    virtual int getKind();
};

extern "C" {
extern dScene_c *lbl_8074E800; // 8074E800: the current scene (set by its create, cleared by its postDelete)
extern u16 lbl_8074B1B0;       // 8074B1B0: the next scene request (a profile name? d_reset: 0xA6 boot, 0xA2 after an error)
extern int lbl_8074E804;       // 8074E804: the next scene's wipe-out type
extern int lbl_8074E808;       // 8074E808: the next scene's wipe-in type
}
