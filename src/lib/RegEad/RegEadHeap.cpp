#include <lib/RegEad/RegEadArray.hpp>

namespace RegEad {

MEMiHeapHead *sHeap;

// 801E90B8
void setHeap(MEMiHeapHead *heap) {
    sHeap = heap;
}

// 801E90C0
void *allocMem(u32 size) {
    return MEMAllocFromExpHeapEx(sHeap, size, 4);
}

// 801E90D0
void freeMem(void *ptr) {
    MEMFreeToExpHeap(sHeap, ptr);
}

} // namespace RegEad
