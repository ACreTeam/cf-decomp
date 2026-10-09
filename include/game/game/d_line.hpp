#pragma once

#include <game/cLib/c_line.hpp>

// The game's line lists (TU 800CD590..800CD670, see dDemoActorList_c). Names inferred:
// dLineMg_c::init is shared by the demo actor list and the m2d draw lists (d_m2d).

// A node sorted by priority (lowest first) by dLineMg_c::insertByPriority.
class dPriLineNd_c : public cLineNd_c {
public:
    /* 0x08 */ u8 mPriority;
};

class dLineMg_c : public cLineMg_c {
public:
    void init(); // 800CD590
    // Inserts node before the first node with a higher priority.
    bool insertByPriority(dPriLineNd_c *node); // 800CD5A0
};
