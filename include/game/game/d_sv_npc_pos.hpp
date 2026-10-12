#pragma once

// d_sv_npc_pos (DOL TU d_sv_npc_pos.cpp, .text 80150524..80150778, .sdata2 80750CD8): packed field positions
// (field units, 32.0 world units each) of event villagers in the town save: dSaveTown_c::mNpcSpotA (+0x6673C, with
// the npc kind; d_npc_info), mNpcSpotB (+0x66741) and mHarvestSpot (+0x66745, the Harvest Festival spot, with a
// side offset). No RTTI: names inferred.

#include <types.h>
#include <game/mLib/m_vec.hpp>

namespace dSvNpcPos {

// A spot with an npc kind (5 bytes).
struct KindPos_c {
    void init();                                // 80150524: kind 0x44, unset
    void set(int kind, const mVec3_c *pos);      // 80150544
    void update(u8 kind, const mVec3_c *pos);   // 80150594: kind and position only (d_a_npc_sp_jingle)
    mVec3_c getPos() const;                     // 801505D4: the unit's center

    /* 0x0 */ u8 mUnitX;
    /* 0x1 */ u8 mUnitZ;
    /* 0x2 */ u8 mKind;
    /* 0x3 */ u8 mFlag;  // NET_PACKET_NPC_SPOT_A_FLAG sets it
    /* 0x4 */ u8 mValid;
}; // size 0x5

// A spot (4 bytes).
struct Pos_c {
    void clear();                               // 801505E4
    void set(int unitX, int unitZ);               // 801505FC
    mVec3_c getPos() const;                     // 80150618: the unit's center

    /* 0x0 */ u8 mUnitX;
    /* 0x1 */ u8 mUnitZ;
    /* 0x2 */ u8 mValid;
    /* 0x3 */ u8 mFlag;  // NET_PACKET_NPC_SPOT_B_FLAG sets it
}; // size 0x4

enum {
    SIDE_POS_FLAG_EVENT = 0x80, // NET_PACKET_HARVEST_SPOT_FLAG (franklin) sets it
    SIDE_POS_FLAG_VALID = 0x40,
};

enum {
    SIDE_CENTER = 0,
    SIDE_LEFT = 1,  // the position was in the unit's left part (x % 32 < 6)
    SIDE_RIGHT = 2, // the position was in the unit's right part (x % 32 >= 25)
};

// A spot that remembers which side of its unit it was (4 bytes).
struct SidePos_c {
    void clear();                               // 80150628
    void set(const mVec3_c *pos);               // 80150648
    mVec3_c getPos() const;                     // 801506F8: the unit's center, moved 14.4 to the side

    /* 0x0 */ u8 mUnitX;
    /* 0x1 */ u8 mUnitZ;
    union {
        /* 0x2 */ u8 mFlags; // SIDE_POS_FLAG_* (d_net ORs the byte; d_sv_npc_pos writes the bits)
        struct {
            /* 0x2 */ u8 mFlag : 1;  // SIDE_POS_FLAG_EVENT
            /* 0x2 */ u8 mValid : 1; // SIDE_POS_FLAG_VALID
        };
    };
    /* 0x3 */ u8 mSide;     // SIDE_*
}; // size 0x4

} // namespace dSvNpcPos
