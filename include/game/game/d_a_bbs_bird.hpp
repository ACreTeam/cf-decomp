#ifndef GAME_D_A_BBS_BIRD_HPP
#define GAME_D_A_BBS_BIRD_HPP

// The bulletin board's pigeon (REL d_a_bbs_birdNP, profile A_BBS_BIRD; the
// "midokudori", unread-notice bird). It sits on the board while a notice is
// unread, pecks, walks and turns along it, and flies away when the player comes
// close. RTTI gives the class names (daBird_c, daBbsBird_c); everything else is
// inferred.

#include <game/mLib/m_3d.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_snd_obj.hpp>
#include <game/mLib/m_mtx.hpp>
#include <lib/nw4r/g3d/res/g3d_resfile.h>

// A bird actor's model and animation.
class daBird_c : public dActor_c {
  public:
    /* 0x0A0 */ m3d::mdl_c mMdl;
    /* 0x0E0 */ m3d::anmChr_c mAnm;
    /* 0x118 */ nw4r::g3d::ResFile mRes; // dSTR's "obj_bbs_bird"
}; // size 0x11C

class daBbsBird_c : public daBird_c {
  public:
    enum STATE_e {
        STATE_HIDE,   // no unread notice: checks again once the camera is away
        STATE_RETURN, // back on the board after flying away
        STATE_WAIT,   // pecks now and then
        STATE_WALK,   // along the board
        STATE_TURN,
        STATE_NOTICE, // the player came close: turns away
        STATE_FLY,    // flies away
        STATE_NUM,
    };

    enum ANM_e {
        ANM_MOVE,
        ANM_PICK,
        ANM_FLY,
        ANM_NUM,
    };

    typedef BOOL (daBbsBird_c::*InitFunc)();
    typedef void (daBbsBird_c::*ExecFunc)();

    virtual int create();   // .text:0x598
    virtual int doDelete(); // .text:0x7B8
    virtual int execute();  // .text:0x65C
    virtual int draw();     // .text:0x6C8

    static BOOL isNight(); // .text:0x178: before 6:00 or from 18:00
    BOOL loadModel();      // .text:0x210
    BOOL setAnm(u32 anm, m3d::playMode_e mode,
                bool end);      // .text:0x334: end = from the last frame
    BOOL playAnm();             // .text:0x424: TRUE when it ended
    f32 getFrame() const;       // .text:0x494
    void entryMdl(mMtx_c *mtx); // .text:0x4E0
    void removeMdl();           // .text:0x530
    void drawShadow();          // .text:0x7E8
    void drawEtc();             // .text:0x89C: empty
    void checkPlayer();         // .text:0x8A0: too close: STATE_NOTICE
    void checkNotice();         // .text:0x944: the state for the notices
    BOOL isFacingSide() const;  // .text:0x9C8: along the board
    BOOL chance(u32 percent);   // .text:0xA48
    void calcMtx(mMtx_c *mtx);  // .text:0xA88
    BOOL setState(int state);   // .text:0xB74
    void executeState();        // .text:0xBE0

    BOOL initHide();      // .text:0xC20
    void executeHide();   // .text:0xC28
    BOOL initReturn();    // .text:0xCE8
    void executeReturn(); // .text:0xCF8
    BOOL initWait();      // .text:0xD8C
    void executeWait();   // .text:0xE70
    BOOL initWalk();      // .text:0x100C
    void executeWalk();   // .text:0x1098
    BOOL initTurn();      // .text:0x11A8
    void executeTurn();   // .text:0x12EC
    BOOL initNotice();    // .text:0x13F0
    void executeNotice(); // .text:0x147C
    BOOL initFly();       // .text:0x14DC
    void executeFly();    // .text:0x1550

    void setPerchPos();

    static daBbsBird_c *s_instance;
    static u8
        s_strDrawn; // set by d_a_str_field each frame it draws the structures

    /* 0x11C */ dBGCF::acch_c mAcch;
    /* 0x164 */ f32 mTargetX; // STATE_WALK
    /* 0x168 */ f32 mMinX;
    /* 0x16C */ f32 mMaxX;
    /* 0x170 */ u8 mTurnPlus; // the last turn's direction
    /* 0x172 */ s16 mTargetAngle;
    /* 0x174 */ s16 mTurnStep;
    /* 0x178 */ u32 mTimer;
    /* 0x17C */ u8 mPecking;
    /* 0x17D */ u8 mFlewAway;
    /* 0x180 */ mVec3_c mHomePos;
    /* 0x18C */ dSndObjSimple_c mSnd;
    /* 0x1BC */ u8 _1BC[4]; // unused
    /* 0x1C0 */ int mState;
}; // size 0x1C4

#endif
