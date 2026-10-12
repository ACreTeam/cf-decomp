#pragma once

// d_main (DOL TU d_main.cpp, .text 800CDD2C..800CDEA0, .bss 805A20F8, .sbss 8074E4D8): main(), which runs the game
// on its own thread (the d_sys setup, then d_sys frames forever); and the reasons the game's processes are stopped.
// No RTTI: the names are inferred (namespace dMain; main is the C entry point).

#include <types.h>

namespace dMain {

// The reasons the fManager_c processes are stopped (sStopReason). The first one set stops them all; clearing the
// last one restarts them.
enum StopReason_e {
    STOP_REASON_CAUTION = 0x2,  // d_caution's window
    STOP_REASON_WIFI_ERR = 0x4, // d_wifi_err's window
    STOP_REASON_PERF = 0x8,     // dScene_c::checkPerf
};

BOOL isStopped(u32 mask); // 800CDD2C: any reason of mask is set
void addStop(u32 mask);   // 800CDD44
void removeStop(u32 mask); // 800CDD68

void init();             // 800CDD88: the d_sys setup
void loop();             // 800CDDB0: d_sys frames, forever
void *threadMain(void *arg); // 800CDDC4

extern u32 sStopReason; // 8074E4D8: StopReason_e bits

} // namespace dMain
