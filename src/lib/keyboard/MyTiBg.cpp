#include <lib/textinput/MyTiBg.h>

namespace textinput {
namespace extend {
namespace bg {

void LayoutByNW4R::create(MEMAllocator *allocator) {
    nw4rmanager::Layout::createWithEventHandler(allocator, NULL);
    init();
}

void LayoutByNW4R::init() {
    mpLayout->Animate(0);
    mpLayout->CalculateMtx(mDrawInfo);
}

} // namespace bg
} // namespace extend
} // namespace textinput
