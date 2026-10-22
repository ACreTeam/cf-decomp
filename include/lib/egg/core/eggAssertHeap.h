#pragma once
#include <lib/egg/core/eggHeap.h>

namespace EGG {

class AssertHeap : public Heap {
public:
    AssertHeap(MEMiHeapHead *heapHead);
    virtual ~AssertHeap();

    virtual EHeapKind getHeapKind() const;
    virtual void initAllocator(Allocator *allocator, long align);
    virtual void *alloc(size_t size, long align);
    virtual void free(void *block);
    virtual void destroy();
    virtual int resizeForMBlock(void *block, size_t size);
    virtual u32 getTotalFreeSize();
    virtual size_t getAllocatableSize(long align);
    virtual size_t adjust();

    static AssertHeap *create(void *buffer, size_t size);
    static AssertHeap *create(size_t size, Heap *parent);
    static size_t getMinSizeForCreate();
};

} // namespace EGG
