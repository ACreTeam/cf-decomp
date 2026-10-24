#include <lib/egg/core/eggFrmHeap.h>

namespace EGG {

FrmHeap::FrmHeap(MEMiHeapHead *heapHead) : Heap(heapHead) {}

FrmHeap::~FrmHeap() {
    dispose();
    MEMDestroyFrmHeap(mHeapHandle);
}

FrmHeap *FrmHeap::create(void *block, size_t size, u16 flags) {
    FrmHeap *createdHeap = NULL;

    void *heapBuffer = block;
    void *heapEnd = ROUND_DOWN_PTR(addOffset(block, size), 4);

    block = ROUND_UP_PTR(block, 4);

    if (block > heapEnd || nw4r::ut::GetOffsetFromPtr(block, heapEnd) < sizeof(FrmHeap) + 4) {
        return NULL;
    }

    MEMiHeapHead *handle =
        MEMCreateFrmHeapEx(addOffset(block, sizeof(FrmHeap)), nw4r::ut::GetOffsetFromPtr(block, heapEnd) - sizeof(FrmHeap), flags);
    if (handle) {
        Heap *contain = Heap::findContainHeap(block);

        createdHeap = new (block) FrmHeap(handle);
        createdHeap->mParentBlock = heapBuffer;
        createdHeap->mParentHeap = contain;
        if (sCreateCallback != NULL) {
            sCreateCallback(createdHeap);
        }
    }

    return createdHeap;
}

FrmHeap *FrmHeap::create(size_t size, Heap *parent, u16 flags) {
    FrmHeap *newHeap = NULL;

    if (parent == NULL) {
        parent = sCurrentHeap;
    }
    if (size == (size_t)-1) {
        size = parent->getAllocatableSize(4);
    }

    void *block = parent->alloc(size, 4);
    if (block) {
        newHeap = FrmHeap::create(block, size, flags);
        if (newHeap == NULL) {
            parent->free(block);
        }
    }

    return newHeap;
}

void FrmHeap::destroy() {
    if (sDestroyCallback != NULL) {
        sDestroyCallback(this);
    }

    Heap *parentHeap = findParentHeap();
    this->~FrmHeap();
    if (parentHeap) {
        parentHeap->free(this);
    }
}

void *FrmHeap::alloc(size_t size, long align) {
    return MEMAllocFromFrmHeapEx(mHeapHandle, size, align);
}

void FrmHeap::free(void *block) {}

void FrmHeap::free(long flags) {
    MEMFreeToFrmHeap(mHeapHandle, flags);
}

int FrmHeap::resizeForMBlock(void *block, size_t size) {
    return MEMResizeForMBlockFrmHeap(mHeapHandle, block, size);
}

u32 FrmHeap::getTotalFreeSize() {
    return getAllocatableSize(4);
}

size_t FrmHeap::getAllocatableSize(long align) {
    return MEMGetAllocatableSizeForFrmHeapEx(mHeapHandle, align);
}

size_t FrmHeap::adjust() {
    u32 adjustedSize = MEMAdjustFrmHeap(mHeapHandle) + 0x38;

    if (adjustedSize > 0x38 && mParentHeap) {
        mParentHeap->resizeForMBlock(mParentBlock, adjustedSize);
        return adjustedSize;
    }

    return 0;
}

void FrmHeap::initAllocator(Allocator *allocator, long align) {
    MEMInitAllocatorForFrmHeap((MEMAllocator *)allocator, mHeapHandle, align);
}

Heap::EHeapKind FrmHeap::getHeapKind() const {
    return HEAP_KIND_FRAME;
}

} // namespace EGG
