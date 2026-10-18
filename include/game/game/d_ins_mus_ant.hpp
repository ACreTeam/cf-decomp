#pragma once

// dInsMusAnt_c: the ant line of the museum's insect rooms.

#include <game/game/d_insect_museum.hpp>

struct dInsMusAntParam_c {
    /* 0x00 */ Vec mOffset;
}; // size 0xC

class dInsMusAnt_c : public dInsMusBase_c {
public:
    typedef void (dInsMusAnt_c::*StateFunc)();

    dInsMusAnt_c();
    virtual ~dInsMusAnt_c();
    virtual BOOL doDelete();
    virtual void createModel(nw4r::g3d::ResFile &res);
    virtual void init();
    virtual void initState();
    virtual void executeState();

    void initWait();
    void stateWait();

    /* 0x210 */ m3d::anmTexPat_c mAnmTexPat;
    /* 0x23C */ m3d::anmTexSrt_c mAnmTexSrt;
    /* 0x268 */ StateFunc mState;
}; // size 0x274
