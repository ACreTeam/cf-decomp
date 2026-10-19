#pragma once

// The city npc base dAcNpcTown_c. DOL TU d_a_npc_town.cpp (.text 80063438..800636C4). The class name is from
// the RTTI of the RELs that derive from it (d_a_npc_town_grace / _out / _theater: Gracie's, the city's and the
// theater's guest villagers, dNpcMng_c profiles 0x52 / 0x51 / 0x53); the rest is named from behaviour.
// The villager is either a moved-out one (a special npc key, from the save) or one built from its template
// in the actor heap. Its vtable and constructor are only in the RELs (none survives in the DOL).

#include <types.h>
#include <game/game/d_a_npc_nml.hpp>

class dAcNpcTown_c : public dAcNpcNml_c {
public:
    // Explicit inline (like dAcNpcNml_c's): an implicit one puts the RELs' header weak inlines out of order.
    dAcNpcTown_c() {}

    virtual int preCreate();      // 80063650
    virtual int getSoundId();     // 800635CC (+0xA0): the voice, only once
    virtual u32 getHeapSize();    // 80063648 (+0xB4)
    virtual void setAnimal();     // 80063438 (+0xDC)

    static u32 calcHeapSize(BOOL withFrm); // 800636A0: dAcNpcNml_c's and a dAnimal_c

    /* 0x219A */ u8 mSoundIdDone; // getSoundId gave the voice
};
