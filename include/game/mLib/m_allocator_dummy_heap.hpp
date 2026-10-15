#pragma once

#include <types.h>
#include <lib/egg/core/eggHeap.h>

// Inert heap used by allocators before a real heap is attached.
class mAllocatorDummyHeap_c : public EGG::Heap {
public:
    mAllocatorDummyHeap_c(); // 802AE4FC
    virtual ~mAllocatorDummyHeap_c(); // 802AE780

    virtual EHeapKind getHeapKind() const { return HEAP_KIND_NONE; } // 802AE808
    virtual void initAllocator(EGG::Allocator *allocator, long alignment); // 802AE540
    virtual void *alloc(size_t, long) { return nullptr; } // 802AE800
    virtual void free(void *) {} // 802AE7FC
    virtual void destroy() {} // 802AE7F8
    virtual int resizeForMBlock(void *, size_t) { return 0; } // 802AE7F0
    virtual u32 getTotalFreeSize() { return 0; } // 802AE7E8
    virtual size_t getAllocatableSize(long) { return 0; } // 802AE7E0
    virtual size_t adjust() { return 0; } // 802AE7D8

    static void *AllocatorAllocForDummyHeap(MEMAllocator *, size_t); // 802AE4F0
    static void AllocatorFreeForDummyHeap(MEMAllocator *, void *); // 802AE4F8
    static mAllocatorDummyHeap_c *getInstance(); // 802AE560

    static mAllocatorDummyHeap_c *m_instance;
    static MEMiHeapHead *m_heapHead; // Zero-initialized heap handle passed to EGG::Heap.
}; // size 0x38
