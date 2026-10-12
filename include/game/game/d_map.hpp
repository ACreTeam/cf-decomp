#pragma once

// d_map (DOL TU d_map.cpp, .text 800CDEA0..800CE43C, .bss 805A2410, .sbss 8074E4E0, .sdata2 80750870): a debug town
// map texture maker (no callers). No RTTI: the names are inferred (namespace dMap).

#include <types.h>

namespace dMap {

// The debug town map: a 4-bit 0x100 x 0x100 texture, 3 x 3 texels per unit, the height levels shaded.
u8 getHeightColor(u8 color, u32 levels, f32 y); // 800CDEA0: color + the level of height y (levels from 84 down by 56 / (levels - 1))
BOOL makeMapColumn(void *tex, int column);      // 800CDF1C: the town's acre column (0..4) into tex
BOOL makeMap(void *tex, u32 column);            // 800CE334: one column, or MAP_COLUMN_ALL

enum {
    MAP_COLUMN_NUM = 5,
    MAP_COLUMN_ALL = MAP_COLUMN_NUM,
};

} // namespace dMap
