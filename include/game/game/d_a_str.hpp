#pragma once

// The structure actor: the base class of the town and city buildings (REL
// d_a_strNP, profile A_STR, RTTI daSTR_c). The RELs d_a_str_town,
// d_a_str_field, d_a_str_common and d_a_str_cdbbs derive from it. Source:
// src/d_a_strNP/d_a_str.cpp. Names are inferred.

// The include order sets the weak vtable / RTTI order in .data (reverse class
// completion).
#include <game/mLib/m_3d.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_bgc.hpp>
#include <game/game/d_str.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_objc.hpp>
#include <game/game/d_save_building.hpp>
#include <game/game/d_snd_obj.hpp>
#include <game/mLib/m_color.hpp>
#include <game/snd/snd_id.hpp>
#include <nw4r/g3d.h>

namespace dMsg {
class Comp_c;
}

// Registers itself in dSTR::getAcList(). Bases: dDemoActor_c, dSTR::alwaysAc_c
// (at 0xBC), dMsg::Rcpt_c (at 0xC0). Its own virtuals start at fBase_c vtable
// +0x118; the overrides of dSTR::alwaysAc_c also get slots there.
class daSTR_c : public dDemoActor_c,
                public dSTR::alwaysAc_c,
                public dMsg::Rcpt_c {
  public:
    // The door kinds (dBuildingInfo_c::mType, getDoorType).
    enum {
        DOOR_NONE,   // no door to enter
        DOOR_HINGED, // a door that swings open (door_knob_in / door_knob_out)
        DOOR_AUTO,   // a sliding door (door_auto_L)
        DOOR_WALK,   // walked into without an animation
    };

    // The door's states (setDoorState / executeDoorState).
    enum {
        DOOR_STATE_CLOSED,
        DOOR_STATE_OPEN_IN,
        DOOR_STATE_OPEN_OUT,
        DOOR_STATE_AUTO_OPEN,
        DOOR_STATE_AUTO_CLOSE,
        DOOR_STATE_WAIT_IN,
        DOOR_STATE_WAIT_OUT,
        DOOR_STATE_NUM,
    };

    // The entering / talking states (setState / executeState).
    enum {
        STATE_WAIT,
        STATE_CHECK_LOCK,    // dSTR::lock: is another net member inside?
        STATE_TALK_PUT_AWAY, // the player puts the tool away
        STATE_TALK_WALK,     // the player walks up to the door
        STATE_TALK,          // the door's message (sys_MESS/SYS_Door)
        STATE_TALK_END,
        STATE_ENTER_PUT_AWAY,
        STATE_ENTER, // the player opens the door (dPlayerMgr::enterDoor)
        STATE_ENTER_DIRECT_PUT_AWAY,
        STATE_ENTER_DIRECT, // dPlayerMgr::enterDoorDirect
        STATE_ENTER_WAIT,   // waits, then changes the scene
        STATE_EXIT,         // the player comes out (DEMO_CMD_ENTER)
        STATE_NUM,
    };

    // mTalkKind: the door message's kind (onTalkStart).
    enum {
        TALK_KIND_MSG = 0,    // getCheckMsg's message, then the talk ends
        TALK_KIND_LOCKED = 2, // getLockedMsg's message, then the player enters
    };

    // mFlags.
    enum {
        FLAG_CREATED = 1 << 0,
        FLAG_MSG = 1 << 1,    // mMsgCode holds a message to show
        FLAG_TALK = 1 << 2,   // the player talked to the door (DEMO_CMD_TALK)
        FLAG_STATES = 1 << 3, // runs the state machine (not in a demo scene)
        FLAG_SCENE_CHANGE = 1 << 4,
    };

    // Moves the joints of the town hall clock (kh_j / km_j) and the
    // curved-ground joint (joint1).
    class nodeCallBack_c : public m3d::mdl_c::callback_c {
      public:
        nodeCallBack_c() : mOwner(NULL) {
        }
        virtual ~nodeCallBack_c() {
        }
        virtual void timingA(ulong nodeId, nw4r::g3d::ChrAnmResult *anmRes,
                             nw4r::g3d::ResMdl resMdl);

        /* 0x04 */ ulong mHourNode;
        /* 0x08 */ ulong mMinuteNode;
        /* 0x0C */ ulong mCurveNode;
        /* 0x10 */ daSTR_c *mOwner;
        /* 0x14 */ u8 mCurve; // fixDoorVerts: the model's vertices were curved
    }; // size 0x18

    // The window lights (m_letter, m_window, m_tw_light...): the TEV konst
    // colour 1 of each material, faded from black by dSTR::light_c.
    class windowLight_c : public dSTR::light_c {
      public:
        windowLight_c();
        BOOL updateColor();
        void set(nw4r::g3d::ResMdl mdl, nw4r::g3d::ScnMdl *scnMdl, BOOL on,
                 int buildingIdx, u8 sub);
        void execute(BOOL on, BOOL flicker);

        /* 0x14 */ s8 mMatID[17];
        /* 0x28 */ int mBuildingIdx;
        /* 0x2C */ nw4r::g3d::ScnMdl *mScnMdl;
        /* 0x30 */ u8 mSub; // the light of the sub model (mFadeMdl)
    }; // size 0x34

    // The city's neon light (m_tw_neon).
    class neonLight_c : public dSTR::light_c {
      public:
        neonLight_c();
        BOOL updateColor();
        void set(nw4r::g3d::ResMdl mdl, nw4r::g3d::ScnMdl *scnMdl, BOOL on,
                 int buildingIdx);
        void execute(BOOL on, BOOL flicker);

        /* 0x14 */ s8 mMatID[1];
        /* 0x18 */ int mBuildingIdx;
        /* 0x1C */ nw4r::g3d::ScnMdl *mScnMdl;
    }; // size 0x20

    // The city's other lights (m_tw_etc, m_tw_etc_00..03).
    class etcLight_c : public dSTR::light_c {
      public:
        etcLight_c();
        BOOL updateColor();
        void set(nw4r::g3d::ResMdl mdl, nw4r::g3d::ScnMdl *scnMdl, BOOL on,
                 int buildingIdx);
        void execute(BOOL on, BOOL flicker);

        /* 0x14 */ s8 mMatID[5];
        /* 0x1C */ int mBuildingIdx;
        /* 0x20 */ nw4r::g3d::ScnMdl *mScnMdl;
    }; // size 0x24

    // No user-declared dtor: the implicit one is inlined into the
    // subclasses' dtors (d_a_str_common).
    daSTR_c();

    virtual int create();
    virtual int preDelete();
    virtual int preExecute();
    virtual void postExecute(MAIN_STATE_e state);
    virtual int draw();

    virtual bool canInteract(dDemoActor_c *actor);
    virtual void onDemoCmd(int cmd, u8 member);
    virtual mVec3_c *getDemoPosition();

    // +0x118
    virtual f32 getModelOffsetY() {
        return 0.0f;
    }
    virtual u16 getDemoCode() const; // the demo id: the unit x / z
    virtual void onTalkStart(int kind, dMsg::Comp_c *comp) {
    }
    virtual BOOL isCopyCol() {
        return TRUE;
    } // copies its collision into the field at create
    virtual BOOL isActive() {
        return TRUE;
    }
    virtual int getUnitX() {
        return mUnitX;
    }
    virtual int getUnitZ() {
        return mUnitZ;
    }
    virtual int getBuildingIdx() {
        return mBuildingIdx;
    }
    virtual const char *getResName() {
        return dSTR::getBuildingResName(mBuildingIdx);
    }
    virtual ulong getMdlBufferOption();
    virtual void beforeCreateMdl() {
    }
    virtual void afterCreateMdl() {
    }
    virtual BOOL isCheckCamera() {
        return TRUE;
    }
    virtual BOOL isCheckViewArea() {
        return TRUE;
    }
    virtual int getDoorAnmNum();
    virtual BOOL getDoorAnm(nw4r::g3d::ResAnmChr *anm, u32 idx);
    virtual int getShadowAnmNum();
    virtual BOOL getShadowAnm(nw4r::g3d::ResAnmShp *anm, u32 idx);
    virtual BOOL setDoorState(int state);
    virtual void executeDoorState();
    virtual int getDoorOpenSe() {
        return SE_door_open;
    }
    virtual int getDoorCreakOpenSe() {
        return SE_door_open_kishimu;
    }
    virtual int getDoorCloseSe() {
        return SE_door_close;
    }
    virtual int getDoorCreakCloseSe() {
        return SE_door_close_kishimu;
    }
    virtual int getKnockSe();
    virtual int getAutoDoorOpenSe() {
        return SE_jidou_door_open;
    }
    virtual int getAutoDoorCloseSe() {
        return SE_jidou_door_close;
    }
    virtual const char *getFadeMdlName();
    virtual BOOL hasFadeMdl() {
        return getFadeMdlName() != NULL;
    }
    virtual int getSceneOffset() {
        return 0;
    }
    virtual BOOL hasKnockSe() {
        return FALSE;
    }
    virtual BOOL isWalkThrough() {
        return FALSE;
    }
    virtual int getDoorEndSe() {
        return -1;
    }
    virtual BOOL isDoorCentered() const {
        return TRUE;
    }
    virtual int getDoorType() const;
    virtual int getCheckMsg() {
        return -1;
    }
    virtual int getLockedMsg() {
        return -1;
    }
    virtual int getMsg1AC() {
        return -1;
    }
    virtual BOOL isDraw() {
        return TRUE;
    }
    virtual void drawMdl();
    virtual nw4r::math::MTX34 *getShadowMtx() {
        return NULL;
    }
    virtual nw4r::math::VEC3 *getShadowScale() {
        return NULL;
    }
    virtual f32 getRoundShadowSize() {
        return 0.0f;
    }
    virtual f32 getMdlScale() {
        return 1.0f;
    }
    virtual BOOL isNight();
    // +0x1CC (vf1C8() && vf1A4() != -1): a window is lit
    // (dInsectField::checkHouseLight, the moth's spawn spot).
    virtual BOOL isWindowLit();
    virtual BOOL isNeonOn() {
        return FALSE;
    }
    virtual BOOL isEtcLightOn() {
        return TRUE;
    }

    BOOL isNoDoor();
    BOOL isAutoDoor();
    BOOL isHingedDoor();
    BOOL isLightFlicker() const;
    BOOL hasMsg() const {
        return (mFlags >> 1) & 1;
    }
    BOOL checkDoorFront();
    BOOL isInDoorRange(const mVec3_c *pos, const s16 *angle, f32 range);
    nw4r::g3d::ScnMdl *getScnMdl();
    void calcMtx(f32 offsetY, f32 offsetZ, nw4r::math::MTX34 *mtx);
    void drawShadow(nw4r::math::MTX34 *mtx);
    void setup();
    void createMdl();
    BOOL isCulled();
    void executeTalkCheck();
    BOOL setState(int state);
    void executeState();
    BOOL fixDoorVerts();
    void updateSoundPos();
    void executeExtra();
    void onSceneChange(u8 scene);
    inline void requestDoorTalk();

    BOOL initDoorClosed();
    void executeDoorClosed();
    BOOL initDoorOpenIn();
    void executeDoorOpenIn();
    BOOL initDoorOpenOut();
    void executeDoorOpenOut();
    BOOL initAutoDoorOpen();
    void executeAutoDoorOpen();
    BOOL initAutoDoorClose();
    void executeAutoDoorClose();
    BOOL initDoorWaitIn();
    void executeDoorWaitIn();
    BOOL initDoorWaitOut();
    void executeDoorWaitOut();

    BOOL initWait();
    void executeWait();
    BOOL initCheckLock();
    void executeCheckLock();
    BOOL initTalkPutAway();
    void executeTalkPutAway();
    BOOL initTalkWalk();
    void executeTalkWalk();
    BOOL initTalk();
    void executeTalk();
    BOOL initTalkEnd();
    void executeTalkEnd();
    BOOL initEnterPutAway();
    void executeEnterPutAway();
    BOOL initEnter();
    void executeEnter();
    BOOL initEnterWait();
    void executeEnterWait();
    BOOL initExit();
    void executeExit();
    BOOL initEnterDirectPutAway();
    void executeEnterDirectPutAway();
    BOOL initEnterDirect();
    void executeEnterDirect();

    /* 0x124 */ int mBuildingIdx;
    /* 0x128 */ u16 mBuildingId;
    /* 0x12A */ u16 mParamHi; // mParam >> 16
    /* 0x12C */ nw4r::g3d::ResFile mResFile;
    /* 0x130 */ m3d::mdl_c mMdl;
    /* 0x170 */ m3d::mdl_c mFadeMdl; // getFadeMdlName in the common brres
    /* 0x1B0 */ u8 _1B0[0x1BC - 0x1B0];
    /* 0x1BC */ m3d::anmChr_c mDoorAnmChr;
    /* 0x1F4 */ u8 _1F4[0x1F8 - 0x1F4];
    /* 0x1F8 */ nodeCallBack_c mNodeCallBack;
    /* 0x210 */ int mShadowMatID;      // m_shadow0 in mShadowMdl
    /* 0x214 */ m3d::mdl_c mShadowMdl; // the second model of the brres
    /* 0x254 */ m3d::anmShp_c mShadowAnm;
    /* 0x28C */ nw4r::g3d::ResAnmChr mDoorAnm[2];
    /* 0x294 */ int mTalkKind; // TALK_KIND_*
    /* 0x298 */ char mResName[0x20];
    /* 0x2B8 */ int mUnitX;
    /* 0x2BC */ int mUnitZ;
    /* 0x2C0 */ dSTR::entArea_c *mEntArea;
    /* 0x2C4 */ dSTR::colArea_c *mColArea;
    /* 0x2C8 */ const char *mName; // dBuildingInfo_c::mName
    /* 0x2CC */ u8 mDoorState;
    /* 0x2D0 */ mVec3_c mDoorPos;
    /* 0x2DC */ s16 mDoorAngle;
    /* 0x2DE */ u16 mDoorTimer;
    /* 0x2E0 */ u16 mAutoDoorTimer;
    /* 0x2E2 */ u16 mEnterTimer;
    /* 0x2E4 */ u16 mExitTimer;
    /* 0x2E8 */ windowLight_c mWindowLight;
    /* 0x31C */ neonLight_c mNeonLight;
    /* 0x33C */ etcLight_c mEtcLight;
    /* 0x360 */ windowLight_c mFadeWindowLight;
    /* 0x394 */ mVec3_c mDemoPos;
    /* 0x3A0 */ int mMsgCode;
    /* 0x3A4 */ mVec3_c mSoundPos;
    /* 0x3B0 */ dSndObjSimple_c mSnd;
    /* 0x3E0 */ dObjc::dpdQuad_c mDpdQuad;
    /* 0x490 */ u32 mFlags;
    /* 0x494 */ int mState;
}; // size 0x498

// Exports of the structure REL.
namespace dAStr {
extern const float cNpcDoorOffset;
mVec3_c getNpcDoorPos(const daSTR_c *str, const mVec3_c *pos, mAng *outAngle,
                      float offset);
BOOL requestNpcEnter(daSTR_c *str); // the door opens for an entering player
BOOL requestNpcExit(daSTR_c *str);
// The structures' view area (dSTR::viewArea_c) from dSTR::getHostIO(), scaled.
f32 getViewFront();     // hostIO _14
f32 getViewBack();      // hostIO _0C
f32 getViewHalfWidth(); // hostIO _16
// 0x24 (frames to wait for the door) if the structure has a hinged door, else
// 0.
int getEnterWait(daSTR_c *str);
// The exit door's wait (the door animation's frame count), 0 without one.
int getExitWait(daSTR_c *str);
BOOL isFadeMode(); // FALSE (d_a_str_town: the fade model's draw priority)
// The field position of a point in a structure's model space.
void toFieldPos(mVec3_c *out, const mVec3_c *local,
                const nw4r::math::MTX34 *mtx);
} // namespace dAStr
