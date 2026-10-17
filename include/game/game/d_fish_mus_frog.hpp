#pragma once

// The frog: sits on the lotus leaf and croaks.

#include <game/game/d_fish_mus_lotus.hpp>

class dFishMusFrog_c : public dFishMusBase_c {
public:
    typedef void (dFishMusFrog_c::*StateFunc)();

    dFishMusFrog_c();
    virtual ~dFishMusFrog_c();
    virtual BOOL create(nw4r::g3d::ResFile res);
    virtual BOOL execute();
    virtual BOOL draw();
    virtual BOOL doDelete();

    void init();
    void initWait();
    void stateWait();
    void initCroak();
    void stateCroak();
    void setWaitTimer();
    void setCroakCount();
    bool isCroakDone() const;

    /* 0x360 */ StateFunc mState;
    /* 0x36C */ dSndObjSimple_c mSound;
    /* 0x39C */ dFishMusObjLotus_c mLotus;
    /* 0x430 */ f32 mFrame;
    /* 0x434 */ f32 mCroakEndFrame;
}; // size 0x438

namespace dFishMuseum {

extern const f32 sCroakFrames[2]; // unreferenced

} // namespace dFishMuseum
