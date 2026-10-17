#pragma once

// The moray eel: comes out of its rock while the player is near the tank.

#include <game/game/d_fish_museum.hpp>

class dFishMusMoray_c : public dFishMusBase_c {
public:
    enum AnmState_e {
        ANM_GO_IN,
        ANM_HIDDEN,
        ANM_COME_OUT,
        ANM_OUT,
    };

    dFishMusMoray_c();
    virtual ~dFishMusMoray_c();
    virtual BOOL create(nw4r::g3d::ResFile res);
    virtual BOOL execute();
    virtual BOOL draw();
    virtual BOOL doDelete();

    void init();
    void checkPlayer();
    void playAnm();

    /* 0x360 */ int mAnmState; // AnmState_e
    /* 0x364 */ f32 mFrame;
    /* 0x368 */ f32 mLoopEnd;
    /* 0x36C */ f32 mLoopStart;
}; // size 0x370
