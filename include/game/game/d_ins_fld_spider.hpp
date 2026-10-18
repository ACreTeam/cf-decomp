#pragma once

// The spider and the bagworm (INSECT_SPIDER, INSECT_BAGWORM): hide in a tree
// and drop down on a thread when a player shakes it, hang for a while and
// climb back up. They fall off if the tree goes away.

#include <game/game/d_insect_field.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_field_info.hpp>

class dInsFldSpider_c : public dInsFldBase_c {
  public:
    typedef void (dInsFldSpider_c::*StateFunc)();

    enum State_e {
        STATE_HIDE = 0, // in the tree
        STATE_DROP = 1,
        STATE_HANG = 2,
        STATE_CLIMB = 3,
        STATE_FALL = 4,    // the tree is gone
        STATE_RELEASE = 5, // released from the net
    };

    // Frames of the "wait" anim (drop, hang loop, climb) and timers.
    enum {
        DESCEND_END_FRAME = 60, // STATE_DROP
        HANG_START_FRAME = 61,  // STATE_HANG loops these
        HANG_END_FRAME = 120,
        ASCEND_START_FRAME = 121, // STATE_CLIMB
        ASCEND_END_FRAME = 179,
        CLIMB_WAIT_TIME = 32, // STATE_CLIMB rises after this many frames
        FALL_FADE_SPEED = 10, // STATE_FALL: alpha per frame
    };

    // The per-type parameters (.data 0x16A8: spider, bagworm).
    struct param_c {
        /* 0x00 */ f32 mOffsetX; // setGroundPos; initDrop: to the side, away
                                 // from the player
        /* 0x04 */ f32 mOffsetY; // above the ground
        /* 0x08 */ f32 mOffsetZ;
        /* 0x0C */ u8 mShakeFrames;  // after the tree is shaken
        /* 0x10 */ f32 mHangTimeMax; // seconds
        /* 0x14 */ f32 mHangTimeMin;
        /* 0x18 */ f32 mAnmRate;
        /* 0x1C */ f32 mGravity;      // released
        /* 0x20 */ f32 mReleaseSpeed; // the spider jumps off
        /* 0x24 */ f32 mReleaseAnmRate;
        /* 0x28 */ f32 _28;
        /* 0x2C */ f32 _2C;
    }; // size 0x30

    static param_c sParam[2]; // .data: one per type
    static const StateFunc sExecute[];
    static const StateFunc sInit[];

    static int getParamIdx(int type);               // 0x215F4
    static const param_c *getSpiderParam(int type); // 0x21628
    static bool isHangTree(const dItem::Item *item,
                           const mVec3_c *pos); // 0x21658: itemSearch_c filter

    dInsFldSpider_c();           // 0x21704
    virtual ~dInsFldSpider_c();  // 0x2177C
    virtual int execute();       // 0x217E4: searches its tree first
    virtual int draw();          // 0x2182C: not while hidden
    void setState(int state);    // 0x2185C
    virtual void setGroundPos(); // 0x21894
    virtual void initState();    // 0x21914
    virtual void executeState(); // 0x2191C
    void initHide();             // 0x219AC
    void initDrop();             // 0x21A3C
    void initHang();             // 0x21AFC
    void initClimb();            // 0x21BB4
    void initFall();             // 0x21C48
    virtual void onRelease();    // 0x21CDC: = initRelease
    void executeHide();          // 0x21DB8
    void executeDrop();          // 0x21E98
    void executeHang();          // 0x21F54
    void executeClimb();         // 0x21FE4
    // 0x22098 (its trailing dead blr is dtk's fn_116_220B8)
    void executeFall();
    void executeRelease();        // 0x220BC
    f32 getDropOffset();          // 0x22104
    bool isOnTree();              // 0x221A8
    void executeReleaseSpider();  // 0x22254
    void executeReleaseBagworm(); // 0x223C0
    bool isSpider();              // 0x224F8

    /* 0x318 */ int mState; // State_e
    /* 0x31C */ dInsectUtil::itemSearch_c mSearch;
    /* 0x330 */ mVec3_c mHangPos; // its catch position (on the thread)
    /* 0x33C */ bool mShaken;
    /* 0x33E */ u16 mHangFrames;
}; // size 0x340
