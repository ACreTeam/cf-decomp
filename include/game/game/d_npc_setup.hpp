#pragma once

// d_npc_setup (DOL TU d_npc_setup.cpp, .text 800F9EF4..800FA08C, .bss 805CEAC8..805CEAF0): the villager setup pack
// "/Npc/Normal/Setup/pack.bin" (loaded into dHeap::npcNmlPackHeap_p at boot by d_s_boot_static): a u32 count at
// +0 and the dAnimalTemplate_c list at +0x20. No RTTI: names inferred.

#include <types.h>
#include <game/game/d_dvd.hpp>

struct dAnimalTemplate_c;

namespace dNpcSetup {

class Pack_c {
public:
    Pack_c() : mpData(NULL) {}

    /* 0x00 */ void *mpData;
    /* 0x04 */ dDvd::loader_c mLoader;
}; // size 0x18

BOOL load();                                // 800F9EF4: requests the pack once; FALSE if the request failed
dAnimalTemplate_c *getList(u32 *num);       // 800F9F64: the templates and their count, or NULL before loading
dAnimalTemplate_c *find(u16 npcIdx, dAnimalTemplate_c *list, u32 num); // 800F9F94: the template of npcIdx or NULL

extern Pack_c sPack; // 805CEAD8

} // namespace dNpcSetup
