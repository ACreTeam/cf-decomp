#pragma once
#include <lib/egg/core/eggHeap.h>
#include <revolution/MEM.h>

namespace EGG {

// [TODO: extend this]
class FrmHeap : public Heap {
public:
    void free(long);

    static FrmHeap *create(void *buffer, size_t size, u16 flags);
    static FrmHeap *create(size_t size, Heap *parent, u16 flags); // 804404A4
};

} // namespace EGG
