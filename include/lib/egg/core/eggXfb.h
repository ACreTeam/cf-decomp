#pragma once

#include <lib/egg/core/eggHeap.h>

namespace EGG {

class Xfb {
public:
    Xfb(Heap *heap);

    u16 mWidth;   // 0x00
    u16 mHeight;  // 0x02
    void *mBuffer; // 0x04
    int mState;    // 0x08
    Xfb *mNext;    // 0x0C
    Xfb *mPrev;    // 0x10

    void *getBuffer() const { return mBuffer; }

    static u32 calcBufferSize(u16 width, u16 height);

private:
    void init(u16 width, u16 height, Heap *heap);
};

} // namespace EGG
