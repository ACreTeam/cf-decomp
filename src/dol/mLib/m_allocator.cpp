#include <game/mLib/m_allocator_dummy_heap.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/mLib/m_heap.hpp>

mAllocatorDummyHeap_c *mAllocatorDummyHeap_c::m_instance;
MEMiHeapHead *mAllocatorDummyHeap_c::m_heapHead;

// 802AE4F0
void *mAllocatorDummyHeap_c::AllocatorAllocForDummyHeap(MEMAllocator *, size_t) {
    return nullptr;
}

// 802AE4F8
void mAllocatorDummyHeap_c::AllocatorFreeForDummyHeap(MEMAllocator *, void *) {}

// 802AE4FC
mAllocatorDummyHeap_c::mAllocatorDummyHeap_c() : EGG::Heap(m_heapHead) {
    m_instance = this;
}

// 802AE540
void mAllocatorDummyHeap_c::initAllocator(EGG::Allocator *allocator, long alignment) {
    static const MEMAllocatorFuncs sAllocatorFunc = {
        AllocatorAllocForDummyHeap,
        AllocatorFreeForDummyHeap
    };
    allocator->funcs = &sAllocatorFunc;
    allocator->heap = mHeapHandle;
    allocator->heapParam1 = alignment;
    allocator->heapParam2 = 0;
}

// 802AE560
mAllocatorDummyHeap_c *mAllocatorDummyHeap_c::getInstance() {
    return m_instance;
}

// 802AE568
mAllocator_c::mAllocator_c() : EGG::Allocator(mAllocatorDummyHeap_c::getInstance(), 4) {}

// 802AE5B4
mAllocator_c::~mAllocator_c() {}

// 802AE60C
bool mAllocator_c::attach(EGG::Heap *heap, int alignment) {
    if (heap == nullptr) {
        heap = EGG::Heap::getCurrentHeap();
    }
    mAlign = alignment;
    mpHeap = heap;
    heap->initAllocator(this, alignment);
    return true;
}

// 802AE65C
void *mAllocator_c::alloc(size_t size) {
    return EGG::Allocator::alloc(size);
}

// 802AE660
void mAllocator_c::free(void *ptr) {
    EGG::Allocator::free(ptr);
}

// 802AE664
void mAllocator_c::init(EGG::Heap *heap) {
    new(heap, 4) mAllocatorDummyHeap_c();
}

// 802AE69C
bool mHeapAllocator_c::createExpHeap(size_t size, EGG::Heap *parent, const char *name, ulong align) {
    destroyHeap();
    EGG::ExpHeap *heap = EGG::ExpHeap::create(size, parent, 0);
    if (heap == nullptr) {
        return false;
    }
    attach(heap, align);
    if (name != nullptr) {
        heap->setName(name);
    }
    return true;
}

// 802AE724
void mHeapAllocator_c::destroyHeap() {
    if (mpHeap != mAllocatorDummyHeap_c::getInstance()) {
        mpHeap->destroy();
        mpHeap = mAllocatorDummyHeap_c::getInstance();
    }
}

// 802AE780
mAllocatorDummyHeap_c::~mAllocatorDummyHeap_c() {}
