#pragma once

#include <lib/egg/core/eggDvdFile.h>
#include <lib/egg/core/eggHeap.h>

namespace EGG {

class DvdRipper {
public:
    class Arg {
    public:
        Arg();

    private:
        u32 _00;
        u32 _04;
        u32 _08;
        u32 _0c;
        u32 _10;
        u8 *_14;
        u8 _18[8];
    };

    typedef void (*DvdRipperCallback)(const Arg &arg);

    enum EAllocDirection {
        ALLOC_DIRECTION_HEAD = 1,
        ALLOC_DIRECTION_TAIL = 2,
    };

    static void *loadToMainRAM(s32 entryNum, void *buffer, Heap *heap, EAllocDirection direction,
                               u32 offset, u32 *readSize, u32 *fileSize); // 8043ED60
    static void *loadToMainRAM(DvdFile *file, void *buffer, Heap *heap, EAllocDirection direction,
                               u32 offset, u32 *readSize, u32 *fileSize); // 8043EE0C

private:
    static DvdRipperCallback sCallback;
    static bool sErrorRetry;
};

} // namespace EGG
