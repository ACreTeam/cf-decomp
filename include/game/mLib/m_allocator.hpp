#pragma once
#include <types.h>
#include <game/mLib/m_heap.hpp>
#include <lib/egg/core/eggAllocator.h>

/**
 * @brief An allocator class that wraps an EGG::Allocator.
 */
class mAllocator_c : public EGG::Allocator {
public:
    mAllocator_c(); // 802AE568
    virtual ~mAllocator_c(); // 802AE5B4

    virtual void *alloc(size_t size) override; // 802AE65C
    virtual void free(void *ptr) override; // 802AE660

    bool attach(EGG::Heap *heap, int alignment); // 802AE60C

    static void init(EGG::Heap *heap); // 802AE664
}; // size 0x1C

/**
 * @brief An allocator with some helper functions that manage the heap
 * from which it allocates memory.
 */
class mHeapAllocator_c : public mAllocator_c {
public:
    // Inline in City Folk: the vtable and the dtor's weak copy are kept in d_kbd.
    mHeapAllocator_c() {} ///< Constructs a new heap allocator.
    virtual ~mHeapAllocator_c() { destroyHeap(); } ///< Destroys the heap allocator.

    bool createExpHeap(size_t size, EGG::Heap *parent, const char *name, ulong align); // 802AE69C: name may be NULL

    void destroyHeap(); // 802AE724: destroys the real heap and reattaches the dummy heap.
};
