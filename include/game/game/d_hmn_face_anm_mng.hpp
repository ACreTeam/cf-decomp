#pragma once

// The per-character face texture animation manager (DOL TU d_hmn_face_anm_mng.cpp, .text
// 800B76BC..800B7EF8, not decompiled). Only what d_a_npc uses.

#include <types.h>
#include <nw4r/g3d.h>

// The face texture animation state of one character: face type (index into the manager's per-type
// texture archives, 13 = none) and the blink timer.
class dHmnFaceAnmMng_c {
public:
    dHmnFaceAnmMng_c();                                     // 800B76BC: mType = 13
    ~dHmnFaceAnmMng_c();                                    // 800B76C8
    void setType(int type);                                  // 800B7708
    BOOL setTex(int idx, int texId);                        // 800B7710: loads texture anim texId (< 0x1D3) into slot idx
    nw4r::g3d::ResAnmTexPat getResAnmTexPat(int idx) const; // 800B78D8 (null for type 13 / nothing loaded)
    BOOL calcBlink();                                       // 800B7DAC: counts down; TRUE while a blink is due
    void resetBlink();                                      // 800B7E28: mBlinkTimer = 0, mBlinkCount = 0
    int getType() const { return mType; }

    /* 0x0 */ int mBlinkTimer; // frames to the next blink (90 + rnd(270))
    /* 0x4 */ u8 mBlinkCount;
    /* 0x5 */ u8 mType;
}; // size 0x8

extern "C" {
int fn_800B7890(int anmId); // 800B7890: eye texture anim id of a body anim (anmId > 0x1BB -> 0x1D3)
int fn_800B78B4(int anmId); // 800B78B4: mouth texture anim id of a body anim (-> 0x1D3)
int fn_800B7E48(u32 texId); // 800B7E48: mouth kind of a texture anim (< 0x1D3, else 2)
int fn_800B7E4C(u32 texId);  // 800B7E4C: default play mode of a texture anim (else 4)
}
