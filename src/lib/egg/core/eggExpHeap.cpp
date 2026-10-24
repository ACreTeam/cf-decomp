#include <lib/egg/core/eggExpHeap.h>

inline void *operator new(unsigned long, void *p) {
    return p;
}

namespace EGG {

ExpHeap::ExpHeap(MEMiHeapHead *heapHead) : Heap(heapHead) {}

ExpHeap::~ExpHeap() {
    dispose();
    MEMDestroyExpHeap(mHeapHandle);
}

ExpHeap *ExpHeap::create(void *block, size_t size, u16 flags) {
    ExpHeap *createdHeap = NULL;

    void *heapBuffer = block;
    void *heapEnd = ROUND_DOWN_PTR(addOffset(block, size), 4);

    block = ROUND_UP_PTR(block, 4);

    if (block > heapEnd || nw4r::ut::GetOffsetFromPtr(block, heapEnd) < sizeof(ExpHeap) + 4) {
        return NULL;
    }

    MEMiHeapHead *handle =
        MEMCreateExpHeapEx(addOffset(block, sizeof(ExpHeap)), nw4r::ut::GetOffsetFromPtr(block, heapEnd) - sizeof(ExpHeap), flags);
    if (handle) {
        Heap *contain = Heap::findContainHeap(block);

        createdHeap = new (block) ExpHeap(handle);
        createdHeap->mParentBlock = heapBuffer;
        createdHeap->mParentHeap = contain;
        if (sCreateCallback != NULL) {
            sCreateCallback(createdHeap);
        }
    }

    return createdHeap;
}

ExpHeap *ExpHeap::create(size_t size, Heap *parent, u16 flags) {
    ExpHeap *newHeap = NULL;

    if (parent == NULL) {
        parent = sCurrentHeap;
    }
    if (size == (size_t)-1) {
        size = parent->getAllocatableSize(4);
    }

    void *block = parent->alloc(size, 4);
    if (block) {
        newHeap = ExpHeap::create(block, size, flags);
        if (newHeap == NULL) {
            parent->free(block);
        }
    }

    return newHeap;
}

void ExpHeap::destroy() {
    if (sDestroyCallback != NULL) {
        sDestroyCallback(this);
    }

    Heap *parentHeap = findParentHeap();
    this->~ExpHeap();
    if (parentHeap) {
        parentHeap->free(this);
    }
}

void *ExpHeap::alloc(size_t size, long align) {
    if (mFlags.onBit(BIT_DISABLE_ALLOC)) {
        OSPanic("eggExpHeap.cpp", 185, "DAME DAME\n");
    }
    return MEMAllocFromExpHeapEx(mHeapHandle, size, align);
}

void ExpHeap::free(void *block) {
    MEMFreeToExpHeap(mHeapHandle, block);
}

int ExpHeap::resizeForMBlock(void *block, size_t size) {
    return MEMResizeForMBlockExpHeap(mHeapHandle, block, size);
}

u32 ExpHeap::getTotalFreeSize() {
    return MEMGetTotalFreeSizeForExpHeap(mHeapHandle);
}

size_t ExpHeap::getAllocatableSize(long align) {
    return MEMGetAllocatableSizeForExpHeapEx(mHeapHandle, align);
}

void ExpHeap::setAllocMode(u16 mode) {
    MEMSetAllocModeForExpHeap(mHeapHandle, mode);
}

size_t ExpHeap::adjust() {
    u32 adjustedSize = MEMAdjustExpHeap(mHeapHandle) + 0x38;

    if (adjustedSize > 0x38 && mParentHeap) {
        mParentHeap->resizeForMBlock(mParentBlock, adjustedSize);
        return adjustedSize;
    }

    return 0;
}

BOOL ExpHeap::checkHeap(u32 flags) {
    return TRUE;
}

size_t ExpHeap::getSizeForMBlock(const void *block) {
    return MEMGetSizeForMBlockExpHeap(block);
}

void ExpHeap::initAllocator(Allocator *allocator, long align) {
    MEMInitAllocatorForExpHeap((MEMAllocator *)allocator, mHeapHandle, align);
}

namespace {
void free_all_visitor(void *block, MEMiHeapHead *heap, u32 param) {
    MEMFreeToExpHeap(heap, block);
}
} // namespace

void ExpHeap::freeAll() {
    dispose();
    MEMVisitAllocatedForExpHeap(mHeapHandle, free_all_visitor, 0);
}

Heap::EHeapKind ExpHeap::getHeapKind() const {
    return HEAP_KIND_EXPAND;
}

} // namespace EGG
