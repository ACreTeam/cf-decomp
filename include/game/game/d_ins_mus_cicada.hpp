#pragma once

// The museum's cicadas and lantern fly (dInsMusCicada_c).

#include <game/game/d_insect_museum.hpp>

struct dInsMusCicadaParam_c {
    /* 0x00 */ Vec mOffset;
    /* 0x0C */ f32 mAnmRate;
    /* 0x10 */ f32 mShadowSize;
    /* 0x14 */ f32 mScareRadius;
    /* 0x18 */ int mScareLimit;
}; // size 0x1C

class dInsMusCicada_c : public dInsMusBase_c {
public:
    typedef void (dInsMusCicada_c::*StateFunc)();

    enum {
        FLEE_END_FRAME = 132,
        RETURN_START_FRAME = 258,
        RETURN_END_FRAME = 349,
    };

    dInsMusCicada_c();
    virtual ~dInsMusCicada_c();
    virtual void calcMtx();
    virtual void init();
    virtual void initState();
    virtual void executeState();

    void initWait();
    void initFlee();
    void initReturn();
    void stateWait();
    void stateFlee();
    void stateReturn();
    bool isLanternFly();

    /* 0x210 */ StateFunc mState;
}; // size 0x21C
