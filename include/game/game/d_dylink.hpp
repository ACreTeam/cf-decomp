#pragma once

// d_dylink (DOL TU d_dylink.cpp, .text 80086C48..80087064, .rodata 8046DAD8,
// .data 804DE8C8, .sdata 80749E40, .sbss 8074E2A0): the profiles' REL modules.
// A DynamicModuleControl per profile (profiles sharing a REL share one), made
// from the profile -> REL name table on the "dDyl::cCc_frmHeap" heap (its debug
// name); the module system is set up on a DVD thread (the string table
// /RVFOREST1NP.str and the d_profile REL). The namespace and cCc_frmHeap come
// from that heap name; the other names follow Twilight Princess' c_dylink
// (inferred).

#include <types.h>
#include <game/framework/f_profile.hpp>

class DynamicModuleControlBase;
class mDvd_callback_c;

namespace EGG {
class Heap;
class FrmHeap;
} // namespace EGG

namespace dDyl {

// A profile and the REL its class is in (NULL: in the DOL).
struct DynamicNameTable_c {
    /* 0x0 */ ProfileName mProfName;
    /* 0x4 */ const char *mName;
}; // size 0x8

enum {
    PROFILE_NUM = fProf::PROFILE_COUNT,
};

// 80086C48: makes the module controls for the profiles of table (on a new frame
// heap in heap)
BOOL initDMC(int profNum, const DynamicNameTable_c *table, int tableNum,
             EGG::Heap *heap);
BOOL isLinked(ProfileName profName); // 80086DD0: TRUE when its REL is linked or
                                     // it has none (d_scene)
BOOL unlink(ProfileName profName);   // 80086E00 (d_scene)
int linkASync(
    ProfileName profName); // 80086E20: fBase_c::PACK_RESULT_e (d_scene, d_sys)
void *initCallback(
    void *arg);         // 80086EA4: the module system setup (on the DVD thread)
void initAsync();       // 80086FB8: (d_sys)
BOOL initAsyncIsDone(); // 80087000: (d_sys)

extern const DynamicNameTable_c *DynamicNameTable; // 8074E2A0
extern int DynamicNameTableNum;                    // 8074E2A4
extern DynamicModuleControlBase **DMC;             // 8074E2A8: per profile
extern int DMC_num;                                // 8074E2AC
extern EGG::FrmHeap *cCc_frmHeap;                  // 8074E2B0
extern BOOL Initialized;                           // 8074E2B4
extern mDvd_callback_c *cDyl_DVD; // 8074E2B8: the initCallback command

} // namespace dDyl
