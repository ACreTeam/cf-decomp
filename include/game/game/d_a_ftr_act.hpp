#pragma once

// The furniture actors of d_a_ftr_act, all derived from daFTR_c.

#include <game/game/d_a_ftr.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_scene.hpp>

class daFtrLp_c : public daFTR_c {
public:
    enum {
        SWITCH_OFF,
        SWITCH_ON,
    };

    virtual BOOL initModel();
    virtual BOOL executeMain();
    virtual BOOL deleteRes();
    virtual BOOL setSwitch(int sw, int param);
    virtual int getSwitchBit(int sw);

    void executeSwitch();
    BOOL initOff();
    void executeOff();
    BOOL initOn();
    void executeOn();

    /* 0x8CC */ u8 mSwitchState;
}; // size 0x8D0

class daFtrTrg_c : public daFTR_c {
public:
    enum {
        SWITCH_OFF,
        SWITCH_ON,
    };

    enum {
        AUTO_TRIGGER_WAIT = 90, // a display copy triggers itself after this many frames
        AUTO_TRIGGER_WAIT_BAG = 10,
        TRIGGER_TIME = 40, // set when triggered; not counted down
    };

    virtual BOOL initModel();
    virtual BOOL executeMain();
    virtual BOOL deleteRes();
    virtual BOOL setSwitch(int sw, int param);

    void executeSwitch();
    BOOL initOff();
    void executeOff();
    BOOL initOn();
    void executeOn();

    /* 0x8CC */ dFTR::counter_c mTimer;
    /* 0x8D0 */ u8 mSwitchState;
}; // size 0x8D4

class daFtrSw_c : public daFTR_c {
public:
    enum {
        SWITCH_OFF,
        SWITCH_ON,
    };

    virtual BOOL initModel();
    virtual BOOL executeMain();
    virtual BOOL deleteRes();
    virtual BOOL setSwitch(int sw, int param);
    virtual int getSwitchBit(int sw);

    void executeSwitch();
    BOOL initOff();
    void executeOff();
    BOOL initOn();
    void executeOn();

    /* 0x8CC */ u8 mSwitchState;
    /* 0x8D0 */ lampList_c mLamps;
}; // size 0x964

class daFtrSw2_c : public daFTR_c {
public:
    enum {
        SWITCH_OFF,
        SWITCH_ON,
        SWITCH_STOP,
    };

    virtual BOOL initModel();
    virtual BOOL executeMain();
    virtual BOOL deleteRes();
    virtual BOOL setSwitch(int sw, int param);
    virtual int getSwitchBit(int sw);

    void executeSwitch();
    BOOL initOff();
    void executeOff();
    BOOL initOn();
    void executeOn();
    BOOL initStop();
    void executeStop();

    /* 0x8CC */ u8 mSwitchState;
    /* 0x8D0 */ lampList_c mLamps;
    /* 0x964 */ f32 mAnmRate;
}; // size 0x968

class daFtrFlp_c : public daFTR_c {
public:
    enum {
        SWITCH_OFF,
        SWITCH_TURN_ON,
        SWITCH_ON,
        SWITCH_TURN_OFF,
    };

    enum {
        AUTO_ON_WAIT = 60, // a display copy switches by itself after this many frames
        AUTO_OFF_WAIT = 30,
    };

    virtual BOOL initModel();
    virtual BOOL executeMain();
    virtual BOOL deleteRes();
    virtual BOOL setSwitch(int sw, int param);
    virtual int getSwitchBit(int sw);

    void executeSwitch();
    BOOL initOff();
    void executeOff();
    BOOL initTurnOn();
    void executeTurnOn();
    BOOL initOn();
    void executeOn();
    BOOL initTurnOff();
    void executeTurnOff();

    /* 0x8CC */ dFTR::counter_c mTimer;
    /* 0x8D0 */ u8 mSwitchState;
}; // size 0x8D4

class daFtr2W_c : public daFTR_c {
public:
    enum {
        SWITCH_OFF,
        SWITCH_TURN_ON,
        SWITCH_ON,
        SWITCH_TURN_OFF,
    };

    enum {
        AUTO_SWITCH_WAIT = 60, // a display copy switches by itself after this many frames
    };

    virtual BOOL initModel();
    virtual BOOL executeMain();
    virtual BOOL deleteRes();
    virtual BOOL setSwitch(int sw, int param);
    virtual int getSwitchBit(int sw);

    void executeSwitch();
    BOOL initOff();
    void executeOff();
    BOOL initTurnOn();
    void executeTurnOn();
    BOOL initOn();
    void executeOn();
    BOOL initTurnOff();
    void executeTurnOff();

    /* 0x8CC */ u8 _8CC[0x8D0 - 0x8CC];
    /* 0x8D0 */ dFTR::counter_c mTimer;
    /* 0x8D4 */ u8 mSwitchState;
}; // size 0x8D8

class daFtrFsw_c : public daFTR_c {
public:
    enum {
        SWITCH_OFF,
        SWITCH_TURN_ON,
        SWITCH_ON,
        SWITCH_TURN_OFF,
    };

    enum {
        AUTO_SWITCH_WAIT = 60, // a display copy switches by itself after this many frames
    };

    virtual BOOL initModel();
    virtual BOOL executeMain();
    virtual BOOL deleteRes();
    virtual BOOL setSwitch(int sw, int param);
    virtual int getSwitchBit(int sw);

    void executeSwitch();
    BOOL initOff();
    void executeOff();
    BOOL initTurnOn();
    void executeTurnOn();
    BOOL initOn();
    void executeOn();
    BOOL initTurnOff();
    void executeTurnOff();

    /* 0x8CC */ dFTR::counter_c mTimer;
    /* 0x8D0 */ u8 mSwitchState;
}; // size 0x8D4

class daFtrFaw_c : public daFTR_c {
public:
    enum {
        SWITCH_OFF,
        SWITCH_ON,
    };

    enum {
        AUTO_ON_WAIT = 120, // a display copy switches by itself after this many frames
        AUTO_ON_WAIT_TOILET = 90,
        AUTO_OFF_WAIT = 120,
        TOILET_ON_TIME = 120, // the toilet switches itself off after flushing this long
    };

    virtual BOOL initModel();
    virtual BOOL executeMain();
    virtual BOOL executeIdle();
    virtual BOOL deleteRes();
    virtual BOOL setSwitch(int sw, int param);
    virtual int getSwitchBit(int sw);

    void executeSwitch();
    BOOL initOff();
    void executeOff();
    BOOL initOn();
    void executeOn();

    /* 0x8CC */ dFTR::counter_c mOnTimer;
    /* 0x8D0 */ dFTR::counter_c mTimer;
    /* 0x8D4 */ u8 mSwitchState;
    /* 0x8D8 */ lampList_c mLamps;
}; // size 0x96C

class daFtrChair_c : public daFTR_c {
public:
    virtual BOOL initModel();
    virtual BOOL executeMain();
    virtual BOOL deleteRes();

    void executeSit();
    void getSeatPos(mVec3_c *out, const dFTR::hold_c *hold);

    static void sit(const dFTR::hold_c *hold, int type) {
        f32 x = hold->mPushPos.x;
        f32 z = hold->mPushPos.z;
        dPlayerMgr::sitOnChair(hold->mPushAngle, type, x, z);
    }

    /* 0x8CC */ u16 mSitTimer;
}; // size 0x8D0

class daFtrBed_c : public daFTR_c {
public:
    typedef dFTR::unitList_c::unit_c unit_c;

    struct unitPair_c {
        unit_c *get(u32 i) { return &mUnit[i & 1]; }

        /* 0x00 */ unit_c mUnit[2];
    }; // size 0x10

    enum {
        BED_OFF,
        BED_LOFT,
        BED_SPEAK,
        BED_TALK,
        BED_CANCEL,
        BED_SELECT,
        BED_MENU,
        BED_REPLACE,
        BED_FINISH,
        BED_STATE_NUM,
    };

    // sys_MESS/SYS_Player.
    enum {
        MSG_SWAP_BED = 0x15, // "Swap this for another bed?"
    };

    // MSG_SWAP_BED choices.
    enum {
        ANSWER_SWAP,
        ANSWER_KEEP,
    };

    virtual void onDemoCmd(int cmd, u8 member);
    virtual BOOL initModel();
    virtual BOOL executeMain();
    virtual BOOL deleteRes();
    virtual BOOL setSwitch(int sw, int param);
    virtual void onAnswer(int arg);

    static BOOL canWalk(const unit_c &unit);
    static void lie(const dFTR::hold_c *hold) {
        f32 x = hold->mCenterPos.x;
        f32 z = hold->mCenterPos.z;
        s16 angle = hold->mPushAngle;
        int type = dPlayerMgr::LIE_FROM_FRONT;
        if (hold->mSide == dFTR::SIDE_BACK) {
            type = dPlayerMgr::LIE_FROM_BACK;
        }
        dPlayerMgr::lieOnBed(angle, type, x, z, 0);
    }

    u8 isPlayerOn() const { return mPlayerOn; }
    void executeLie();
    u32 getHeadUnits(unitPair_c *units);
    unit_c getNearUnit(const mVec3_c *pos);
    u32 getSideUnits(unitPair_c *units);
    int checkSeat(const mVec3_c *pos, mAng angle, const mVec3_c *ofs, mVec3_c *out);
    void calcSeatPos(mVec3_c *out, const mVec3_c *pos, mAng angle, const mVec3_c *ofs);
    void executeSwitch();

    BOOL initOff();
    void executeOff();
    BOOL initLoft();
    void executeLoft();
    BOOL initSpeak();
    void executeSpeak();
    BOOL initTalk();
    void executeTalk();
    BOOL initCancel();
    void executeCancel();
    BOOL initSelect();
    void executeSelect();
    BOOL initMenu();
    void executeMenu();
    BOOL initReplace();
    void executeReplace();
    BOOL initFinish();
    void executeFinish();

    /* 0x8CC */ u16 mLieTimer;
    /* 0x8CE */ u8 _8CE[0x8D0 - 0x8CE];
    /* 0x8D0 */ u16 mFinishTimer;
    /* 0x8D4 */ int mNewUnitX;
    /* 0x8D8 */ int mNewUnitZ;
    /* 0x8DC */ dItem::Item mNewItem;
    /* 0x8DE */ u8 mPlayerOn;
    /* 0x8DF */ u8 mSwitchState;
}; // size 0x8E0

class daFtrChest_c : public daFTR_c {
public:
    enum {
        AUTO_OPEN_WAIT = 70, // a display copy opens and closes by itself after this many frames
        AUTO_CLOSE_WAIT = 30,
    };

    // sys_MESS/SYS_Chest: remarks on snooping in someone else's storage.
    enum {
        SNOOP_MSG_FIRST = 1,
        SNOOP_MSG_NUM_VILLAGER = 20, // villager houses use all of them
        SNOOP_MSG_NUM_PLAYER = 5,    // other players' houses use the first few
    };

    enum {
        SWITCH_CLOSED,
        SWITCH_OPENING,
        SWITCH_OPEN,
        SWITCH_CLOSING,
    };

    enum {
        MSG_IDLE,
        MSG_OPEN_START,
        MSG_OPEN_WALK,
        MSG_OPEN,
        MSG_MENU,
        MSG_CLOSE,
        MSG_TALK_START,
        MSG_TALK_WALK,
        MSG_TALK_OPEN,
        MSG_TALK,
        MSG_TALK_END,
        MSG_TALK_CLOSE,
        MSG_NUM,
    };

    virtual void onDemoCmd(int cmd, u8 member);
    virtual BOOL isWait();
    virtual BOOL initModel();
    virtual BOOL executeMain();
    virtual BOOL deleteRes();
    virtual BOOL setSwitch(int sw, int param);
    virtual int getSwitchBit(int sw);

    nw4r::g3d::ResAnmChr getAnmChr();
    void executeSwitch();
    BOOL initClosed();
    void executeClosed();
    BOOL initOpening();
    void executeOpening();
    BOOL initOpen();
    void executeOpen();
    BOOL initClosing();
    void executeClosing();

    int getChestType();
    BOOL setMsgState(int state);
    void executeMsgState();
    BOOL initMsgIdle();
    void executeMsgIdle();
    BOOL initMsgOpenStart();
    void executeMsgOpenStart();
    BOOL initMsgOpenWalk();
    void executeMsgOpenWalk();
    BOOL initMsgOpen();
    void executeMsgOpen();
    BOOL initMsgMenu();
    void executeMsgMenu();
    BOOL initMsgClose();
    void executeMsgClose();
    BOOL initMsgTalkStart();
    void executeMsgTalkStart();
    BOOL initMsgTalkWalk();
    void executeMsgTalkWalk();
    BOOL initMsgTalkOpen();
    void executeMsgTalkOpen();
    BOOL initMsgTalk();
    void executeMsgTalk();
    BOOL initMsgTalkEnd();
    void executeMsgTalkEnd();
    BOOL initMsgTalkClose();
    void executeMsgTalkClose();

    /* 0x8CC */ u32 mDebugMsgCount;
    /* 0x8D0 */ dFTR::counter_c mTimer;
    /* 0x8D4 */ u8 mSwitchState;
    /* 0x8D8 */ int mMsgState;
}; // size 0x8DC

class daFtrClock_c : public daFTR_c {
public:
    enum {
        CHIME_NONE = -1,
        CHIME_WAIT = 60,
    };

    // dFtrm_c::getTimer frames of the tick and tock sounds.
    enum {
        TICK_FRAME = 27,
        TOCK_FRAME = 87,
    };

    virtual BOOL initModel();
    virtual BOOL executeMain();
    virtual BOOL executeIdle();
    virtual BOOL deleteRes();

    /* 0x8CC */ s8 mChime;
    /* 0x8CD */ s8 mPrevChime;
    /* 0x8D0 */ dTime_c mTime;
    /* 0x8F8 */ dTime_c mPrevTime;
    /* 0x920 */ u8 _920;
    /* 0x921 */ u8 _921;
    /* 0x922 */ s16 mChimeTimer;
}; // size 0x924

class daFtrAudio_c : public daFTR_c {
public:
    enum {
        SWITCH_OFF,
        SWITCH_ON,
    };

    enum DEMO_e {
        DEMO_NONE,
        DEMO_MENU_WAIT,
        DEMO_MENU,
        DEMO_TALK_WAIT,
        DEMO_TALK,
        DEMO_TALK_END,
        DEMO_NUM,
    };

    // sys_MESS/SYS_Player.
    enum {
        MSG_SONG_TITLE = 0x16, // shows the song's name
        MSG_NO_MUSIC = 0x17,   // "There's no music in the stereo."
    };

    enum {
        SONG_NUM = 78, // K.K. songs a stereo can play
    };

    virtual void onDemoCmd(int cmd, u8 member);
    virtual BOOL initModel();
    virtual BOOL executeMain();
    virtual BOOL executeIdle();
    virtual BOOL deleteRes();
    virtual BOOL setSwitch(int sw, int param);
    virtual int getSwitchBit(int sw);
    virtual BOOL isSwitchable() { return mDemoState != DEMO_NONE; }

    void playMusic(int song);
    void stopMusic(int song, BOOL clearRoomSong);
    BOOL setDemo(int state);
    void executeDemo();
    BOOL initDemoNone();
    void executeDemoNone();
    BOOL initDemoMenuWait();
    void executeDemoMenuWait();
    BOOL initDemoMenu();
    void executeDemoMenu();
    BOOL initDemoTalkWait();
    void executeDemoTalkWait();
    BOOL initDemoTalk();
    void executeDemoTalk();
    BOOL initDemoTalkEnd();
    void executeDemoTalkEnd();
    void executeSwitch();
    BOOL initOff();
    void executeOff();
    BOOL initOn();
    void executeOn();

    /* 0x8CC */ u8 mPlaying;
    /* 0x8CD */ u8 mSong;
    /* 0x8D0 */ lampList_c mLamps;
    /* 0x964 */ u8 mSwitchState;
    /* 0x968 */ int mDemoState;
}; // size 0x96C

class daFtrTv_c : public daFTR_c {
public:
    enum {
        SWITCH_OFF,
        SWITCH_ON,
    };

    virtual ~daFtrTv_c();
    virtual BOOL initModel();
    virtual BOOL executeMain();
    virtual BOOL executeIdle();
    virtual BOOL deleteRes();
    virtual BOOL setSwitch(int sw, int param);
    virtual int getSwitchBit(int sw);

    BOOL bindProgramRes(BOOL on);
    void setNearest();
    void executeSwitch();
    BOOL initOff();
    void executeOff();
    BOOL initOn();
    void executeOn();

    /* 0x8CC */ u8 mSwitchState;
    /* 0x8D0 */ lampList_c mLamps;
    /* 0x964 */ u8 mNearest;
    /* 0x965 */ u8 mDoorState;
}; // size 0x968

class daFtrTvF_c : public daFtrTv_c {
public:
    enum {
        SWITCH_OFF,
        SWITCH_OPEN,
        SWITCH_ON,
        SWITCH_CLOSE,
    };

    virtual BOOL initModel();
    virtual BOOL executeMain();
    virtual BOOL deleteRes();
    virtual BOOL setSwitch(int sw, int param);
    virtual int getSwitchBit(int sw);

    void executeSwitch();
    BOOL initOff();
    void executeOff();
    BOOL initOpen();
    void executeOpen();
    BOOL initOn();
    void executeOn();
    BOOL initClose();
    void executeClose();
}; // size 0x968

class daFtrPcart_c : public daFTR_c {
public:
    virtual BOOL onMoveStart(BOOL push);
    virtual BOOL initModel();
    virtual BOOL executeMain();
    virtual BOOL deleteRes();
}; // size 0x8CC

class daFtrHnw_c : public daFTR_c {
public:
    enum {
        START_BAR_MASK = 0xF, // the bar of the song the gyroid starts playing on
        WAIT_NONE = 0xFFFF,   // mWait before the song is set up
    };

    enum {
        SWITCH_OFF,
        SWITCH_PLAY,
        SWITCH_DANCE,
    };

    virtual mVec3_c getScale() const;
    virtual BOOL initModel();
    virtual BOOL executeMain();
    virtual BOOL executeIdle();
    virtual BOOL deleteRes();
    virtual BOOL setSwitch(int sw, int param);
    virtual int getSwitchBit(int sw);

    nw4r::g3d::ResAnmChr getDanceAnm();
    nw4r::g3d::ResAnmChr getIdleAnm();
    void sendSwitch(u32 sw) {
        changeSwitch(sw, FALSE);
        dFTR::sendOwnerSwitch(dScene::getCurrentScene(), &mPos, getLayer());
    }

    void executeSwitch();
    BOOL initOff();
    void executeOff();
    BOOL initPlay();
    void executePlay();
    BOOL initDance();
    void executeDance();

    /* 0x8CC */ u16 mWait;
    /* 0x8D0 */ dSndHaniwa_c mSnd;
    /* 0x924 */ f32 mPhase;
    /* 0x928 */ u8 mSndReady;
    /* 0x929 */ u8 mAppeared;
    /* 0x92A */ u8 mOwnerSent;
    /* 0x92C */ dFTR::counter_c mCounter;
    /* 0x930 */ u8 mSwitchState;
}; // size 0x934

class daFtrKiller_c : public daFTR_c {
public:
    enum {
        SWITCH_OFF,
        SWITCH_ON,
    };

    virtual void onRemove();
    virtual BOOL initModel();
    virtual BOOL executeMain();
    virtual BOOL deleteRes();
    virtual BOOL setSwitch(int sw, int param);

    void executeSwitch();
    BOOL initOff();
    void executeOff();
    BOOL initOn();
    void executeOn();

    /* 0x8CC */ dFTR::counter_c mTimer;
    /* 0x8D0 */ u8 mSwitchState;
}; // size 0x8D4

class daFtrCloth_c : public daFTR_c {
public:
    enum {
        SWITCH_DRESSED,
        SWITCH_BARE,
    };

    daFtrCloth_c();

    virtual BOOL createRes();
    virtual nw4r::g3d::ResMdl getResMdl();
    virtual BOOL initModel();
    virtual BOOL executeMain();
    virtual BOOL deleteRes();
    virtual int getSwitchBit(int sw);
    virtual BOOL setSwitch(int sw, int param);

    static BOOL switchOn(daFTR_c *ftr);
    static BOOL switchOff(daFTR_c *ftr);

    void executeSwitch();
    BOOL initDressed();
    void executeDressed();
    BOOL initBare();
    void executeBare();

    /* 0x8CC */ u8 mSwitchState;
    /* 0x8D0 */ nw4r::g3d::ResFile mMannequinRes;
}; // size 0x8D4

class daFtrMyobj_c : public daFTR_c {
public:
    // The seeker index of a my-design item: the player's slot above the design's index.
    enum {
        DESIGN_SHIFT = 3,
        DESIGN_NUM = 1 << DESIGN_SHIFT,
    };

    daFtrMyobj_c();

    virtual BOOL createRes();
    virtual nw4r::g3d::ResMdl getResMdl();
    virtual BOOL initModel();
    virtual BOOL executeMain();
    virtual void calcMdl(const mMtx_c *mtx);
    virtual void entryMdl();
    virtual BOOL deleteRes();

    u8 isMale() const;

    /* 0x8CC */ nw4r::g3d::ResFile mMannequinRes;
    /* 0x8D0 */ dItem::resLoader_c mDesignRes;
    /* 0x940 */ int mPlayer;
    /* 0x944 */ int mDesignIdx;
    /* 0x948 */ u8 mLoaded;
    /* 0x94C */ nw4r::g3d::ResFile mStandRes;
    /* 0x950 */ m3d::mdl_c mCapMdl;
}; // size 0x990

class daFtrTaobj_c : public daFTR_c {
public:
    daFtrTaobj_c();

    virtual BOOL createRes();
    virtual nw4r::g3d::ResMdl getResMdl();
    virtual BOOL initModel();
    virtual BOOL executeMain();
    virtual BOOL deleteRes();

    /* 0x8CC */ nw4r::g3d::ResFile mMannequinRes;
    /* 0x8D0 */ dItem::resLoader_c mDesignRes;
    /* 0x940 */ u8 mLoaded;
}; // size 0x944

class daFtrCmps_c : public daFTR_c, public m3d::mdl_c::callback_c {
public:
    virtual void onRotateStart(BOOL left);
    virtual void onRotate(BOOL left);
    virtual BOOL initModel();
    virtual BOOL executeMain();
    virtual BOOL deleteRes();
    virtual void timingA(ulong nodeId, nw4r::g3d::ChrAnmResult *anmRes, nw4r::g3d::ResMdl resMdl);

    int getSwingAngle() const { return 65536.0f * (mSwing / 360.0f); }

    /* 0x8CC m3d::mdl_c::callback_c */
    /* 0x8D0 */ ulong mNeedleNode;
    /* 0x8D4 */ s16 mRotateCount;
    /* 0x8D6 */ mAng mSwingPhase;
    /* 0x8D8 */ f32 mSwingTarget;
    /* 0x8DC */ f32 mSwing;
}; // size 0x8E0

class daFtrInstrument_c : public daFTR_c {
public:
    virtual BOOL initModel();
    virtual BOOL executeMain();
    virtual BOOL executeIdle();
    virtual BOOL deleteRes();
    virtual BOOL setSwitch(int sw, int param);

    /* 0x8CC */ dSndObjInst_c mSndInst;
}; // size 0x908

class daFtrBk_c : public daFTR_c {
public:
    enum {
        SWITCH_OFF,
        SWITCH_ON,
    };

    virtual BOOL initModel();
    virtual BOOL executeMain();
    virtual BOOL deleteRes();
    virtual BOOL setSwitch(int sw, int param);

    void executeSwitch();
    BOOL initOff();
    void executeOff();
    BOOL initOn();
    void executeOn();

    /* 0x8CC */ u8 mSwitchState;
}; // size 0x8D0

class daFtrMetro_c : public daFTR_c {
public:
    enum {
        SWITCH_OFF,
        SWITCH_START,
        SWITCH_ON,
        SWITCH_STOP,
        SWITCH_RETURN,
    };

    virtual BOOL initModel();
    virtual BOOL executeMain();
    virtual BOOL executeIdle();
    virtual BOOL deleteRes();
    virtual BOOL setSwitch(int sw, int param);
    virtual int getSwitchBit(int sw);

    void executeSwitch();
    BOOL initOff();
    void executeOff();
    BOOL initStart();
    void executeStart();
    BOOL initOn();
    void executeOn();
    BOOL initStop();
    void executeStop();
    BOOL initReturn();
    void executeReturn();

    void countTick() {
        if (!isSyncDisabled() && !mAnmChr.checkFrame(0.0f) && mAnmChr.checkFrame(60.0f)) {
            mTickCount++;
        }
    }

    /* 0x8CC */ u8 mTickCount;
    /* 0x8CD */ u8 mReturning;
    /* 0x8CE */ u8 mSwitchState;
}; // size 0x8D0

class daFtrFlower_c : public daFTR_c {
public:
    virtual nw4r::g3d::ResMdl getResMdl();
}; // size 0x8CC

class daFtrCatalogCW_c : public daFTR_c {
public:
    virtual BOOL initModel();
    virtual BOOL createRes();
    virtual nw4r::g3d::ResMdl getResMdl();
    virtual BOOL executeMain();
    virtual BOOL deleteRes();

    /* 0x8CC */ u8 mLoaded;
    /* 0x8CD */ u8 mIsWall;
    /* 0x8D0 */ dItem::resLoader_c mLoader;
    /* 0x940 */ nw4r::g3d::ResFile mFile;
}; // size 0x944

class daFtrUmb_c : public daFTR_c {
public:
    virtual BOOL initModel();
}; // size 0x8CC

class daFtrBalloon_c : public daFTR_c {
public:
    virtual BOOL initModel();
    virtual nw4r::g3d::ResMdl getResMdl();
    virtual BOOL executeMain();
}; // size 0x8CC

class daFtrWindmill_c : public daFTR_c {
public:
    virtual BOOL initModel();
}; // size 0x8CC

class daFtrAcc_c : public daFTR_c {
public:
    enum {
        SWITCH_OFF,
        SWITCH_ON,
    };

    virtual mVec3_c getScale() const;
    virtual BOOL initModel();
    virtual BOOL createRes();
    virtual nw4r::g3d::ResMdl getResMdl();
    virtual BOOL executeMain();
    virtual void calcMdl(const mMtx_c *mtx);
    virtual void entryMdl();
    virtual BOOL deleteRes();
    virtual BOOL setSwitch(int sw, int param);
    virtual int getSwitchBit(int sw);

    static BOOL requestOff(daFTR_c *ftr);
    static BOOL requestOn(daFTR_c *ftr);

    void executeSwitch();
    BOOL initOff();
    void executeOff();
    BOOL initOn();
    void executeOn();

    /* 0x8CC */ u8 mSwitchState;
    /* 0x8D0 */ nw4r::g3d::ResFile mStandFile;
    /* 0x8D4 */ m3d::mdl_c mAccMdl;
}; // size 0x914

class daFtrMonitor_c : public daFTR_c {
public:
    enum {
        SWITCH_OFF,
        SWITCH_ON,
    };

    enum {
        AUTO_ON_WAIT = 60, // a display copy switches by itself after this many frames
        AUTO_OFF_WAIT = 120,
    };

    virtual BOOL initModel();
    virtual BOOL executeMain();
    virtual BOOL deleteRes();
    virtual BOOL setSwitch(int sw, int param);
    virtual int getSwitchBit(int sw);

    void executeSwitch();
    BOOL initOff();
    void executeOff();
    BOOL initOn();
    void executeOn();

    /* 0x8CC */ dFTR::counter_c mTimer;
    /* 0x8D0 */ lampList_c mLamps;
    /* 0x964 */ u8 mSwitchState;
}; // size 0x968

class daFtrCicada_c : public daFTR_c {
public:
    struct sound_c {
        /* 0x0 */ int mItem;
        /* 0x4 */ int mInterval;
        /* 0x8 */ int mSoundId;
    }; // size 0xC

    virtual BOOL initModel();
    virtual BOOL executeMain();
    virtual BOOL deleteRes();

    /* 0x8CC */ dSndObjInsect_c mSndInsect;
    /* 0x8FC */ int mTimer;
    /* 0x900 */ int mInterval;
    /* 0x904 */ int mSoundId;
}; // size 0x908

class daFtrTel_c : public daFTR_c {
public:
    enum {
        SWITCH_OFF,
        SWITCH_PICK_UP,
        SWITCH_ON,
        SWITCH_PUT_DOWN,
    };

    enum TALK_STATE_e {
        TALK_STATE_WAIT,
        TALK_STATE_RING,
        TALK_STATE_TALK,
        TALK_STATE_TALK_END,
        TALK_STATE_HANG_UP,
        TALK_STATE_NUM,
    };

    enum {
        MSG_FORTUNE_HELLO = 0x01, // sys_MESS/SYS_Fortune_tel: the fortune hotline's greeting
        MSG_WISH_TAGS = 0x3D,     // npc_SP/NPC_obj: reading the bamboo grass's wish tags
    };

    virtual void onDemoCmd(int cmd, u8 member);
    virtual BOOL initModel();
    virtual BOOL executeMain();
    virtual BOOL deleteRes();
    virtual BOOL setSwitch(int sw, int param);
    virtual int getSwitchBit(int sw);

    void executeSwitch();
    BOOL initOff();
    void executeOff();
    BOOL initPickUp();
    void executePickUp();
    BOOL initOn();
    void executeOn();
    BOOL initPutDown();
    void executePutDown();
    BOOL setTalkState(int state);
    void executeTalkState();
    BOOL initWait();
    void executeWait();
    BOOL initRing();
    void executeRing();
    BOOL initTalk();
    void executeTalk();
    BOOL initTalkEnd();
    void executeTalkEnd();
    BOOL initHangUp();
    void executeHangUp();

    /* 0x8CC */ mVec3_c mSpeakPos;
    /* 0x8D8 */ s16 mSpeakAngle;
    /* 0x8DA */ u16 mTalkTimer;
    /* 0x8DC */ u8 mSwitchState;
    /* 0x8E0 */ int mTalkState;
}; // size 0x8E4

class daFtrAtm_c : public daFTR_c {
public:
    enum TALK_STATE_e {
        TALK_STATE_WAIT,
        TALK_STATE_TALK,
        TALK_STATE_TALK_END,
        TALK_STATE_SUSPEND,
        TALK_STATE_MENU,
        TALK_STATE_MENU_END,
        TALK_STATE_PAID_OFF,
        TALK_STATE_PAID_OFF_END,
        TALK_STATE_NUM,
    };

    enum ANSWER_e {
        ANSWER_PAY,
        ANSWER_DEPOSIT,
        ANSWER_WITHDRAW,
        ANSWER_REISSUE_CARD,
        ANSWER_QUIT,
        ANSWER_NUM,
    };

    enum MENU_e {
        MENU_PAY,
        MENU_DEPOSIT,
        MENU_WITHDRAW,
    };

    // npc_SP/NPC_obj.
    enum {
        MSG_WELCOME_MAIN_BRANCH = 0x01,
        MSG_WELCOME_BRANCH = 0x02,
        MSG_THANKS = 0x03,
        MSG_NO_DEBT = 0x04,
        MSG_DEBT = 0x05,          // shows the remaining debt
        MSG_SAVINGS_FULL = 0x06,
        MSG_SAVINGS = 0x07,       // shows the balance
        MSG_NO_SAVINGS = 0x08,
        MSG_NOT_A_CLIENT = 0x09,
        MSG_CARD_REISSUED = 0x0A,
        MSG_WELCOME = 0x0B,
    };

    // sys_MESS/SYS_Player.
    enum {
        MSG_PLAYER_DEBT_PAID = 0x03, // "No more debt for me!"
    };

    // dSelect::Comp_c::setSysMessage: the choices of the ATM's menu.
    enum {
        SEL_MSG_PAY = 8,
        SEL_MSG_DEPOSIT,
        SEL_MSG_WITHDRAW,
        SEL_MSG_REISSUE_CARD,
        SEL_MSG_QUIT,
    };

    virtual void onDemoCmd(int cmd, u8 member);
    virtual BOOL initModel();
    virtual BOOL executeMain();
    virtual BOOL deleteRes();
    virtual void onMessageStart(int arg);
    virtual void onMessageEnd(int kind);
    virtual void onAnswer(int arg);

    BOOL setTalkState(int state);
    void executeTalkState();
    BOOL initWait();
    void executeWait();
    BOOL initTalk();
    void executeTalk();
    BOOL initTalkEnd();
    void executeTalkEnd();
    BOOL initSuspend();
    void executeSuspend();
    BOOL initMenu();
    void executeMenu();
    BOOL initMenuEnd();
    void executeMenuEnd();
    BOOL initPaidOff();
    void executePaidOff();
    BOOL initPaidOffEnd();
    void executePaidOffEnd();

    /* 0x8CC */ int mMenuMsgCode;
    /* 0x8D0 */ u8 _8D0[0x8D4 - 0x8D0];
    /* 0x8D4 */ int mMenu;
    /* 0x8D8 */ int mDebt;
    /* 0x8DC */ int mAnswers[ANSWER_NUM];
    /* 0x8F0 */ int mTalkState;
}; // size 0x8F4

class daFtrBarley_c : public daFTR_c {
public:
    enum {
        SWITCH_OFF,
        SWITCH_ON,
    };

    enum {
        AUTO_SWAY_WAIT = 90, // a display copy sways by itself after this many frames
        SWAY_TIME = 40, // set when trampled; not counted down
    };

    virtual BOOL initModel();
    virtual BOOL executeMain();
    virtual BOOL deleteRes();
    virtual BOOL setSwitch(int sw, int param);

    static BOOL isMoving(dDemoActor_c *actor);
    BOOL isTrampled();
    void executeSwitch();
    BOOL initOff();
    void executeOff();
    BOOL initOn();
    void executeOn();

    /* 0x8CC */ dFTR::counter_c mCounter;
    /* 0x8D0 */ u8 mSwitchState;
}; // size 0x8D4

class daFtrRoller_c : public daFTR_c {
public:
    virtual BOOL onMoveStart(BOOL push);
}; // size 0x8CC
