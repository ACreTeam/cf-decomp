#pragma once

// The flea (INSECT_FLEA): shaken out of a villager by the player's net
// (dInsectField_c::addFlea). It jumps off the spot and fades out.

#include <game/game/d_insect_field.hpp>

class dInsFldFlea_c : public dInsFldBase_c {
  public:
    typedef void (dInsFldFlea_c::*StateFunc)();

    enum State_e {
        STATE_WAIT = 0,
        STATE_JUMP = 1,
    };

    // The per-type parameters (.data 0x1590, one entry).
    struct param_c {
        /* 0x00 */ f32 mOffsetX; // setGroundPos: from the net position
        /* 0x04 */ f32 mOffsetY; // above the ground
        /* 0x08 */ f32 mOffsetZ;
        /* 0x0C */ f32 mJumpSpeed; // forward
        /* 0x10 */ f32 mJumpSpeedY;
        /* 0x14 */ f32 mGravity;
        /* 0x18 */ f32 _18;
        /* 0x1C */ f32 _1C;
    }; // size 0x20

    static param_c sParam[1]; // .data: one per type
    static const StateFunc sExecute[];
    static const StateFunc sInit[];

    static int getParamIdx(int type);             // 0x1E478
    static const param_c *getFleaParam(int type); // 0x1E48C

    dInsFldFlea_c();                                    // 0x1E4BC
    virtual ~dInsFldFlea_c();                           // 0x1E500
    virtual BOOL create(const nw4r::g3d::ResFile &res); // 0x1E558: sFlea = this
    virtual BOOL doDelete();                            // 0x1E598: sFlea = NULL
    virtual void setGroundPos();                        // 0x1E608
    virtual void initState();                           // 0x1E678
    virtual void executeState();                        // 0x1E6B0
    virtual void onRelease();                           // 0x1E734: = initJump

    void setState(int state); // 0x1E5D0
    void initWait();          // 0x1E6E8
    void executeWait();       // 0x1E7D4
    void executeJump();       // 0x1E7D8

    /* 0x318 */ int mState; // State_e
}; // size 0x31C
