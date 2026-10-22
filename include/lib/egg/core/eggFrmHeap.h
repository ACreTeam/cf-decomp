#pragma once
#include <lib/egg/core/eggHeap.h>
#include <revolution/mem.h>

namespace EGG {

class FrmHeap : public Heap {
public:
    FrmHeap(MEMiHeapHead *heapHead);
    virtual ~FrmHeap();

    virtual EHeapKind getHeapKind() const;
    virtual void initAllocator(Allocator *allocator, long align);
    virtual void *alloc(size_t size, long align);
    virtual void free(void *block);
    virtual void destroy();
    virtual int resizeForMBlock(void *block, size_t size);
    virtual u32 getTotalFreeSize();
    virtual size_t getAllocatableSize(long align);
    virtual size_t adjust();

    void free(long flags);

    static FrmHeap *create(void *buffer, size_t size, u16 flags);
    static FrmHeap *create(size_t size, Heap *parent, u16 flags);
};

} // namespace EGG
