#include <lib/egg/core/eggAssertHeap.h>

namespace EGG {

AssertHeap::AssertHeap(MEMiHeapHead *heapHead) : Heap(heapHead) {}

AssertHeap::~AssertHeap() {
    dispose();
}

AssertHeap *AssertHeap::create(void *buffer, size_t size) {
    AssertHeap *createdHeap = NULL;

    u32 heapEnd = nw4r::ut::RoundDown((u32)buffer + size, 4);
    u32 heapStart = nw4r::ut::RoundUp((u32)buffer, 4);

    if (heapStart > heapEnd || heapEnd - heapStart < 0x38 + 4) {
        return NULL;
    }

    MEMiHeapHead *handle = MEMCreateFrmHeapEx((void *)(heapStart + 0x38), heapEnd - heapStart - 0x38, 0);
    if (handle) {
        Heap *contain = Heap::findContainHeap((void *)heapStart);

        createdHeap = new ((void *)heapStart) AssertHeap(handle);
        createdHeap->mParentBlock = buffer;
        createdHeap->mParentHeap = contain;
        if (sCreateCallback != NULL) {
            sCreateCallback(createdHeap);
        }
    }

    return createdHeap;
}

AssertHeap *AssertHeap::create(size_t size, Heap *parent) {
    AssertHeap *newHeap = NULL;

    if (parent == NULL) {
        parent = sCurrentHeap;
    }
    if (size == (size_t)-1) {
        size = parent->getAllocatableSize(4);
    } else if (size == 0) {
        size = getMinSizeForCreate();
    }

    void *block = parent->alloc(size, 4);
    if (block) {
        newHeap = AssertHeap::create(block, size);
        if (newHeap == NULL) {
            parent->free(block);
        }
    }

    return newHeap;
}

void AssertHeap::destroy() {
    if (sDestroyCallback != NULL) {
        sDestroyCallback(this);
    }

    Heap *parentHeap = findParentHeap();
    this->~AssertHeap();
    if (parentHeap) {
        parentHeap->free(this);
    }
}

void *AssertHeap::alloc(size_t size, long align) {
    return NULL;
}

void AssertHeap::free(void *block) {}

int AssertHeap::resizeForMBlock(void *block, size_t size) {
    return 0;
}

u32 AssertHeap::getTotalFreeSize() {
    return 0;
}

size_t AssertHeap::getAllocatableSize(long align) {
    return 0;
}

size_t AssertHeap::adjust() {
    return 0;
}

void AssertHeap::initAllocator(Allocator *allocator, long align) {}

size_t AssertHeap::getMinSizeForCreate() {
    return 0x80;
}

Heap::EHeapKind AssertHeap::getHeapKind() const {
    return HEAP_KIND_ASSERT;
}

} // namespace EGG
