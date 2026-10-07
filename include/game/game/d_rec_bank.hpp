#pragma once

#include <types.h>

namespace EGG {
class Heap;
class FrmHeap;
}

// A pool of same-sized frame heaps. Source: src/dol/game/d_rec_bank.cpp (.text 8010655C..801068AC;
// the string "dRecBank::bankTbl_c::m_frmHeap_p" names its heaps). Only RELs use it: the furniture
// manager (d_ftr_managerNP, 809CB054) creates a table of 0x8400-byte heaps (one per piece of furniture
// the scene allows, getSceneParamA), and each furniture actor (d_a_ftrNP) takes a slot with get() for
// its resources and gives it back with release().
namespace dRecBank {

class bankTbl_c {
public:
    struct entry_c {
        /* 0x0 */ EGG::FrmHeap *m_frmHeap_p; // the game's name (create's heap name string)
        /* 0x4 */ u8 mUsed;
    }; // size 0x8

    ~bankTbl_c();                                    // 8010655C
    void clear();                                    // 8010659C
    BOOL destroy();                                  // 801065B8
    EGG::FrmHeap *get(u16 *slot);                    // 80106678: *slot's heap, or claims a free one into *slot
    void release(u16 *slot);                         // 80106724: frees the heap's contents, *slot = 0xFFFF
    int create(EGG::Heap *heap, int size, int num);  // 801067B4: returns the heaps created

    // Layout from the methods (no vtable: the destructor is not virtual).
    /* 0x00 */ EGG::Heap *mDefaultHeap;              // the entries array is allocated from it
    /* 0x04 */ EGG::Heap *mHeap;                     // the frame heaps' parent (create's heap, or mDefaultHeap)
    /* 0x08 */ u32 mNum;
    /* 0x0C */ entry_c *mEntries;
    /* 0x10 */ u32 mUsedNum;
    /* 0x14 */ u16 mNext;                            // where get() starts looking
}; // size 0x18

} // namespace dRecBank
