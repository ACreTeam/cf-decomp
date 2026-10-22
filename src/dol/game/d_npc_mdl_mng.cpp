// d_npc_mdl_mng.cpp: the villager models. .text 800F9B88..800F9EF4, .ctors 804656E8, .rodata 80475728..80475748,
// .data 804ED190..804ED1F0, .bss 805CEA48..805CEAC8, .sdata 8074AF08..8074AF10.
// See include/game/game/d_npc_mdl_mng.hpp. The data class and all member names are inferred.
#include <game/game/d_npc_mdl_mng.hpp>
#include <nw4r/g3d/res/g3d_resfile.h>
#include <revolution/os.h>
#include <cstdio>
#include <cstring>
#include <game/game/d_heap.hpp>

#define NPC_MDL_SPECIES_NUM 0x21
#define NPC_MDL_ARC_WORK_SIZE 0xDBB60
#define NPC_MDL_HEAP_EXTRA 0x100000
#define NPC_MDL_SIZE 0x7200
#define NPC_SP_MDL_SIZE 0xD780

// The model archive (one file-local instance; inferred).
class mdlData_c {
public:
    mdlData_c();
    ~mdlData_c();

    static u32 getWorkSize();
    static void *getMdl(u32 *size, u8 species);
    BOOL load();

    /* 0x00 */ dNpcMdlMng_c::arc_c mArc;
}; // size 0x74

static const char l_arcName[] = "/Npc/Normal/Model/model.arc";

static mdlData_c l_data;

// 800F9B88
mdlData_c::mdlData_c() {}

// 800F9BCC
mdlData_c::~mdlData_c() {}

// 800F9C30
u32 mdlData_c::getWorkSize() {
    return NPC_MDL_ARC_WORK_SIZE;
}

// 800F9C3C
void *mdlData_c::getMdl(u32 *size, u8 species) {
    return l_data.mArc.getMdl(size, species);
}

// 800F9C54
BOOL mdlData_c::load() {
    EGG::Heap *heap = dHeap::npcModelHeap_p;
    if (heap == NULL) {
        return TRUE;
    }
    if (!mArc.load(l_arcName, heap, 0)) {
        return FALSE;
    }
    return TRUE;
}

// 800F9CA0
void dNpcMdlMng_c::arc_c::onLoaded() {
    arcBank_c::onLoaded();
}

// 800F9CDC
void *dNpcMdlMng_c::arc_c::getMdl(u32 *size, u8 species) {
    if (species < NPC_MDL_SPECIES_NUM) {
        u32 fileSize = 0;
        char name[12];
        sprintf(name, "%d.brmdl", species);
        void *data = arcBank_c::getFile(name, &fileSize);
        if (size != NULL) {
            *size = fileSize;
        }
        return data;
    }
    return NULL;
}

// 800F9D5C
u32 dNpcMdlMng_c::getHeapSize() {
    return mdlData_c::getWorkSize() + NPC_MDL_HEAP_EXTRA;
}

// 800F9D80
BOOL dNpcMdlMng_c::load() {
    return l_data.load();
}

// 800F9D8C
u32 dNpcMdlMng_c::getMdlSize() {
    return NPC_MDL_SIZE;
}

// 800F9D94
BOOL dNpcMdlMng_c::copyMdl(void *dst, u32 *size, u8 species) {
    u32 fileSize = 0;
    if (size == NULL) {
        size = &fileSize;
    }
    void *data = mdlData_c::getMdl(size, species);
    if (data != NULL) {
        memcpy(dst, data, *size);
        DCFlushRange(dst, *size);
        nw4r::g3d::ResFile res(dst);
        if (res.IsValid()) {
            res.Init();
            res.Bind(res);
        }
        return TRUE;
    }
    return FALSE;
}

// 800F9E40
u32 dNpcMdlMng_c::getSpMdlSize() {
    return NPC_SP_MDL_SIZE;
}
