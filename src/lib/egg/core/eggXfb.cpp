#include <lib/egg/core/eggXfb.h>

#include <lib/egg/core/eggSystem.h>
#include <lib/egg/core/eggVideo.h>

namespace EGG {

void Xfb::init(u16 width, u16 height, Heap *heap) {
    mWidth = width;
    mHeight = height;

    u32 bufferSize = calcBufferSize(width, height);

    if (heap == NULL) {
        heap = Heap::getCurrentHeap();
    }

    mBuffer = new (heap, 32) u8[bufferSize];

    mPrev = NULL;
    mNext = NULL;
    mState = 0;
}

Xfb::Xfb(Heap *heap) {
    Video *video = BaseSystem::mConfigData->getVideo();
    const GXRenderModeObj *rm = video->pRenderMode;
    init(rm->fbWidth, rm->xfbHeight, heap);
}

u32 Xfb::calcBufferSize(u16 width, u16 height) {
    width = (width + 15) & ~15;
    return width * height * sizeof(u16);
}

} // namespace EGG
