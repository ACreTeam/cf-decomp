#pragma once
#include <types.h>

// The game's bg check (DOL TU d_bgcf.cpp, not split yet, between d_base and d_field_block, around
// 8006C50C..80076700). The namespace name and these classes come from the RTTI:
// dBGCF::floor_c, wall_c, column_c, clmcb_c, dtcb_c, mvbg_c, addDat_c, copyChk_c, and the
// callback dtcbCorrect_c.
// acch_c and acchWall_c have no RTTI; their names are inferred.

namespace dBGCF {

// Collision callback base (RTTI dBGCF::clmcb_c, vtable 804A6BA8). The real class lives in the DOL
// (dtor 80076690, getAttr 80076688); our weak copies are dropped at link.
class clmcb_c {
public:
    virtual ~clmcb_c() {}
    virtual BOOL getAttr(f32 *height, f32 *param, int *attr, int x, int z); // 0x0C: 80076688
};

// One per-check record inside acch_c (cleared by fn_800715A8 before a check).
class acchWall_c {
public:
    acchWall_c(); // 800714C8
    ~acchWall_c() {}

    /* 0x00 */ u32 _00[0x1C / 4];
}; // size 0x1C

// An actor's bg collision state ("actor collision check"). fn_80074234(radius, this, &pos,
// &prevPos, angle, ...) corrects the position against the bg: it runs the floor check and a
// dtcbCorrect_c pass, then fills the results below.
class acch_c {
public:
    acch_c() { init(); }

    void init(); // 8007162C

    /* 0x00 */ u8 _00;           // cleared at the end of each check
    /* 0x04 */ u32 _04;          // bit 1: passed on to the floor check
    /* 0x08 */ u32 mHitFlags;    // bit 0: on the ground; bits 1, 2: wall hits
    /* 0x0C */ void *mGroundPoly;
    /* 0x10 */ acchWall_c mWall;
    /* 0x2C */ f32 mPushOut[3];     // how far the check moved the position
    /* 0x38 */ f32 mGroundNormal[3];
    /* 0x44 */ f32 mStepHeight;     // still counts as ground this far above the floor
}; // size 0x48

} // namespace dBGCF
