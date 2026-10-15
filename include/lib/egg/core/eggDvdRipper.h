#pragma once

#include <types.h>

namespace EGG {
class Heap;

class DvdRipper {
public:
    enum EAllocDirection {
        ALLOC_DIRECTION_HEAD = 1,
        ALLOC_DIRECTION_TAIL = 2,
    };

    static void *loadToMainRAM(s32 entryNum, void *buffer, Heap *heap,
        EAllocDirection direction, u32 offset, u32 *readSize, u32 *fileSize); // 8043ED60
};
}
