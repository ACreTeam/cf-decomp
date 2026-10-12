#pragma once

// The villager models (DOL TU d_npc_mdl_mng.cpp, .text 800F9B88..800F9EF4).
// "/Npc/Normal/Model/model.arc" holds one model per species ("<species>.brmdl", 0x21 of them); the archive
// lives in a file-local object of d_npc_mdl_mng.cpp. Class names from RTTI ("dNpcMdlMng_c::arc_c"); the
// member names are inferred.

#include <types.h>
#include <game/game/d_dvd.hpp>

class dNpcMdlMng_c {
public:
    // "/Npc/Normal/Model/model.arc"
    class arc_c : public dDvd::arcBank_c {
    public:
        virtual ~arc_c() {} // 800F9E94 (weak)
        virtual void onLoaded(); // 800F9CA0

        void *getMdl(u32 *size, u8 species); // 800F9CDC: "<species>.brmdl" in the archive (species < 0x21)
    };

    static u32 getHeapSize(); // 800F9D5C: size of the npc model heap (dHeap::npcModelHeap_p)
    static BOOL load();       // 800F9D80: loads the archive into dHeap::npcModelHeap_p
    static u32 getMdlSize();  // 800F9D8C: size of a villager's model buffer (0x7200)
    static BOOL copyMdl(void *dst, u32 *size, u8 species); // 800F9D94: copies and binds a species' model into dst
    static u32 getSpMdlSize(); // 800F9E40: size of a special npc's model file (0xD780)
};
