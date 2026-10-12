#pragma once

// d_main (DOL TU d_main.cpp, .text 800CDD2C..800CE43C, .bss 805A20F8, .sbss 8074E4D8, .sdata2 80750870): main(),
// which runs the game on its own thread (the d_sys setup, then d_sys frames forever); the reasons the game's
// processes are stopped; and a debug town map texture maker (no callers). No RTTI: the names are inferred
// (namespace dMain; main is the C entry point).

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

// The debug town map: a 4-bit 0x100 x 0x100 texture, 3 x 3 texels per unit, the height levels shaded.
u8 getHeightColor(u8 color, u32 levels, f32 y); // 800CDEA0: color + the level of height y (levels from 84 down by 56 / (levels - 1))
BOOL makeMapColumn(void *tex, int column);      // 800CDF1C: the town's acre column (0..4) into tex
BOOL makeMap(void *tex, u32 column);            // 800CE334: one column, or MAP_COLUMN_ALL

enum {
    MAP_COLUMN_NUM = 5,
    MAP_COLUMN_ALL = MAP_COLUMN_NUM,
};

extern u32 sStopReason; // 8074E4D8: StopReason_e bits

} // namespace dMain
