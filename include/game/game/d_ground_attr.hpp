#pragma once

// d_ground_attr (DOL TU d_ground_attr.cpp, .text 800A858C..800A88CC, .rodata 8046F268..8046F378): the ground
// under a position as the footstep sounds and the footprints see it. Namespace from the file name; all names
// inferred from the callers (dAcNpc_c::footstep_c::playSound, the d_effect footprint, d_player_mgr).

#include <types.h>
#include <game/mLib/m_vec.hpp>

// Footstep sound ids (dBGCF::getFootstepSe and the room floor table). Names inferred from where they are used.
enum {
    GROUND_SE_GRASS = 0x192E,        // the season ground default (also replaces GROUND_SE_GRASS_ALT)
    GROUND_SE_GRASS_ALT = 0x193A,
    GROUND_SE_ROOM_DEFAULT = 0x1936, // the room floor table's default entry
    GROUND_SE_ROOM_WOOD = 0x1942,
    GROUND_SE_LEAVES = 0x1946,       // season bres 4..8
    GROUND_SE_SNOW = 0x194E,         // season bres SCENE_SEASON_BRES_SNOW
    GROUND_SE_ROOM_FLOOR = 0x1962,
    GROUND_SE_ROOM_CARPET = 0x196A,
};

namespace dGroundAttr {

int getAttr(const mVec3_c *pos);                // 800A858C: BG attribute (bgAttr_e) at pos; the carpet's in a room
int getFootstepSe(const mVec3_c *pos, u8 *wear); // 800A8648: footstep sound id at pos, grass wear into *wear
u8 getFootprint(const mVec3_c *pos);             // 800A87B0: 0 none, 0xFF sand/wave, else the snow's grass wear
u8 getSnowWear(const mVec3_c *pos);              // 800A8850: grass wear of snowy soil, 0 otherwise

} // namespace dGroundAttr
