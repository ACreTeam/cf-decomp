// The villager setup pack (dNpcSetup). .text 800F9EF4..800FA08C, .ctors 804656EC, .rodata 80475748..80475768,
// .bss 805CEAC8..805CEAF0. See include/game/game/d_npc_setup.hpp.
#include <game/game/d_npc_setup.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_heap.hpp>

static const char l_fileName[] = "/Npc/Normal/Setup/pack.bin";

namespace dNpcSetup {

Pack_c sPack;

// 800F9EF4
BOOL load() {
    Pack_c *pack = &sPack;
    if (pack->mpData == NULL && dHeap::npcNmlPackHeap_p != NULL) {
        pack->mpData = pack->mLoader.request(l_fileName, 0, dHeap::npcNmlPackHeap_p);
        if (pack->mpData == NULL) {
            return FALSE;
        }
    }
    return TRUE;
}

// 800F9F64
dAnimalTemplate_c *getList(u32 *num) {
    Pack_c *pack = &sPack;
    if (pack->mpData != NULL) {
        u8 *data = (u8 *)pack->mpData;
        *num = *(u32 *)data;
        return (dAnimalTemplate_c *)(data + 0x20);
    }
    return NULL;
}

// 800F9F94
dAnimalTemplate_c *find(u16 npcIdx, dAnimalTemplate_c *list, u32 num) {
    dAnimalTemplate_c *found = NULL;
    if (list != NULL && npcIdx != 0xFFFF) {
        for (u32 i = 0; i < num; i++, list++) {
            int idx = list->mNpcIdx;
            if (npcIdx == (u16)idx) {
                found = list;
                break;
            }
        }
    }
    return found;
}

} // namespace dNpcSetup
