#pragma once
#include <game/mLib/m_mtx.hpp>

// The bus stop and the bus (REL d_a_str_commonNP, profiles STR_BUS_STOP and
// STR_BUS, RTTI daStrBusStop_c / daStrBus_c). Both are structures (daSTR_c).
// The bus stop creates the bus and drives the ride between the town and the
// city; the bus plays its animations in the door state machine. Source:
// src/d_a_str_commonNP/d_a_str_common.cpp. Names are inferred.

#include <game/game/d_a_str.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_effect.hpp>

// The bus stop. Talking to it asks for the bus (its sign's message), then the
// player walks to the stop, boards and leaves the scene; coming back (the
// dSTR::getBusInfo() request), the player gets off and the bus drives away.
// After the opening ride, Rover sends the player to the town hall and the map
// tip follows.
class daStrBusStop_c : public daSTR_c {
  public:
    // setState / executeState.
    enum {
        STATE_WAIT,
        STATE_TALK,     // the stop's sign (npc_SP/NPC_obj, MSG_TO_*)
        STATE_TALK_END, // the talk is over without a ride
        STATE_WALK,     // the player walks to the stop and puts the tool away
        STATE_CALL,     // the bus arrives and opens
        STATE_PUT_AWAY,
        STATE_BOARD,
        STATE_BOARDED,
        STATE_DEPART, // the bus leaves with the player (the scene change)
        STATE_ARRIVE, // the bus arrives with the player (the bus request)
        STATE_GET_OFF,
        STATE_LEAVE,      // the player walks out of the bus
        STATE_GUIDE_TALK, // the opening ride: MSG_GO_TO_TOWN_HALL
        STATE_GUIDE_TURN, // the player turns to the bus
        STATE_GUIDE_TALK_END,
        STATE_DRIVE_AWAY,   // the empty bus leaves
        STATE_MAP_TIP_TURN, // the player turns to the camera
        STATE_MAP_TIP,      // MSG_MAP_TIP
        STATE_MAP_TIP_END,
        STATE_NUM,
    };

    // npc_SP/NPC_obj (STATE_TALK).
    enum {
        MSG_TO_CITY = 0x1F,       // "Bus to the city." (Want to take the bus?)
        MSG_TO_TOWN = 0x21,       // "Bus to <town>." (Want to go to <town>?)
        MSG_NOT_COMING = 0x23,    // "Looks like the bus isn't coming..."
        MSG_SNOWBALL = 0x24,      // "...delayed due to a massive snowball"
        MSG_ROUTE_BLOCKED = 0x25, // "...if someone's standing in its route!"
    };

    // sys_MESS/SYS_Player (the opening ride).
    enum {
        MSG_MAP_TIP = 1,         // "To open the map, point the Wii Remote..."
        MSG_GO_TO_TOWN_HALL = 4, // Rover: "Head on over to the town hall..."
    };

    // mWalkStep (STATE_WALK).
    enum {
        WALK_CHECK_TOOL,
        WALK_PUT_AWAY,
        WALK_DONE,
    };

    // The answers to the stop's question (onAnswer).
    enum {
        ANSWER_RIDE,
        ANSWER_CANCEL,
    };

    virtual int doDelete();
    virtual int execute();
    virtual bool canInteract(dDemoActor_c *actor);
    virtual void onDemoCmd(int cmd, u8 member);
    virtual BOOL isCopyCol() {
        return FALSE;
    }
    virtual void afterCreateMdl();
    virtual void drawMdl();
    virtual int getVoiceMode();
    virtual void onAnswer(int answer);

    void updateBusWall();
    void addWalls();
    void executeWalls();
    void removeWalls();
    BOOL setState(int state);
    void executeState();
    static BOOL checkSnowBall();
    static BOOL checkNpc();
    void playIntroBgm();
    void stopIntroBgm();

    BOOL initWait();
    void executeWait();
    BOOL initTalk();
    void executeTalk();
    BOOL initTalkEnd();
    void executeTalkEnd();
    BOOL initWalk();
    void executeWalk();
    BOOL initCall();
    void executeCall();
    BOOL initPutAway();
    void executePutAway();
    BOOL initBoard();
    void executeBoard();
    BOOL initBoarded();
    void executeBoarded();
    BOOL initDepart();
    void executeDepart();
    BOOL initArrive();
    void executeArrive();
    BOOL initGetOff();
    void executeGetOff();
    BOOL initLeave();
    void executeLeave();
    BOOL initGuideTalk();
    void executeGuideTalk();
    BOOL initGuideTurn();
    void executeGuideTurn();
    BOOL initGuideTalkEnd();
    void executeGuideTalkEnd();
    BOOL initDriveAway();
    void executeDriveAway();
    BOOL initMapTipTurn();
    void executeMapTipTurn();
    BOOL initMapTip();
    void executeMapTip();
    BOOL initMapTipEnd();
    void executeMapTipEnd();

    static daStrBusStop_c *sInstance;

    /* 0x498 */ u16 mWalkWait;
    /* 0x49A */ u16 mOpenWait;
    /* 0x49C */ u16 mGetOffWait;
    /* 0x49E */ u16 _49E;
    /* 0x4A0 */ bool mIntroRide; // the opening ride (busInfo_c::mType 1)
    /* 0x4A2 */ u16 mTalkWait;
    /* 0x4A4 */ u16 mDepartWait;
    /* 0x4A6 */ s16 mSceneWait;
    /* 0x4A8 */ u8 mHoldStart; // this stop called dDemo::onHoldStart
    /* 0x4A9 */ u8 mFirstCity; // PRIVATE_FLAG0_CITY_VISITED not set yet
    /* 0x4AC */ dObjc::acpos_c mAcPos;
    /* 0x4FC */ u8 mBusWallOn; // the bus stands at the stop
    /* 0x500 */ f32 mBusWallScale;
    /* 0x504 */ int mWalkStep; // WALK_*
    /* 0x508 */ dBGCF::mvbg_c mBackWall;
    /* 0x5A4 */ dBGCF::mvbg_c mSideWall;
    /* 0x640 */ dBGCF::mvbg_c mBusWall; // dSTR::getBusStopBox
    /* 0x6DC */ int mStopState;
}; // size 0x6E0

// The bus. Its door states (mDoorState) are its animations.
class daStrBus_c : public daSTR_c {
  public:
    // setBusState / executeDoorState (mDoorState).
    enum {
        BUS_STATE_HIDDEN,
        BUS_STATE_ARRIVE, // obj_bus_arrive
        BUS_STATE_ARRIVED,
        BUS_STATE_OPEN, // obj_bus_open
        BUS_STATE_OPENED,
        BUS_STATE_PLAYER_IN,  // obj_bus_plyin
        BUS_STATE_PLAYER_OUT, // obj_bus_plyout
        BUS_STATE_CLOSE,      // obj_bus_close
        BUS_STATE_CLOSED,
        BUS_STATE_DEPART, // obj_bus_departure
        BUS_STATE_GONE,
        BUS_STATE_NUM,
    };

    // Moves the wipers (j_wiper_L0 / R0 / L1 / R1) in the rain and the snow.
    class nodeCallBack_c : public m3d::mdl_c::callback_c {
      public:
        virtual ~nodeCallBack_c() {
        }
        virtual void timingA(ulong nodeId, nw4r::g3d::ChrAnmResult *anmRes,
                             nw4r::g3d::ResMdl resMdl);

        BOOL isWiping();

        /* 0x04 */ ulong mWiperL0;
        /* 0x08 */ ulong mWiperR0;
        /* 0x0C */ ulong mWiperL1;
        /* 0x10 */ ulong mWiperR1;
        /* 0x14 */ mAng mAngle;
        /* 0x16 */ mAng mPrevAngle;
    }; // size 0x18

    virtual int doDelete();
    virtual int execute();
    virtual BOOL isCopyCol() {
        return FALSE;
    }
    virtual void afterCreateMdl();
    virtual void executeDoorState();
    virtual BOOL isDraw();
    virtual void drawMdl();
    virtual mMtx_c *getShadowMtx();
    virtual nw4r::math::VEC3 *getShadowScale();

    static nw4r::g3d::ResFile getResFile() {
        return sInstance != NULL ? sInstance->mResFile
                                 : nw4r::g3d::ResFile(NULL);
    }
    static BOOL requestBusState(u32 state, int frame);
    static u8 getBusState();
    const mVec3_c *getNodePos(const char *name, BOOL field);
    void getNodeMtx(mMtx_c *mtx, const char *name);
    static void getMainMtx(mMtx_c *mtx);
    static const mVec3_c *getMainPos(BOOL field);
    static const mVec3_c *getWiperPos(BOOL field);
    void nextStateAtAnmEnd(u32 state);
    BOOL setBusAnm(const char *name, m3d::playMode_e playMode, BOOL atEnd);
    BOOL setBusState(u32 state, int frame);
    static void startBusSound(u32 id);
    static void resetCount();

    BOOL initHidden();
    void executeHidden();
    BOOL initArrive();
    void executeArrive();
    BOOL initArrived();
    void executeArrived();
    BOOL initOpen();
    void executeOpen();
    BOOL initOpened();
    void executeOpened();
    BOOL initPlayerIn();
    void executePlayerIn();
    BOOL initPlayerOut();
    void executePlayerOut();
    BOOL initClose();
    void executeClose();
    BOOL initClosed();
    void executeClosed();
    BOOL initDepart();
    void executeDepart();
    BOOL initGone();
    void executeGone();

    static daStrBus_c *sInstance;

    /* 0x498 */ nodeCallBack_c mNodeCallBack;
    /* 0x4B0 */ u16 mOpenWait;
    /* 0x4B4 */ dSndObjSimple_c mWiperSnd;
    /* 0x4E4 */ dObjc::acpos_c mAcPos;
    /* 0x534 */ dLevelEffect_c mArriveEffect;
    /* 0x5C8 */ dLevelEffect_c mDepartEffect;
}; // size 0x65C

namespace dAStrCommon {
// The bus wall's scale while no bus stands at the stop (inferred: not used
// outside this REL).
extern const f32 cBusWallScaleMin;
} // namespace dAStrCommon
