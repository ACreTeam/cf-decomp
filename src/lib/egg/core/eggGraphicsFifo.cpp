#include <lib/egg/core/eggGraphicsFifo.h>

namespace EGG {

GraphicsFifo *GraphicsFifo::sGraphicsFifo;
GXBool GraphicsFifo::sGpStatus[5];

GraphicsFifo *GraphicsFifo::create(ulong size, Heap *heap) {
    if (heap == NULL) {
        heap = Heap::getCurrentHeap();
    }

    sGraphicsFifo = new (heap, 4) GraphicsFifo(size, heap);
    return sGraphicsFifo;
}

GraphicsFifo::~GraphicsFifo() {
    do {
        GXGetGPStatus(&sGpStatus[0], &sGpStatus[1], &sGpStatus[2], &sGpStatus[3], &sGpStatus[4]);
    } while (!sGpStatus[2]);

    Heap::free(mBufBase, NULL);
}

GraphicsFifo::GraphicsFifo(ulong size, Heap *heap) {
    mBufSize = (size + 31) & ~31;

    mBufBase = Heap::alloc(mBufSize + 0xa0, 32, heap);
    mBufBase = (void *)(((u32)mBufBase + 31) & ~31);

    mFifoObj = GXInit(mBufBase, mBufSize);
}

} // namespace EGG
