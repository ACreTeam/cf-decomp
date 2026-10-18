#pragma once

// The ant (INSECT_ANT): sits on a spoiled turnip or a candy lying on the
// ground and fades out once that item is gone.

#include <game/game/d_insect_field.hpp>
#include <game/game/d_field_assessment.hpp>

class dInsFldAnt_c : public dInsFldBase_c {
  public:
    typedef void (dInsFldAnt_c::*StateFunc)();

    enum State_e {
        STATE_WAIT = 0,    // on the item
        STATE_FADE = 1,    // the item is gone: fade out
        STATE_RELEASE = 2, // released from the net
    };

    // The per-type parameters (.data 0x13A8, one entry).
    struct param_c {
        /* 0x00 */ f32 mOffsetX; // setGroundPos
        /* 0x04 */ f32 mOffsetY; // above the ground
        /* 0x08 */ f32 mOffsetZ;
        /* 0x0C */ f32 mGravity; // executeRelease
        /* 0x10 */ f32 mSpeed;
        /* 0x14 */ f32 mReleaseAnmRate;
        /* 0x18 */ f32 mShadowSize;
        /* 0x1C */ f32 _1C;
        /* 0x20 */ u8 mFadeSpeed; // alpha per frame (STATE_FADE)
    }; // size 0x24

    static param_c sParam[1]; // .data: one per type
    static const StateFunc sExecute[];
    static const StateFunc sInit[];
    static const int sTypes[];

    static int getParamIdx(int type);            // 0x17864
    static const param_c *getAntParam(int type); // 0x17884

    dInsFldAnt_c();                                     // 0x178B4
    virtual ~dInsFldAnt_c();                            // 0x17934
    virtual BOOL create(const nw4r::g3d::ResFile &res); // 0x1799C
    virtual int draw();                                 // 0x17ACC
    virtual BOOL doDelete();                            // 0x17AF0
    void createAntMdl(const nw4r::g3d::ResFile
                          &res); // 0x17B6C: createMdl with a tex srt buffer
    void setState(int state);    // 0x17CC0
    virtual void setGroundPos(); // 0x17CF8
    virtual void initState();    // 0x17D68
    virtual void executeState(); // 0x17D70
    void initWait();             // 0x17DD4
    void initFade();             // 0x17E2C
    virtual void onRelease();    // 0x17EB0: = initRelease
    void executeWait();          // 0x17F3C
    void executeFade();          // 0x17FA4
    void executeRelease();       // 0x1800C
    void setAnmTexSrt(const nw4r::g3d::ResAnmTexSrt &anm); // 0x18168
    BOOL isOnBadTurnip() const;                            // 0x181D4
    BOOL isOnCandy() const;                                // 0x18230
    void setMatAlpha(u8 alpha); // 0x1828C: TEV KColor 1 alpha of material "m0"

    static int sAntNum; // .bss 0x18

    /* 0x318 */ int mState; // State_e
    /* 0x31C */ m3d::anmTexSrt_c mAnmTexSrt;
    /* 0x348 */ nw4r::g3d::ResAnmTexSrt mAnmTexSrtWait;
    /* 0x34C */ bool mAnmTexSrtCreated;
}; // size 0x350
