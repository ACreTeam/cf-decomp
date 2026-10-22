#include <lib/egg/core/eggAllocator.h>
#include <lib/egg/core/eggHeap.h>

namespace EGG {

Allocator::Allocator(Heap *heap, long align) : mpHeap(heap), mAlign(align) {
    heap->initAllocator(this, align);
}

Allocator::~Allocator() {}

void *Allocator::alloc(unsigned long size) {
    return Heap::alloc(size, mAlign, mpHeap);
}

void Allocator::free(void *block) {
    Heap::free(block, mpHeap);
}

} // namespace EGG
