#pragma once
#include <types.h>
#include <lib/egg/core/eggHeap.h>
#include <revolution/MEM.h>

namespace EGG {
class Allocator : public MEMAllocator {
public:
    Allocator(Heap *heap, long align);

    virtual ~Allocator();
    virtual void *alloc(unsigned long size);
    virtual void free(void *block);

public:
    Heap *mpHeap;  // 0x14
    int mAlign;    // 0x18
};
} // namespace EGG
