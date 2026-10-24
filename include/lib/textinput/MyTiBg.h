#pragma once

#include <lib/textinput/tiNw4rManager.h>

namespace textinput {
namespace extend {
namespace bg {

class Base {
  public:
    virtual void create(MEMAllocator *allocator) {
    }
    virtual void init() {
    }
};

class LayoutByNW4R : public Base, public nw4rmanager::Layout {
  public:
    LayoutByNW4R(nw4r::lyt::MultiArcResourceAccessor *multiArc,
                 const char *layoutBinName, EventObserver *eventObserver)
        : Layout(multiArc, layoutBinName, eventObserver) {
    }

    virtual ~LayoutByNW4R() {
    }
    virtual void create(MEMAllocator *allocator);
    virtual void init();
    virtual void draw() {
        nw4rmanager::Layout::draw();
    }
};

} // namespace bg
} // namespace extend

typedef extend::bg::LayoutByNW4R Background;

} // namespace textinput
