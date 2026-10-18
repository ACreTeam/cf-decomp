#pragma once

// The structure manager (REL d_str_managerNP, profile STR_MANAGER, not decompiled; RTTI dStrm_c).
// It loads the structure resources (dSTR::brresBank_c) into the str heap. Only what the player REL
// reads is declared; names are inferred.

#include <types.h>
#include <game/framework/f_base.hpp>
#include <game/mLib/m_allocator.hpp>

class dStrm_c : public fBase_c {
public:
    static BOOL isLoaded() { return sInstance != NULL ? sInstance->mLoaded : FALSE; }
    static mAllocator_c *getAllocator() { return sInstance != NULL ? &sInstance->mAllocator : NULL; }

    static dStrm_c *sInstance; // d_str_managerNP .bss 0x48: set in create, cleared in doDelete

    /* 0x64 */ u8 _64_0 : 5;
    /* 0x64 */ u8 _64_5 : 1; // set with a building position on visit days
    /* 0x64 */ u8 mLoaded : 1; // the structure resources are loaded (fn_123_9A8)
    /* 0x64 */ u8 mCreated : 1;
    /* 0x68 */ mAllocator_c mAllocator; // the structure models (d_a_bbs_bird)
};
