#pragma once

// dInsMusSpider_c: the bagworm and the spider of the museum's insect rooms, dropping down on their thread.

#include <game/game/d_insect_museum.hpp>

struct dInsMusSpiderParam_c {
    /* 0x00 */ Vec mOffset;
    /* 0x0C */ f32 mHangTimeMax;
    /* 0x10 */ f32 mHangTimeMin;
    /* 0x14 */ f32 mWaitTimeMax;
    /* 0x18 */ f32 mWaitTimeMin;
    /* 0x1C */ f32 mAnmRate;
    /* 0x20 */ f32 mShadowSize;
    /* 0x24 */ f32 mNearRadius;
}; // size 0x28

class dInsMusSpider_c : public dInsMusBase_c {
public:
    typedef void (dInsMusSpider_c::*StateFunc)();

    enum {
        DESCEND_END_FRAME = 60,
        HANG_START_FRAME = 61,
        HANG_END_FRAME = 120,
        ASCEND_START_FRAME = 121,
        ASCEND_END_FRAME = 179,
    };

    dInsMusSpider_c();
    virtual ~dInsMusSpider_c();
    virtual BOOL draw();
    virtual void init();
    virtual void initState();
    virtual void executeState();

    void initWait();
    void initDescend();
    void initHang();
    void initAscend();
    void stateWait();
    void stateDescend();
    void stateHang();
    void stateAscend();

    /* 0x210 */ StateFunc mState;
    /* 0x21C */ u16 mHangTime;
    /* 0x21E */ u16 mWaitTime;
}; // size 0x220
