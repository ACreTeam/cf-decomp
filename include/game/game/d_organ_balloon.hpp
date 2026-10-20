#ifndef D_ORGAN_BALLOON_HPP
#define D_ORGAN_BALLOON_HPP

// m_3d.hpp, f_base.hpp, d_balloon_string.hpp in this order: the RTTI order of
// dBalloonString_c, fBase_c, m3d::proc_c (reverse class completion order)
#include <game/mLib/m_3d.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_balloon_string.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_vec.hpp>
#include <lib/egg/core/eggFrmHeap.h>

// The balloons of the day (REL d_organ_balloonNP, profile ORGAN_BALLOON, heap
// "dOrganBalloon::m_frmHeap"): three "obj_balloon" models tied to the parent
// actor (Phineas) by three strings, swaying in a shared wind. The colors are
// picked at create from a date-seeded dRandom_c, all different, plus one more
// for the prize balloon (mColor). Class names from RTTI ("dOrganBalloon_c",
// "dOrganBalloonString_c"); member names are inferred.
// Source: src/d_organ_balloonNP/d_organ_balloon.cpp. See
// notes/TUs/d_organ_balloon.txt.

// One balloon's string: the held string of dBalloonString_c with a longer
// segment, its root tangent swinging back and forth, the organ's wind and a
// push away from the other two balloons.
class dOrganBalloonString_c : public dBalloonString_c {
  public:
    enum {
        SWING_PHASE_STEP = 0x100, // mSwingPhase per frame
        ROOT_TILT = -0x2EE0,      // X rotation of the root tangent
    };

    dOrganBalloonString_c() : mIndex(0), mSwingAngle(0), _1D0(0) {
    }

    static dOrganBalloonString_c *create(EGG::Heap *heap) {
        dOrganBalloonString_c *string = new (heap, 4) dOrganBalloonString_c();
        string->mAllocator.attach(heap, 4);
        string->createProc();
        return string;
    }

    virtual const mMtx_c *calcInit(const mMtx_c *mtx); // .text 0x794
    virtual const mMtx_c *calcHold(const mMtx_c *mtx); // .text 0xAE8

    /* 0x1C8 */ int mIndex;       // the balloon's index in the organ
    /* 0x1CC */ mAng mSwingAngle; // yaw of the root tangent
    /* 0x1CE */ mAng mSwingPhase; // advances SWING_PHASE_STEP per frame
    /* 0x1D0 */ int _1D0;         // cleared at construction, otherwise unused
}; // size 0x1D4

class dOrganBalloon_c : public fBase_c {
  public:
    enum {
        BALLOON_NUM = 3,
        COLOR_NUM = 8,          // anmMatClr frames of obj_balloon
        HEAP_SIZE = 0xC00,      // m_frmHeap, for the strings
        RANDOM_SEED = 0x9D,     // replaced by the date seed right away
        RANDOM_SALT = 0xB51,    // dRandom_c::initFromDateWithSalt
        WIND_PHASE_STEP = 0x80, // mWindPhase per frame
        BALLOON_ANGLE_STEP = 0x10000 / BALLOON_NUM, // yaw between the balloons
    };

    virtual int create();   // .text 0xF0
    virtual int doDelete(); // .text 0x540
    virtual int execute();  // .text 0x404
    virtual int draw();     // .text 0x4C0

    void calcMtx(); // .text 0x60C

    /* 0x064 */ mAllocator_c mAllocator;
    /* 0x080 */ EGG::FrmHeap *m_frmHeap;
    /* 0x084 */ dOrganBalloonString_c *mpStrings[BALLOON_NUM];
    /* 0x090 */ m3d::smdl_c mModels[BALLOON_NUM];
    /* 0x0B4 */ m3d::anmMatClr_c mColorAnms[BALLOON_NUM];
    /* 0x138 */ mVec3_c mPos;  // the parent actor's position at create
    /* 0x144 */ mVec3_c mWind; // added to the strings' gravity
    /* 0x150 */ mAng mWindPhase;
    /* 0x154 */ int mColor; // the prize balloon color (anmMatClr frame), random

    static dOrganBalloon_c *s_instance; // .bss 0x8, set at create
}; // size 0x158

#endif
