#include <lib/egg/core/eggAssertHeap.h>

namespace EGG {

AssertHeap::AssertHeap(MEMiHeapHead *heapHead) : Heap(heapHead) {}

AssertHeap::~AssertHeap() {
    dispose();
}

AssertHeap *AssertHeap::create(void *block, size_t size) {
    AssertHeap *createdHeap = NULL;

    void *heapBuffer = block;
    void *heapEnd = ROUND_DOWN_PTR(addOffset(block, size), 4);

    block = ROUND_UP_PTR(block, 4);

    if (block > heapEnd || nw4r::ut::GetOffsetFromPtr(block, heapEnd) < sizeof(AssertHeap) + 4) {
        return NULL;
    }

    MEMiHeapHead *handle =
        MEMCreateFrmHeapEx(addOffset(block, sizeof(AssertHeap)), nw4r::ut::GetOffsetFromPtr(block, heapEnd) - sizeof(AssertHeap), 0);
    if (handle) {
        Heap *contain = Heap::findContainHeap(block);

        createdHeap = new (block) AssertHeap(handle);
        createdHeap->mParentBlock = heapBuffer;
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
