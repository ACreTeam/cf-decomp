#pragma once

#include <types.h>
#include <revolution/gx.h>
#include <lib/revolution/os/OSThread.h>

namespace EGG {
class Heap;
}

namespace mPerf {

void setThreadColor(OSThread *thread, const GXColor *color); // 802B8D30
void resetThreadColor(OSThread *thread); // 802B8D90

// Frame meter hooks are stubs in this build.
void create(EGG::Heap *heap); // 802B8DE8
void begin(); // 802B8DEC
void update(); // 802B8DF0
void end(); // 802B8DF4
BOOL isReady(); // 802B8DF8

} // namespace mPerf
