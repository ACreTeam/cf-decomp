#pragma once

// A cockroach in a player's house room (REL d_a_cockroachNP, profile A_COCKROACH), spawned by dCockroachManager_c.
// It waits, runs to random spots on the floor, jumps away from players and dies when stepped on. Online, the scene
// owner runs it and shares its state and target through the manager's record (dCockroachRecord_c).

#include <game/mLib/m_3d.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_insect_util.hpp>
#include <game/game/d_snd_obj.hpp>

class daCockroachBase_c : public dActor_c {
public:
    daCockroachBase_c();
    virtual ~daCockroachBase_c();
    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();

    void *getResFile();
    void createModel(nw4r::g3d::ResFile *res);
    void setAnm(int anm);
    void setAnmRate(f32 rate);
    void setAnmFrame(f32 frame);
    f32 getAnmFrame();
    bool isWallBumped();
    bool isWallBlocked();
    mVec3_c getWallDir();
    bool isCrushed();
    bool isPlayerNear(f32 radius, int player);
    bool isPlayerIdle(int player);
    bool isSteppedOn();
    f32 getGroundY();
    void setAlpha(u8 alpha);
    bool isRemote();
    mVec3_c getNearestFreePos(const mVec3_c &pos);

    static const f32 sRadius;

    /* 0x0A0 */ dBGCF::acch_c mAcch;
    /* 0x0E8 */ dInsectUtil::playerCheck_c mPlayerCheck;
    /* 0x0F4 */ mVec3_c mTargetPos;
    /* 0x100 */ u16 mTimer;
    /* 0x102 */ u8 mRecordIdx;
    /* 0x104 */ int mAlpha;
    /* 0x108 */ u8 mKillSent;
    /* 0x10C */ int mPlayer; // the player that stepped on it (dCockroachRecord_c::PLAYER_NONE)
    /* 0x110 */ dSndObjSimple_c mSnd;
    /* 0x140 */ m3d::mdl_c mMdl;
    /* 0x180 */ m3d::anmChr_c mAnmChr;
    /* 0x1B8 */ m3d::anmVis_c mAnmVis;
    /* 0x1F0 */ u8 mCheckBg;
    /* 0x1F1 */ u8 mFurnitureFrames;
    /* 0x1F2 */ u8 mOutsideFrames;
    /* 0x1F3 */ u8 mOwner;
}; // size 0x1F4

class daCockroach_c : public daCockroachBase_c {
public:
    enum State_e {
        STATE_WAIT,
        STATE_RUN,
        STATE_JUMP,
        STATE_STEPPED, // stepped on by this console's player
        STATE_KILLED,  // killed by another console's player
        STATE_NUM,
    };

    enum Anm_e {
        ANM_WAIT,
        ANM_FLY,
        ANM_DIE00,
        ANM_DIE01,
        ANM_NUM,
    };

    typedef void (daCockroach_c::*StateFunc)();

    daCockroach_c();
    virtual ~daCockroach_c();
    virtual int create();
    virtual int execute();
    virtual int draw();

    void initWait();
    void initRun();
    void initJump();
    void initStepped();
    void initKilled();
    void executeWait();
    void executeRun();
    void executeJump();
    void executeStepped();
    void executeKilled();
    void checkState();
    void changeState(int state);
    mVec3_c calcTargetPos();
    void setTarget();

    /* 0x1F4 */ int mState;
    /* 0x1F8 */ int mPrevState;
    /* 0x1FC */ StateFunc mStateFunc;
    /* 0x208 */ u16 mWaitTime;
    /* 0x20A */ u16 mRunTime;
    /* 0x20C */ u16 mRunLimit;
    /* 0x20E */ u16 mRunFrames;
    /* 0x210 */ f32 mAnmRate;
    /* 0x214 */ u16 _214;
    /* 0x216 */ u16 mWallFrames;
    /* 0x218 */ s16 mPrevAngle;
    /* 0x21A */ s16 mTargetSpread;
}; // size 0x21C
