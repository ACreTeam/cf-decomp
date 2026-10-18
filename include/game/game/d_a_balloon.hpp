#ifndef GAME_D_A_BALLOON_HPP
#define GAME_D_A_BALLOON_HPP

// The present balloons (REL d_a_balloonNP): dBalloonMng_c (profile BALLOON_MNG)
// sends one over the town now and then; daBalloon_c (profile BALLOON) drifts
// along a route with one present balloon, or three balloons carrying the golden
// slingshot, until the slingshot pops them. RTTI gives the class names
// (daBalloon_c, dBalloonMng_c, dBalloonMng_HIO_c); everything else is inferred.

#include <game/game/d_effect.hpp>
#include <game/game/d_fltg.hpp>
#include <game/game/d_line_draw.hpp>
#include <game/game/d_objc.hpp>
#include <game/game/d_snd_obj.hpp>
#include <game/mLib/m_3d.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_vec.hpp>

class daBalloon_c : public dFltg::actor_c {
  public:
    enum STATE_e {
        STATE_WAIT,  // one balloon
        STATE_POP,   // the balloon was shot: the present falls
        STATE_GOLD3, // three balloons left
        STATE_GOLD2,
        STATE_GOLD1,
        STATE_GOLD0, // all popped: the golden slingshot falls
        STATE_NUM,
    };

    enum { BALLOON_MAX = 3 };

    typedef BOOL (daBalloon_c::*InitFunc)();
    typedef void (daBalloon_c::*ExecFunc)();

    daBalloon_c();          // .text:0x6BC
    virtual ~daBalloon_c(); // .text:0x994

    virtual int create();   // .text:0xB04
    virtual int execute();  // .text:0xCE8
    virtual int draw();     // .text:0xD5C
    virtual int doDelete(); // .text:0xF50

    virtual int getKind() {
        return 0;
    }
    virtual int getRaccoIdx() {
        return mRaccoIdx;
    }

    static void *operator new(size_t size); // .text:0xFF8: the static buffer
    static void operator delete(void *mem); // .text:0x1044

    BOOL setState(int state); // .text:0x1048
    void executeState();      // .text:0x10B4

    BOOL initWait();     // .text:0x10F4
    void executeWait();  // .text:0x10FC
    BOOL initPop();      // .text:0x1114
    void executePop();   // .text:0x111C
    BOOL initGold3();    // .text:0x1234
    void executeGold3(); // .text:0x123C
    void calcGold3();    // .text:0x12B8
    BOOL initGold2();    // .text:0x139C
    void executeGold2(); // .text:0x1570
    BOOL getLeftPair(
        int *left,
        int *right);     // .text:0x15D4: the two balloons left, left to right
    void calcGold2();    // .text:0x1688
    BOOL initGold1();    // .text:0x1A9C
    void executeGold1(); // .text:0x1C08
    BOOL getLastOne(int *idx); // .text:0x1C54
    void calcGold1();          // .text:0x1CB0
    BOOL initGold0();          // .text:0x1EC0
    void executeGold0();       // .text:0x1EC8
    void checkPop(); // .text:0x1F4C: pops the balloons the slingshot hit
    void setPopColor(int color);                             // .text:0x2098
    mVec3_c *getNodePos(m3d::bmdl_c *mdl, const char *node); // .text:0x22F8
    void initOne();                                          // .text:0x23C4
    void initThree();                                        // .text:0x24EC
    void calcMove();                                         // .text:0x2774
    void calcSway();                                         // .text:0x29D8
    void calcSwayGold();                                     // .text:0x2B2C
    void calcBob();                                          // .text:0x2C7C
    void calcGround(); // .text:0x2D8C: climbs over the ground ahead
    void calcMtx();    // .text:0x2EDC
    bool isRouteEnd(); // .text:0x3008
    bool isFar();      // .text:0x304C: not drawn

    static daBalloon_c *s_instance;

    /* 0x0D0 */ int mState;
    /* 0x0D4 */ m3d::smdl_c mMdl[BALLOON_MAX];
    /* 0x0F8 */ m3d::anmMatClr_c mAnmClr; // the color of the single balloon
    /* 0x124 */ dObjc::acpos_c mObjc[BALLOON_MAX];
    /* 0x214 */ dEffect_c mPopEffect;
    /* 0x290 */ dEffect_c mPopColorEffect;
    /* 0x30C */ mVec3_c mBalloonPos[BALLOON_MAX];
    /* 0x330 */ mVec3_c mBalloonTarget[BALLOON_MAX];
    /* 0x354 */ mVec3_c mHitPos[BALLOON_MAX];
    /* 0x378 */ mVec3_c mHitTarget[BALLOON_MAX];
    /* 0x39C */ u8 mGold;
    /* 0x3A0 */ int mBalloonNum;
    /* 0x3A4 */ mVec3_c mKnotPos; // where the strings meet
    /* 0x3B0 */ mVec3_c mItemPos; // the present / slingshot below
    /* 0x3BC */ mVec3_c mBalloonStep[BALLOON_MAX];
    /* 0x3E0 */ mAng mSwayXAngle;
    /* 0x3E2 */ s16 mSwayPhase[BALLOON_MAX];
    /* 0x3E8 */ mAng mSwayZAngle;
    /* 0x3EA */ mAng mBobAngle;
    /* 0x3EC */ f32 mFallSpeed;
    /* 0x3F0 */ mAng3_c mBalloonAngle[BALLOON_MAX];
    /* 0x404 */ mVec3_c mPivot;
    /* 0x410 */ s16 mBalloonTargetAngle[BALLOON_MAX];
    /* 0x418 */ f32 mEndX; // the route's end
    /* 0x41C */ int mRaccoIdx;
    /* 0x420 */ int mLeftNum;
    /* 0x424 */ u8 mLeft[BALLOON_MAX];
    /* 0x428 */ dLineDrawProc_c mLines; // the strings
    /* 0x460 */ mVec3_c mStringPos;
    /* 0x46C */ s16 mStringAngle[BALLOON_MAX];
    /* 0x472 */ s16 mStringTargetAngle;
    /* 0x474 */ u8 _474;
}; // size 0x478

class dBalloonMng_c : public fBase_c {
  public:
    dBalloonMng_c();          // .text:0xFC
    virtual ~dBalloonMng_c(); // .text:0x168

    virtual int create();   // .text:0x1D8
    virtual int execute();  // .text:0x288
    virtual int draw();     // .text:0x2AC
    virtual int doDelete(); // .text:0x2B4

    void calc();                   // .text:0x304: sends a balloon now and then
    BOOL updateMinute(int minute); // .text:0x4F0: TRUE when the minute changed

    static dBalloonMng_c *s_instance;

    /* 0x064 */ mAllocator_c mAllocator; // the balloons' models
    /* 0x080 */ EGG::FrmHeap *m_frmHeap_p;
    /* 0x084 */ u8 mColor;   // the next balloon's color, 0..7
    /* 0x088 */ f32 mChance; // of a balloon at the next check (+1/8 per miss)
    /* 0x08C */ int mMinute; // the last checked minute
    /* 0x090 */ u8 mSent;    // a balloon was sent
    /* 0x091 */ u8 mGold;    // the next balloon carries the golden slingshot
}; // size 0x94

#endif
