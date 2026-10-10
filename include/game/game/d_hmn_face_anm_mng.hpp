#pragma once

// The per-character face texture animation manager (DOL TU d_hmn_face_anm_mng.cpp, .text
// 800B76BC..800B7EF8). The face texture animations come from "/FaceAnm/FaceAnm.arc" (one
// "<id>.brres" per texture animation, 0x1D3 of them); each face type has a two-slot buffer
// (eye, mouth) the current animation is copied into.
// The archive and the buffers live in a file-local object in d_hmn_face_anm_mng.cpp.
// Names other than the class names and the ones d_a_npc already used are inferred.

#include <types.h>
#include <game/game/d_dvd.hpp>
#include <nw4r/g3d.h>

// The face texture animation state of one character: face type (index into the manager's per-type
// texture archives, 13 = none) and the blink timer.
class dHmnFaceAnmMng_c {
public:
    // "/FaceAnm/FaceAnm.arc"
    class arc_c : public dDvd::arcBank_c {
    public:
        virtual ~arc_c() {} // 800B7E98
        virtual void onLoaded(); // 800B7BAC

        void *getAnm(u32 *size, int texId); // 800B7BE8: "<texId>.brres" in the archive (inferred name)
    };

    dHmnFaceAnmMng_c();                                     // 800B76BC: mType = 13
    ~dHmnFaceAnmMng_c();                                    // 800B76C8
    void setType(int type);                                  // 800B7708
    BOOL setTex(int idx, int texId);                        // 800B7710: loads texture anim texId (< 0x1D3) into slot idx
    BOOL setEyeTex(int texId);                              // 800B77D8
    BOOL setMouthTex(int texId);                            // 800B77E4
    BOOL setEyeTexByAnm(int anmId);                         // 800B77F0
    BOOL setMouthTexByAnm(int anmId);                       // 800B7840
    nw4r::g3d::ResAnmTexPat getResAnmTexPat(int idx) const; // 800B78D8 (null for type 13 / nothing loaded)
    nw4r::g3d::ResAnmTexPat getEyeResAnmTexPat() const;     // 800B793C
    nw4r::g3d::ResAnmTexPat getMouthResAnmTexPat() const;   // 800B7944
    void setBlinkCount();                                   // 800B7D60: 1 blink, or 2 (about 1 in 9)
    BOOL calcBlink();                                       // 800B7DAC: counts down; TRUE while a blink is due
    void resetBlink();                                      // 800B7E28: mBlinkTimer = 0, mBlinkCount = 0
    int getType() const { return mType; }

    static u32 getWorkSize(); // 800B7E38
    static BOOL load();       // 800B7E3C

    /* 0x0 */ u32 mBlinkTimer; // frames to the next blink (90 + rnd(270))
    /* 0x4 */ u8 mBlinkCount;
    /* 0x5 */ u8 mType;
}; // size 0x8

extern "C" {
int fn_800B7890(int anmId); // 800B7890: eye texture anim id of a body anim (anmId > 0x1BB -> 0x1D3)
int fn_800B78B4(int anmId); // 800B78B4: mouth texture anim id of a body anim (-> 0x1D3)
int fn_800B7E48(u32 texId); // 800B7E48: mouth kind of a texture anim (< 0x1D3, else 2)
int fn_800B7E4C(u32 texId);  // 800B7E4C: default play mode of a texture anim (else 4)
}
