#pragma once
#include <lib/egg/core/eggHeap.h>

namespace EGG {

class ExpHeap : public Heap {
public:
    ExpHeap(MEMiHeapHead *heapHead);
    virtual ~ExpHeap();

    virtual EHeapKind getHeapKind() const;
    virtual void initAllocator(Allocator *allocator, long align);
    virtual void *alloc(size_t size, long align);
    virtual void free(void *block);
    virtual void destroy();
    virtual int resizeForMBlock(void *block, size_t size);
    virtual u32 getTotalFreeSize();
    virtual size_t getAllocatableSize(long align);
    virtual size_t adjust();

    static ExpHeap *create(void *buffer, size_t size, u16 flags);
    static ExpHeap *create(size_t size, Heap *parent, u16 flags);

    void setAllocMode(u16 mode);
    BOOL checkHeap(u32 flags);
    static size_t getSizeForMBlock(const void *block);
    void freeAll();
};

} // namespace EGG
