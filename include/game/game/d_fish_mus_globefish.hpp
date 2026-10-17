#pragma once

// The ocean sunfish: crosses the tank back and forth.

#include <game/game/d_fish_mus_common_move.hpp>

class dFishMusGlobefish_c : public dFishMusCommonMove_c {
public:
    typedef void (dFishMusGlobefish_c::*StateFunc)();

    dFishMusGlobefish_c();
    virtual ~dFishMusGlobefish_c();
    virtual BOOL create(nw4r::g3d::ResFile res);
    virtual BOOL execute();
    virtual BOOL doDelete();
    virtual void initSwim();

    void init();
    void stateSwim();

    /* 0x418 */ StateFunc mState;
    /* 0x424 */ u16 mCrossCount; // even: swimming to mPoint0Pos, odd: to mPoint5Pos
}; // size 0x428
