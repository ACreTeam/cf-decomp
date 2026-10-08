// dRecBank::bankTbl_c: a pool of same-sized frame heaps (see include/game/game/d_rec_bank.hpp).
// .text 8010655C..801068AC.
#include <game/game/d_rec_bank.hpp>
#include <lib/egg/core/eggExpHeap.h>
#include <game/mLib/m_heap.hpp>
#include <lib/egg/core/eggFrmHeap.h>

// The heap the entry array comes from (not identified yet; also used by d_dsn).
extern EGG::ExpHeap *lbl_8074E440;

namespace dRecBank {

// 8010655C
bankTbl_c::~bankTbl_c() {}

// 8010659C
void bankTbl_c::clear() {
    mHeap = NULL;
    mNum = 0;
    mUsedNum = 0;
    mEntries = NULL;
    mNext = 0;
}

// 801065B8
BOOL bankTbl_c::destroy() {
    BOOL ret;
    if (mHeap != NULL) {
        if (mEntries != NULL) {
            for (entry_c *entry = mEntries; entry < mEntries + mNum; entry++) {
                if (entry->m_frmHeap_p != NULL) {
                    ::mHeap::destroyFrmHeap(entry->m_frmHeap_p);
                    entry->m_frmHeap_p = NULL;
                }
                entry->mUsed = FALSE;
            }
            mDefaultHeap->free(mEntries);
        }
        ret = TRUE;
        mHeap = NULL;
        mDefaultHeap = NULL;
        mNum = 0;
        mEntries = NULL;
    } else {
        ret = FALSE;
    }
    return ret;
}

// 80106678
EGG::FrmHeap *bankTbl_c::get(u16 *slot) {
    u16 cur = *slot;
    if (cur < mNum) {
        return mEntries[cur].m_frmHeap_p;
    }
    if (mEntries != NULL) {
        for (u32 i = 0; i < mNum; i++) {
            u16 idx = mNext + i;
            if (idx >= mNum) {
                idx %= mNum;
            }
            if (!mEntries[idx].mUsed) {
                mEntries[idx].mUsed = TRUE;
                *slot = idx;
                EGG::FrmHeap *heap = mEntries[idx].m_frmHeap_p;
                mUsedNum++;
                return heap;
            }
        }
    }
    return NULL;
}

// 80106724
void bankTbl_c::release(u16 *slot) {
    u16 idx = *slot;
    if (idx < mNum) {
        EGG::FrmHeap *heap = mEntries[idx].m_frmHeap_p;
        if (heap != NULL) {
            heap->free(3);
            mUsedNum--;
        }
        mEntries[idx].mUsed = FALSE;
        *slot = 0xFFFF;
    }
}

// 801067B4
int bankTbl_c::create(EGG::Heap *heap, int size, int num) {
    if (mHeap == NULL) {
        mNum = 0;
        mUsedNum = 0;
        EGG::Heap *defaultHeap = lbl_8074E440;
        mHeap = heap != NULL ? heap : defaultHeap;
        mDefaultHeap = defaultHeap;
        u32 bytes = num * sizeof(entry_c);
        mEntries = (entry_c *)defaultHeap->alloc(bytes, 0x20);
        if (mEntries != NULL) {
            u32 heapSize = ROUND_UP(size, 0x20);
            for (entry_c *entry = mEntries; entry < (entry_c *)((u8 *)mEntries + bytes); entry++) {
                entry->mUsed = FALSE;
                entry->m_frmHeap_p = ::mHeap::createFrmHeap(heapSize, mHeap, "dRecBank::bankTbl_c::m_frmHeap_p",
                                                      0x20, (::mHeap::AllocOptBit_t)0);
                if (entry->m_frmHeap_p != NULL) {
                    mNum++;
                }
            }
        }
    }
    return mNum;
}

} // namespace dRecBank
