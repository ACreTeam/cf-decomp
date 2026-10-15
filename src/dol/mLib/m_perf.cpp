#include <game/mLib/m_perf.hpp>
#include <lib/egg/core/eggThread.h>
#include <lib/egg/core/eggThreadMeter.h>

namespace mPerf {

// 802B8D30
void setThreadColor(OSThread *thread, const GXColor *color) {
    if (color != nullptr && EGG::Thread::sThreadMeter != nullptr) {
        EGG::Thread::sThreadMeter->setThreadColor(thread, *color);
    }
}

// 802B8D90
void resetThreadColor(OSThread *thread) {
    if (EGG::Thread::sThreadMeter != nullptr) {
        EGG::Thread::sThreadMeter->setThreadColor(thread, EGG::Thread::sThreadMeter->getDefaultColor());
    }
}

// 802B8DE8
void create(EGG::Heap *heap) {}

// 802B8DEC
void begin() {}

// 802B8DF0
void update() {}

// 802B8DF4
void end() {}

// 802B8DF8
BOOL isReady() {
    return TRUE;
}

} // namespace mPerf
