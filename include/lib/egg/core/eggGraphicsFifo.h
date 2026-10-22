#pragma once

#include <lib/egg/core/eggHeap.h>
#include <revolution/gx.h>

namespace EGG {

class GraphicsFifo {
public:
    static GraphicsFifo *create(ulong size, Heap *heap);

    GraphicsFifo(ulong size, Heap *heap);
    virtual ~GraphicsFifo();

private:
    GXFifoObj *mFifoObj; // 0x04
    void *mBufBase;      // 0x08
    ulong mBufSize;      // 0x0C

    static GraphicsFifo *sGraphicsFifo;
    static GXBool sGpStatus[5];
};

} // namespace EGG
