#pragma once
#include <lib/egg/core/eggHeap.h>

namespace EGG {

// [TODO: extend this]
class ExpHeap : public Heap {
public:
    void setGroupID(u16 groupID);
    void setAllocMode(u16 mode);

    static EGG::ExpHeap *create(void *buffer, size_t size, u16 flags);
    static EGG::ExpHeap *create(size_t, EGG::Heap *, u16);
    static size_t getSizeForMBlock(const void *block);
};

} // namespace EGG


extern "C" {
BOOL fn_804402BC(EGG::Heap *heap, int arg); // 804402BC (eggExpHeap.o): returns TRUE; d_sys calls it per frame for both root heaps
}
