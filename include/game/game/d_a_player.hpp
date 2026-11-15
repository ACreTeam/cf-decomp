#pragma once

// The player actor (REL d_a_playerNP, profile A_PLAYER, not decompiled yet).
// Class names from RTTI: "dAcPlayer_c", "dAcPlayer_c::myFace_c",
// "dAcPlayer_c::myMdlCallback_c". Member types and offsets come from the
// constructor (.text 0x528); the size from the profile's construct function
// (new(0x2314)). Field names come from their uses in the REL and in dPlayerMgr;
// unnamed fields keep their offsets.
//
// The REL runs one state machine: dPlayerMgr::changeState ->
// setChangeStateCallback -> a table of 0xAB states (dPlayerMgr::State), 0x24
// bytes each (init / execute / end member pointers).

#include <types.h>
#include <game/game/d_a_npc.hpp>
#include <game/game/d_audio_obj.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_fish_field.hpp>
#include <game/game/d_hmn_body_mng.hpp>
#include <game/game/d_hmn_face_anm_mng.hpp>
#include <game/game/d_hmn_head_mng.hpp>
#include <game/game/d_hmn_palette_mng.hpp>
#include <game/game/d_insect_field.hpp>
#include <game/game/d_m3d.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_objc.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_player_mngr.hpp>
#include <game/game/d_snd_obj.hpp>
#include <game/mLib/m_3d.hpp>
#include <game/mLib/m_3d/face_ex.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/mLib/m_vec.hpp>

class dHmnCloth_c;
class dHmnHeadItem_c;
class dHmnToolBank_c;
class dInsFldBase_c;
class dFdBase_c;
struct putUnit_c;
namespace nw4r {
namespace ef {
class Particle;
}
} // namespace nw4r
namespace EGG {
class FrmHeap;
}

class dAcPlayer_c : public dDemoActor_c, public dMsg::Rcpt_c {
  public:
    using dActor_c::mAngle;
    using dActor_c::mPos;

    // getAngleDirection / getInputDirection: the stick direction relative to mAngle.y (mSeatPose while
    // holding furniture, the side of a bed / chair otherwise).
    enum Direction_e {
        DIR_FRONT = 0, // push
        DIR_LEFT = 1,
        DIR_BACK = 2,  // pull
        DIR_RIGHT = 3,
        DIR_NONE = 4,  // no stick input
    };

    // mEquipStep (the outfit: shirt / hat / accessory) and mHeldEquipStep (the held item): the
    // loading of the new equipment (checkEquipLoaded / checkToolLoaded) before it is applied.
    enum EquipStep_e {
        EQUIP_STEP_NONE = 0,
        EQUIP_STEP_LOAD = 1,        // waiting for the load
        EQUIP_STEP_LOADED = 2,      // loaded: apply it
        EQUIP_STEP_APPLIED = 3,
        EQUIP_STEP_LOAD_INIT = 4,   // the first load (initEquipment)
        EQUIP_STEP_INIT_LOADED = 5, // the first load is done (held item)
    };

    // mToolTarget: what the axe (checkAxeHit) / the shovel (checkDigTarget) hits.
    enum ToolTarget_e {
        TOOL_TARGET_AXE_MISS = 1,
        TOOL_TARGET_AXE_HIT = 2,
        TOOL_TARGET_AXE_BREAK = 3,
        TOOL_TARGET_AXE_OBJECT = 4,    // an objc in the way (STATE_AXE_HIT_OBJECT)
        TOOL_TARGET_AXE_UNIT = 5,      // the front unit: a field task (dFgobjManager::useTool)
        TOOL_TARGET_SHOVEL_LOW = 6,    // the target is lower (STATE_SHOVEL_SWING_LOW)
        TOOL_TARGET_SHOVEL_HIT = 7,    // blocked ground (STATE_SHOVEL_HIT)
        TOOL_TARGET_SHOVEL_OBJECT = 8, // an objc in the way (STATE_SHOVEL_HIT_OBJECT)
        TOOL_TARGET_SHOVEL_UNIT = 9,   // the front unit: a field task (dFgobjManager::useTool)
        TOOL_TARGET_NONE = 10,         // not checked yet
    };

    // Message codes (INF1 indices) of "sys_MESS/SYS_Player".
    enum {
        MSG_JOB_DONE = 0x02,            // "Wahooo! I'm all done with work! Now I'm free!"
        MSG_DUG_UP = 0x0B,              // "Look at that! I dug up ...!"
        MSG_DUG_UP_POCKETS_FULL = 0x0C, // "But my pockets are full! What should I do?" Swap it! / Bury it again.
        MSG_POCKETS_FULL = 0x0D,        // "Oh, hang on a second! My pockets are full." Swap it! / Drop it!
        MSG_POCKETS_FULL_LEAVE = 0x0E,  // "My pockets are full, so I'll have to leave this right where it is."
        MSG_FOUND_CLOVER = 0x0F,        // "Check it out! I found a four-leaf clover!"
        MSG_AXE_BROKE = 0x10,           // "Awwww! I broke my axe..."
        MSG_MOSQUITO_BITE = 0x11,       // "Itchy itchy itchy! I got bit by a mosquito..."
        MSG_BEE_STING = 0x12,           // "I got stung by a bee!"
        MSG_EQUIP_POCKETS_FULL = 0x13,  // "Your pockets are full, so you can't put your equipment away."
        MSG_GOT_GOLD_NET = 0x29,        // "I got the golden net."
        MSG_GOT_GOLD_ROD = 0x2A,        // "I got the golden fishing rod!"
        MSG_GOT_GOLD_AXE = 0x2B,        // "I got the golden axe!"
        MSG_GOT_GOLD_SCOOP = 0x2C,      // "I got the golden shovel!"
        MSG_GOT_GOLD_WATERING = 0x2D,   // "I got the golden watering can!"
        MSG_GOT_GOLD_PACHINKO = 0x2E,   // "I got the golden slingshot!"
        MSG_GOT_MUSHROOM_FTR = 0x2F,    // "I thought it was just a mushroom, but it was really ..."
        MSG_NO_MOVE_WITH_GUESTS = 0x33, // "You cannot move items when you have guests over!"
        MSG_NO_PICK_UP_WITH_GUESTS = 0x34, // "You cannot pick up items when you have guests over!"
    };

    // Message codes shared by "sys_MESS/SYS_Get_bug" and "sys_MESS/SYS_Get_fish" (the catch is code type + 1).
    enum {
        MSG_CATCH_LET_GO = 0x47,          // "But my pockets are full! Oh well, I guess I'll have to let it go!"
        MSG_CATCH_SWAP_OR_RELEASE = 0x48, // "Should I swap it with something or release it?" Swap! / Release!
        MSG_CATCH_SWAP_OR_RELEASE_FISH = 0x49, // the same (fish only)
        MSG_CATCH_SO_THAT_MEANS = 0x4A,   // "Huh? So that means..."
        MSG_CATCH_COMPLETE = 0x4B,        // "Wahoo! I've caught every single kind of bug / fish!"
    };

    // mMsgStep: the messages queued after the current one (onMessageEnd) and the answers (onAnswer).
    enum MsgStep_e {
        MSG_STEP_ASK_DROP = 0,              // MSG_POCKETS_FULL is open (swap / drop)
        MSG_STEP_SWAP = 1,                  // answered "swap"
        MSG_STEP_SWAP_DUG = 2,              // answered "swap" to MSG_DUG_UP_POCKETS_FULL
        MSG_STEP_SWAP_INSECT = 3,           // answered "swap" for a caught insect
        MSG_STEP_SWAP_FISH = 4,             // answered "swap" for a caught fish
        MSG_STEP_QUEUE_ASK_DROP = 5,        // MSG_POCKETS_FULL follows
        MSG_STEP_ASK_DROP_QUEUED = 6,       // the queued MSG_POCKETS_FULL is open
        MSG_STEP_QUEUE_ASK_BURY = 7,        // MSG_DUG_UP_POCKETS_FULL follows
        MSG_STEP_ASK_BURY = 8,              // MSG_DUG_UP_POCKETS_FULL is open
        MSG_STEP_QUEUE_COMPLETE = 9,        // MSG_CATCH_SO_THAT_MEANS follows
        MSG_STEP_QUEUE_COMPLETE_2 = 10,     // MSG_CATCH_COMPLETE follows
        MSG_STEP_COMPLETE_QUEUED = 11,      // MSG_CATCH_COMPLETE is queued
        MSG_STEP_COMPLETE = 12,             // MSG_CATCH_COMPLETE started (the catch pose)
        MSG_STEP_QUEUE_ASK_RELEASE = 13,    // MSG_CATCH_SWAP_OR_RELEASE follows
        MSG_STEP_ASK_RELEASE = 14,          // MSG_CATCH_SWAP_OR_RELEASE is open
        MSG_STEP_RELEASE = 15,              // the catch is released
        MSG_STEP_QUEUE_ASK_RELEASE_FISH = 16, // MSG_CATCH_SWAP_OR_RELEASE_FISH follows
        MSG_STEP_ASK_RELEASE_FISH = 17,     // MSG_CATCH_SWAP_OR_RELEASE_FISH is open
        MSG_STEP_RELEASE_FISH = 18,         // the fish is released
        MSG_STEP_QUEUE_LET_GO = 19,         // MSG_CATCH_LET_GO follows
        MSG_STEP_NONE = 20,                 // nothing follows / declined
    };

    // The Mii face worn as a mask (mFace::model_c drawn as an m3d proc). It draws with the TEV
    // stages 1 / 2 of the "mii_head_mat" material (dUki_c's resource file) and the "mii_head_env"
    // environment texture on top of the RFL core stage. Size 0x1E0.
    class myFace_c : public m3d::faceEx_c {
      public:
        myFace_c();          // .text 0x2B4B0
        virtual ~myFace_c(); // .text 0x2B504

        virtual void drawXluCore();  // .text 0x2BEDC: faceEx_c's, with the fog
        virtual void setupDrawOpa(); // .text 0x2B9F4: the environment texture and TEV stages 1 / 2
        virtual void setupDrawXlu() { setupDrawOpa(); } // .text 0x2C2C4 (weak)

        void loadTevSetting(); // .text 0x2B58C: the TEV stages 1 / 2 of "mii_head_mat"

        mFace::model_c *getModel() { return &mModel; }

        // The two TEV stages (GX_TEVSTAGE1 / 2) read by loadTevSetting (defaults where the material
        // has none) and set by setupDrawOpa.
        /* 0x0E0 */ GXTevColorChan mSwapTable[4]; // GX_TEV_SWAP2: r, g, b, a
        /* 0x0F0 */ GXTevSwapSel mRasSwap[2];
        /* 0x0F8 */ GXTevSwapSel mTexSwap[2];
        /* 0x100 */ GXIndTexStageID mIndStage[2];
        /* 0x108 */ GXIndTexFormat mIndFormat[2];
        /* 0x110 */ GXIndTexBiasSel mIndBias[2];
        /* 0x118 */ GXIndTexMtxID mIndMtx[2];
        /* 0x120 */ GXIndTexWrap mIndWrapS[2];
        /* 0x128 */ GXIndTexWrap mIndWrapT[2];
        /* 0x130 */ GXBool mIndAddPrev[2];
        /* 0x132 */ GXBool mIndUtcLod[2];
        /* 0x134 */ GXIndTexAlphaSel mIndAlphaSel[2];
        /* 0x13C */ GXTexCoordID mTexCoord[2];
        /* 0x144 */ GXTexMapID mTexMap[2];
        /* 0x14C */ GXChannelID mChannel[2];
        /* 0x154 */ GXTevColorArg mColorA[2];
        /* 0x15C */ GXTevColorArg mColorB[2];
        /* 0x164 */ GXTevColorArg mColorC[2];
        /* 0x16C */ GXTevColorArg mColorD[2];
        /* 0x174 */ GXTevOp mColorOp[2];
        /* 0x17C */ GXTevBias mColorBias[2];
        /* 0x184 */ GXTevScale mColorScale[2];
        /* 0x18C */ GXBool mColorClamp[2];
        /* 0x190 */ GXTevRegID mColorOut[2];
        /* 0x198 */ GXTevAlphaArg mAlphaA[2];
        /* 0x1A0 */ GXTevAlphaArg mAlphaB[2];
        /* 0x1A8 */ GXTevAlphaArg mAlphaC[2];
        /* 0x1B0 */ GXTevAlphaArg mAlphaD[2];
        /* 0x1B8 */ GXTevOp mAlphaOp[2];
        /* 0x1C0 */ GXTevBias mAlphaBias[2];
        /* 0x1C8 */ GXTevScale mAlphaScale[2];
        /* 0x1D0 */ GXBool mAlphaClamp[2];
        /* 0x1D4 */ GXTevRegID mAlphaOut[2];
        /* 0x1DC */ u8 mBrightness; // the material color (setModelBrightness)
        u8 _1DD[3];
    };

    // The calcWorld hook of the body model: turns / curves the bones and stores
    // the node positions and matrices that dPlayerMgr hands out. Node indices are
    // looked up by name in create. Its inline dtor is emitted alongside myFace_c's
    // setupDrawXlu. Size 0x14C.
    class myMdlCallback_c : public m3d::mdlEx_c::callback_c {
      public:
        virtual ~myMdlCallback_c() {}
        virtual void timingA(ulong nodeId, nw4r::g3d::ChrAnmResult *anmRes, nw4r::g3d::ResMdl resMdl);
        virtual void timingB(ulong nodeId, nw4r::g3d::WorldMtxManip *manip, nw4r::g3d::ResMdl resMdl);
        virtual void timingC(nw4r::math::MTX34 *mtx, nw4r::g3d::ResMdl resMdl);

        /* 0x004 */ mVec3_c mSoundPos; // "root"; dPlayerMgr::getSoundPosition
        /* 0x010 */ mVec3_c mBasePos;  // "base"
        /* 0x01C */ mVec3_c mHandPos;  // "Lhand"; dPlayerMgr::getHandPosition
        /* 0x028 */ mVec3_c mRHandPos; // "Rhand"
        /* 0x034 */ mVec3_c mRFootPos; // "Rfoot3"
        /* 0x040 */ mVec3_c mLFootPos; // "Lfoot3"
        /* 0x04C */ mVec3_c mHeadPos;  // "head"; dPlayerMgr::lookAt
        /* 0x058 */ mMtx_c mRFootMtx;
        /* 0x088 */ mMtx_c mLFootMtx;
        /* 0x0B8 */ mMtx_c mFeelMtx;   // "feel"
        /* 0x0E8 */ mMtx_c mHandMtx;   // "Lhand"; dPlayerMgr::getHandTransform
        /* 0x118 */ f32 mCurveBaseY;   // dHmnAnm_c::curveMtx base height
        /* 0x11C */ f32 mRootY;        // the root node's world height
        /* 0x120 */ int mAnmId;        // the body animation (dHmnAnm_c ids)
        /* 0x124 */ ulong mBaseNode;
        /* 0x128 */ ulong mRootNode;
        /* 0x12C */ ulong mHandNode;
        /* 0x130 */ ulong mRHandNode;
        /* 0x134 */ ulong mRFootNode;
        /* 0x138 */ ulong mLFootNode;
        /* 0x13C */ ulong mHeadNode;
        /* 0x140 */ ulong mFeelNode;
        /* 0x144 */ s16 mBaseRotY;  // -mAngle.x of the actor
        /* 0x146 */ s16 mHeadRotY;
        /* 0x148 */ s16 mHeadRotX;
        /* 0x14A */ s16 mRootRotX;
    };

    f32 getMoveSpeed() const {
        return mMoveSpeed;
    }

    m3d::fanm_c *getAnm() {
        return &mAnm;
    }

    virtual ~dAcPlayer_c();

    // One entry of the state table (.rodata 0x138, 0xAB entries): init / execute / end.
    struct stateProc_s {
        void (dAcPlayer_c::*mInit)();
        void (dAcPlayer_c::*mExecute)();
        void (dAcPlayer_c::*mEnd)(int nextState);
    };
    static const stateProc_s sStateProcs[]; // .rodata 0x138

    dAcPlayer_c();                              // .text 0x528
    virtual int create();                       // .text 0x2BFB0
    virtual int doDelete();                     // .text 0xABC
    virtual int execute();                      // .text 0xCB4
    virtual int draw();                         // .text 0xC90
    virtual mVec3_c *demoHook58();              // .text 0x1EF0: the head position
    virtual int demoHook64(mVec3_c *pos);       // .text 0x1F0C: the hand position
    virtual void onMessageStart(int arg);       // .text 0x286BC
    virtual void onMessageEnd(int kind);        // .text 0x286D4
    virtual void onAnswer(int arg);             // .text 0x287C8

    void executeMain();                         // .text 0xCD8
    void drawMain();                            // .text 0xE20
    void calcState77Bg();                       // .text 0xFAC
    void calcBgCheck();                         // .text 0x10BC
    void calcShadow();                          // .text 0x1280
    void checkExit();                           // .text 0x1520
    void initActor();                           // .text 0x1674
    void calcModelMtx();                        // .text 0x1990
    void setHeadPartMtx(m3d::mdl_c *mdl);       // .text 0x1BDC
    void getNodeMtx(mMtx_c *mtx, const char *name); // .text 0x1C18
    void getRootMtx(mMtx_c *mtx);               // .text 0x1C90
    void getHeadMtx(mMtx_c *mtx);               // .text 0x1C9C
    void getRHandMtx(mMtx_c *mtx);              // .text 0x1CA8
    void setModelMtxDirect();                   // .text 0x1CB4
    u32 getParamPlayerNo(u32 param);            // .text 0x1EE0
    u32 getParamBit29(u32 param);               // .text 0x1EE8
    void setLookAngles(s16 yaw, s16 tilt, s16 a, s16 b); // .text 0x1F48
    void lookAt(const mVec3_c *target, s16 a, s16 b);    // .text 0x1F68
    int getTanLevel();                          // .text 0x2080
    int getSkinColorNo();                       // .text 0x20D0
    int getShoeNo();                            // .text 0x2140
    u32 getFaceNo();                            // .text 0x21C0
    int getGenderBodyNo();                      // .text 0x2224
    int getShoeNo2();                           // .text 0x226C
    u32 getHairNo();                            // .text 0x22EC
    void checkUnitChange();                     // .text 0x2334
    BOOL isOutOfCameraRange();                  // .text 0x248C
    static u8 objcWeightTalk(const dObjc::base_c *self, const dObjc::base_c *other, f32 *rate);    // .text 0x24FC
    static u8 objcWeightTalkOnly(const dObjc::base_c *self, const dObjc::base_c *other, f32 *rate);// .text 0x25CC
    static u8 objcWeightEnter(const dObjc::base_c *self, const dObjc::base_c *other, f32 *rate);   // .text 0x263C
    static u8 objcWeightHeight(const dObjc::base_c *self, const dObjc::base_c *other, f32 *rate);  // .text 0x26C4
    static u8 objcWeightRemote(const dObjc::base_c *self, const dObjc::base_c *other, f32 *rate);  // .text 0x2794
    static u8 objcWeightFixed(const dObjc::base_c *self, const dObjc::base_c *other, f32 *rate);   // .text 0x28E0
    static void objcHit(dObjc::base_c *other);  // .text 0x2920
    void startSound(u32 id);                    // .text 0x2A64
    void startWeatherSound(u32 id, u32 rainId); // .text 0x2A90
    void holdSound(u32 id);                     // .text 0x2AD0
    void startToolSound(u32 id);                // .text 0x2AFC
    void startFootSound(int se, f32 speed);     // .text 0x2B04
    void checkFootSound();                      // .text 0x2C04
    void checkFootSound(f32 f0, f32 f1, f32 f2, f32 f3); // .text 0x2C70
    void calcSound();                           // .text 0x2D34

    // ---- State 0x00 STATE_IDLE
    void initIdle();               // .text 2FF4
    void endIdle(int nextState);   // .text 3124
    void executeIdle();            // .text 319C
    void updateObjBalloon();       // .text 3428: the name balloon of the item in front (dObjBal_c)
    BOOL checkLeaveIdle();         // .text 33A4: another state took over, or the stick moves (STATE_WALK)
    // ---- State 0x01 STATE_WALK
    void initWalk();               // .text 35A0
    void endWalk(int nextState);   // .text 3704
    void executeWalk();            // .text 384C
    void checkWalkTransition();    // .text 3960: idle / run / trip (STATE_TRIP) from the walk
    void setMoveAnmRate(f32 maxSpeed, f32 maxRate, f32 minRate); // .text 3B88
    void setPushAnmRate(f32 ratio); // .text 3C54: pushing a snowman
    void calcWalk();               // .text 3D28: walls and pushed objects slow the move down
    void setMoveAnm(int anmId, BOOL force); // .text 3E5C: walk (1) / run (2) / dash (3)
    void calcTurn();               // .text 3FDC: turn the body (mAngle3D) and the actor to _2052
    void calcTurnToTarget();       // .text 41BC: remote players: turn to mTargetPos
    void updateMoveAnm(BOOL noLean); // .text 4364: the move animation by speed, and the lean (_2050)
    BOOL isMovingToTarget(f32 distSq, s16 angle); // .text 4520: remote players
    void startMoveAnm(f32 morph, f32 frame, int anmId); // .text 45C4
    s16 calcTurnStep(f32 ratio, s16 maxStep, s16 minStep) const; // .text 473C
    void executeWalkMine();        // .text 4798: this console's player
    void executeWalkRemote();      // .text 49B4: another console's player
    f32 getRemoteTargetDistSq(s16 *angle, int mode); // .text 4BA0: mTargetPos from dPlayerMgr::getRemoteTransform
    void clampApproachSpeed(f32 dist, f32 *speed) const; // .text 4D04: slow down near mTargetPos
    void syncToolAnm();            // .text 4D8C: the held net / windmill follows the move
    void updateRemoteMove(s16 angle, BOOL animate); // .text 4DF8
    // The mRemoteActionParams payloads of these states (dPlayerMngr_c::actionParam_c, 0xC bytes): what
    // this console's player sends, read back by the other consoles' copies of the actor.
    struct moveParam_c { // STATE_WALK
        void set(f32 speed, u8 count); // .text 5018
        /* 0x0 */ f32 mSpeed;
        /* 0x4 */ u8 mCount; // counts up every 10 frames (_22BE)
    };
    struct runParam_c { // STATE_RUN
        void set(s16 angle); // .text 57A8
        /* 0x0 */ s16 mAngle;
    };
    struct equipParam_c { // STATE_CHANGE_EQUIPMENT
        void set(u16 equipFlags, u8 step); // .text 5D30
        /* 0x0 */ u16 mEquipFlags; // mEquipFlags
        /* 0x2 */ u8 mStep;        // the remote copy of mStep
    };
    struct heldItemParam_c { // STATE_CHANGE_HELD_ITEM (written by .text 75CC)
        /* 0x0 */ u8 mChange; // mHeldItemChange
        /* 0x1 */ u8 mStep;   // the remote copy of mStep
        /* 0x2 */ u8 mHoldPose;
    };
    struct demoStartParam_c { // STATE_DEMO_START (written by .text 7A6C)
        /* 0x0 */ u8 mEatSpecial; // STATE_EAT_SPECIAL
    };
    moveParam_c *getMoveParam() { return reinterpret_cast<moveParam_c *>(&mRemoteActionParams); }
    runParam_c *getRunParam() { return reinterpret_cast<runParam_c *>(&mRemoteActionParams); }
    equipParam_c *getEquipParam() { return reinterpret_cast<equipParam_c *>(&mRemoteActionParams); }
    heldItemParam_c *getHeldItemParam() { return reinterpret_cast<heldItemParam_c *>(&mRemoteActionParams); }
    demoStartParam_c *getDemoStartParam() { return reinterpret_cast<demoStartParam_c *>(&mRemoteActionParams); }
    // ---- State 0x02 STATE_RUN
    void endRun(int nextState);    // .text 5024
    void executeChangeEquipMine(int nextState, int changeAnm, int waitAnm); // .text 514C
    void initRun();                // .text 53A4
    void executeRun();             // .text 5564
    static u16 countDown(u16 *timer); // .text 57B0: the timer after the decrement
    // ---- State 0x05 STATE_CHANGE_EQUIPMENT
    void initChangeEquipment();    // .text 57CC
    void executeChangeEquipment(); // .text 5810
    void endChangeEquipment(int nextState); // .text 5870
    void executeChangeEquipRemote(int changeAnm, int waitAnm); // .text 58D0
    void startChangeEquip();       // .text 5B0C
    void createChangeEffect();     // .text 5C20: the dust of the change (by surface)
    void playWearSound();          // .text 5C90
    // ---- State 0x07 STATE_DEMO_START
    void initDemoStart();          // .text 5D3C
    int checkNetSwingHit();        // .text 5F84: 0 swinging, 1 hit (ground / water), 2 caught, 3 stopped
    void stepGetInsectMsg();       // .text 6370: "sys_MESS/SYS_Get_bug"
    void createCatchEffect(dInsFldBase_c *actor); // .text 6580: "af_ply_kore" for some insects
    void stepGetFishMsg();         // .text 65D8: "sys_MESS/SYS_Get_fish"
    void stepPlayerMsg();          // .text 688C: "sys_MESS/SYS_Player" 0x33
    void checkDigUnit(u32 mode, BOOL alt); // .text 6988: the shovel hits the target unit
    void calcDigEffects();         // .text 6B0C: dig HMN_ANM_SCOOP_STUMP1 / HMN_ANM_SCOOP_DIG1
    void calcBuryEffects();        // .text 6D60
    // ---- State 0x06 STATE_CHANGE_HELD_ITEM
    void initChangeHeldItem();     // .text 6E50
    bool isHoldPose() const { return (mActorFlags & dPlayerMgr::ACTOR_FLAG_MENU_ACTION) && !(mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_EQUIP_FROM_MENU); }

    // Views of mRemoteActionParams: the local player's state handlers write them (static setters
    // below), the remote copies of the player read them.
    struct heldItemParam_s {   // STATE_CHANGE_HELD_ITEM
        u8 mChange;            // mHeldItemChange
        u8 mStep;
        u8 mHeldOut;           // the tool stays out (HMN_ANM_MENU_THINK1 / HMN_ANM_MENU_EQUIP1)
    };
    struct demoStartParam_s {  // STATE_DEMO_START
        u8 mEating;            // HMN_ANM_MENU_THINK2 ("af_mnp_delicious")
    };
    struct demoWaitParam_s {   // STATE_DEMO_WAIT
        u16 mAnmId;
        s16 mLookYaw;
        s16 mLookTilt;
        u8 mToolOut;
    };
    struct doorParam_s {       // STATE_ENTER_DOOR / STATE_EXIT_DOOR
        u8 mDoorType;
        u8 mWait;              // the frames the door takes to open
    };
    struct walkThroughDoorParam_s { // STATE_WALK_THROUGH_DOOR
        u8 mDemoWaitNext;      // ACTOR_FLAG2_EQUIP_FROM_MENU: STATE_DEMO_WAIT after, else STATE_ENTER_DOOR
    };
    struct netCatchParam_s {   // STATE_NET_CATCH
        u16 mItem;             // the caught insect's item
    };
    struct netPocketParam_s {  // STATE_NET_POCKET
        u8 mPocketFull;        // ACTOR_FLAG_POCKETS_FULL
    };
    struct floatPosParam_s {   // fishing: the float's (target) position
        f32 mX;
        f32 mZ;
    };
    struct fishingWaitParam_s { // STATE_FISHING_WAIT
        f32 mX;
        f32 mZ;
        u8 mFloatState;        // dUki_c::State_e
    };

    void executeChangeHeldItem();               // .text 0x6F58
    void endChangeHeldItem(int nextState);      // .text 0x6FA4
    void executeChangeHeldItemLocal();          // .text 0x7060
    void executeChangeHeldItemRemote();         // .text 0x731C
    static void setHeldItemParam(heldItemParam_s *param, u8 change, u8 step, u8 heldOut); // .text 0x75CC
    void initChangeDesign();                    // .text 0x75DC
    void executeChangeDesign();                 // .text 0x7618
    void endChangeDesign(int nextState);        // .text 0x7678
    void initMiiMask();                         // .text 0x76D8
    void executeMiiMask();                      // .text 0x774C
    void endMiiMask(int nextState);             // .text 0x78B0
    void endDemoStart(int nextState);           // .text 0x7908
    void executeDemoStart();                    // .text 0x790C
    static void setDemoStartParam(demoStartParam_s *param, BOOL eating); // .text 0x7A6C
    void initDemoWait();                        // .text 0x7A74
    void endDemoWait(int nextState);            // .text 0x7BA8
    void executeDemoWait();                     // .text 0x7C3C
    void faceTalkActor();                       // .text 0x7D48
    static void setDemoWaitParam(demoWaitParam_s *param, u16 anmId, s16 lookYaw, s16 lookTilt,
                                 BOOL toolOut); // .text 0x7EB4
    void initEnterDoor();                       // .text 0x7EC8
    void executeEnterDoor();                    // .text 0x8100
    void endEnterDoor(int nextState);           // .text 0x831C
    static void setEnterDoorParam(doorParam_s *param, u8 doorType, u8 wait); // .text 0x832C
    void initExitDoor();                        // .text 0x8338
    void executeExitDoor();                     // .text 0x85BC
    void endExitDoor(int nextState);            // .text 0x881C
    static void setExitDoorParam(doorParam_s *param, u8 doorType); // .text 0x88AC
    void initWalkThroughDoor();                 // .text 0x88B4
    void executeWalkThroughDoor();              // .text 0x8964
    void endWalkThroughDoor(int nextState);     // .text 0x8AB8
    static void setWalkThroughDoorParam(walkThroughDoorParam_s *param, u8 demoWaitNext); // .text 0x8AC8
    void initNetSwing();                        // .text 0x8AD0
    void executeNetSwing();                     // .text 0x8C40
    void endNetSwing(int nextState);            // .text 0x8F74
    void getModelOffsetPos(mVec3_c *pos, f32 x, f32 y, f32 z); // .text 0x904C
    void checkNetInsect();                      // .text 0x916C
    BOOL checkNetHit();                         // .text 0x92D4
    void initNetCatch();                        // .text 0x9660
    void executeNetCatch();                     // .text 0x98E0
    void endNetCatch(int nextState);            // .text 0x9B0C
    static void setNetCatchParam(netCatchParam_s *param, u16 item); // .text 0x9BC4
    void initNetPocket();                       // .text 0x9BCC
    void executeNetPocket();                    // .text 0x9CB0
    void endNetPocket(int nextState);           // .text 0x9E90
    static void setNetPocketParam(netPocketParam_s *param, u8 pocketFull); // .text 0x9F1C
    void initFishingCast();                     // .text 0x9F24
    void executeFishingCast();                  // .text 0xA02C
    void endFishingCast(int nextState);         // .text 0xA1B8
    BOOL isFishingState(int state);             // .text 0xA268
    static void setFishingCastParam(floatPosParam_s *param, f32 x, f32 z); // .text 0xA298
    void initFishingCastMiss();                 // .text 0xA2A4
    void executeFishingCastMiss();              // .text 0xA3A0
    void endFishingCastMiss(int nextState);     // .text 0xA4D0
    static void setFishingCastMissParam(floatPosParam_s *param, f32 x, f32 z); // .text 0xA53C
    void initFishingWait();                     // .text 0xA548
    void executeFishingWait();                  // .text 0xA644
    void endFishingWait(int nextState);         // .text 0xA8DC
    static void setFishingWaitParam(fishingWaitParam_s *param, f32 x, f32 z, u8 floatState); // .text 0xA948
    void initFishingHooked();                   // .text 0xA958
    void executeFishingHooked();                // .text 0xAB10
    void endFishingHooked(int nextState);       // .text 0xACAC
    static void setFishingHookedParam(floatPosParam_s *param, f32 x, f32 z); // .text 0xAD58
    void initFishingFail();                     // .text 0xAD64

    // ---- State 0x1C STATE_FISHING_FAIL : the fish got away, reel in the empty line
    void executeFishingFail();             // .text AE34
    void endFishingFail(int nextState);    // .text AF00
    // The mRemoteActionParams payloads of the fishing states: the float's position (dUki_c::mPos[0]).
    struct fishingFailParam_c { // STATE_FISHING_FAIL
        void set(f32 x, f32 z); // .text AF88
        /* 0x0 */ f32 mFloatX;
        /* 0x4 */ f32 mFloatZ;
    };
    struct fishingPutBackParam_c { // STATE_FISHING_PUT_BACK
        void set(f32 x, f32 z); // .text B1E8
        /* 0x0 */ f32 mFloatX;
        /* 0x4 */ f32 mFloatZ;
    };
    struct fishingLandParam_c { // STATE_FISHING_LAND
        void set(f32 x, f32 z); // .text B3D8
        /* 0x0 */ f32 mFloatX;
        /* 0x4 */ f32 mFloatZ;
    };
    struct fishingPutAwayParam_c { // STATE_FISHING_PUT_AWAY
        void set(u8 full); // .text BA08
        /* 0x0 */ u8 mFull; // ACTOR_FLAG_POCKETS_FULL: no room for the fish (next STATE 0x5C)
    };
    fishingFailParam_c *getFishingFailParam() { return reinterpret_cast<fishingFailParam_c *>(&mRemoteActionParams); }
    fishingPutBackParam_c *getFishingPutBackParam() { return reinterpret_cast<fishingPutBackParam_c *>(&mRemoteActionParams); }
    fishingLandParam_c *getFishingLandParam() { return reinterpret_cast<fishingLandParam_c *>(&mRemoteActionParams); }
    fishingPutAwayParam_c *getFishingPutAwayParam() { return reinterpret_cast<fishingPutAwayParam_c *>(&mRemoteActionParams); }
    // ---- State 0x1D STATE_FISHING_PUT_BACK: a fish too big / not wanted goes back
    void initFishingPutBack();             // .text AF94
    void executeFishingPutBack();          // .text B098
    void endFishingPutBack(int nextState); // .text B164
    // ---- State 0x1E STATE_FISHING_LAND: pull the hooked fish out (setPoleGet1)
    void initFishingLand();                // .text B1F4
    void executeFishingLand();             // .text B2D0
    void endFishingLand(int nextState);    // .text B350
    // ---- State 0x1F STATE_FISHING_GET: show the fish, "SYS_Get_fish" (setPoleGet2)
    void initFishingGet();                 // .text B3E4
    void executeFishingGet();              // .text B5F8
    void endFishingGet(int nextState);     // .text B660
    // ---- State 0x20 STATE_FISHING_PUT_AWAY: put the rod / fish away (setPolePutaway)
    void initFishingPutAway();             // .text B738
    void executeFishingPutAway();          // .text B7F4
    void endFishingPutAway(int nextState); // .text B990
    // ---- State 0x0B STATE_RETURN_HOME: outdoors the player collapses (0x84 / 0x85 with a tool) and is
    // sent to its house (requestHomeScene, frame 141); indoors it leaves through the door (7 / 0xA)
    void initReturnHome();                 // .text BA10
    void executeReturnHome();              // .text BBC0
    void endReturnHome(int nextState);     // .text BC28
    struct returnHomeParam_c { // STATE_RETURN_HOME
        void set(u8 doorType); // .text BC88
        /* 0x0 */ u8 mDoorType; // dSTR::getCurBuildingType, 2 when collapsing outdoors
    };
    returnHomeParam_c *getReturnHomeParam() { return reinterpret_cast<returnHomeParam_c *>(&mRemoteActionParams); }
    // The furniture states (dFTR::holdList_c). mSeatPose holds the stick direction (Direction_e) while
    // holding; mSeatSide the side of a bed / chair.
    // The pose code of encodeSeatPose / decodeSeatPose: the target angle's quadrant and mSeatSide.
    struct ftrGrabParam_c { // STATE_FTR_GRAB
        void set(f32 x, f32 z, u8 pose); // .text C0D8
        /* 0x0 */ f32 mX; // mTargetPos
        /* 0x4 */ f32 mZ;
        /* 0x8 */ u8 mPose;
    };
    struct ftrRotateParam_c { // STATE_FTR_ROTATE
        void set(u8 left); // .text CAE8
        /* 0x0 */ u8 mLeft; // ACTOR_FLAG_FTR_ROTATE_LEFT
    };
    struct getIntoBedParam_c { // STATE_GET_INTO_BED
        void set(f32 x, f32 z, u8 pose); // .text D720
        /* 0x0 */ f32 mX;
        /* 0x4 */ f32 mZ;
        /* 0x8 */ u8 mPose;
    };
    struct getOutOfBedParam_c { // STATE_GET_OUT_OF_BED
        void set(BOOL right); // .text DC18
        /* 0x0 */ u8 mRight; // mSeatPose == DIR_RIGHT
    };
    struct bedRollParam_c { // STATE_BED_ROLL
        void set(BOOL right); // .text DF3C
        /* 0x0 */ u8 mRight;
    };
    struct bedRollOverParam_c { // STATE_BED_ROLL_OVER
        void set(BOOL right); // .text E268
        /* 0x0 */ u8 mRight;
    };
    struct bedRollBlockedParam_c { // STATE_BED_ROLL_BLOCKED
        void set(BOOL right); // .text E428
        /* 0x0 */ u8 mRight;
    };
    struct sitDownFrontParam_c { // STATE_SIT_DOWN_FRONT
        void set(f32 x, f32 z, u8 pose); // .text E8C8
        /* 0x0 */ f32 mX;
        /* 0x4 */ f32 mZ;
        /* 0x8 */ u8 mPose;
    };
    struct sitDownSideParam_c { // STATE_SIT_DOWN_SIDE
        void set(f32 x, f32 z, u8 pose); // .text EC30
        /* 0x0 */ f32 mX;
        /* 0x4 */ f32 mZ;
        /* 0x8 */ u8 mPose;
    };
    ftrGrabParam_c *getFtrGrabParam() { return reinterpret_cast<ftrGrabParam_c *>(&mRemoteActionParams); }
    ftrRotateParam_c *getFtrRotateParam() { return reinterpret_cast<ftrRotateParam_c *>(&mRemoteActionParams); }
    getIntoBedParam_c *getGetIntoBedParam() { return reinterpret_cast<getIntoBedParam_c *>(&mRemoteActionParams); }
    getOutOfBedParam_c *getGetOutOfBedParam() { return reinterpret_cast<getOutOfBedParam_c *>(&mRemoteActionParams); }
    bedRollParam_c *getBedRollParam() { return reinterpret_cast<bedRollParam_c *>(&mRemoteActionParams); }
    bedRollOverParam_c *getBedRollOverParam() { return reinterpret_cast<bedRollOverParam_c *>(&mRemoteActionParams); }
    bedRollBlockedParam_c *getBedRollBlockedParam() { return reinterpret_cast<bedRollBlockedParam_c *>(&mRemoteActionParams); }
    sitDownFrontParam_c *getSitDownFrontParam() { return reinterpret_cast<sitDownFrontParam_c *>(&mRemoteActionParams); }
    sitDownSideParam_c *getSitDownSideParam() { return reinterpret_cast<sitDownSideParam_c *>(&mRemoteActionParams); }
    // ---- State 0x23 STATE_FTR_GRAB: walk to the furniture and take hold (startHold)
    void initFtrGrab();                    // .text BC90
    void executeFtrGrab();                 // .text BEA0
    void endFtrGrab(int nextState);        // .text C0B0
    // ---- State 0x24 STATE_FTR_HOLD: holding on, the stick picks push / pull / rotate
    void initFtrHold();                    // .text C0E8
    void executeFtrHold();                 // .text C268
    void endFtrHold(int nextState);        // .text C6A4
    BOOL isNoStickInput();                 // .text C718: isNoMoveInput (the move speed is 0)
    void updateHoldDir();                  // .text C71C: mSeatPose = the stick direction (frame 48 on)
    BOOL isHoldDirChanged(int dir);        // .text C77C: the stick let go or turned away from dir
    // ---- State 0x25 STATE_FTR_ROTATE (startRotateLeft / startRotateRight)
    void initFtrRotate();                  // .text C7E4
    void executeFtrRotate();               // .text C90C
    void endFtrRotate(int nextState);      // .text CAE4
    // ---- State 0x26 STATE_FTR_PUSH_START: lean on it; canPush at frame 11
    void initFtrPushStart();               // .text CAF0
    void executeFtrPushStart();            // .text CB18
    void endFtrPushStart(int nextState);   // .text CC28
    // ---- State 0x27 STATE_FTR_PUSH (startPush)
    void initFtrPush();                    // .text CC2C
    void executeFtrPush();                 // .text CD14
    void endFtrPush(int nextState);        // .text CDEC
    // ---- State 0x28 STATE_FTR_PULL_START: canPull at frame 11
    void initFtrPullStart();               // .text CE78
    void executeFtrPullStart();            // .text CEA0
    void endFtrPullStart(int nextState);   // .text CFB0
    // ---- State 0x29 STATE_FTR_PULL (startPull)
    void initFtrPull();                    // .text CFB4
    void executeFtrPull();                 // .text D054
    void endFtrPull(int nextState);        // .text D174
    // ---- State 0x2A STATE_LIE_DOWN: ask for the bed (dFTR::requestPull)
    void initLieDown();                    // .text D240
    void executeLieDown();                 // .text D2EC
    void endLieDown(int nextState);        // .text D390
    // ---- State 0x2B STATE_GET_INTO_BED (HMN_ANM_BED_SLEEPL1 / HMN_ANM_BED_SLEEPR1, then 0x37 lying)
    void initGetIntoBed();                 // .text D394
    void executeGetIntoBed();              // .text D4EC
    void endGetIntoBed(int nextState);     // .text D624
    void setBedPos();                      // .text D688: on the bed, turned by mSeatSide
    // ---- State 0x2C STATE_GET_UP: ask to stand up beside the bed (dFTR::requestRotate)
    void initGetUp();                      // .text D730
    void executeGetUp();                   // .text D7FC
    void endGetUp(int nextState);          // .text D8AC
    // ---- State 0x2D STATE_GET_OUT_OF_BED (HMN_ANM_BED_WAKEUPL1 / HMN_ANM_BED_WAKEUPR1)
    void initGetOutOfBed();                // .text D8B0
    void executeGetOutOfBed();             // .text D9D4
    void endGetOutOfBed(int nextState);    // .text DB40
    void setGetOutOfBedPos();              // .text DBB4: at the root node, turned to the side
    // ---- State 0x2E STATE_BED_WAIT (HMN_ANM_BED_SLEEP_WAIT1)
    void initBedWait();                    // .text DC20
    void executeBedWait();                 // .text DC64
    void endBedWait(int nextState);        // .text DD98
    // ---- State 0x2F STATE_BED_ROLL: ask to roll over (dFTR::requestMove)
    void initBedRoll();                    // .text DDA8
    void executeBedRoll();                 // .text DEAC
    void endBedRoll(int nextState);        // .text DF38
    // ---- State 0x30 STATE_BED_ROLL_OVER (HMN_ANM_BED_TURNLR2 / HMN_ANM_BED_TURNRL2)
    void initBedRollOver();                // .text DF44
    void executeBedRollOver();             // .text E088
    void endBedRollOver(int nextState);    // .text E124
    void setBedRollPos();                  // .text E188
    // ---- State 0x31 STATE_BED_ROLL_BLOCKED: the roll was refused (HMN_ANM_BED_TURNLR3 / HMN_ANM_BED_TURNRL3)
    void initBedRollBlocked();             // .text E270
    void executeBedRollBlocked();          // .text E3A4
    void endBedRollBlocked(int nextState); // .text E424
    // ---- State 0x35 STATE_SIT_DOWN: ask for the chair (dFTR::requestHold)
    void initSitDown();                    // .text E430
    void executeSitDown();                 // .text E4EC
    void endSitDown(int nextState);        // .text E5C0
    // ---- State 0x36 STATE_SIT_DOWN_FRONT (HMN_ANM_CHAIR_SITDOWNF1, then 0x31 seated)
    void initSitDownFront();               // .text E5C4
    void executeSitDownFront();            // .text E6B4
    void endSitDownFront(int nextState);   // .text E7EC
    void setChairFrontPos();               // .text E850
    // ---- State 0x37 STATE_SIT_DOWN_SIDE (HMN_ANM_CHAIR_SITDOWNL1 / HMN_ANM_CHAIR_SITDOWNR1)
    void initSitDownSide();                // .text E8D8
    void executeSitDownSide();             // .text E9FC
    void endSitDownSide(int nextState);    // .text EB34
    void setChairSidePos();                // .text EB98
    // ---- State 0x38 STATE_CHAIR_WAIT (HMN_ANM_CHAIR_SITDOWN_WAIT1)
    void initChairWait();                  // .text EC40
    void executeChairWait();               // .text EC84

    // Remote payload of the unit-targeted states (tree shaking): the target unit.
    struct unitParam_s {
        u8 mUnitX; // mTargetUnitX
        u8 mUnitZ; // mTargetUnitZ
    };
    struct chairFaceFrontParam_s { // 0x3B
        u8 mFromLeft;              // mSeatPose DIR_RIGHT, else DIR_LEFT
    };
    // ---- State 0x38 STATE_CHAIR_WAIT
    void endChairWait(int nextState);           // .text 0xED80
    // ---- State 0x39 STATE_CHAIR_POSE
    void initChairPose();                       // .text 0xED90
    void executeChairPose();                    // .text 0xEE70
    void endChairPose(int nextState);           // .text 0xEF48
    // ---- State 0x3A STATE_CHAIR_STAND_UP
    void initChairStandUp();                    // .text 0xEF4C
    void executeChairStandUp();                 // .text 0xF040
    void endChairStandUp(int nextState);        // .text 0xF190
    void setStandUpPos();                       // .text 0xF204: mPos = _205C, shadow back to full
    // ---- State 0x3B STATE_CHAIR_FACE_FRONT
    void initChairFaceFront();                  // .text 0xF230
    void executeChairFaceFront();               // .text 0xF344
    void endChairFaceFront(int nextState);      // .text 0xF400
    static void setChairFaceFrontParam(chairFaceFrontParam_s *param, BOOL fromLeft); // .text 0xF434
    // ---- State 0x21 STATE_TAKE_OUT_TOOL
    void initTakeOutTool();                     // .text 0xF43C
    void executeTakeOutTool();                  // .text 0xF4A8
    void endTakeOutTool(int nextState);         // .text 0xF62C
    // ---- State 0x22 STATE_PUT_AWAY_TOOL
    void initPutAwayTool();                     // .text 0xF654
    void executePutAwayTool();                  // .text 0xF6E4
    void endPutAwayTool(int nextState);         // .text 0xF85C
    // ---- State 0x3D STATE_SHAKE_TREE_END
    void initShakeTreeEnd();                    // .text 0xF8F0
    void executeShakeTreeEnd();                 // .text 0xF9B0
    void endShakeTreeEnd(int nextState);        // .text 0xFB2C
    static void setShakeTreeEndParam(unitParam_s *param, u8 unitX, u8 unitZ); // .text 0xFBA4
    // ---- State 0x3C STATE_SHAKE_TREE
    void initShakeTree();                       // .text 0xFBB0
    void executeShakeTree();                    // .text 0xFC64
    void endShakeTree(int nextState);           // .text 0xFDD4
    static void setShakeTreeParam(unitParam_s *param, u8 unitX, u8 unitZ); // .text 0xFF10

    // ---- State 0x3E STATE_SHOVEL_START: the swing; picks the result in mToolTarget (checkDigTarget)
    void initShovelStart();                     // .text 0xFF1C
    void executeShovelStart();                  // .text 0x10044
    void endShovelStart(int nextState);         // .text 0x1013C
    void initDigTilt(int frames);               // .text 0x101C0: lean (_22CE, mRootRotX) to the target unit's slope
    void calcDigTilt(f32 tiltEnd, f32 untiltStart); // .text 0x10348
    void resetDigTilt();                        // .text 0x103DC
    void initDigApproach(f32 frames, BOOL backOff); // .text 0x103F0: the step (_21D0[0]) to mTargetPos
    void calcDigApproach(f32 endFrame);         // .text 0x1055C
    void checkDigTarget();                      // .text 0x105CC: mToolTarget
    // ---- State 0x3F STATE_SHOVEL_WAIT: waits for the field task (_229C)
    void initShovelWait();                      // .text 0x1079C
    void executeShovelWait();                   // .text 0x10830
    void endShovelWait(int nextState);          // .text 0x1093C
    // ---- State 0x41 STATE_SHOVEL_HIT
    struct shovelHitParam_s {
        s16 mAngle;
        u8 mUnitX;
        u8 mUnitZ;
        u8 _4;
        u8 mChop;       // FG_MNG_KIND_HIT_MONEY_ROCK: procChopResult at frame 12
        u8 mTaskStage1; // mShovelTaskStage == 1
    };
    void initShovelHit();                       // .text 0x10940
    void executeShovelHit();                    // .text 0x10AE4
    void endShovelHit(int nextState);           // .text 0x10CDC
    static void setShovelHitParam(shovelHitParam_s *param, s16 angle, u8 unitX, u8 unitZ, u8 arg4, BOOL chop,
                                  BOOL taskStage1); // .text 0x10D70

    // ---- State 0x40 STATE_SHOVEL_DIG: digs at the unit; spawnDigUp (_2260), with a tree spawnFellTree (_2262)
    struct shovelDigParam_s {
        u16 mTreeItem; // _2262
        u16 mItem;     // _2260
        u8 mUnitX;
        u8 mUnitZ;
        u8 mTree;      // FG_MNG_KIND_DIG_OUT (HMN_ANM_SCOOP_STUMP1)
    };
    void initShovelDig();                       // .text 0x10D8C
    void executeShovelDig();                    // .text 0x10EEC
    void endShovelDig(int nextState);           // .text 0x11068
    void spawnDigResult(BOOL tree);             // .text 0x110E0
    void createDigEffect(const char *sandEffect, const char *effect); // .text 0x11174: at the target unit
    static void setShovelDigParam(shovelDigParam_s *param, u16 treeItem, u16 item, u8 unitX, u8 unitZ,
                                  u8 tree);     // .text 0x11230
    // ---- State 0x42 STATE_SHOVEL_MISS (HMN_ANM_SCOOP_CANCEL1)
    void initShovelMiss();                      // .text 0x11248
    void executeShovelMiss();                   // .text 0x11270
    void endShovelMiss(int nextState);          // .text 0x112E8
    // ---- State 0x43 STATE_SHOVEL_SWING_LOW (HMN_ANM_SCOOP_SUKA1): the target is lower (TOOL_TARGET_SHOVEL_LOW)
    void initShovelSwingLow();                  // .text 0x112EC
    void executeShovelSwingLow();               // .text 0x11314
    void endShovelSwingLow(int nextState);      // .text 0x113F0
    // ---- State 0x44 STATE_PUT_ITEM_IN_HOLE
    void initPutItemInHole();                   // .text 0x113F4
    void executePutItemInHole();                // .text 0x11504
    void endPutItemInHole(int nextState);       // .text 0x116E0
    // ---- State 0x45 STATE_SHOVEL_BURY (HMN_ANM_SCOOP_BURY1): fills the hole (after STATE_PUT_ITEM_IN_HOLE)
    struct shovelBuryParam_s {
        u16 mItem;  // _218C
        u8 mUnitX;
        u8 mUnitZ;
        u8 mPutItem; // putItemInHole, not from a task: the hold pose (HMN_ANM_MENU_THINK1) after
    };
    void initShovelBury();                      // .text 0x116E4
    void executeShovelBury();                   // .text 0x117D0
    void endShovelBury(int nextState);          // .text 0x11A10
    void createStampEffect();                   // .text 0x11A88: "af_ply_stamp_*" at the target unit
    static void setShovelBuryParam(shovelBuryParam_s *param, u16 item, u8 unitX, u8 unitZ,
                                   u8 putItem); // .text 0x11AF8
    // ---- State 0x46 STATE_SHOVEL_DIG_UP_ITEM (HMN_ANM_SCOOP_GET1): shows the dug-up item (mActionItem)
    struct digUpItemParam_s { // written by digUpParam_c::set
        u16 mItem;
        u8 mUnitX;
        u8 mUnitZ;
        u8 mSpecial; // FG_MNG_KIND_DIG_UP_FULL / FG_MNG_KIND_PICK_UP_SPECIAL_FULL; ACTOR_FLAG_POCKETS_FULL on the remote side
    };
    void executeShovelDigUpItem();              // .text 0x11B0C
    // ---- State 0x57 STATE_STAMP_HOLE: fills a hole with the foot ("af_ply_footbury")
    void executeStampHole();                    // .text 0x11F1C
    // ---- State 0x59 STATE_PITFALL_FALL
    void executePitfallFall();                  // .text 0x120D4
    // ---- State 0x5B STATE_PITFALL_CLIMB_OUT
    void executePitfallClimbOut();              // .text 0x12390
    // ---- State 0x12 STATE_AXE_CHOP
    void executeAxeChop();                      // .text 0x124EC
    void createAxeHitEffect();                  // .text 0x126C8: "af_itm_axe_hitstar"
    // ---- State 0x15 STATE_AXE_BREAK
    void executeAxeBreak();                     // .text 0x1271C
    // ---- State 0x46 (init / end)
    void initShovelDigUpItem();                 // .text 0x129C4
    void endShovelDigUpItem(int nextState);     // .text 0x12AFC
    void stepDigUpMsg();                        // .text 0x12BA0: "sys_MESS/SYS_Player" 0xB (item name) / 0x2C / 0x2E

    // The mRemoteActionParams payloads of these states.
    struct digUpParam_c { // state 0x46
        void set(u16 item, u8 unitX, u8 unitZ, u8 full); // .text 12E10
        /* 0x0 */ u16 mItem;
        /* 0x2 */ u8 mUnitX;
        /* 0x3 */ u8 mUnitZ;
        /* 0x4 */ u8 mFull; // ACTOR_FLAG_POCKETS_FULL: no room in the pockets
    };
    struct showDugItemParam_c { // STATE_SHOW_DUG_ITEM
        void set(u16 item, u8 full); // .text 12F44
        /* 0x0 */ u16 mItem;
        /* 0x2 */ u8 mFull;
    };
    struct fillHoleParam_c { // STATE_STAMP_HOLE
        void set(u8 unitX, u8 unitZ); // .text 1306C
        /* 0x0 */ u8 mUnitX;
        /* 0x1 */ u8 mUnitZ;
    };
    struct pickUpParam_c { // STATE_PICK_UP_ITEM
        void set(u16 item, u8 unitX, u8 unitZ, u8 full); // .text 13A70
        /* 0x0 */ u16 mItem;
        /* 0x2 */ u8 mUnitX;
        /* 0x3 */ u8 mUnitZ;
        /* 0x4 */ u8 mFull;
    };
    struct pickUpMsgParam_c { // STATE_PICK_UP_ITEM_MSG
        void set(u16 item, u8 unitX, u8 unitZ, u8 full); // .text 14384
        /* 0x0 */ u16 mItem;
        /* 0x2 */ u8 mUnitX;
        /* 0x3 */ u8 mUnitZ;
        /* 0x4 */ u8 mFull;
    };
    digUpParam_c *getDigUpParam() { return reinterpret_cast<digUpParam_c *>(&mRemoteActionParams); }
    showDugItemParam_c *getShowDugItemParam() { return reinterpret_cast<showDugItemParam_c *>(&mRemoteActionParams); }
    fillHoleParam_c *getFillHoleParam() { return reinterpret_cast<fillHoleParam_c *>(&mRemoteActionParams); }
    pickUpParam_c *getPickUpParam() { return reinterpret_cast<pickUpParam_c *>(&mRemoteActionParams); }
    pickUpMsgParam_c *getPickUpMsgParam() { return reinterpret_cast<pickUpMsgParam_c *>(&mRemoteActionParams); }
    void spawnDugItem(int full);           // .text 12DF4: state 0x46's dFgobjManager::spawnDigUp
    // ---- State 0x47 STATE_SHOW_DUG_ITEM (HMN_ANM_SCOOP_PUTAWAY1)
    void initShowDugItem();                // .text 12E24
    void executeShowDugItem();             // .text 12EC4
    void endShowDugItem(int nextState);    // .text 12F40
    // ---- State 0x57 STATE_STAMP_HOLE (HMN_ANM_HOLE_BURY1)
    void initFillHole();                   // .text 12F50
    void endFillHole(int nextState);       // .text 13000
    // ---- State 0x51 STATE_PICK_UP_REACH (HMN_ANM_PICKUP_WAIT1)
    void initPickUpReach();                // .text 13078
    void executePickUpReach();             // .text 130EC
    void endPickUpReach(int nextState);    // .text 13160
    // ---- State 0x52 STATE_PICK_UP_ITEM (HMN_ANM_PICKUP1): the item flies to the body and shrinks
    void initPickUpItem();                 // .text 13164
    void executePickUpItem();              // .text 132BC
    void endPickUpItem(int nextState);     // .text 133C0
    void initPickUpItemPos();              // .text 1341C: mItemPos on the field, mShowPos by the root node
    void setItemPosToHand();               // .text 13548
    void updatePickUpItemMine(f32 scale, f32 frame);   // .text 1357C
    void updatePickUpItemRemote(f32 scale, f32 frame); // .text 13708
    void takeFieldItem(BOOL release);      // .text 13900: sound / grass effect, fgMngTask_procAt / releaseAt
    // ---- State 0x53 STATE_PICK_UP_ITEM_MSG (HMN_ANM_PICKUP_GET1): lift the item up, SYS_Player message
    void initPickUpItemMsg();              // .text 13A84
    void executePickUpItemMsg();           // .text 13C1C
    void endPickUpItemMsg(int nextState);  // .text 13DC8
    void setLiftItemPos(BOOL chase);       // .text 13E64
    void updateLiftItem(f32 frame);        // .text 14024
    void stepPickUpMsg();                  // .text 14194
    void executeShowItem(int anmId, BOOL full, BOOL toolAfter); // .text 144B8: shrink into the pockets
    struct showPickedItemParam_c { // STATE_SHOW_PICKED_ITEM
        void set(u16 item, u8 full); // .text 14658
        /* 0x0 */ u16 mItem;
        /* 0x2 */ u8 mFull;
    };
    struct pullWeedParam_c { // STATE_PULL_WEED
        void set(u8 unitX, u8 unitZ); // .text 148E0
        /* 0x0 */ u8 mUnitX;
        /* 0x1 */ u8 mUnitZ;
    };
    struct pitfallFallParam_c { // STATE_PITFALL_FALL
        void set(u8 unitX, u8 unitZ); // .text 14B90
        /* 0x0 */ u8 mUnitX;
        /* 0x1 */ u8 mUnitZ;
    };
    struct pitfallStruggleParam_c { // STATE_PITFALL_STRUGGLE
        void set(f32 rate); // .text 15114
        /* 0x0 */ f32 mRate; // the struggle animation's rate (_21D0[6])
    };
    showPickedItemParam_c *getShowPickedItemParam() { return reinterpret_cast<showPickedItemParam_c *>(&mRemoteActionParams); }
    pullWeedParam_c *getPullWeedParam() { return reinterpret_cast<pullWeedParam_c *>(&mRemoteActionParams); }
    pitfallFallParam_c *getPitfallFallParam() { return reinterpret_cast<pitfallFallParam_c *>(&mRemoteActionParams); }
    pitfallStruggleParam_c *getPitfallStruggleParam() { return reinterpret_cast<pitfallStruggleParam_c *>(&mRemoteActionParams); }
    // ---- State 0x54 STATE_SHOW_PICKED_ITEM (HMN_ANM_PICKUP_GET2)
    void initShowPickedItem();             // .text 14398
    void executeShowPickedItem();          // .text 14438
    void endShowPickedItem(int nextState); // .text 144B4
    // ---- State 0x55 STATE_PULL_WEED (HMN_ANM_PICKUP_WEEDS1)
    void initPullWeed();                   // .text 14664
    void executePullWeed();                // .text 14748
    void endPullWeed(int nextState);       // .text 14884
    // ---- State 0x58 STATE_PITFALL_TRIGGER: waits for the buried seed's fgMngTask (_229C)
    void initPitfallTrigger();             // .text 148EC
    void executePitfallTrigger();          // .text 148F0
    void endPitfallTrigger(int nextState); // .text 14964
    // ---- State 0x59 STATE_PITFALL_FALL (HMN_ANM_PITFALL_FALL1)
    void initPitfallFall();                // .text 14968
    void endPitfallFall(int nextState);    // .text 14B14
    // ---- State 0x5A STATE_PITFALL_STRUGGLE (HMN_ANM_PITFALL_FALL3): stick / buttons / shake (dPad::isShakeHeld).
    // _21D0[2..5]: the last stick vector per input mode, [6] the animation rate, [7] the energy left
    // (600), [8] the burst counter; _22A4[7] the cooldown.
    void initPitfallStruggle();            // .text 14B9C
    void executePitfallStruggle();         // .text 14C9C
    void endPitfallStruggle(int nextState); // .text 15104
    struct pitfallExitParam_c { // STATE_PITFALL_CLIMB_OUT
        void set(u8 unitX, u8 unitZ); // .text 152D4
        /* 0x0 */ u8 mUnitX;
        /* 0x1 */ u8 mUnitZ;
    };
    struct axeChopParam_c { // STATE_AXE_CHOP
        void set(u8 unitX, u8 unitZ); // .text 160A4
        /* 0x0 */ u8 mUnitX;
        /* 0x1 */ u8 mUnitZ;
    };
    struct axeHitParam_c { // STATE_AXE_HIT
        void set(s16 angle, u8 unitX, u8 unitZ, u8 arg, u8 fell); // .text 16508
        /* 0x0 */ s16 mAngle;
        /* 0x2 */ u8 mUnitX;
        /* 0x3 */ u8 mUnitZ;
        /* 0x4 */ u8 _4;
        /* 0x5 */ u8 mFell; // FG_MNG_KIND_HIT_MONEY_ROCK: dFgobjManager::procChopResult
    };
    pitfallExitParam_c *getPitfallExitParam() { return reinterpret_cast<pitfallExitParam_c *>(&mRemoteActionParams); }
    axeChopParam_c *getAxeChopParam() { return reinterpret_cast<axeChopParam_c *>(&mRemoteActionParams); }
    axeHitParam_c *getAxeHitParam() { return reinterpret_cast<axeHitParam_c *>(&mRemoteActionParams); }
    // ---- State 0x5B STATE_PITFALL_CLIMB_OUT (HMN_ANM_PITFALL_OUTA1 / HMN_ANM_PITFALL_OUTB1)
    void initPitfallExit();                // .text 1511C
    void endPitfallExit(int nextState);    // .text 1523C
    // ---- State 0x5C STATE_POCKET_FULL (HMN_ANM_PUTAWAY_WAIT1): SYS_Player 0xD, then the menus 0x25..0x28
    void initPocketFull();                 // .text 152E0
    void executePocketFull();              // .text 15340
    void endPocketFull(int nextState);     // .text 15380
    void stepPocketFull();                 // .text 1538C
    // ---- State 0x10 STATE_AXE_SWING (HMN_ANM_AXE_SWING1): mToolTarget = checkAxeHit's result
    void initAxeSwing();                   // .text 15764
    void executeAxeSwing();                // .text 15834
    void endAxeSwing(int nextState);       // .text 15910
    int checkAxeHit(mVec3_c *pos);         // .text 15988: 1 miss, 2 hit, 3 the axe breaks, 4 objc hit, 5 chop
    s8 checkAxeBreak(int kind);            // .text 15B6C: 0 no, 1 broke (no next axe), -1 not an axe
    BOOL isAxeReplaced();                  // .text 15C14: the equipped axe differs from the tool bank's
    void updateAxeModel();                 // .text 15CA4: dHmnToolBank_c::setAxeItem, the wear sounds
    void checkAxeTarget();                 // .text 15D34
    // ---- State 0x11 STATE_AXE_STRIKE (HMN_ANM_AXE_SWING_WAIT1): picks the next state from mToolTarget
    void initAxeStrike();                  // .text 15DA8
    void executeAxeStrike();               // .text 15E20
    void endAxeStrike(int nextState);      // .text 15F40
    // ---- State 0x12 STATE_AXE_CHOP (HMN_ANM_AXE_HIT1)
    void initAxeChop();                    // .text 15F44
    void endAxeChop(int nextState);        // .text 15FF8
    BOOL startChopTree();                  // .text 1607C
    // ---- State 0x14 STATE_AXE_MISS (HMN_ANM_AXE_SUKA1)
    void initAxeMiss();                    // .text 160B0
    void executeAxeMiss();                 // .text 160F0
    void endAxeMiss(int nextState);        // .text 161D8
    // ---- State 0x13 STATE_AXE_HIT (HMN_ANM_AXE_HANE1)
    void initAxeHit();                     // .text 161E8
    void executeAxeHit();                  // .text 162F4
    void endAxeHit(int nextState);         // .text 1647C
    struct showCatchParam_c { // STATE_SHOW_CATCH
        void set(u16 item, u8 step); // .text 1716C
        /* 0x0 */ u16 mItem;
        /* 0x2 */ u8 mStep; // mStep: 5 / 6 show, 7 put away (HMN_ANM_MENU_PUTAWAY1)
    };
    showCatchParam_c *getShowCatchParam() { return reinterpret_cast<showCatchParam_c *>(&mRemoteActionParams); }
    // ---- State 0x15 STATE_AXE_BREAK (HMN_ANM_AXE_BROKE1)
    void initAxeBreak();                   // .text 16520
    void endAxeBreak(int nextState);       // .text 165B4
    // ---- State 0x16 STATE_AXE_BREAK_MSG (HMN_ANM_AXE_BROKE2): SYS_Player 0x10
    void initAxeBreakMsg();                // .text 165F4
    void executeAxeBreakMsg();             // .text 16658
    void endAxeBreakMsg(int nextState);    // .text 166A4
    void stepAxeBreakMsg();                // .text 16708
    // ---- State 0x5F STATE_SHOW_CATCH (HMN_ANM_MENU_SHOW1 insect / 0x54 fish): mShowInsect / mShowFish in the hand
    void initShowCatch();                  // .text 16818
    void executeShowCatch();               // .text 168E8
    void endShowCatch(int nextState);      // .text 16A74
    void createShowCatch(BOOL insect);     // .text 16AB0
    void removeShowCatch(BOOL insect);     // .text 16B50
    void updateShowCatch(BOOL insect);     // .text 16BB0: scale in, the catch effect, the sounds

    // The mRemoteActionParams payloads of the show / pick-up states.
    struct showCatchAtParam_c { // STATE_SHOW_CATCH_AT
        void set(u16 item, u8 unitX, u8 unitZ, u8 fromPocket); // .text 176CC
        /* 0x0 */ u16 mItem;
        /* 0x2 */ u8 mUnitX; // mShowPos >> 5
        /* 0x3 */ u8 mUnitZ;
        /* 0x4 */ u8 mFromPocket; // ACTOR_FLAG_MENU_ACTION: a new insect / fish is made in the hand
    };
    struct pickUpFtrParam_c { // STATE_PICK_UP
        void set(u16 item, u8 unitX, u8 unitZ, u8 kind, BOOL full); // .text 17C88
        /* 0x0 */ u16 mItem;
        /* 0x2 */ u8 mUnitX;
        /* 0x3 */ u8 mUnitZ;
        /* 0x4 */ u8 mKind; // mPickUpKind
        /* 0x5 */ u8 mFull; // no room: animation 0 and the pocket message
    };
    struct eatParam_c { // STATE_EAT
        void set(u16 item); // .text 18CD4
        /* 0x0 */ u16 mItem;
    };
    showCatchAtParam_c *getShowCatchAtParam() { return reinterpret_cast<showCatchAtParam_c *>(&mRemoteActionParams); }
    pickUpFtrParam_c *getPickUpFtrParam() { return reinterpret_cast<pickUpFtrParam_c *>(&mRemoteActionParams); }
    eatParam_c *getEatParam() { return reinterpret_cast<eatParam_c *>(&mRemoteActionParams); }
    void putAwayShowCatch(BOOL insect);    // .text 16FA0: STATE_SHOW_CATCH step 7: shrink the catch away
    // ---- State 0x60 STATE_SHOW_CATCH_AT: let the caught insect / fish go at mShowPos
    void initShowCatchAt();                // .text 17178
    void executeShowCatchAt();             // .text 172C4
    void endShowCatchAt(int nextState);    // .text 17388
    void releaseCatch(BOOL insect, BOOL fromPocket); // .text 17428: fn_116_67F8 / dFishFldShadow_c::throwBack
    void watchReleasedCatch(BOOL insect);  // .text 175B0: look at it until it is gone
    // ---- State 0x5D STATE_PICK_UP: take an item from furniture (dFtrm_c::sendEvent / getEffectPos)
    void initPickUp();                     // .text 176E0
    void executePickUp();                  // .text 17874
    void endPickUp(int nextState);         // .text 17B50
    void stepPickUpFullMsg();              // .text 17B60: "sys_MESS/SYS_Player" 0xE / 0x34
    // ---- State 0x6F STATE_INSECT_BITE (HMN_ANM_STING_MOSQUITO1 / HMN_ANM_STING_MOSQUITO2)
    void initInsectBite();                 // .text 17CA0
    void executeInsectBite();              // .text 17D3C
    void endInsectBite(int nextState);     // .text 17E38
    void stepInsectBiteMsg();              // .text 17E9C: "sys_MESS/SYS_Player" 0x11
    // ---- State 0x62 STATE_BEE_STING (HMN_ANM_STING_BEE1..0x8B): a held balloon flies away
    void initBeeSting();                   // .text 17F98
    void executeBeeSting();                // .text 1805C
    void endBeeSting(int nextState);       // .text 18374
    void stepBeeStingMsg();                // .text 183F0: "sys_MESS/SYS_Player" 0x12
    // ---- State 0x79 STATE_EAT_REMOVE_MASK (HMN_ANM_MENU_CHANGE1)
    void initEatRemoveMask();              // .text 184F4
    void executeEatRemoveMask();           // .text 18568
    void endEatRemoveMask(int nextState);  // .text 186BC
    void removeMaskForEating();            // .text 18714: ACTOR_FLAG_BEE_STUNG off, the face back
    // ---- State 0x78 STATE_EAT (HMN_ANM_MENU_EAT1; 0x97 STATE_EAT_SPECIAL shares updateEat)
    void initEat();                        // .text 18760
    void executeEat();                     // .text 187E0
    void endEat(int nextState);            // .text 18850
    void calcEatItemPos(mVec3_c *pos);     // .text 18864: the hand, then towards the mouth (_21D0[9] / [10])
    void updateEat(BOOL mine);             // .text 18A1C: the eaten item shrinks (setFgObjItem)
    void calcEatEffect();                  // .text 18BD4: "af_ply_eat_yodare" at the "mouth" node
    // ---- State 0x7D STATE_SHAKE_UMBRELLA (HMN_ANM_UMB_ROTATE1): "af_itm_umbrella" when it rains
    void executeShakeUmbrella();           // .text 18CDC
    // ---- State 0x77 STATE_TRIP (init / end below)
    void executeTrip();                    // .text 18F98: a balloon flies off; STATE_RELEASE_BALLOON after
    // ---- State 0x7A STATE_CATCH_POSE: hold up the caught insect / fish (net 0x17 / rod 0x23), the jingle
    struct catchPoseParam_c { // STATE_CATCH_POSE
        void set(u8 insect); // .text 196DC
        /* 0x0 */ u8 mInsect; // ACTOR_FLAG_INSECT
    };
    catchPoseParam_c *getCatchPoseParam() { return reinterpret_cast<catchPoseParam_c *>(&mRemoteActionParams); }
    void initCatchPose();                  // .text 19248
    void executeCatchPose();               // .text 193B4
    void endCatchPose(int nextState);      // .text 1949C
    void startCatchPoseAnm(BOOL insect);   // .text 1957C
    void stepCatchPose();                  // .text 19600: STATE_NET_POCKET / STATE_FISHING_PUT_AWAY / STATE_SHOW_CATCH_AT
    // ---- States 0x32..0x34 STATE_CHEST_OPEN / WAIT / CLOSE (HMN_ANM_CHESTA_OPEN1..0x79 by mChestType)
    struct chestOpenParam_c { // STATE_CHEST_OPEN
        void set(f32 x, f32 z, u8 type); // .text 19968
        /* 0x0 */ f32 mX; // mTargetPos
        /* 0x4 */ f32 mZ;
        /* 0x8 */ u8 mType; // mChestType
    };
    struct chestWaitParam_c { // STATE_CHEST_WAIT
        void set(u8 type); // .text 19A5C
        /* 0x0 */ u8 mType;
    };
    struct chestCloseParam_c { // STATE_CHEST_CLOSE
        void set(u8 type); // .text 19BC0
        /* 0x0 */ u8 mType;
    };
    chestOpenParam_c *getChestOpenParam() { return reinterpret_cast<chestOpenParam_c *>(&mRemoteActionParams); }
    chestWaitParam_c *getChestWaitParam() { return reinterpret_cast<chestWaitParam_c *>(&mRemoteActionParams); }
    chestCloseParam_c *getChestCloseParam() { return reinterpret_cast<chestCloseParam_c *>(&mRemoteActionParams); }
    void initChestOpen();                  // .text 196E4
    void executeChestOpen();               // .text 1986C
    void endChestOpen(int nextState);      // .text 19964
    void initChestWait();                  // .text 19978
    void executeChestWait();               // .text 19A18
    void endChestWait(int nextState);      // .text 19A58
    void initChestClose();                 // .text 19A64
    void executeChestClose();              // .text 19B3C
    void endChestClose(int nextState);     // .text 19BBC
    // ---- State 0x7B STATE_OPEN_UMBRELLA (HMN_ANM_UMB_OPEN1)
    void initOpenUmbrella();               // .text 19BC8
    void executeOpenUmbrella();            // .text 19C3C
    void endOpenUmbrella(int nextState);   // .text 19D84
    // ---- State 0x7C STATE_CLOSE_UMBRELLA (HMN_ANM_UMB_OPEN1 backwards)
    void initCloseUmbrella();              // .text 19DF8
    void executeCloseUmbrella();           // .text 19E94
    void endCloseUmbrella(int nextState);  // .text 19FE4
    // ---- State 0x7D STATE_SHAKE_UMBRELLA
    void initShakeUmbrella();              // .text 1A048
    void endShakeUmbrella(int nextState);  // .text 1A0C4
    void getUmbrellaEffectParam(f32 *y, f32 *scale); // .text 1A0E0: by the held umbrella
    // ---- State 0x77 STATE_TRIP (HMN_ANM_TUMBLE_DOWNA1 / HMN_ANM_TUMBLE_DOWNB1, then 0x7B / 0x7D / 0x7E on the ground)
    void initTrip();                       // .text 1A264
    void endTrip(int nextState);           // .text 1A384
    void createTripEffect();               // .text 1A42C: "*_ply_koke_*" by the surface
    // ---- State 0x96 STATE_RELEASE_BALLOON: watch the balloon fly off (mScriptedActionPos)
    void initReleaseBalloon();             // .text 1A440
    void executeReleaseBalloon();          // .text 1A4DC
    void endReleaseBalloon(int nextState); // .text 1A580
    // ---- State 0x74 STATE_FAINT : "af_ply_kizetsu" at the "feel" node
    void executeFaint();                   // .text 1A5F4
    // ---- State 0x7E STATE_PICK_UP_UNIT_ITEM: the target unit's item (setTargetUnitItem), ground (0x4E) or
    // upper layer (0x70, dFgDraw::getItemPosition); the payload is takeItemParam_c
    void initPickUpUnitItem();             // .text 1A79C
    void executePickUpUnitItem();          // .text 1A7E8
    void endPickUpUnitItem(int nextState); // .text 1A878
    void initPickUpUnitItemMine();         // .text 1A8E0
    void initPickUpUnitItemRemote();       // .text 1AA3C
    void executePickUpUnitItemMine();      // .text 1AB7C

    // ---- State 0x7E (pickup1 / kagu_putaway1): the remote side and the mRemoteActionParams payload
    struct takeItemParam_c {
        void set(u16 item, u8 unitX, u8 unitZ, u8 layer, u8 cancel); // .text 1B1CC
        /* 0x0 */ u16 mItem;
        /* 0x2 */ u8 mUnitX;
        /* 0x3 */ u8 mUnitZ;
        /* 0x4 */ u8 mLayer;  // _22DC: 0 the ground (pickup1), else furniture (kagu_putaway1)
        /* 0x5 */ u8 mCancel; // the take was refused (animation 0)
    };
    takeItemParam_c *getTakeItemParam() { return reinterpret_cast<takeItemParam_c *>(&mRemoteActionParams); }
    void updateTakeItemRemote();           // .text 1AE98: the item flies to the hand and shrinks (remote)
    // ---- State 0x74 STATE_FAINT (sting_downA1 / sting_downB1 with a net / rod / umbrella)
    void initFaint();                      // .text 1B1E4
    void endFaint(int nextState);          // .text 1B300
    void requestHomeScene();               // .text 1B304: dScene request to the player's house entrance
    // ---- State 0x75 STATE_FAINT_GET_UP (sting_up1)
    void initFaintGetUp();                 // .text 1B430
    void executeFaintGetUp();              // .text 1B48C
    void endFaintGetUp(int nextState);     // .text 1B590
    // ---- State 0x72 STATE_MAILBOX_OPEN (post_open1): face north (-0x8000), walk to mTargetPos
    void initMailboxOpen();                // .text 1B594
    void executeMailboxOpen();             // .text 1B61C
    void endMailboxOpen(int nextState);    // .text 1B684
    // ---- State 0x73 STATE_MAILBOX_CLOSE (post_open1 reversed)
    void initMailboxClose();               // .text 1B688
    void executeMailboxClose();            // .text 1B72C
    void endMailboxClose(int nextState);   // .text 1B7AC
    // ---- State 0x70 STATE_CHAT (menu_chat1)
    void initChat();                       // .text 1B7B0
    void executeChat();                    // .text 1B7D8
    void endChat(int nextState);           // .text 1B850
    // ---- dHandItem states 0x63..0x6E (trans_* / transF_* for fish, mail_*)
    void initReceiveItem();                // .text 1B854: trans_pass1
    void executeReceiveItem();             // .text 1B8EC: trans_pass2, then dHandItem STATE_APPEAR
    void endReceiveItem(int nextState);    // .text 1BA38
    void initPassItem();                   // .text 1BA3C: trans_pass3
    void executePassItem();                // .text 1BAB4: dHandItem STATE_FALL
    void endPassItem(int nextState);       // .text 1BB24
    void initPassItemWait();               // .text 1BB28: trans_pass_wait1
    void executePassItemWait();            // .text 1BB8C: until the partner holds the item
    void endPassItemWait(int nextState);   // .text 1BBE0
    void initHandItemReach();              // .text 1BBE4: walk to the item (dHandItem::getPos)
    void executeHandItemReach();           // .text 1BD6C
    void endHandItemReach(int nextState);  // .text 1BEAC
    void initHandItemTake();               // .text 1BEB0: trans_receive1
    void executeHandItemTake();            // .text 1BF4C: swapActors, dHandItem STATE_TAKE / STATE_HOLD
    void endHandItemTake(int nextState);   // .text 1C008
    void initHandItemHold();               // .text 1C00C: trans_receive2
    void executeHandItemHold();            // .text 1C084: dHandItem STATE_SHRINK
    void endHandItemHold(int nextState);   // .text 1C0F4
    void initHandItemPutAway();            // .text 1C0F8: trans_putaway1
    void executeHandItemPutAway();         // .text 1C170: gold net / rod -> mReactionKind
    void endHandItemPutAway(int nextState); // .text 1C234
    void initGiveItem();                   // .text 1C238: mail_pass1
    void executeGiveItem();                // .text 1C294
    void endGiveItem(int nextState);       // .text 1C310
    void initMailReceive();                // .text 1C314: mail_receive1
    void executeMailReceive();             // .text 1C370
    void endMailReceive(int nextState);    // .text 1C3B4
    void initMailReceiveEnd();             // .text 1C3B8: mail_receive2
    void executeMailReceiveEnd();          // .text 1C414
    void endMailReceiveEnd(int nextState); // .text 1C458
    void initMailPutAway();                // .text 1C45C: mail_putaway1
    void executeMailPutAway();             // .text 1C4B8
    void endMailPutAway(int nextState);    // .text 1C50C
    void initHandItemReturn();             // .text 1C510: mail_pass1 reversed
    void executeHandItemReturn();          // .text 1C5A4: gold watering can -> mReactionKind
    void endHandItemReturn(int nextState); // .text 1C648
    // ---- State 0x7F STATE_ROOM_EXIT_TURN
    void initStairsTurn();               // .text 1C64C
    void executeStairsTurn();            // .text 1C650: turn to mTargetAngle, then 0x80 / 0x82 by mExitType
    void endStairsTurn(int nextState);   // .text 1C6CC
    // ---- Stairs (stair_up1 / stair_up2 / stair_down1 / stair_down2): mShadowAlpha and the model alpha
    // (setModelColor) fade over frames 18..45 when leaving, 0..18 when arriving. mActorFlags2 0x200000: the
    // scene change is requested; ACTOR_FLAG_NO_OBJC: the remote player is fading.
    struct goUpstairsParam_c { // STATE_GO_UPSTAIRS
        void set(f32 x, f32 z, u8 pose); // .text 1CB38
        /* 0x0 */ f32 mX;
        /* 0x4 */ f32 mZ;
        /* 0x8 */ u8 mPose; // encodeSeatPose (mTargetAngle)
    };
    struct goDownstairsParam_c { // STATE_GO_DOWNSTAIRS
        void set(f32 x, f32 z, u8 pose); // .text 1D28C
        /* 0x0 */ f32 mX;
        /* 0x4 */ f32 mZ;
        /* 0x8 */ u8 mPose;
    };
    goUpstairsParam_c *getGoUpstairsParam() { return reinterpret_cast<goUpstairsParam_c *>(&mRemoteActionParams); }
    goDownstairsParam_c *getGoDownstairsParam() { return reinterpret_cast<goDownstairsParam_c *>(&mRemoteActionParams); }
    // ---- State 0x80 STATE_GO_UPSTAIRS (stair_up1): dScene requestExit (mExitNo)
    void initGoUpstairs();                 // .text 1C6D0
    void executeGoUpstairs();              // .text 1C7CC
    void endGoUpstairs(int nextState);     // .text 1CACC
    // ---- State 0x81 STATE_ARRIVE_UPSTAIRS (stair_up2): the objc position at the base node
    void initArriveUpstairs();             // .text 1CB48
    void executeArriveUpstairs();          // .text 1CBEC: then the cockroach check and finishSceneStart
    void endArriveUpstairs(int nextState); // .text 1CDE0
    // ---- State 0x82 STATE_GO_DOWNSTAIRS (stair_down1)
    void initGoDownstairs();               // .text 1CE3C
    void executeGoDownstairs();            // .text 1CF38
    void endGoDownstairs(int nextState);   // .text 1D220
    // ---- State 0x83 STATE_ARRIVE_DOWNSTAIRS (stair_down2)
    void initArriveDownstairs();           // .text 1D29C
    void executeArriveDownstairs();        // .text 1D334
    void endArriveDownstairs(int nextState); // .text 1D528
    // ---- State 0x89 STATE_WATER (jyoro_raise1): the watering can; dFgobjManager::useTool (tool 4).
    // mActorFlags2 0x40: the water effect (mEffect) is running.
    void updateWaterEffect();              // .text 1D584: af_itm_jyoro_water while it reaches walkable ground
    void initWater();                      // .text 1E398
    void executeWater();                   // .text 1E4C8
    void endWater(int nextState);          // .text 1E598
    // ---- State 0x8A STATE_BLOW_DANDELION (tanpopo_fly1): af_itm_tanpopo; the dandelion is used up
    void executeBlowDandelion();           // .text 1D7D0
    void initBlowDandelion();              // .text 1E60C
    void endBlowDandelion(int nextState);  // .text 1E6B0
    // ---- State 0x8E STATE_CRACKER (cracker_setoff1): af_itm_cracker
    void executeCracker();                 // .text 1D934
    // ---- State 0x8D STATE_FIREWORKS (fire_setoff1): af_itm_firework(2), dEffectLight_c glow
    void executeFireworks();               // .text 1DAB4
    BOOL updateThrownAxe();                // .text 1DD6C: the axe flies to mShowPos (the fountain): af_itm_ono_wc
    // ---- State 0xA2 STATE_BLOW_BUBBLES (tanpopo_fly1): af_itm_syabon
    void executeBlowBubbles();             // .text 1DECC
    // ---- State 0xA4 STATE_FORTUNE_HIT (fortune_hit1)
    void initFortuneHit();                 // .text 1E038
    // ---- State 0x8B STATE_TRICK_CHANGE (halloween_change1): mStep 8 / 9
    void executeTrickChange();             // .text 1E098
    void setToolPartAnm(f32 blend, BOOL bothArms); // .text 1E2B4: the held tool's arm animation (mPartAnm)
    // ---- States 0x85..0x87 STATE_PHONE_* (tel1 / tel_wait1): the phone (daRmobjTel_c::pickUp / talk / hangUp)
    void initPhonePickUp();                // .text 1E71C: walk to (304, 399), face north
    void executePhonePickUp();             // .text 1E848
    void endPhonePickUp(int nextState);    // .text 1E8DC
    void initPhoneTalk();                  // .text 1E8E0
    void executePhoneTalk();               // .text 1E90C
    void endPhoneTalk(int nextState);      // .text 1E94C
    void initPhoneHangUp();                // .text 1E950
    void executePhoneHangUp();             // .text 1E9C0
    void endPhoneHangUp(int nextState);    // .text 1EA38
    // ---- State 0x84 STATE_REACT (loan_complete1): mReactionKind; BGM, camera mode, SYS_Player message
    void initReact();                      // .text 1EA3C
    void executeReact();                   // .text 1EB4C
    void endReact(int nextState);          // .text 1EC4C: mReactionKind = REACTION_NONE
    void stepReactMsg();                   // .text 1ECC0: mStep 10..12

    // ---- State 0x71 STATE_EMOTION
    struct emotionParam_c { // STATE_EMOTION
        void set(u8 emotion); // .text 1F1C8
        /* 0x0 */ u8 mEmotion;
    };
    emotionParam_c *getEmotionParam() { return reinterpret_cast<emotionParam_c *>(&mRemoteActionParams); }
    void initEmotion();                         // .text 1EE74
    void executeEmotion();                      // .text 1EECC
    void endEmotion(int nextState);             // .text 1EF34
    void startEmotion();                        // .text 1EF98: animation, manpu, the timer _22A4[7]
    void updateEmotion(int nextState, BOOL sound); // .text 1F03C
    // ---- State 0x03 STATE_MOVE_TO
    void initMoveTo();                          // .text 1F1D0
    void executeMoveTo();                       // .text 1F248
    void endMoveTo(int nextState);              // .text 1F370
    // ---- States 0x8F..0x94 STATE_BUS_*
    void initBusBoard();                        // .text 1F380
    void executeBusBoard();                     // .text 1F3A8
    void endBusBoard(int nextState);            // .text 1F4F4
    void initBusBoarded();                      // .text 1F508
    void executeBusBoarded();                   // .text 1F514
    void endBusBoarded(int nextState);          // .text 1F518
    void initBusRide();                         // .text 1F51C
    void executeBusRide();                      // .text 1F528
    void endBusRide(int nextState);             // .text 1F52C
    void initBusGetOff();                       // .text 1F530
    void executeBusGetOff();                    // .text 1F57C
    void endBusGetOff(int nextState);           // .text 1F5C0
    void initBusGotOff();                       // .text 1F5C4
    void executeBusGotOff();                    // .text 1F5D0
    void endBusGotOff(int nextState);           // .text 1F5D4
    void initBusLeave();                        // .text 1F5D8
    void executeBusLeave();                     // .text 1F61C
    void endBusLeave(int nextState);            // .text 1F77C
    // ---- State 0x8E STATE_CRACKER / 0x8D STATE_FIREWORKS
    void initCracker();                         // .text 1F790
    void endCracker(int nextState);             // .text 1F838
    void initFireworks();                       // .text 1F8A8: dHmnToolBank_c::startHanabi
    void endFireworks(int nextState);           // .text 1F96C
    // ---- State 0x8C STATE_SCENE_START / 0xAA STATE_SCENE_START_HIDDEN
    void initSceneStart();                      // .text 1F9F8
    void executeSceneStart();                   // .text 1FA44
    void endSceneStart(int nextState);          // .text 1FA80
    void initSceneStartHidden();                // .text 1FAA0
    void executeSceneStartHidden();             // .text 1FAEC
    void endSceneStartHidden(int nextState);    // .text 1FB28: keeps ACTOR_FLAG_DISABLED
    // ---- State 0x4A STATE_ENTER_ROOM
    struct enterRoomParam_s { // STATE_ENTER_ROOM
        /* 0x0 */ f32 mX;
        /* 0x4 */ f32 mZ;
        /* 0x8 */ u8 mKind; // 1 / 2: delayed by sRemoteActionDelayFrames / sRemoteActionAlternateDelayFrames
    };
    enterRoomParam_s *getEnterRoomParam() { return reinterpret_cast<enterRoomParam_s *>(&mRemoteActionParams); }
    void initEnterRoom();                       // .text 1FB44
    void executeEnterRoom();                    // .text 1FD50
    void endEnterRoom(int nextState);           // .text 1FED0
    static void setEnterRoomParam(enterRoomParam_s *param, f32 x, f32 z, u8 kind); // .text 1FF24
    // ---- State 0x49 STATE_EXIT_ROOM
    struct exitRoomParam_s { // STATE_EXIT_ROOM
        /* 0x0 */ f32 mSpeed;
        /* 0x4 */ s16 mAngle;
        /* 0x6 */ u8 mUnitX;
        /* 0x7 */ u8 mUnitZ;
    };
    exitRoomParam_s *getExitRoomParam() { return reinterpret_cast<exitRoomParam_s *>(&mRemoteActionParams); }
    void initExitRoom();                        // .text 1FF34
    void executeExitRoom();                     // .text 200D8: fades the model out (_22DF[4]) near mTargetPos
    void endExitRoom(int nextState);            // .text 202B0
    BOOL isRoomSeScene();                       // .text 202C8: houses' main floors, villager houses, town hall, store, tailor
    static void setExitRoomParam(exitRoomParam_s *param, f32 speed, s16 angle, u8 unitX, u8 unitZ); // .text 20328
    // ---- State 0x4E STATE_CHANGE_EQUIPMENT_QUICK
    void initChangeEquipmentQuick();            // .text 2033C
    void executeChangeEquipmentQuick();         // .text 20340
    void endChangeEquipmentQuick(int nextState); // .text 203E0
    // ---- State 0x95 STATE_GODDESS_ITEM
    void initGoddessItem();                     // .text 203E4
    void executeGoddessItem();                  // .text 2045C
    void endGoddessItem(int nextState);         // .text 20634
    BOOL sinkThrownAxe();                       // .text 2068C: the axe spins down into the fountain
    // ---- States 0x9C STATE_DRINK_COFFEE / 0x9D STATE_FINISH_COFFEE
    void initDrinkCoffee();                     // .text 20788
    void executeDrinkCoffee();                  // .text 20808
    void endDrinkCoffee(int nextState);         // .text 20934
    void initFinishCoffee();                    // .text 20938
    void executeFinishCoffee();                 // .text 209B0
    void endFinishCoffee(int nextState);        // .text 20A6C
    // ---- State 0x9E STATE_TURN_FRONT
    void initTurnFront();                       // .text 20A78

    void executeTurnFront();                    // .text 20BA8
    void endTurnFront(int nextState);           // .text 20C34
    // ---- States 0x99 STATE_SLINGSHOT_AIM / 0x9A STATE_SLINGSHOT_SHOOT / 0x9B STATE_SLINGSHOT_WATCH
    void initSlingshotAim();                    // .text 20C38
    void executeSlingshotAim();                 // .text 20CFC
    void endSlingshotAim(int nextState);        // .text 20E60
    void initSlingshotShoot();                  // .text 20EE0
    void executeSlingshotShoot();               // .text 20F84
    void endSlingshotShoot(int nextState);      // .text 21084
    void initSlingshotWatch();                  // .text 21114
    void executeSlingshotWatch();               // .text 21164
    void endSlingshotWatch(int nextState);      // .text 21168
    // ---- State 0x9F STATE_BED_HAIR
    void initBedHair();                         // .text 211D4
    void executeBedHair();                      // .text 21228
    void endBedHair(int nextState);             // .text 212B8
    // ---- State 0xA1 STATE_LISTEN_KK
    struct listenKKParam_s { // STATE_LISTEN_KK
        /* 0x0 */ s16 mLookYaw;
        /* 0x2 */ s16 mLookTilt;
    };
    listenKKParam_s *getListenKKParam() { return reinterpret_cast<listenKKParam_s *>(&mRemoteActionParams); }
    void initListenKK();                        // .text 212D8
    void executeListenKK();                     // .text 213EC
    void endListenKK(int nextState);            // .text 2155C
    static void setListenKKParam(listenKKParam_s *param, s16 lookYaw, s16 lookTilt); // .text 215A8
    // ---- State 0xA2 STATE_BLOW_BUBBLES
    void initBlowBubble();                      // .text 215B4
    void endBlowBubble(int nextState);          // .text 21634
    // ---- State 0xA9 STATE_PUT_AWAY_TOOL_FULL
    struct putAwayToolFullParam_s { // STATE_PUT_AWAY_TOOL_FULL
        /* 0x0 */ u8 mStep; // the remote copy of mStep
    };
    putAwayToolFullParam_s *getPutAwayToolFullParam() { return reinterpret_cast<putAwayToolFullParam_s *>(&mRemoteActionParams); }
    void initPutAwayToolFull();                 // .text 2169C
    void executePutAwayToolFull();              // .text 2171C
    void endPutAwayToolFull(int nextState);     // .text 21A2C
    void stepPocketFullMsg();                   // .text 21A88: "sys_MESS/SYS_Player" 0x13
    static void setPutAwayToolFullParam(putAwayToolFullParam_s *param, u8 step); // .text 21BD4
    // ---- States 0xA6..0xA8 STATE_SHOE_SHINE_*
    void initShoeShineStart();                  // .text 21BDC
    void executeShoeShineStart();               // .text 21C54
    void endShoeShineStart(int nextState);      // .text 21E04
    void initShoeShine();                       // .text 21E08
    void executeShoeShine();                    // .text 21E44
    void endShoeShine(int nextState);           // .text 21E84
    void initShoeShineEnd();                    // .text 21EC4
    void executeShoeShineEnd();                 // .text 21EEC
    void endShoeShineEnd(int nextState);        // .text 21F30
    // ---- States 0xA4 STATE_FORTUNE_HIT / 0xA5 STATE_FORTUNE_FAINT
    void executeFortuneHit();                   // .text 21F34
    void endFortuneHit(int nextState);          // .text 21F8C
    void updateFortuneHitEffect();              // .text 21F90: "af_ply_kizetsu" at the "feel" node
    void initFortuneFaint();                    // .text 22064
    void executeFortuneFaint();                 // .text 2208C
    void endFortuneFaint(int nextState);        // .text 221D4
    // ---- State 0x88 STATE_FLINCH
    void initFlinch();                          // .text 221D8
    void executeFlinch();                       // .text 22200
    void endFlinch(int nextState);              // .text 22278
    // ---- State 0x61 STATE_SURPRISED
    void initSurprised();                       // .text 2227C
    void executeSurprised();                    // .text 22298
    void endSurprised(int nextState);           // .text 22310
    // ---- State 0x8B STATE_TRICK_CHANGE
    void initTrickChange();                     // .text 22374
    void endTrickChange(int nextState);         // .text 223B0
    // ---- State 0x76 STATE_TURN_TO_TALKER
    void initTurnToTalker();                    // .text 22410
    void executeTurnToTalker();                  // .text 2246C
    void endTurnToTalker(int nextState);        // .text 22574
    // ---- State 0x04 STATE_TURN_TO
    void initTurnTo();                          // .text 22578
    void executeTurnTo();                       // .text 2263C
    void endTurnTo(int nextState);              // .text 22694
    // ---- State 0x98 STATE_CHANGE_HELD_ITEM_DEMO
    void initChangeHeldItemDemo();              // .text 22698
    void executeChangeHeldItemDemo();           // .text 226A4
    void endChangeHeldItemDemo(int nextState);  // .text 22784
    // ---- States 0x17 STATE_AXE_HIT_OBJECT / 0x48 STATE_SHOVEL_HIT_OBJECT
    struct hitObjectParam_s { // STATE_AXE_HIT_OBJECT / STATE_SHOVEL_HIT_OBJECT
        /* 0x0 */ u8 mHit; // the tool hit an objc (mObjcPos / mObjcPos2)
    };
    hitObjectParam_s *getHitObjectParam() { return reinterpret_cast<hitObjectParam_s *>(&mRemoteActionParams); }
    void initAxeHitObject();                    // .text 227D4
    void executeAxeHitObject();                 // .text 228A0
    void endAxeHitObject(int nextState);        // .text 22A08
    static void setHitObjectParam(hitObjectParam_s *param, u8 hit); // .text 22A60
    void initShovelHitObject();                 // .text 22A68
    void executeShovelHitObject();              // .text 22B94

    // ---- State 0x48 STATE_SHOVEL_HIT_OBJECT : the shovel hits an npc / snowman
    void endShovelHitActor(int nextState);  // .text 22CE8: mObjcPos / mObjcPos2 off, the hit targets cleared
    struct shovelHitActorParam_c { // STATE_SHOVEL_HIT_OBJECT
        void set(u8 hit); // .text 22D48
        /* 0x0 */ u8 mHit;
    };
    shovelHitActorParam_c *getShovelHitActorParam() { return reinterpret_cast<shovelHitActorParam_c *>(&mRemoteActionParams); }
    // ---- State 0x4F STATE_CHANGE_HAIR / 0x50 STATE_CHANGE_MII_MASK: the barber (barber_cut1..3 / barber_mii1..3)
    void initChangeHair();                  // .text 22D50
    void executeChangeHair();               // .text 22DC4
    void endChangeHair(int nextState);      // .text 22E08: back to chair_sitdown_wait1
    void updateBarber(BOOL miiMask);        // .text 22E50: steps 0x11..0x14 (mStep); the new hair / mask
    void initChangeMiiMask();               // .text 23028
    void executeChangeMiiMask();            // .text 23058
    void endChangeMiiMask(int nextState);   // .text 2309C
    // ---- State 0x97 STATE_EAT_SPECIAL (menu_eat2, then menu_think2; "af_mnp_delicious")
    void initEatSpecial();                  // .text 230E4
    void executeEatSpecial();               // .text 23164
    void endEatSpecial(int nextState);      // .text 23210
    struct eatSpecialParam_c { // STATE_EAT_SPECIAL
        void set(u16 item); // .text 23224
        /* 0x0 */ u16 mItem;
    };
    eatSpecialParam_c *getEatSpecialParam() { return reinterpret_cast<eatSpecialParam_c *>(&mRemoteActionParams); }
    // ---- State 0x56 STATE_STAMP_HOLE_WAIT (hole_bury_wait1): turn to the hole, wait for dFgobjManager::fillHole
    void initFillHoleWait();                // .text 2322C
    void executeFillHoleWait();             // .text 232C8
    void endFillHoleWait(int nextState);    // .text 23390
    // ---- State 0x4B STATE_MII_MASK_IMMEDIATE (nml_change1)
    void initMiiMaskImmediate();            // .text 23394
    void executeMiiMaskImmediate();         // .text 23448
    void endMiiMaskImmediate(int nextState); // .text 23620
    void updateMiiMask();                   // .text 23678: the head / face models for the worn Mii mask
    // ---- State 0xA0 STATE_CHANGE_OUTFIT_AND_HELD
    void initChangeOutfitAndHeld();         // .text 236E8
    void executeChangeOutfitAndHeld();      // .text 23844
    void endChangeOutfitAndHeld(int nextState); // .text 23B50
    struct outfitAndHeldParam_c { // STATE_CHANGE_OUTFIT_AND_HELD
        void set(u16 equipFlags, u8 heldItemChange, u8 step); // .text 23C24
        /* 0x0 */ u16 mEquipFlags;    // mEquipFlags
        /* 0x2 */ u8 mHeldItemChange; // mHeldItemChange
        /* 0x3 */ u8 mStep;           // the remote copy of mStep
    };
    outfitAndHeldParam_c *getOutfitAndHeldParam() { return reinterpret_cast<outfitAndHeldParam_c *>(&mRemoteActionParams); }
    // ---- State 0x5E STATE_SWAP_ITEM (dFgobjManager::swapItem with _218C)
    void initSwapItem();                    // .text 23C34
    void executeSwapItem();                 // .text 23C94
    void endSwapItem(int nextState);        // .text 23D08
    // ---- State 0xA3 STATE_PUT_HELD_ITEM (dFgobjManager::putHeldItem with mActionItem)
    void initPutHeldItem();                 // .text 23D0C
    void executePutHeldItem();              // .text 23D6C
    void endPutHeldItem(int nextState);     // .text 23DE0
    // ---- The state machine
    void setState(int state);               // .text 23DE4: end(state), init, the remote state sync
    void endState(int nextState);           // .text 23EE8: the current state's end handler
    static void changeStateCallback(int playerNo, int state); // .text 23F70: dPlayerMgr's change state callback
    void clearFrameRequests();              // .text 23FAC
    void calcMove();                        // .text 24000: manpu, look-at, speed, position (every frame)
    void calcSlopeSpeed();                  // .text 241A4: mTargetSpeed on a slope (mAcch.mGroundNormal)
    void setMoveTarget(const mVec3_c *pos, f32 speed); // .text 242E0: mTargetPos / mTargetSpeed
    void startMoveToTarget();               // .text 24300
    BOOL moveToTarget();                    // .text 24388: TRUE when there (snaps to mTargetPos)
    void setStateObjc(int state);           // .text 2447C: objc weight / callback and the move reset of a state
    BOOL setAnm(int anmId, int mode);       // .text 24628: morph 60
    BOOL setAnmNoMorph(int anmId, int mode); // .text 24648
    BOOL setAnm(int anmId, f32 morph, f32 frame, f32 rate, f32 blend, int mode, BOOL force); // .text 24668
    BOOL setToolAnm(int anmId, BOOL force, f32 blend); // .text 24788: the tool's part animation
    void getFrontPos(mVec3_c *out);         // .text 248D8: 32 in front
    void getPosAtAngle(mVec3_c *out, s16 angle, f32 dist); // .text 248E8: from mPos
    void calcPosAtAngle(mVec3_c *out, const mVec3_c *pos, s16 angle, f32 dist); // .text 248F4: on the ground
    ulong getToolNodeIdx();                 // .text 249C0: the tool bank node of the held tool
    void stopMove();                        // .text 24A18
    void stopMoveAll();                     // .text 24A4C: and mSpeedF
    BOOL checkActionInput(BOOL other);      // .text 24A84: talk / pick up / tool / startPitfallTrigger
    BOOL checkDpadInput();                  // .text 24B48: tool switch / put away; the room camera
    BOOL canHoldTool(const dItem::Item *item); // .text 24D84
    void changeHeldItem(const dItem::Item *item); // .text 24DF0: STATE_CHANGE_HELD_ITEM
    BOOL checkTalkInput();                  // .text 24E84
    BOOL checkToolSwingInput();             // .text 24F24
    BOOL checkUseInput();                   // .text 25004
    BOOL checkFtrGrabInput();               // .text 25068
    BOOL checkToolUseInput();               // .text 250DC
    void startFishingCast();                // .text 251D8
    BOOL checkAxeIntoFountain();            // .text 25290: setAxeInFountain
    BOOL checkPickUpInput();                // .text 253D0
    BOOL checkPickUpRoom();                 // .text 25448: furniture / table items in the house
    BOOL checkPickUpOutdoor();              // .text 256B4: fill a hole / pick up
    BOOL checkMoveOrAction();               // .text 257D0: STATE_IDLE when the stick moves
    BOOL pickUpFromUnit(int ftr, const putUnit_c *unit); // .text 25860: STATE_PICK_UP
    BOOL isRafflesiaAt(const mVec3_c *pos); // .text 258EC
    void breakHitSnowmen();                 // .text 259C4
    int getNpcToolReaction();               // .text 25A18: dAcNpc_c::vt68 with the held item (2 without npc)
    void createHeldToolGlitter();           // .text 25AA4: "af_itm_gold" / "af_itm_silver" at the hand
    void createToolGlitter(const dItem::Item *item); // .text 25BBC
    void calcRunFootEffects();              // .text 25CE0
    void createRunEffect(mVec3_c *foot, int side); // .text 25D6C
    void createFootprint(const mVec3_c *pos, int side); // .text 25DEC: 0 right, 1 left, 2 both feet
    static void wateringParticleCallback(nw4r::ef::Particle *particle, u32 param); // .text 25E90
    void updateSlipEffect();                // .text 26134
    void createGroundEffect(const char *sandEffect, const char *effect); // .text 261B8: at mPos
    void createFootEffect(const char *effect, const mVec3_c *pos); // .text 26254: sand / snow only
    void createSurfaceEffect(const char *const *names, const mVec3_c *pos, const mAng3_c *angle); // .text 262F4
    void startSlipEffect();                 // .text 26390
    BOOL isInHouseRoom();                   // .text 26444: a player house room (not the loft)
    void setFaceAnm(int anmId, int mode, BOOL force); // .text 264A4
    void updateFaceAnm();                   // .text 266BC: the blink
    void snapToGround();                    // .text 26764
    void setNetObjc(f32 radius, f32 height, f32 dist, f32 y); // .text 267C4: mObjcPos ahead of the net
    void clearFieldRequest();               // .text 26858
    void setTargetUnit(const mVec3_c *pos); // .text 268A4
    void setTargetUnit(int x, int z) { mTargetUnitX = x; mTargetUnitZ = z; }
    void setTargetUnitHere(mVec3_c *pos);   // .text 268E0
    void getFrontUnit(mVec3_c *pos);        // .text 268FC: l_frontCheckDist ahead
    void getNearFrontUnit(mVec3_c *pos);    // .text 26950: 20.8 ahead
    void getFrontUnit26(mVec3_c *pos);      // .text 269A4: 26 ahead
    void getNetFrontUnit(mVec3_c *pos);     // .text 269F4: 32 ahead of the net
    void getFreeFrontUnit(mVec3_c *pos);    // .text 26A54: 32 ahead, else +-l_angle10

    BOOL setToolTargetUnit(mVec3_c *pos, int unitX, int unitZ); // .text 0x26B84: the unit in front of the tool (FALSE: own unit, 48 ahead)
    BOOL isBlockedGround(mVec3_c *pos);         // .text 0x26C94: stone, or wood that is not a stump
    BOOL canWalkToUnit(int unitX, int unitZ);   // .text 0x26D54: dBGCF::checkWalkable from mPos
    BOOL waitFieldTask(int failState, bool wait, u32 mode); // .text 0x26DE0: the field task (_229C) result -> state
    BOOL applyFieldTask();                      // .text 0x26F04: the accepted task's unit / items / kind (mTaskKind)
    BOOL changeStateByTaskKind();               // .text 0x26F9C: the state for the task kind mTaskKind
    void setTargetUnitItem(int layer);          // .text 0x27314: the target unit's item -> _218C / mActionItem / _2188
    BOOL checkPutEffectDone();                  // .text 0x273F0: dFgobjManager::playPutEffect (_22A0) finished
    void checkWalkIntoTree();                   // .text 0x27464: a tree 14 ahead shakes (ACTOR_FLAG2_UNIT_ACTION)
    BOOL startPitfallTrigger();                 // .text 0x27568: dFgobjManager::requestPitfall -> STATE_PITFALL_TRIGGER
    BOOL startShakeTree();                      // .text 0x27614: a tree in front (wood, not a stump) -> STATE_SHAKE_TREE
    BOOL changeStateAfterMsg(int state);        // .text 0x27744: the message closed: detach, change state
    BOOL setStepAfterMsg(int step);             // .text 0x277E8: the message closed: detach, mStep = step
    void clearActorFlags101();                  // .text 0x27884
    BOOL putInPocket(dItem::Item *item);        // .text 0x27898: FALSE when the pockets are full
    void deleteHookedFish();                    // .text 0x27958: the float's fish (dFishFldShadow_c::requestDelete)

    int getStartState();                        // .text 0x2798C: the state from mParam (remote: dPlayerMngr_c)
    void setStartState();                       // .text 0x27AC0: create: the first state (remote action sync)
    void setStartAnm();                         // .text 0x27C38: create: the tool and the end frame of the start state
    u16 getFieldRealItem(const mVec3_c *pos, int layer, dFdBase_c *field);     // .text 0x27EE8: an item (not a field object) at pos
    u16 getFieldPickableItem(const mVec3_c *pos, int layer, dFdBase_c *field); // .text 0x27F5C: not a mushroom, not buried
    void addModelOffset(mVec3_c *pos, f32 x, f32 y, f32 z); // .text 0x27FF8: pos += (x, y, z) in the root node's space
    void setIdleAnm();                          // .text 0x28100: the wait animation (and the net / rod wait)
    void setNetWalkAnm();                       // .text 0x281E8: the held net follows the walk
    void showTool();                            // .text 0x2826C: mToolScale 1, ACTOR_FLAG_TOOL_ACTIVE, the tool animation
    void hideTool();                            // .text 0x282A0: mToolScale 0, the part animation removed
    void updateSunTimer();                      // .text 0x282D4: summer days outdoors -> PRIVATE_FLAG0_SUNTAN
    void setFgObjItem(u16 item, const mVec3_c *pos, const mVec3_c *scale); // .text 0x28498: mFgObjItem drawn at pos
    void startRumble(int kind);                 // .text 0x284D0: dRumble::start for this console's player
    void setDigAnm(int anm);                    // .text 0x28520: the shovel's dug block at the target unit
    void createBodyModel();                     // .text 0x28900: body model, animations, node ids, callback
    void releaseClothTex();                     // .text 0x28B70: "cloth" palette / texture released, rebound
    void setupHeadModel(nw4r::g3d::ResFile file); // .text 0x28BDC: head model, palettes, face textures
    void releaseHeadModel();                    // .text 0x28F74: ACTOR_FLAG_HEAD_MDL
    void setBodyKColor(const GXColor &color);         // .text 0x29074: "b0" konst colour 1 (the skin)
    void createMiiHeap();                       // .text 0x2911C: ACTOR_FLAG_MII_FACE: the Mii heap
    void createModels();                        // .text 0x29198: create: body, head, equipment
    void setupBodyPalette();                    // .text 0x2926C: skin colour _221C (l_skinColors)
    void bindCloth();                           // .text 0x292D4: the cloth file bound to the body
    void *loadHeadData(int kind);               // .text 0x29330: head _2214
    void *loadMaskHeadData(int kind);           // .text 0x29380: head 0x1A
    void createHeadMdl(nw4r::g3d::ResFile file); // .text 0x293D0: mHeadMdl, ACTOR_FLAG_HEAD_MDL
    void createHairMdl(nw4r::g3d::ResFile file); // .text 0x29430: mHairMdl, ACTOR_FLAG_HAIR_MDL
    void releaseHairModel();                    // .text 0x29490
    void releaseAccessoryModel();               // .text 0x29508: mAccessoryMdl, ACTOR_FLAG_ACCESSORY_MDL
    void freeHeadHeap();                        // .text 0x29558
    void freeAccessoryHeap();                   // .text 0x295C0
    void loadHead();                            // .text 0x29E0C
    void applyEquipment();                      // .text 0x2A36C
    void applyTool();                           // .text 0x2A758

    void initEquipment();                       // .text 0x2961C: create: mEquipFlags from the hat's hide bone, the Mii mask
    void initHeldTool();                        // .text 0x29934: create: the held hand tool (hide bone 9) to load (mHeldEquipStep)
    void loadCapHead();                         // .text 0x29A20: the head (cap variant for hide bone 0) and the cap model
    void loadFullMask();                        // .text 0x29B0C: only the cap model (hide bone 2: no head)
    void loadMaskHead();                        // .text 0x29B7C: head 0x1A and the cap model with the hair colour
    void loadHeadAndCap();                      // .text 0x29D78: hide bone 7
    void loadAccessory();                       // .text 0x29E5C: mAccessoryMdl, ACTOR_FLAG_ACCESSORY_MDL
    void removeAccessory();                     // .text 0x29EF0
    BOOL checkEquipLoaded(BOOL load);           // .text 0x29EF4: mEquipStep LOAD / LOAD_INIT -> LOADED / APPLIED when every part is loaded
    BOOL checkToolLoaded(BOOL load);            // .text 0x2A0A4: mHeldEquipStep LOAD / LOAD_INIT -> LOADED / INIT_LOADED
    BOOL checkEquipReady();                     // .text 0x2A184
    BOOL checkToolReady();                      // .text 0x2A2B0
    void removeTool();                          // .text 0x2A874: put away, mToolScale 0
    void resetFaceTex();                        // .text 0x2A8DC: the eye / mouth textures of the face numbers

    // One key of a Mii mask expression track (.data 0xC..0x670, one track per dHmnAnm_c sub index):
    // at mFrame of the body animation the mask shows mExpression (RFLExpression). mFrame -1 ends it.
    struct faceExprKey_s {
        s16 mFrame;
        u8 mExpression;
    };

    void setModelColor(u8 brightness);                         // .text 0x2AA60: the models' material color (fades to black)
    void setMatColor(u8 brightness, nw4r::g3d::ResMdl *mdl);   // .text 0x2AB2C
    void rebindShoePalette();                                  // .text 0x2ABE8: STATE_SHOE_SHINE
    void createMiiFace();                                      // .text 0x2AC90: mFace from mMiiMask (or the first valid Mii)
    void setFaceMtxFromHead();                                 // .text 0x2AE88
    void setFaceMtx(mMtx_c *headMtx);                          // .text 0x2AEC4: the head matrix turned / scaled for the mask
    void calcFaceExpression();                                 // .text 0x2AF84: the Mii mask's expression keys / blinks
    void setFaceExpression(u8 expression);                     // .text 0x2B03C
    void setFaceExprAnm(int anmId);                            // .text 0x2B130: the expression track of a body animation
    void calcFaceBlink();                                      // .text 0x2B198
    BOOL putOnMiiMask();                                       // .text 0x2B238: the face becomes Mii mMiiMask
    void takeOffMiiMask();                                     // .text 0x2B3B4
    void checkMiiFace();                                       // .text 0x2B3D8: ACTOR_FLAG_MII_FACE when a Mii is available
    void syncRemoteState();                                    // .text 0x2C314: a remote player follows its record
    void getRemoteActionParams(dPlayerMngr_c::actionParam_c *param); // .text 0x2C4F0
    void updateNetRecords();                                   // .text 0x2C560: the local player's position / angle / state records
    void sendStatePacket();                                    // .text 0x2C64C: NET_PACKET_PLAYER_STATE
    void followRemoteTransform();                              // .text 0x2C6F8: a remote player moves to its record
    void calcPadInput();                                       // .text 0x2C810: the local player's buttons, stick and pointer
    void setMoveInput();                                       // .text 0x2CBF8
    void calcMoveInput();                                      // .text 0x2CC38: the stick angle with the turn direction
    void checkInputTurn(s16 angle, s16 prevAngle);             // .text 0x2CD68
    f32 calcPointerSpeed(const mVec3_c *target);               // .text 0x2CDA4: walking to the pointer
    int getInputDirection();                                   // .text 0x2CED4: the input direction (Direction_e) relative to mAngle.y
    int getInputDirectionOrNone();                             // .text 0x2CF14: as getInputDirection, DIR_NONE without input
    int getAngleDirection(s16 angle);                          // .text 0x2CF6C: 0 front, 1 left, 2 back, 3 right
    s16 decodeSeatPose(int *side, int pose);                   // .text 0x2CFB0: a furniture pose code -> angle / side
    int encodeSeatPose(s16 angle, int side);                   // .text 0x2D0A0
    s16 getInputAngle();                                       // .text 0x2D1BC: mMoveAngle, camera relative for the stick
    BOOL isNoMoveInput();                                      // .text 0x2D20C: mMoveSpeed <= 0
    void checkPointerTarget(const mVec3_c *pos);               // .text 0x2D22C: A pressed with the pointer on something
    void checkPointerTurn(const mVec3_c *pos);                 // .text 0x2D698: turn to the pointer to use the tool
    BOOL calcStickInput(f32 *speed, f32 *angle, f32 *accel);   // .text 0x2D748
    void startPointerTurn(f32 speed);                          // .text 0x2D918
    void calcPointerTurn();                                    // .text 0x2D9C0
    void endPointerTurn();                                     // .text 0x2D9E0
    BOOL checkPointerInput(const mVec3_c *pos);                // .text 0x2D9F0
    s16 getActionAngle();                                      // .text 0x2DAF4: the pointer turn's angle, else mAngle.y
    void checkPointerOnScreen(f32 width, f32 height, u8 *result); // .text 0x2DB28: 1 inside, 2 outside (once)
    BOOL isInSpeechRange(f32 minDist, f32 maxDist, const mVec3_c *pos); // .text 0x2DBD0

    /* 0x0000 dDemoActor_c, 0x00BC dMsg::Rcpt_c */
    /* 0x0120 */ dAudioObjPlayer_c mAudioObj;
    /* 0x017C */ dAudioObjNpc_c mVoiceObj;
    /* 0x01CC */ dSndObjSimple_c mSndObj;
    /* 0x01FC */ dSndObjPinwheel_c mPinwheelSnd;
    /* 0x0230 */ mAllocator_c mAllocator;          // mpHeap
    /* 0x024C */ mAllocator_c mMiiAllocator;       // mpMiiHeap
    /* 0x0268 */ mAllocator_c mHeadAllocator;      // mpHeadHeap
    /* 0x0284 */ mAllocator_c mAccessoryAllocator; // mpAccessoryHeap
    /* 0x02A0 */ m3d::mdlEx_c mMdl;
    /* 0x02CC */ m3d::anmChrPart_c mPartAnm;
    /* 0x0304 */ m3d::mdl_c mHeadMdl;      // ACTOR_FLAG_HEAD_MDL
    /* 0x0344 */ m3d::mdl_c mHairMdl;      // ACTOR_FLAG_HAIR_MDL
    /* 0x0384 */ m3d::mdl_c mAccessoryMdl; // ACTOR_FLAG_ACCESSORY_MDL
    /* 0x03C4 */ m3d::anmChr_c mAnm; // the body animation (mAnmId)
    /* 0x03FC */ m3d::anmTexPat_c mTexPatAnm;
    /* 0x0428 */ myFace_c mFace;
    /* 0x0608 */ myMdlCallback_c mMdlCallback;
    /* 0x0754 */ dBGCF::acch_c mAcch;
    /* 0x079C */ dBGCF::acch_c mAcch2;
    /* 0x07E4 */ dObjc::ac_c mObjc;
    /* 0x0828 */ dObjc::acpos_c mObjcPos;
    /* 0x0878 */ dObjc::acpos_c mObjcPos2;
    // Five separate members, not an array: the ctor / dtor build and destroy each inline.
    /* 0x08C8 */ dLevelEffect_c mLevelEffect0;
    /* 0x095C */ dLevelEffect_c mLevelEffect1;
    /* 0x09F0 */ dLevelEffect_c mLevelEffect2;
    /* 0x0A84 */ dLevelEffect_c mLevelEffect3;
    /* 0x0B18 */ dLevelEffect_c mLevelEffect4;
    /* 0x0BAC */ dEffect_c mEffect;
    /* 0x0C28 */ dHmnHeadMng_c mHeadMng;
    /* 0x0C29 */ dHmnPaletteMng_c mPaletteMng;
    /* 0x0C2A */ dHmnBodyMng_c mBodyMng;
    /* 0x0C2C */ dHmnCloth_c *mpCloth;       // dPlayerMngr_c::getCloth
    /* 0x0C30 */ dHmnHeadItem_c *mpHeadItem; // dPlayerMngr_c::getHeadItem
    /* 0x0C34 */ dHmnToolBank_c *mpToolBank; // dPlayerMngr_c::getToolBank
    /* 0x0C38 */ dHmnFaceAnmMng_c mFaceAnmMng;
    /* 0x0C40 */ dAcNpc_c::manpuMgr_c mManpu;
    /* 0x2040 */ dInsectLink_c mInsectLink; // removed in doDelete / the dtor
    /* 0x2048 */ dFishLink_c mFishLink;
    /* 0x2050 */ s16 _2050;
    /* 0x2052 */ s16 _2052;
    /* 0x2054 */ s16 _2054;
    /* 0x2056 */ s16 _2056;
    /* 0x2058 */ s16 _2058;
    /* 0x205A */ s16 _205A;
    /* 0x205C */ mVec3_c _205C;
    /* 0x2068 */ mVec3_c mTargetPos; // moveTo / lookAt target
    /* 0x2074 */ mVec3_c mShowPos;   // showCatchAt
    /* 0x2080 */ mVec3_c mShadowPos; // dShadow::requestRound
    /* 0x208C */ mVec3_c mToolScale; // x >= 1.0f: the tool is out (put away before a demo)
    /* 0x2098 */ mVec3_c _2098;
    /* 0x20A4 */ mVec3_c mItemPos; // the picked-up / shown item (dFgDraw::getItemPosition, then the hand)
    /* 0x20B0 */ mVec3_c _20B0; // the acch position checked in state 0x77
    /* 0x20BC */ mVec3_c mEatEffectPos; // STATE_EAT: the "mouth" node (af_ply_eat_yodare)
    /* 0x20C8 */ mVec3_c _20C8; // dFgobjManager::fgObjCallback position
    /* 0x20D4 */ mVec3_c _20D4; // dFgobjManager::fgObjCallback scale
    /* 0x20E0 */ mVec3_c mPointerOffset; // where the pointer touched the player / the held furniture, from mPos
    /* 0x20EC */ mVec3_c
        mScriptedActionPos; // dPlayerMgr::getScriptedActionPosition; the tool's position
    /* 0x20F8 */ mMtx_c mToolMtx;  // the tool bank's matrix (the right hand, scaled)
    /* 0x2128 */ mMtx_c mModelMtx; // the body model's local matrix
    /* 0x2158 */ mMtx_c mUmbrellaEffectMtx; // STATE_SHAKE_UMBRELLA: scale / offset of "af_itm_umbrella"
    /* 0x2188 */ dItem::Item _2188; // ITEM_ID_NONE at construction; the dug-up / picked-up item (getKind, setItemName)
    /* 0x218A */ dItem::Item mActionItem; // the shown / held action item (getActiveActionItem)
    /* 0x218C */ u16 _218C;
    /* 0x2190 */ int mTargetUnitX; // getActionUnitCoordinates, putItemInHole
    /* 0x2194 */ int mTargetUnitZ;
    /* 0x2198 */ EGG::FrmHeap *mpHeap; // getGeneralHeapSize
    /* 0x219C */ EGG::FrmHeap
        *mpMiiHeap; // getMiiHeapSize ("dAcPlayer_c::m_heap_Mii_p")
    /* 0x21A0 */ EGG::FrmHeap *mpHeadHeap;      // getHeadHeapSize
    /* 0x21A4 */ EGG::FrmHeap *mpAccessoryHeap; // getAccessoryHeapSize
    /* 0x21A8 */ void *mShowInsect;           // STATE_SHOW_CATCH: the held insect (fn_116_F7C)
    /* 0x21AC */ dFishFldShadow_c *mShowFish; // STATE_SHOW_CATCH: the held fish (dFishField_c::addPlayerFish)
    /* 0x21B0 */ const faceExprKey_s *mFaceExprKey; // the Mii mask's expression keys of the body animation (-1 ends)
    /* 0x21B4 */ fBase_c *_21B4; // its id is the talk actor id while waiting in a demo
    /* 0x21B8 */ f32 _21B8;
    /* 0x21BC */ f32 mTargetSpeed; // moveTo speed
    /* 0x21C0 */ f32 _21C0[2];
    /* 0x21C8 */ f32 mShadowScale; // dShadow::requestRound
    /* 0x21CC */ f32 mShadowAlpha;
    /* 0x21D0 */ f32 _21D0[11];
    /* 0x21FC */ f32 mScale; // the body model's scale
    /* 0x2200 */ f32 _2200;  // set from an objc hit's height (dObjc weight callback)
    /* 0x2204 */ f32 _2204;
    /* 0x2208 */ int mToolTarget; // ToolTarget_e
    /* 0x220C */ int mTaskKind;   // dFgMngKind_e of the accepted field task
    /* 0x2210 */ int mTaskResult; // dFgobjManager::REQUEST_RESULT_*
    /* 0x2214 */ int _2214;
    /* 0x2218 */ int _2218;
    /* 0x221C */ int _221C;
    /* 0x2220 */ int mAnmId;
    /* 0x2224 */ int _2224;
    /* 0x2228 */ int _2228;
    /* 0x222C */ int mState;       // dPlayerMgr::State
    /* 0x2230 */ int mRemoteSyncA; // setRemoteActionSync
    /* 0x2234 */ int mStep;        // the step of the current state (its execute switch)
    /* 0x2238 */ int mEquipStep;     // EquipStep_e
    /* 0x223C */ int mHeldEquipStep; // EquipStep_e
    /* 0x2240 */ int mPlayerNo; // the player / network member index
    /* 0x2244 */ int mDoorType; // enterDoor / exitDoor
    /* 0x2248 */ int mSeatPose; // getUpFromBed / changeSeatedPose
    /* 0x224C */ int mSeatSide; // SIT_FROM_* / LIE_FROM_*
    /* 0x2250 */ int mMsgStep;     // MsgStep_e: the follow-up of the pockets-full / catalog messages
    /* 0x2254 */ int mChestType; // openChest / closeChest
    /* 0x2258 */ int mExitType; // dScene::dSceneChange_c::getExitPos
    /* 0x225C */ int mShadowType; // dShadow::requestRound flags
    /* 0x2260 */ u16 _2260;
    /* 0x2262 */ u16 _2262;
    /* 0x2264 */ u16 mFgObjItem; // drawn with dFgobjManager::fgObjCallback
    /* 0x2266 */ u16 _2266;
    /* 0x2268 */ int _2268;
    /* 0x226C */ int mRemoteSyncB; // setRemoteActionSync
    /* 0x2270 */ u32 mActorFlags;  // dPlayerMgr::ACTOR_FLAG_*
    /* 0x2274 */ u32 mActorFlags2; // dPlayerMgr::ACTOR_FLAG2_*
    /* 0x2278 */ int _2278;
    /* 0x227C */ int _227C;
    /* 0x2280 */ int mSyncValue; // setRemoteSyncValue; dPlayerMngr frame on
                                 // each state change
    /* 0x2284 */ int mUnitX;     // getUnitCoordinates
    /* 0x2288 */ int mUnitZ;
    /* 0x228C */ int mActionUnitX; // getActionUnitAndKind
    /* 0x2290 */ int mActionUnitZ;
    /* 0x2294 */ int mExitNo; // dBGCF::getOpenNext
    /* 0x2298 */ int mDoorTarget; // enterDoor / walkThroughDoor
    /* 0x229C */ int _229C;
    /* 0x22A0 */ int _22A0;
    /* 0x22A4 */ u16 _22A4[8];
    /* 0x22B4 */ u16 mEquipFlags; // dPlayerMgr::EQUIPMENT_*
    /* 0x22B6 */ u16 mMiiMask;    // changeMiiMask
    /* 0x22B8 */ u16 _22B8[5];
    /* 0x22C2 */ s16 _22C2;
    /* 0x22C4 */ s16 mTargetAngle; // turnTo / doors / furniture
    /* 0x22C6 */ s16 mLookTilt;    // lookAt
    /* 0x22C8 */ s16 mLookYaw;
    /* 0x22CA */ s16 _22CA; // lookAt
    /* 0x22CC */ s16 _22CC; // lookAt
    /* 0x22CE */ s16 _22CE;
    /* 0x22D0 */ s16 mTurnFrames; // turnToInFrames
    /* 0x22D2 */ s16 _22D2[3];
    /* 0x22D8 */ u8 _22D8[2];
    /* 0x22DA */ u8 mHeldItemChange; // dPlayerMgr::HELD_ITEM_CHANGE_*; the tool load kind (1 load, 2 remove) while mHeldEquipStep is set
    /* 0x22DB */ u8 _22DB;
    /* 0x22DC */ u8 mPickUpKind; // STATE_PICK_UP: 0 HMN_ANM_PICKUP1, 1 HMN_ANM_KAGU_PUTAWAY1 (dFtrm_c::sendEvent at frame 6)
    /* 0x22DD */ u8 mReactionKind; // dPlayerMgr::REACTION_* (STATE_REACT)
    /* 0x22DE */ u8 mEmotion;      // playEmotion
    /* 0x22DF */ u8 _22DF[5];
    /* 0x22E4 */ u8 mEquipChangeMode; // changeEquipment mode
    /* 0x22E5 */ u8 _22E5;
    /* 0x22E6 */ u8 mShovelTaskStage; // the field task's mStage when the shovel hits (STATE 0x41)
    /* 0x22E7 */ u8 _22E7[5];
    /* 0x22EC */ f32 mMoveSpeed;
    /* 0x22F0 */ f32 _22F0;
    /* 0x22F4 */ f32 _22F4;
    /* 0x22F8 */ s16 mMoveAngle; // getMoveAngle
    /* 0x22FA */ u8 _22FA[4];
    /* 0x22FE */ u8 _22FE[6];
    /* 0x2304 */ u8 _2304[4];
    /* 0x2308 */ dPlayerMngr_c::actionParam_c mRemoteActionParams;
}; // size 0x2314
