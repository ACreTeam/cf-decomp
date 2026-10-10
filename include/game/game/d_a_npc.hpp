#pragma once

// The NPC actor base class dAcNpc_c (RTTI "dAcNpc_c", derives dDemoActor_c) and its helper classes.
// Source: src/dol/game/d_a_npc.cpp (.text 800120A8..8002ED4C, built with -sym on). Class names with
// RTTI come from the RTTI strings; everything else (members, functions, helper classes without RTTI)
// is named from behaviour. See notes/d_ac_npc_dependencies.txt.
//
// The nested classes with RTTI are defined in the order objc_c, toolBase_c, mdlCallback_c, texAnm_c,
// chrPartAnm_c, anm_c, move_c, action_c, recept_c, resBase_c, clothBase_c: MWCC emits the vtables in
// reverse class-completion order (dAcNpc_c 804A0200 first, objc_c 804A05B8 last).

#include <types.h>
// Include order preserves the original RTTI data layout.
#include <game/mLib/m_3d.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_m3d.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_objc.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_audio_obj.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_script.hpp>
#include <game/game/d_hmn_face_anm_mng.hpp>
#include <game/game/d_hmn_tool_mng.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/mLib/m_vec.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_mtx.hpp>
#include <revolution/MTX.h>
#include <cstring>

namespace EGG {
class FrmHeap;
}
namespace dHmnName {
class Word_c;
}
class dDemo_c;
class dLandID_c;
class dPersonalID_c;
class dPlayerID_c;
class dAnmPersonalID_c;
class dMail_c;
class dTime_c;
class dEquip_c;
class dDesign_c;

// .sdata2 constants defined at the top of d_a_npc.cpp (8074FDE0.., in this order). The target references
// them from the request functions (and from d_a_npc_sp, d_npc_talk_quest_q08, d_npc_talk_fmarket,
// d_npc_talk_birthday) instead of pooled literals: they are default arguments (morph frames / anm rates /
// ranges) of action_c / lookAt_c / recept_c request functions. Placeholder names.
extern const f32 cNpcMorphFrames; // 12.0f default morph frames
extern const f32 l_8074FDE4; // 1.0f default anm rate (action_c::request)
extern const f32 l_8074FDE8; // 1.0f default anm rate (d_a_npc_sp, d_npc_talk_quest_q08, d_npc_talk_fmarket)
extern const f32 l_8074FDEC; // 2.0f
extern const f32 cNpcAnmRateMax; // 1.05f action_c::calcAnmRate
extern const f32 cNpcAnmRateMin; // 0.44f action_c::calcAnmRate
extern const f32 cNpcLookRange; // 104.0f lookAt_c range
extern const f32 l_8074FE10; // 256.0f
extern const f32 l_8074FE14; // 48.0f

class dAcNpc_c : public dDemoActor_c {
public:
    class model_c;
    class face_c;
    class manpuMgr_c;
    class action_c;
    class lookAt_c;
    class earCtrl_c;
    class talk_c;
    class recept_c;
    class toolBase_c;
    class clothBase_c;
    class resBase_c;

    // Ids of the actions of action_c (index into the action table 8049FB68).
    enum ACTION_e {
        ACTION_WAIT,         // 0
        ACTION_WALK,         // 1
        ACTION_RUN,          // 2
        ACTION_TURN,         // 3
        ACTION_TURN_WALK,    // 4
        ACTION_TURN_RUN,     // 5
        ACTION_WALK_FACING,  // 6
        ACTION_RUN_FACING,   // 7
        ACTION_EMOTION,      // 8
        ACTION_ANM,          // 9
        ACTION_CATCH,        // 10
        ACTION_HAND_ITEM,    // 11
        ACTION_CHANGE_CLOTH, // 12
        ACTION_RECEIVE,      // 13
        ACTION_GIVE,         // 14
        ACTION_HAND_OVER,    // 15
        ACTION_EAT,          // 16
        ACTION_INSPECT,      // 17
        ACTION_CHANGE_CLOTH2, // 18
        ACTION_MOVE_DEST,    // 19
        ACTION_MOVE_PARAM,   // 20
        ACTION_NUM           // 21: no action / no request
    };

    // Element types of the top-of-file .rodata position tables (placeholder names).
    struct posTable33_s { int mKey; Vec mPos[33]; };   // 0x190
    struct posTable97_s { int mKey; Vec mPos[97]; };   // 0x490

    // Speed set of move_c: target speed and the chase steps of npc->mSpeedF. The statics
    // l_moveParamStop / l_moveParamWalk / l_moveParamRun (__sinit) are of this type. Size 0xC.
    struct moveParam_c {
        moveParam_c() { memset(this, 0, sizeof(moveParam_c)); }
        moveParam_c(f32 speed, f32 accel, f32 decel) : mSpeed(speed), mAccel(accel), mDecel(decel) {}
        ~moveParam_c() {} // weak 8002E9B8

        /* 0x0 */ f32 mSpeed;
        /* 0x4 */ f32 mAccel; // chase step while mAccel < mSpeed
        /* 0x8 */ f32 mDecel; // otherwise
    }; // size 0xC

    // The three body animation ids of the npc (npc+0xE8): wait, walk, run. Size 0xC.
    class anmSet_c {
    public:
        void setAnm0(int anmId);   // 80017F84
        void setAnm1(int anmId);   // 80017F8C
        void setAnm2(int anmId);   // 80017F94
        int getAnm(int idx) const; // 80017F9C

        /* 0x0 */ int mWaitAnm; // the idle one (create() starts the model with it)
        /* 0x4 */ int mWalkAnm;
        /* 0x8 */ int mRunAnm;
    }; // size 0xC

    // The npc's push cylinder with layered weights: mDefWeight, overridden by mTempWeight while flag 0
    // is set, both overridden by a fixed weight while flag 1 is set. 0x80 means 0xF0. RTTI
    // dAcNpc_c::objc_c, vtable 804A05B8 {ac_c::getPos, ac_c::getID, dtor}.
    class objc_c : public dObjc::ac_c {
    public:
        objc_c();                           // 80017FDC
        virtual ~objc_c();                  // 8001802C
        void onWeightFlag(ulong bit);       // 8001806C
        void offWeightFlag(ulong bit);      // 8001808C
        bool isWeightFlag(ulong bit) const; // 800180AC
        bool isTempWeight() const;          // 800180D4
        bool isFixWeight() const;           // 800180DC
        static u8 convWeight(u8 weight);    // 800180E4
        void setDefWeight(u8 weight);       // 800180F4
        void setTempWeight(u8 weight);      // 80018138
        void resetTempWeight();             // 80018194
        void setFixWeight(u8 weight);       // 800181F4
        void resetFixWeight();              // 80018234

        /* 0x44 */ u8 mDefWeight;
        /* 0x45 */ u8 mTempWeight;
        /* 0x46 */ u8 mWeightFlag; // bit 0: temp weight, bit 1: fixed weight
    }; // size 0x48

    // Hand tool (umbrella/net/rod...) display: the item the npc holds (getHoldItem()) is loaded into a
    // dHmnToolBank_c and drawn at the npc's hand matrix (mNodeMtx0E) scaled by mScale. Not embedded in
    // dAcNpc_c: returned by the virtual getTool(); constructed by subclasses in other TUs. RTTI
    // dAcNpc_c::toolBase_c, vtable 804A0594 = {dtor}.
    class toolBase_c {
    public:
        toolBase_c();                           // 8001D9B4
        virtual ~toolBase_c();                  // 8001DA30
        BOOL isEnable() const;                  // 8001DA8C
        void checkItem(dAcNpc_c *npc);          // 8001DAA8
        BOOL isNextReady() const;               // 8001DB58
        BOOL isNextLoaded(dAcNpc_c *npc) const; // 8001DBA8
        BOOL change(f32 scale);                 // 8001DC3C
        void putAway();                         // 8001DCD0
        BOOL loadNext(dAcNpc_c *npc);           // 8001DD20
        void execute(dAcNpc_c *npc);            // 8001DDBC
        void draw();                            // 8001DEB0
        static u32 getToolAnmId(int toolType);  // 8001DF08
        u32 getAnmId() const;                   // 8001DF44
        void init() {
            mItem = dItem::ITEM_ID_NONE;
            mNextItem = dItem::ITEM_ID_NONE;
            mLoadState = 2;
            mScale = 1.0f;
            mLock = false;
        }

        /* 0x004 */ dItem::Item mItem;     // displayed tool
        /* 0x006 */ dItem::Item mNextItem; // requested tool (0xFFF1 = none)
        /* 0x008 */ int mLoadState;        // 0 load failed, 1 loaded, 2 pending/idle
        /* 0x00C */ dHmnToolBank_c mBank;
        /* 0x728 */ f32 mScale;
        /* 0x72C */ bool mLock;            // don't follow the held item
    }; // size 0x730

    // The calc-world user hook of the npc model (model_c +0x2C). timingA patches node animation results
    // (translation, head look rotation), timingB captures world matrices of 7 nodes into the npc.
    // RTTI dAcNpc_c::mdlCallback_c, vtable 804A0558.
    class mdlCallback_c : public m3d::mdlEx_c::callback_c {
    public:
        mdlCallback_c(dAcNpc_c *npc);                                                                  // 800120A8
        virtual ~mdlCallback_c();                                                                      // 800120C4
        virtual void timingA(ulong nodeId, nw4r::g3d::ChrAnmResult *anmRes, nw4r::g3d::ResMdl resMdl); // 8001210C
        virtual void timingB(ulong nodeId, nw4r::g3d::WorldMtxManip *manip, nw4r::g3d::ResMdl resMdl); // 800123F4
        virtual void timingC(nw4r::math::MTX34 *mtx, nw4r::g3d::ResMdl resMdl);                        // 8001289C

        /* 0x4 */ int mAnmId;      // current body animation id, 0x1BC = none
        /* 0x8 */ dAcNpc_c *mpNpc; // owner
    }; // size 0xC

    // Eye (slot 0) / mouth (slot 1) texture-pattern animation; the textures come from the face anim
    // manager per face type. RTTI dAcNpc_c::texAnm_c, vtable 804A0518.
    class texAnm_c : public m3d::anmTexPat_c {
    public:
        texAnm_c();                                                                                          // 800167D0
        virtual ~texAnm_c();                                                                                 // 80016848
        bool isValid() const;                                                                                // 800168B0
        void createTex(int faceType, nw4r::g3d::ResMdl mdl, nw4r::g3d::ResFile file, mAllocator_c *allocator, size_t *pSize, long count); // 800168CC
        BOOL setTexAnm(m3d::bmdl_c &mdl, nw4r::g3d::ResFile file, ulong idx, int texId, m3d::playMode_e playMode); // 80016998
        BOOL releaseTexAnm(ulong idx);                                                                       // 80016A9C
        bool isTexAnmStop(ulong idx) const;                                                                  // 80016B34
        m3d::playMode_e getTexPlayMode(ulong idx) const;                                                     // 80016BC0
        virtual void remove();                                                                               // 80016C40

        /* 0x2C */ dHmnFaceAnmMng_c mFace;
        /* 0x34 */ int mTexId[2]; // 0x1D3 = none; eye tex 0 = the blinking default
    }; // size 0x3C

    // A chr animation played on a part of the model (arms, mouth). RTTI dAcNpc_c::chrPartAnm_c,
    // vtable 804A04C0.
    class chrPartAnm_c : public m3d::anmChrPart_c {
    public:
        chrPartAnm_c();                                                                                      // 80016CB4
        virtual ~chrPartAnm_c();                                                                             // 80016CF8
        BOOL setPartAnm(m3d::mdlEx_c &mdl, int anmId, int partIdx, m3d::playMode_e playMode, f32 frame, f32 rate, f32 blend); // 80016D58
        void releasePartAnm(m3d::mdlEx_c &mdl, int partIdx, f32 blend);                                      // 80016E64
        void setPartNode(m3d::mdlEx_c &mdl, u8 partIdx, ulong nodeId) const;                                 // 80016EAC
        void createAnm(nw4r::g3d::ResMdl mdl, int anmId, mAllocator_c *allocator);                           // 80016ED8
        virtual void remove();                                                                               // 80016F4C

        /* 0x38 */ int mAnmId; // 0x1BC = none
    }; // size 0x3C

    // The body animation. RTTI dAcNpc_c::anm_c, vtable 804A0478.
    class anm_c : public m3d::anmChr_c {
    public:
        anm_c();                                                                                             // 800182A0
        virtual ~anm_c();                                                                                    // 800182E4
        BOOL changeAnm(m3d::mdlEx_c &mdl, mdlCallback_c *callback, int anmId, m3d::playMode_e playMode, BOOL force, f32 frame, f32 rate, f32 blend); // 80018340
        void createAnm(nw4r::g3d::ResMdl mdl, int anmId, mAllocator_c *allocator);                           // 80018488
        virtual void remove();                                                                               // 800184FC

        /* 0x38 */ int mAnmId; // 0x1BC = none
    }; // size 0x3C

    // The face (npc+0x23C): eye/mouth textures (blinking, per-animation faces) and the talking mouth
    // (a random "talk" mouth texture, or on models without a mouth material the "mouth" node part
    // animation 0xC1/0xC2).
    class face_c {
    public:
        face_c();                                                                                            // 80016F50
        ~face_c();                                                                                           // 80016FA4
        static bool hasMouthMat(nw4r::g3d::ResMdl mdl);                                                      // 8001700C
        static bool hasMouthMat(m3d::bmdl_c *mdl);                                                           // 80017064
        static bool hasEyeMat(nw4r::g3d::ResMdl mdl);                                                        // 800170A0
        static bool hasEyeMat(m3d::bmdl_c *mdl);                                                             // 800170F8
        bool isValid() const;                                                                                // 80017134
        static bool isTalkMouthTex(int texId);                                                               // 80017138
        BOOL setTex(m3d::mdlEx_c *mdl, nw4r::g3d::ResFile file, int idx, int texId, m3d::playMode_e playMode); // 80017180
        BOOL setAnmTex(m3d::mdlEx_c *mdl, nw4r::g3d::ResFile file, int anmId, m3d::playMode_e playMode);     // 80017380
        BOOL setAnmTex(dAcNpc_c *npc, int anmId, m3d::playMode_e playMode);                                  // 80017428
        BOOL releaseTex(m3d::mdlEx_c *mdl, int idx);                                                         // 800174B8
        BOOL releaseTex(dAcNpc_c *npc, int idx);                                                             // 80017584
        static int getRandomIdx();                                                                           // 800175DC
        static int getMouthKind(dAcNpc_c *npc);                                                              // 80017614
        static int getTalkMouthTex(dAcNpc_c *npc);                                                           // 8001764C
        BOOL setTalkMouthTex(dAcNpc_c *npc);                                                                 // 800176BC
        BOOL restoreMouthTex(m3d::mdlEx_c *mdl, nw4r::g3d::ResFile file);                                    // 8001775C
        static int getTalkMouthAnm();                                                                        // 800177A8
        BOOL setTalkMouthAnm(m3d::mdlEx_c *mdl);                                                             // 800177D4
        BOOL stopTalkMouthAnm(m3d::mdlEx_c *mdl);                                                            // 800178C4
        static bool isTalkMouthAnm(int anmId);                                                               // 80017908
        void startTalkMouth(dAcNpc_c *npc);                                                                  // 8001793C
        void endTalkMouth(dAcNpc_c *npc);                                                                    // 800179B8
        bool isTalkMouth(dAcNpc_c *npc) const;                                                               // 80017A50
        bool isTalkMouthStop(dAcNpc_c *npc) const;                                                           // 80017ACC
        void setTalk();                                                                                      // 80017B4C
        void clearTalk();                                                                                    // 80017B58
        bool isTalk() const;                                                                                 // 80017B64
        void create(int faceType, m3d::mdlEx_c *mdl, nw4r::g3d::ResFile file, mAllocator_c *allocator);       // 80017B78
        void remove();                                                                                       // 80017C80
        void reset(m3d::mdlEx_c *mdl);                                                                       // 80017CE4
        void execute(dAcNpc_c *npc);                                                                         // 80017D74

        /* 0x00 */ texAnm_c mTexAnm;
        /* 0x3C */ chrPartAnm_c mMouthAnm;          // part 3 on node "mouth"
        /* 0x78 */ int mSavedMouthTex;              // the mouth texture to restore after talking (0x1D3 = none)
        /* 0x7C */ m3d::playMode_e mSavedMouthMode; // (4 = none)
        /* 0x80 */ u8 mTalk;                        // talking: execute() flaps the mouth
        /* 0x81 */ u8 mDisable;                     // never set in d_a_npc; blocks setTex/releaseTex/talk anims
    }; // size 0x84

    // The npc's model (npc+0x140): the multi-part model, its calc-world callback, the body animation
    // and two arm part animations. Returned by the virtual getModel().
    class model_c {
    public:
        model_c(dAcNpc_c *npc);                                                                              // 80018500
        ~model_c();                                                                                          // 80018578
        static const char *getMatName(ulong idx);                                                            // 80018604
        static const char *getTexName(ulong idx);                                                            // 80018620
        BOOL create(nw4r::g3d::ResMdl mdl, int anmId, mAllocator_c *allocator, ulong anmNum, ulong bufferOption); // 8001863C
        void remove();                                                                                       // 800187B8
        void play();                                                                                         // 80018854
        void calc(dAcNpc_c *npc);                                                                      // 80018874
        BOOL setAnm(int anmId, m3d::playMode_e playMode, f32 frame, f32 rate, f32 blend, BOOL force);        // 800188EC
        BOOL setRarmAnm(int anmId, m3d::playMode_e playMode, f32 frame, f32 rate, f32 blend);                // 80018914
        void stopRarmAnm(f32 blend);                                                                         // 80018998
        BOOL setLarmAnm(int anmId, m3d::playMode_e playMode, BOOL both, f32 frame, f32 rate, f32 blend);     // 800189FC
        void stopLarmAnm(f32 blend);                                                                         // 80018AC4
        void entry();                                                                                        // 80018B28
        BOOL setFaceTex(dAcNpc_c *npc, ulong idx, const char *texName);                                      // 80018B38
        void setFrame(f32 frame, BOOL play);                                                                 // 80018C80
        BOOL copyAnm(model_c *dst) const;                                                                    // 80018CD8

        /* 0x00 */ m3d::mdlEx_c mMdl;
        /* 0x2C */ mdlCallback_c mCallback;
        /* 0x38 */ anm_c mAnm;
        /* 0x74 */ chrPartAnm_c mRarmAnm; // part 1 (node "Rarm1")
        /* 0xB0 */ chrPartAnm_c mLarmAnm; // part 2 (node "Larm1", optionally also "Rarm1")
        /* 0xEC */ f32 _EC;               // 1.0f; only copied (copyAnm)
        /* 0xF0 */ ulong mRarmNodeId;     // -1 = no such node
        /* 0xF4 */ ulong mLarmNodeId;
        /* 0xF8 */ u8 mCreated;
        /* 0xF9 */ u8 mPlay;              // play() advances the animations (else only calcBlend)
    }; // size 0xFC

    // Walk-to-point controller: target/goal position, speed parameters, yaw turning. RTTI
    // dAcNpc_c::move_c, vtable 804A0454 = {dtor}.
    class move_c {
    public:
        move_c();                                                                            // 8001DF6C
        virtual ~move_c();                                                                   // 8001E018
        void reset();                                                                        // 8001E0C8
        void setTarget(const mVec3_c &pos);                                                  // 8001E134
        void setGoal(const mVec3_c &pos);                                                    // 8001E1CC
        void setParam(const moveParam_c &param);                                             // 8001E1E8
        void calcPos(dAcNpc_c *npc);                                                         // 8001E1F4
        static mAng chaseAngle(mAng *angle, const mAng *target, const mAng *step, int mode); // 8001E32C
        void calcTargetAngle(const dAcNpc_c *npc);                                           // 8001E454
        BOOL isDetour() const;                                                               // 8001E48C
        void resetGoal();                                                                    // 8001E4CC
        bool isArrived(dAcNpc_c *npc, BOOL toTarget) const;                            // 8001E4D4
        void calcAngle(dAcNpc_c *npc);                                                       // 8001E580
        void calc(dAcNpc_c *npc);                                                            // 8001E5F8
        void setAngle(mAng angle) { mTargetAngle = angle; }
        void setTurnSpeed(mAng speed) { mTurnSpeed = speed; }
        mAng getTurnSpeed() const { return mTurnSpeed; }
        void clear() { // inline (ctor and dtor bodies)
            mTarget = mVec3_c::Zero;
            mGoal = mVec3_c::Zero;
            mTurnMode = 0;
            mParam.mSpeed = 0.0f;
            mParam.mAccel = 0.0f;
            mParam.mDecel = 0.0f;
            mArriveDist = 0.0f;
            mTargetAngle = 0;
            mTurnSpeed = 0;
            mTurnDelta = 0;
            mSnapAngle = false;
        }

        /* 0x00 */ // vtable
        /* 0x04 */ mVec3_c mTarget;   // final destination
        /* 0x10 */ mVec3_c mGoal;     // current waypoint (== mTarget unless detouring)
        /* 0x1C */ int mTurnMode;     // chaseAngle mode (1/2 force direction)
        /* 0x20 */ moveParam_c mParam;
        /* 0x2C */ f32 mArriveDist;
        /* 0x30 */ mAng mTargetAngle;
        /* 0x32 */ mAng mTurnSpeed;
        /* 0x34 */ mAng mTurnDelta;   // last applied yaw change (read by lookAt_c)
        /* 0x36 */ bool mSnapAngle;   // set the yaw directly instead of turning
    }; // size 0x38

    // Parameters of an action request; copied wholesale (memcpy) request -> current -> work. Size 0x60.
    class actionPrm_c {
    public:
        void init();                        // 8001E7A0
        void set(const actionPrm_c &other); // 8001E7F0

        /* 0x00 */ dActor_c *mpActor;      // the hand-item partner (fn_80194B8C); requestCatch/requestHandItem
        /* 0x04 */ int _4;                 // requestHandItem / requestChangeCloth2
        /* 0x08 */ moveParam_c mMoveParam;
        /* 0x14 */ f32 mArriveDist;        // copied to move_c::mArriveDist (1.0 by default)
        /* 0x18 */ mVec3_c mTarget;
        /* 0x24 */ mVec3_c mGoal;
        /* 0x30 */ f32 mMorph;             // anim blend frames (12.0 default)
        /* 0x34 */ f32 mFrame;             // start frame (requestAnm / requestMoveParam)
        /* 0x38 */ mAng mAngle;            // target yaw
        /* 0x3A */ mAng mTurnSpeed;
        /* 0x3C */ int mTurnMode;
        /* 0x40 */ int mEmotion;           // requestEmotion
        /* 0x44 */ int mAnmId;             // requestAnm / requestMoveParam
        /* 0x48 */ int _48;                // requestCatch (=6) / requestHandItem
        /* 0x4C */ int mVariant;                // requestCatch / requestHandItem (variant, 0/1)
        /* 0x50 */ int _50;                // requestCatch / requestHandItem
        /* 0x54 */ int mPlayMode;          // init 4; requestAnm / requestMoveParam
        /* 0x58 */ int mEffectIdx;         // init 2; requestHandItem / requestEat (eat-fish effect, < 2 valid)
        /* 0x5C */ dItem::Item mItem;      // init 0xFFF1
        /* 0x5E */ u8 mForce;              // requestAnm
    }; // size 0x60

    // NPC action state machine at npc+0x16C0: 21 actions (ACTION_e), each a PTMF triple {init, exec,
    // postExec} in the table 8049FB68; requests carry a priority and are applied at the start of
    // calc(). RTTI dAcNpc_c::action_c, vtable 804A0428 = {dtor}.
    class action_c : public move_c {
    public:
        typedef int (action_c::*initFunc_t)(dAcNpc_c *npc);
        typedef void (action_c::*execFunc_t)(dAcNpc_c *npc);
        struct actionFunc_s {
            /* 0x00 */ initFunc_t mInit;     // called by changeAction
            /* 0x0C */ execFunc_t mExec;     // called by calc
            /* 0x18 */ execFunc_t mPostExec; // called by calcAfter
        }; // size 0x24

        action_c();                                                                                          // 8001E5FC
        virtual ~action_c();                                                                                 // 8001E714
        actionPrm_c *getPrm();                                                                               // 8001E7F8
        actionPrm_c *getWorkPrm();                                                                           // 8001E800
        actionPrm_c *getReqPrm();                                                                            // 8001E808
        void init(dAcNpc_c *npc);                                                                            // 8001E810
        void clearRequest();                                                                                 // 8001E8C4
        BOOL hasRequest() const;                                                                             // 8001E904
        void setRequest(int action, int prio);                                                               // 8001E920
        void changeAction(dAcNpc_c *npc, int action, int prio);                                              // 8001E92C
        void checkRequest(dAcNpc_c *npc);                                                                    // 8001E98C
        void calcAnmRate(dAcNpc_c *npc, f32 max = cNpcAnmRateMax, f32 min = cNpcAnmRateMin);                                                   // 8001EA58
        void calcMove(dAcNpc_c *npc);                                                                        // 8001EB80
        void calc(dAcNpc_c *npc);                                                                            // 8001EB84
        void calcAfter(dAcNpc_c *npc);                                                                       // 8001EC10
        BOOL request(int action, int prio, const mVec3_c &pos, const mAng &angle, const mAng &turnSpeed, int turnMode, const moveParam_c &moveParam, f32 arriveDist = l_8074FDE4, f32 morph = cNpcMorphFrames); // 8001EC78
        BOOL requestWait(int prio, f32 morph = cNpcMorphFrames);                                                               // 8001EDAC
        BOOL requestWalk(int prio, const mVec3_c &pos, const mAng &turnSpeed, int turnMode, const moveParam_c &moveParam, f32 arriveDist, f32 morph); // 8001EE04
        BOOL requestRun(int prio, const mVec3_c &pos, const mAng &turnSpeed, int turnMode, const moveParam_c &moveParam, f32 arriveDist, f32 morph); // 8001EE50
        BOOL requestTurn(int prio, const mAng &angle, const mAng &turnSpeed, int turnMode, f32 morph = cNpcMorphFrames);       // 8001EE9C
        BOOL requestTurnWalk(int prio, const mVec3_c &pos, const mAng &angle, const mAng &turnSpeed, int turnMode, const moveParam_c &moveParam, f32 arriveDist, f32 morph); // 8001EED0
        BOOL requestTurnRun(int prio, const mVec3_c &pos, const mAng &angle, const mAng &turnSpeed, int turnMode, const moveParam_c &moveParam, f32 arriveDist, f32 morph); // 8001EF24
        BOOL requestWalkFacing(int prio, const mVec3_c &pos, const mAng &turnSpeed, int turnMode, const moveParam_c &moveParam, f32 arriveDist, f32 morph); // 8001EF78
        BOOL requestRunFacing(int prio, const mVec3_c &pos, const mAng &turnSpeed, int turnMode, const moveParam_c &moveParam, f32 arriveDist, f32 morph); // 8001EFC4
        BOOL requestEmotion(int prio, int emotion, f32 morph);                                               // 8001F010
        BOOL requestAnm(int prio, int anmId, f32 frame, f32 morph, int playMode, u8 force);                  // 8001F0D8
        BOOL requestCatch(int prio, dActor_c *actor, const dItem::Item &item, int arg50, int arg4C);         // 8001F19C
        BOOL requestHandItem(int prio, dActor_c *actor, int arg48, const dItem::Item &item, int arg50, int arg4C, int arg4, int effectIdx); // 8001F250
        BOOL requestChangeCloth();                                                                           // 8001F314
        BOOL requestReceive();                                                                               // 8001F3D0
        BOOL requestGive();                                                                                  // 8001F48C
        BOOL requestHandOver();                                                                              // 8001F548
        BOOL requestEat(int effectIdx);                                                                      // 8001F604
        BOOL requestInspect();                                                                               // 8001F6D0
        BOOL requestChangeCloth2(int prio, const dItem::Item &item, int arg4);                               // 8001F78C
        BOOL requestMoveDest(int prio);                                                                      // 8001F830
        BOOL requestMoveParam(int prio, int anmId, const mVec3_c &pos, const moveParam_c &moveParam, int playMode, f32 arriveDist, f32 frame, f32 morph); // 8001F888
        void setTargetPos(const mVec3_c &pos);                                                               // 8001F928
        void setGoalPos(const mVec3_c &pos);                                                                 // 8001FA00
        void setTargetAngle(const mAng &angle);                                                              // 8001FA80
        void resetGoal();                                                                                    // 8001FAE0
        int initWait(dAcNpc_c *npc);                                                                         // 8001FB18
        void execWait(dAcNpc_c *npc);                                                                        // 8001FC0C
        int initWalk(dAcNpc_c *npc);                                                                         // 8001FC18
        void execWalk(dAcNpc_c *npc);                                                                        // 8001FD64
        int initRun(dAcNpc_c *npc);                                                                          // 8001FE10
        void execRun(dAcNpc_c *npc);                                                                         // 8001FF5C
        int initTurn(dAcNpc_c *npc);                                                                         // 80020008
        void execTurn(dAcNpc_c *npc);                                                                        // 800201A8
        int initTurnWalk(dAcNpc_c *npc);                                                                     // 80020294
        void turnWalkStep0(dAcNpc_c *npc);                                                                   // 80020394
        void turnWalkStep1(dAcNpc_c *npc);                                                                   // 80020414
        void execTurnWalk(dAcNpc_c *npc);                                                                    // 80020418
        int initTurnRun(dAcNpc_c *npc);                                                                      // 80020458
        void turnRunStep0(dAcNpc_c *npc);                                                                    // 80020558
        void turnRunStep1(dAcNpc_c *npc);                                                                    // 800205D8
        void execTurnRun(dAcNpc_c *npc);                                                                     // 800205DC
        int initFaceMove(dAcNpc_c *npc, initFunc_t moveInit);                                                // 8002061C
        void execFaceMoveTurn(dAcNpc_c *npc, initFunc_t moveInit);                                           // 800206CC
        void execFaceMoveMove(dAcNpc_c *npc, initFunc_t moveInit);                                           // 800207F0
        int initWalkFacing(dAcNpc_c *npc);                                                                   // 8002092C
        void walkFacingStep0(dAcNpc_c *npc);                                                                 // 8002096C
        void walkFacingStep1(dAcNpc_c *npc);                                                                 // 800209AC
        void execWalkFacing(dAcNpc_c *npc);                                                                  // 800209EC
        int initRunFacing(dAcNpc_c *npc);                                                                    // 80020A2C
        void runFacingStep0(dAcNpc_c *npc);                                                                  // 80020A6C
        void runFacingStep1(dAcNpc_c *npc);                                                                  // 80020AAC
        void execRunFacing(dAcNpc_c *npc);                                                                   // 80020AEC
        int initEmotion(dAcNpc_c *npc);                                                                      // 80020B50
        void execEmotion(dAcNpc_c *npc);                                                                     // 80020D3C
        int initAnm(dAcNpc_c *npc);                                                                          // 80020EFC
        void postExecAnm(dAcNpc_c *npc);                                                                     // 80021018
        int initCatch(dAcNpc_c *npc);                                                                        // 800210C4
        void catchStep0(dAcNpc_c *npc);                                                                      // 800212D0
        void catchStep1(dAcNpc_c *npc);                                                                      // 800213F0
        void catchStep2(dAcNpc_c *npc);                                                                      // 80021508
        void catchStep3(dAcNpc_c *npc);                                                                      // 80021614
        void catchStep4(dAcNpc_c *npc);                                                                      // 80021654
        void catchStep5(dAcNpc_c *npc);                                                                      // 8002171C
        void catchStep6(dAcNpc_c *npc);                                                                      // 80021760
        void catchStep7(dAcNpc_c *npc);                                                                      // 80021864
        void catchStep8(dAcNpc_c *npc);                                                                      // 80021958
        void catchStep9(dAcNpc_c *npc);                                                                      // 80021A5C
        void catchStep10(dAcNpc_c *npc);                                                                     // 80021B84
        void catchStep11(dAcNpc_c *npc);                                                                     // 80021C74
        void execCatch(dAcNpc_c *npc);                                                                       // 80021D8C
        int initHandItem(dAcNpc_c *npc);                                                                     // 80021DCC
        void handItemStartA(dAcNpc_c *npc);                                                                  // 80021FBC
        void handItemWaitA(dAcNpc_c *npc);                                                                   // 80022144
        void handItemWalkA(dAcNpc_c *npc);                                                                   // 800222F0
        void handItemTakeA(dAcNpc_c *npc);                                                                   // 800223DC
        void handItemHoldA(dAcNpc_c *npc);                                                                   // 80022464
        void handItemUseA(dAcNpc_c *npc);                                                                    // 80022558
        void handItemActA(dAcNpc_c *npc);                                                                    // 80022B80
        BOOL updateChangeAnm(dAcNpc_c *npc, dItem::Item item, int anmId, BOOL unused);                       // 80022CE4
        void handItemWaitCloth(dAcNpc_c *npc);                                                               // 80022ED4
        void handItemChange(dAcNpc_c *npc);                                                                  // 80023018
        void handItemPutA(dAcNpc_c *npc);                                                                    // 80023090
        void handItemRetryPut(dAcNpc_c *npc);                                                                // 8002319C
        void handItemWaitPut(dAcNpc_c *npc);                                                                 // 800231DC
        void handItemWaitEnd(dAcNpc_c *npc);                                                                 // 800232A4
        void handItemEatFish(dAcNpc_c *npc);                                                                 // 800232E8
        void handItemRelease(dAcNpc_c *npc);                                                                 // 80023444
        void handItemStartB(dAcNpc_c *npc);                                                                  // 8002355C
        void handItemWaitB(dAcNpc_c *npc);                                                                   // 80023604
        void handItemTakeB(dAcNpc_c *npc);                                                                   // 80023644
        void handItemGiveB(dAcNpc_c *npc);                                                                   // 80023700
        void handItemRetryGiveB(dAcNpc_c *npc);                                                              // 80023794
        void handItemHoldB(dAcNpc_c *npc);                                                                   // 800237D8
        void handItemUseB(dAcNpc_c *npc);                                                                    // 800238AC
        void handItemPutB(dAcNpc_c *npc);                                                                    // 80023AB4
        void handItemReturnB(dAcNpc_c *npc);                                                                 // 80023BA8
        void execHandItem(dAcNpc_c *npc);                                                                    // 80023C30
        int initChangeCloth(dAcNpc_c *npc);                                                                  // 80023C70
        static void createChangeEffect(const mVec3_c &pos, mAng angY);                                       // 80023E44
        void changeClothWaitCloth(dAcNpc_c *npc);                                                            // 80023EC4
        void changeClothWaitAnm(dAcNpc_c *npc);                                                              // 80024020
        void postExecChangeCloth(dAcNpc_c *npc);                                                             // 80024098
        int initReceive(dAcNpc_c *npc);                                                                      // 800240D8
        void receiveReq(dAcNpc_c *npc);                                                                      // 8002422C
        void receiveWaitAnm(dAcNpc_c *npc);                                                                  // 80024348
        void postExecReceive(dAcNpc_c *npc);                                                                 // 800244AC
        int initGive(dAcNpc_c *npc);                                                                         // 800244EC
        void giveReq(dAcNpc_c *npc);                                                                         // 8002463C
        void giveWaitHoldOut(dAcNpc_c *npc);                                                                 // 80024754
        void giveWaitPlayer(dAcNpc_c *npc);                                                                  // 80024860
        void giveWaitRelease(dAcNpc_c *npc);                                                                 // 800248A0
        void giveWaitEnd(dAcNpc_c *npc);                                                                     // 80024968
        void execGive(dAcNpc_c *npc);                                                                        // 800249AC
        int initHandOver(dAcNpc_c *npc);                                                                     // 800249EC
        void handOverReq(dAcNpc_c *npc);                                                                     // 80024A74
        void handOverWaitOn(dAcNpc_c *npc);                                                                  // 80024AB8
        void handOverWaitOff(dAcNpc_c *npc);                                                                 // 80024AF8
        void execHandOver(dAcNpc_c *npc);                                                                    // 80024B40
        int initEat(dAcNpc_c *npc);                                                                          // 80024B80
        void eatReq(dAcNpc_c *npc);                                                                          // 80024D5C
        void eatWaitAnm(dAcNpc_c *npc);                                                                      // 80024EE4
        void execEat(dAcNpc_c *npc);                                                                         // 80025040
        int initInspect(dAcNpc_c *npc);                                                                      // 80025080
        void inspectReq(dAcNpc_c *npc);                                                                      // 800251D4
        void inspectWaitAnm(dAcNpc_c *npc);                                                                  // 800252D4
        void execInspect(dAcNpc_c *npc);                                                                     // 800253EC
        int initChangeCloth2(dAcNpc_c *npc);                                                                 // 8002542C
        void changeCloth2WaitCloth(dAcNpc_c *npc);                                                           // 800255C0
        void changeCloth2WaitAnm(dAcNpc_c *npc);                                                             // 800256F8
        void postExecChangeCloth2(dAcNpc_c *npc);                                                            // 80025770
        int initMoveDest(dAcNpc_c *npc);                                                                     // 800257B0
        void moveDestIdle(dAcNpc_c *npc);                                                                    // 80025AFC
        void moveDestWalk(dAcNpc_c *npc);                                                                    // 80025E7C
        void moveDestTurn(dAcNpc_c *npc);                                                                    // 80026170
        void moveDestTurnInPlace(dAcNpc_c *npc);                                                             // 800264E4
        void execMoveDest(dAcNpc_c *npc);                                                                    // 80026848
        int initMoveParam(dAcNpc_c *npc);                                                                    // 80026888
        void execMoveParam(dAcNpc_c *npc);                                                                   // 800269B0

        void setGait(u32 gait) { mPrevGait = mGait; mGait = gait; }

        /* 0x000 */ // move_c
        /* 0x038 */ dAcNpc_c *mpNpc;                 // set by init(); used by request()
        /* 0x03C */ int mActionId;                   // running action (ACTION_NUM before init)
        /* 0x040 */ int mGait;                       // latest net (daub) move type: 1 walk, 2 run (init 0x15)
        /* 0x044 */ int mPrevGait;                   // the previous one (init 0x15)
        /* 0x048 */ int mEmotion;                    // emotion index (init 0x53)
        /* 0x04C */ const actionFunc_s *mpActionFunc; // &l_actionFuncTbl[mActionId] (not set by the ctor)
        /* 0x050 */ dLevelEffect_c mEffect;          // eat-fish effect (requestEat)
        /* 0x0E4 */ int mReqAction;                  // ACTION_NUM = none
        /* 0x0E8 */ int mReqPrio;
        /* 0x0EC */ actionPrm_c mReqPrm;             // parameters of the pending request
        /* 0x14C */ actionPrm_c mPrm;                // parameters of the running action
        /* 0x1AC */ BOOL mCanChange;                 // init 1; any request is accepted while set (= action finished)
        /* 0x1B0 */ int mCurPrio;
        /* 0x1B4 */ int mEmotionMode;                // emotion: start-anm mode (1: wait for stop, then loop)
        /* 0x1B8 */ int mSavedAnmId;                 // saved npc wait anm (0x1BC = none)
        /* 0x1BC */ actionPrm_c mWorkPrm;            // copy of mPrm the running action may update
        /* 0x21C */ u8 mStep;                        // action phase; reset by changeAction
    }; // size 0x220

    struct manpuEfData_s;
    // One manpu (comic face symbol, "afm_mnp_*") effect slot: 4 per manpu_c (at manpu_c+0x30). Its
    // create / execute / fade handlers come from the table 8049E7F0 (manpuEfData_s, by mType).
    class manpuEf_c {
    public:
        typedef BOOL (manpuEf_c::*func_t)(const mMtx_c *mtx, const mVec3_c *ofs, const mVec3_c *ofsL,
                                          const mVec3_c *ofsR);

        manpuEf_c();                                                                                         // 80018E64
        ~manpuEf_c();                                                                                        // 80018F04
        void clear();                                                                                        // 80018FB0
        bool isValidType(int type) const;                                                                    // 80018FCC
        const manpuEfData_s *getData(int type) const;                                                        // 80018FE4
        void set(int type);                                                                                  // 80019030
        bool isActive() const;                                                                               // 8001908C
        BOOL callCreate(const mMtx_c *mtx, const mVec3_c *ofs, const mVec3_c *ofsL, const mVec3_c *ofsR);    // 800190DC
        BOOL callExecute(const mMtx_c *mtx, const mVec3_c *ofs, const mVec3_c *ofsL, const mVec3_c *ofsR);   // 80019160
        BOOL callFade(const mMtx_c *mtx, const mVec3_c *ofs, const mVec3_c *ofsL, const mVec3_c *ofsR);      // 800191EC
        BOOL createEffect(const mMtx_c *mtx, int arg);                                                       // 80019278
        BOOL create(const mMtx_c *mtx, const mVec3_c *ofs, const mVec3_c *ofsL, const mVec3_c *ofsR);        // 800192E4
        BOOL createFlag(const mMtx_c *mtx, const mVec3_c *ofs, const mVec3_c *ofsL, const mVec3_c *ofsR);    // 800192EC
        BOOL createFollow(const mMtx_c *mtx, const mVec3_c *ofs, const mVec3_c *ofsL, const mVec3_c *ofsR);  // 800192F4
        BOOL follow(const mMtx_c *mtx, const mVec3_c *ofs, const mVec3_c *ofsL, const mVec3_c *ofsR);        // 80019350
        BOOL createNakuL(const mMtx_c *mtx, const mVec3_c *ofs, const mVec3_c *ofsL, const mVec3_c *ofsR);   // 800193A4
        BOOL createNakuR(const mMtx_c *mtx, const mVec3_c *ofs, const mVec3_c *ofsL, const mVec3_c *ofsR);   // 80019504
        BOOL createTereCheekL(const mMtx_c *mtx, const mVec3_c *ofs, const mVec3_c *ofsL, const mVec3_c *ofsR); // 80019664
        BOOL createTereCheekR(const mMtx_c *mtx, const mVec3_c *ofs, const mVec3_c *ofsL, const mVec3_c *ofsR); // 800197C4
        BOOL createFollowNeboke(const mMtx_c *mtx, const mVec3_c *ofs, const mVec3_c *ofsL, const mVec3_c *ofsR); // 80019924
        BOOL followNeboke(const mMtx_c *mtx, const mVec3_c *ofs, const mVec3_c *ofsL, const mVec3_c *ofsR);  // 80019A60
        BOOL createFollowTame(const mMtx_c *mtx, const mVec3_c *ofs, const mVec3_c *ofsL, const mVec3_c *ofsR); // 80019B9C
        BOOL followTame(const mMtx_c *mtx, const mVec3_c *ofs, const mVec3_c *ofsL, const mVec3_c *ofsR);    // 80019CD8
        BOOL createMu(const mMtx_c *mtx, const mVec3_c *ofs, const mVec3_c *ofsL, const mVec3_c *ofsR);      // 80019E14
        BOOL fadeOutMu(const mMtx_c *mtx, const mVec3_c *ofs, const mVec3_c *ofsL, const mVec3_c *ofsR);     // 80019E50
        BOOL createFollowWaruL(const mMtx_c *mtx, const mVec3_c *ofs, const mVec3_c *ofsL, const mVec3_c *ofsR); // 80019EE8
        BOOL followWaruL(const mMtx_c *mtx, const mVec3_c *ofs, const mVec3_c *ofsL, const mVec3_c *ofsR);   // 8001A048
        BOOL createFollowWaruR(const mMtx_c *mtx, const mVec3_c *ofs, const mVec3_c *ofsL, const mVec3_c *ofsR); // 8001A1A8
        BOOL followWaruR(const mMtx_c *mtx, const mVec3_c *ofs, const mVec3_c *ofsL, const mVec3_c *ofsR);   // 8001A308
        BOOL createFollowWaruKiraL(const mMtx_c *mtx, const mVec3_c *ofs, const mVec3_c *ofsL, const mVec3_c *ofsR); // 8001A468
        BOOL followWaruKiraL(const mMtx_c *mtx, const mVec3_c *ofs, const mVec3_c *ofsL, const mVec3_c *ofsR); // 8001A5C8
        BOOL createFollowWaruKiraR(const mMtx_c *mtx, const mVec3_c *ofs, const mVec3_c *ofsL, const mVec3_c *ofsR); // 8001A728
        BOOL followWaruKiraR(const mMtx_c *mtx, const mVec3_c *ofs, const mVec3_c *ofsL, const mVec3_c *ofsR); // 8001A888

        /* 0x000 */ const manpuEfData_s *mpData; // &l_manpuEfData[mType] or NULL
        /* 0x004 */ dLevelEffect_c mEffect;      // one-shot (createEffect, fn_80087B40)
        /* 0x098 */ dLevelEffect_c mEffect2;     // following (createFollow / follow)
        /* 0x12C */ int mType;                   // 0x20 = none
        /* 0x130 */ u8 _130;
        /* 0x131 */ u8 mAlpha;                   // "mu" fade-out (createMu / fadeOutMu)
    }; // size 0x134

    // One row of the manpu effect table 8049E7F0 (indexed by manpuEf_c::mType, < 0x20).
    struct manpuEfData_s {
        /* 0x00 */ manpuEf_c::func_t mCreate;
        /* 0x0C */ manpuEf_c::func_t mExecute;
        /* 0x18 */ manpuEf_c::func_t mFade;  // NULL except "afm_mnp_mu_st"
        /* 0x24 */ char mName[0x20];         // "afm_mnp_smile_st" ...
    }; // size 0x44

    struct manpuData_s;
    // One active manpu (emotion symbol) on the npc, tied to an animation id and three offsets; driven
    // by a start / execute / end PTMF state machine from the table 8049F070 (manpuData_s, 78 types).
    class manpu_c {
    public:
        typedef BOOL (manpu_c::*func_t)(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angle,
                                        dAudioObjNpc_c *audio, const mVec3_c *ofs, const mVec3_c *ofsL,
                                        const mVec3_c *ofsR);
        enum { STATE_START = 0, STATE_EXEC = 1, STATE_END = 2, STATE_NONE = 3 };

        manpu_c();                                                                                           // 8001A9E8
        ~manpu_c();                                                                                          // 8001AA4C
        void reset();                                                                                        // 8001AAB4
        BOOL isActive() const;                                                                               // 8001AB74
        BOOL set(int type, u32 anmId, const mVec3_c *ofs, const mVec3_c *ofsL, const mVec3_c *ofsR);         // 8001AB9C
        BOOL isEnding() const;                                                                               // 8001AC6C
        void end();                                                                                          // 8001AC80
        BOOL execute(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angle, dAudioObjNpc_c *audio);      // 8001ACDC
        manpuEf_c *getFreeEf();                                                                              // 8001AD98
        void startSound(dAudioObjNpc_c *audio, u32 soundId) const;                                          // 8001ADF8
        void holdSound(dAudioObjNpc_c *audio, u32 soundId) const;                                           // 8001AE28
        BOOL createOneShot(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angle, const char *name, u32 effFrame); // 8001AE58
        BOOL startHatena(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angle, dAudioObjNpc_c *audio, const mVec3_c *ofs, const mVec3_c *ofsL, const mVec3_c *ofsR, u32 soundId); // 8001AF5C
        int start00(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001B014
        int exec00(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001B044
        int startA(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2, u32 se); // 8001B058
        int start01(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001B110
        int exec01(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001B140
        int start02(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001B154
        int exec02(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001B20C
        int startSmile(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2, u32 se); // 8001B220
        int start03(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001B2D8
        int execSmile(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2, u32 se); // 8001B308
        int exec03(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001B37C
        int startAseru(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2, u32 se); // 8001B3AC
        int execAseru(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2, u32 se); // 8001B464
        int start04(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001B4D8
        int exec04(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001B508
        int startLove(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2, u32 se); // 8001B538
        int start05(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001B5F4
        int exec05(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001B624
        int start24(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001B638
        int start25(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001B664
        int start26(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001B690
        int start06(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001B6BC
        int exec06(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001B7A0
        int startPikon(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2, u32 se); // 8001B834
        int start07(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001B8F0
        int exec07(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001B920
        int start23(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001B934
        int start08(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001B9B0
        int exec08(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001BA64
        int start09(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001BA78
        int exec09(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001BB2C
        int startPun(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2, u32 se); // 8001BB94
        int start10(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001BC7C
        int execPun(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2, u32 se); // 8001BCAC
        int exec10(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001BDFC
        int start11(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001BE2C
        int exec11(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001BF04
        int start12(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001BF84
        int exec12(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C03C
        int start27(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C050
        int start28(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C07C
        int start29(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C0B0
        int start30(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C0DC
        int start31(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C108
        int start32(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C134
        int start33(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C164
        int start34(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C194
        int start35(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C1C0
        int exec35(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C1F0
        int start36(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C218
        int exec36(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C248
        int start37(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C278
        int start38(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C2A4
        int start39(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C2D4
        int start40(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C304
        int start41(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C330
        int start42(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C35C
        int start43(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C388
        int start44(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C3B4
        int start45(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C3E4
        int exec45(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C414
        int start46(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C444
        int start47(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C474
        int start48(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C4A0
        int start49(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C4CC
        int exec49(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C4FC
        int start50(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C52C
        int start51(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C558
        int start52(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C584
        int start53(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C5B4
        int start54(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C5E0
        int exec54(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C610
        int start55(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C640
        int start56(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C694
        int start57(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C6EC
        int start58(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C718
        int exec58(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C748
        int start59(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C778
        int start60(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C7A4
        int exec60(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C7D4
        int start61(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C804
        int start62(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C834
        int start63(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C864
        int start64(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C894
        int start65(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C8C4
        int start66(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C8F0
        int start67(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C91C
        int start68(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C948
        int start69(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C974
        int start70(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C9A4
        int start71(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C9D0
        int start72(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001C9FC
        int exec72(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001CA2C
        int start73(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001CA5C
        int start13(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001CA88
        int exec13(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001CB40
        int startHa(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2, u32 se); // 8001CBA8
        int start14(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001CCA8
        int exec14(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001CCD8
        int start15(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001CCEC
        int exec15(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001CDA8
        int start16(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001CDEC
        int exec16(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001CEA4
        int end16(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001CEB8
        int start17(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001CECC
        int exec17(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001CF84
        int start18(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001CF98
        int exec18(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001D068
        int startHyu(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2, u32 se); // 8001D07C
        int start19(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001D150
        int start20(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001D180
        int exec20(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001D258
        int startNeboke(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2, u32 se); // 8001D2E8
        int start21(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001D3A4
        int exec21(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001D3D4
        int start22(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001D3E8
        int exec22(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001D4A0
        int start74(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001D4B4
        int start75(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001D4E0
        int start76(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001D510
        int start77(const mMtx_c *mtx, int anmId, u8 frame, const mAng *angY, dAudioObjNpc_c *audio, const mVec3_c *pos, const mVec3_c *v1, const mVec3_c *v2); // 8001D53C

        /* 0x000 */ const manpuData_s *mpData;
        /* 0x004 */ u32 mState;       // STATE_*
        /* 0x008 */ int mAnmId;       // 0x1BC = none
        /* 0x00C */ mVec3_c mOfs;
        /* 0x018 */ mVec3_c mOfsL;
        /* 0x024 */ mVec3_c mOfsR;
        /* 0x030 */ manpuEf_c mEf[4];
    }; // size 0x500

    // One row of the manpu table 8049F070: {start, execute, end} handlers, indexed by manpu_c::mState.
    struct manpuData_s {
        /* 0x00 */ manpu_c::func_t mFunc[3];
    }; // size 0x24

    // The npc's four animation-synced manpu effects (npc+0x2C0).
    class manpuMgr_c {
    public:
        // Data block passed by the emotion action: animation id, manpu type, stop the others.
        struct startData_s {
            /* 0x0 */ u32 mAnmId;
            /* 0x4 */ u8 mType;
            /* 0x5 */ u8 mStopOthers;
        }; // size 0x8

        manpuMgr_c();                                                                                        // 8001D56C
        ~manpuMgr_c();                                                                                       // 8001D5BC
        void init();                                                                                         // 8001D620
        static BOOL isValidType(u32 type);                                                                   // 8001D624
        BOOL start(u32 type, u32 anmId, BOOL stopOthers, const mVec3_c *pos0, const mVec3_c *pos1, const mVec3_c *pos2); // 8001D63C
        BOOL start(const startData_s *data, const mVec3_c *pos0, const mVec3_c *pos1, const mVec3_c *pos2);  // 8001D6C8
        void stopAll();                                                                                      // 8001D72C
        void clear();                                                                                        // 8001D778
        void execute(const mMtx_c *mtx, u32 anmId, const mAng *angle, dAudioObjNpc_c *audio, f32 frame);     // 8001D7C4
        manpu_c *getFreeEntry();                                                                             // 8001D870
        manpu_c *getEndingEntry();                                                                           // 8001D8D0
        manpu_c *getEntry();                                                                                 // 8001D940

        /* 0x0000 */ manpu_c mEntry[4];
    }; // size 0x1400

    // Head look-at controller (npc+0x18E0): finds the "head" node, receives its world matrix in
    // mdlCallback_c::timingB and turns the head (pitch/yaw angle chasers) toward a target. Requests
    // carry a priority.
    class lookAt_c {
    public:
        enum LOOK_TYPE_e {
            LOOK_FRONT,     // 0 lookFront     (setFront)
            LOOK_PLAYER,    // 1 lookPlayer    (setPlayer)
            LOOK_POS,       // 2 lookPos       (setPos)
            LOOK_PLAYER_NO, // 3 lookPlayerIdx (setPlayerNo)
            LOOK_ANGLE,     // 4 lookAngle     (manual targets)
            LOOK_AROUND,    // 5 lookAround    (setAround)
            LOOK_ACTOR,     // 6 lookActor     (setActor)
            LOOK_TYPE_NUM   // 7
        };
        // One head axis: current angle chased toward the target by step, target clamped to +-max.
        struct angle_s {
            /* 0x0 */ mAng mCur;
            /* 0x2 */ s16 mTarget;
            /* 0x4 */ s16 mStep;
            /* 0x6 */ s16 mMax;
        }; // size 0x8

        lookAt_c();                                                                                          // 80026A8C
        ~lookAt_c();                                                                                         // 80026B5C
        void init(nw4r::g3d::ResMdl mdl);                                                                    // 80026B9C
        BOOL hasHead() const;                                                                                // 80026BF8
        BOOL isHeadNode(u32 nodeId) const;                                                                   // 80026C10
        void calcHeadPos();                                                                                  // 80026C34
        BOOL set(int type, int prio, const mVec3_c &pos, int playerNo, const mAng &pitch, const mAng &yaw, const mAng &pitchStep, const mAng &yawStep, bool checkFov, f32 range = cNpcLookRange); // 80026CC8
        BOOL setFront(int prio, const mAng &pitchStep, const mAng &yawStep);                                 // 80026D74
        BOOL setPlayer(int prio, bool checkFov, const mAng &pitchStep, const mAng &yawStep, f32 range);      // 80026DD0
        BOOL setPos(int prio, const mVec3_c &pos, bool checkFov, const mAng &pitchStep, const mAng &yawStep, f32 range); // 80026E28
        BOOL setPlayerNo(int prio, int playerNo, bool checkFov, const mAng &pitchStep, const mAng &yawStep, f32 range); // 80026E7C
        BOOL setActor(int prio, dDemoActor_c *actor, bool checkFov, const mAng &pitchStep, const mAng &yawStep, f32 range); // 80026ED4
        BOOL setAround(int prio, const mAng &pitch, const mAng &yaw, const mAng &pitchStep, const mAng &yawStep); // 80026F68
        static BOOL canSee(const dAcNpc_c *npc, const mVec3_c &target, const mAng &fov, f32 range);          // 80026FB8
        static mAng calcYawDiff(const mVec3_c &target, const mVec3_c &from, const mAng &base);               // 80027070
        mAng calcPitchDiff(const mVec3_c &target, const mVec3_c &from, const mAng &base) const;              // 800270B4
        mAng getYaw(const mVec3_c &target, const mVec3_c &from, const mAng &base) const;                     // 8002713C
        mAng clampYaw(const mAng &ang) const;                                                                // 80027188
        mAng getPitch(const mVec3_c &target, const mVec3_c &from, const mAng &base) const;                   // 800271F8
        mAng clampPitch(const mAng &ang) const;                                                              // 80027238
        void chase(angle_s &a);                                                                              // 800272A8
        void chaseAll();                                                                                     // 800272B8
        static BOOL checkRange(const mVec3_c &target, const mVec3_c &from, f32 range);                       // 800272F4
        BOOL isInRange(const mVec3_c &target, const mVec3_c &from) const;                                    // 80027370
        static BOOL checkFov(const mVec3_c &target, const mVec3_c &from, const mAng &base, const mAng &fov); // 80027384
        BOOL isInFov(const mVec3_c &target, const mVec3_c &from, const mAng &base) const;                    // 800273C4
        void calcLook(dAcNpc_c *npc, const mVec3_c &target, const mVec3_c *rangePos);                        // 800273F4
        void lookPos(dAcNpc_c *npc);                                                                         // 800275D0
        void lookFront(dAcNpc_c *npc);                                                                       // 800275DC
        void lookPlayerNo(dAcNpc_c *npc, int playerNo);                                                      // 800275EC
        void lookPlayerIdx(dAcNpc_c *npc);                                                                   // 80027688
        void lookPlayer(dAcNpc_c *npc);                                                                      // 80027690
        void lookActor(dAcNpc_c *npc);                                                                       // 80027698
        void lookAngle(dAcNpc_c *npc);                                                                       // 800277AC
        void lookAround(dAcNpc_c *npc);                                                                      // 800277B0
        void execute(dAcNpc_c *npc);                                                                         // 8002783C
        static BOOL isWithin(const mAng &ang, const mAng &limit);                                            // 80027A04
        static int searchPlayer(int playerNo, const dAcNpc_c *npc, const mAng &fov, f32 range);              // 80027A4C
        int searchPlayer(int playerNo, const dAcNpc_c *npc) const;                                           // 80027B08
        BOOL isLooking() const;                                                                              // 80027B20
        void setPrio(int prio);                                                                              // 80027B48

        /* 0x00 */ u32 mHeadNodeId;       // "head" node id, -1 = none
        /* 0x04 */ int mType;             // LOOK_TYPE_e
        /* 0x08 */ int mPrio;             // request priority
        /* 0x0C */ dDemoActor_c *mpActor; // LOOK_ACTOR target
        /* 0x10 */ angle_s mPitch;
        /* 0x18 */ angle_s mYaw;
        /* 0x20 */ f32 mRange;            // xz distance limit, 0 = unlimited
        /* 0x24 */ int mPlayerNo;         // LOOK_PLAYER_NO player index
        /* 0x28 */ mVec3_c mTargetPos;    // LOOK_POS target
        /* 0x34 */ mVec3_c mHeadPos;      // head position in field coords (returned by demoHook58)
        /* 0x40 */ mMtx_c mHeadMtx;       // head node world mtx (timingB)
        /* 0x70 */ int mTimer;            // lookAround wait timer
        /* 0x74 */ mAng mFov;             // half field of view (isInFov)
        /* 0x76 */ bool mIsLooking;       // target reached / looking
        /* 0x77 */ bool mCheckFov;
        /* 0x78 */ bool mLocked;          // blocks set() and the per-type update
        /* 0x79 */ bool mReset;           // chase both angles back to 0 (also: item shown in hand)
        /* 0x7A */ u8 _7A[2];
    }; // size 0x7C

    // Swinging ears (npc+0x195C): two spring-damped joints ("Lear"/"Rear" nodes) applied in
    // mdlCallback_c::timingB. Per-npc-type parameters from l_80465A10[60][2] (type = getEarType()).
    class earCtrl_c {
    public:
        struct param_s {
            /* 0x00 */ f32 mLimitY[2]; // damping factor kept along local Y, [0] positive / [1] negative side
            /* 0x08 */ f32 mLimitZ[2]; // same along local Z
            /* 0x10 */ f32 mLength;    // tip offset along local X
            /* 0x14 */ f32 mSpring;    // pull toward the rest tip
            /* 0x18 */ f32 mDamping;   // velocity factor
        }; // size 0x1C

        class ear_c {
        public:
            ear_c();                                                                      // 80027B50
            ~ear_c();                                                                     // 80027BB4
            void init(nw4r::g3d::ResMdl mdl, const char *nodeName, const param_s *param); // 80027BF4
            BOOL isPosSet() const;                                                        // 80027C8C
            BOOL isNode(u32 nodeId) const;                                                // 80027CD4
            static void limitAxis(mVec3_c *v, const f32 *limit, f32 ax, f32 ay, f32 az);  // 80027CF8
            void calc(nw4r::g3d::WorldMtxManip *manip);                                   // 80027E08

            /* 0x00 */ u32 mNodeId;  // -1 = none
            /* 0x04 */ mVec3_c mPos; // simulated tip (world); Zero = not started
            /* 0x10 */ mVec3_c mVel;
            /* 0x1C */ param_s mParam;
        }; // size 0x38

        earCtrl_c();                                            // 8002808C
        ~earCtrl_c();                                           // 800280D4
        void init(nw4r::g3d::ResMdl mdl, int type);             // 80028138
        static const param_s *getParam(int type, int idx);      // 800281B8
        int searchNode(u32 nodeId) const;                       // 800281F0
        static BOOL isValidIdx(int idx);                        // 80028260
        BOOL isEarNode(u32 nodeId) const;                       // 80028278
        ear_c *getEar(u32 nodeId);                              // 8002829C
        void calc(nw4r::g3d::WorldMtxManip *manip, u32 nodeId); // 800282F0

        /* 0x00 */ ear_c mEars[2];
    }; // size 0x70

    // Talk session controller (npc+0x19CC): a requested kind (0 = turn to an angle then talk, 1 = talk
    // in place, 2 = none) is latched into mKind by execute(); its init/exec PTMF pair drives the
    // camera focus, the message start, the lip sync of the speaker and the cleanup.
    class talk_c {
    public:
        talk_c();                                                                                      // 8002D0F8
        ~talk_c();                                                                                     // 8002D0FC
        void init();                                                                                   // 8002D13C
        bool isBusy() const;                                                                           // 8002D174
        bool requestTurnTalk(const mAng &angle, const mAng &angle2, BOOL noCamera);                    // 8002D19C
        bool requestTalk(bool stopAction, BOOL noCamera);                                              // 8002D1BC
        void setKeepCamera(BOOL keep);                                                                 // 8002D1FC
        void updateMouth(dAcNpc_c *npc);                                                               // 8002D204
        bool request(int kind, const mAng &angle, const mAng &angle2, bool stopAction, BOOL noCamera); // 8002D29C
        void startTalk(dAcNpc_c *npc);                                                                 // 8002D2DC
        void changeKind(dAcNpc_c *npc);                                                                // 8002D458
        void execute(dAcNpc_c *npc);                                                                   // 8002D4C8
        void finish(dAcNpc_c *npc);                                                                    // 8002D530
        void initTurnTalk(dAcNpc_c *npc);                                                              // 8002D5FC
        void stepTurnTalkTurn(dAcNpc_c *npc);                                                          // 8002D708
        void stepTurnTalkMsg(dAcNpc_c *npc);                                                           // 8002D788
        void stepTurnTalkEnd(dAcNpc_c *npc);                                                           // 8002D958
        void execTurnTalk(dAcNpc_c *npc);                                                              // 8002D9E0
        void initTalk(dAcNpc_c *npc);                                                                  // 8002DA20
        void stepTalkStart(dAcNpc_c *npc);                                                             // 8002DB24
        void stepTalkMsg(dAcNpc_c *npc);                                                               // 8002DB58
        void stepTalkEnd(dAcNpc_c *npc);                                                               // 8002DD28
        void execTalk(dAcNpc_c *npc);                                                                  // 8002DDB0

        /* 0x00 */ int mKind;        // 0 turnTalk, 1 talk, 2 none
        /* 0x04 */ int mReqKind;     // requested kind, 2 = none
        /* 0x08 */ BOOL mNoCamera;   // 0: focus the camera on the npc (init) and restore it (finish)
        /* 0x0C */ BOOL mKeepCamera; // nonzero: finish() leaves the camera; cleared by request()
        /* 0x10 */ mAng mAngle;      // yaw to turn to
        /* 0x12 */ mAng mAngle2;     // second angle passed to action_c::requestTurn
        /* 0x14 */ bool mStopAction; // initTalk: requestWait when set
        /* 0x15 */ bool mNoMouth;    // updateMouth: no lip sync when set
        /* 0x16 */ u8 mStep;         // step inside the kind (3 = done)
        /* 0x17 */ u8 _17;
    }; // size 0x18

    // XZ box relative to a center (camera target or a player's position) = what the camera shows;
    // used for draw culling and (static instance l_80564BE8) "visible to any player" checks.
    class viewArea_c {
    public:
        viewArea_c();                                                             // 80028330
        ~viewArea_c();                                                            // 80028374
        BOOL isOutside(const mVec3_c *pos, const nw4r::math::VEC3 *center) const; // 800283B4
        BOOL isUnitOutside(int ux, int uz, const nw4r::math::VEC3 *center) const; // 8002842C
        BOOL isOutside(const mVec3_c *pos) const;                                 // 800284F8
        BOOL isOutside(const dActor_c *actor) const;                              // 80028504
        BOOL isOutsideAll(u32 playerNo, const mVec3_c *pos) const;                // 8002850C
        BOOL isOutsideAll(u32 playerNo, const dActor_c *actor) const;             // 800285F4
        BOOL isUnitOutside(int ux, int uz) const;                                 // 800285FC
        BOOL isUnitOutsideAll(u32 playerNo, int ux, int uz) const;                // 80028608

        /* 0x00 */ mVec3_c mMin;
        /* 0x0C */ mVec3_c mMax;
    }; // size 0x18

    // Ground probe at two points ahead of the npc (npc+0x19FC).
    class frontChk_c {
    public:
        frontChk_c();                                                    // 800286E4
        ~frontChk_c();                                                   // 800286F0
        void init();                                                     // 80028730
        void clear();                                                    // 80028740
        static BOOL checkStep(const mVec3_c *pos, f32 prevY, f32 *outY); // 8002874C
        void calc(dAcNpc_c *npc);                                  // 800288D0

        /* 0x00 */ f32 mGroundY[2];
        /* 0x08 */ u8 mHitFlags; // bit i = side i blocked
    }; // size 0xC

    // Footprint effects and footstep sounds on the walk animations' contact frames (npc+0x1BE4).
    class footstep_c {
    public:
        footstep_c();                  // 800289B0
        ~footstep_c();                 // 800289B4
        void init();                   // 800289F4
        void playSound(dAcNpc_c *npc); // 80028A14
        void setPrintL(dAcNpc_c *npc); // 80028AB8
        void setPrintR(dAcNpc_c *npc); // 80028B04
        void calc(dAcNpc_c *npc);      // 80028B50

        /* 0x00 */ f32 mPrevSpeed; // npc->mSpeedF of the last frame
        /* 0x04 */ s16 mPrevAngY;  // npc->mAngle.y
        /* 0x06 */ s16 mPrevTurn;  // npc->mAction.mTurnSpeed
        /* 0x08 */ u8 mEnable;     // init 1
    }; // size 0xC

    // The npc's message receiver: connects the message controller (Rcpt_c::mpController) with the
    // talking npc (mpNpc), an optional second npc (mpPartner) and the player; reacts to message tags
    // and runs "requests" (18 kinds, PTMF table 804A0000) set by the talk/quest TUs. Abstract; the
    // subclasses are in d_a_npc_nml / d_a_npc_sp. RTTI dAcNpc_c::recept_c, vtable 804A0360.
    class recept_c : public dMsg::Rcpt_c {
    public:
        // Filter for the item-select menu (fn_8019A8B4 calls it per pocket slot). Provisional.
        typedef BOOL (*itemFilterFn)(void *pockets, int flag);

        // Parameters of the pending request (recept_c+0x84; init 80029DA8). Offsets in the comments:
        // inside reqParam_s / inside recept_c.
        struct reqParam_s {

            /* 0x00 0x84 */ int mMenuKind;               // menu sub-kind 0..0x34 (init 0x35)
            /* 0x04 0x88 */ u32 mReq1Arg;                // request 1 argument (setRequest1) -> controller+0x6C60
            /* 0x08 0x8C */ u8 _08[4];
            /* 0x0C 0x90 */ const dMail_c *mpMail;       // reqMailMenu
            /* 0x10 0x94 */ const dEquip_c *mpEquip;     // requestEquip
            /* 0x14 0x98 */ int mEquipMode;
            /* 0x18 0x9C */ int mEquipKind;
            /* 0x1C 0xA0 */ int _1C;                     // requestItemAct / requestItemActEx
            /* 0x20 0xA4 */ int _20;
            /* 0x24 0xA8 */ int _24;
            /* 0x28 0xAC */ int _28;
            /* 0x2C 0xB0 */ int _2C;                     // init 0x1BC; requestAct09
            /* 0x30 0xB4 */ f32 _30;                     // init 12.0f; requestAct09
            /* 0x34 0xB8 */ f32 _34;                     // requestAct09
            /* 0x38 0xBC */ int _38;                     // init 4; requestAct09
            /* 0x3C 0xC0 */ int _3C;                     // init 2; requestItemActEx / requestHandAct10
            /* 0x40 0xC4 */ dItem::Item mItem;           // init 0xFFF1
            /* 0x42 0xC6 */ u16 mItemMsgParam;           // menu arg (fn_8019A804/85C 2nd arg)
            /* 0x44 0xC8 */ u8 mMoveCamera;              // init 1; requestChangeSpeaker
            /* 0x45 0xC9 */ u8 mHair;                    // requestHairChange
            /* 0x46 0xCA */ u8 mHairColor;
            /* 0x47 0xCB */ u8 _47;
            /* 0x48 0xCC */ int mKeepMsg;                // 1 = keep the message open after the menu
            /* 0x4C 0xD0 */ u32 mMenuArgD0;              // init -1; reqMenu29
            /* 0x50 0xD4 */ int mMenuNo;                 // reqMenu0D
            /* 0x54 0xD8 */ itemFilterFn mpItemFilter;   // reqSelectItem
            /* 0x58 0xDC */ u16 *mpInputBuf;             // reqTextInput*
            /* 0x5C 0xE0 */ u32 mMenuArgE0;              // reqMenu31
            /* 0x60 0xE4 */ u16 mItemArg;                // reqSelectItem
            /* 0x62 0xE6 */ u8 mItemMenuNo;              // reqSelectItem (default 0x29)
            /* 0x63 0xE7 */ u8 _63;
        }; // size 0x64

        // The message controller. dMsg::Comp_c (Rcpt_c::mpController) is the object the d_msg TU's
        // dDemo_c methods (state functions fn_801A334C, fn_801A4D74 ...) work on; both names are in
        // Matching units' symbols, so they cannot be merged: d_a_npc views the controller as dDemo_c.
        dDemo_c *getController() const { return reinterpret_cast<dDemo_c *>(mpController); }
        void setVoiceType(int value) { mVoiceType = value; }

        // The message to start (getMsgInfo).
        struct msgInfo_s {
            const char *mLabel;
            u16 mCode;
        };

        recept_c();                                                                             // 80028CD4
        virtual ~recept_c();                                                                    // 80028D64
        virtual void init();                                                                    // 80028DA4
        void saveDemoFlag(dDemoActor_c *actor);                                                 // 80028DF8
        void restoreDemoFlag(dDemoActor_c *actor);                                              // 80028E54
        virtual int getVoiceMode();                                                               // 80028EB8
        void setLandName(const dLandID_c *land, int idx);                                       // 80028EC0
        void setPersonalName(const dPersonalID_c *id, int idx);                                 // 80028ED4
        void setPlayerName(const dPlayerID_c *id, int idx);                                     // 80028EE8
        void setAnmPersonalName(const dAnmPersonalID_c *id, int idx);                           // 80028EFC
        void setItemName(const dItem::Item *item, int idx);                                     // 80028F10
        void setMonthName(int month, int idx);                                                  // 80028F24
        void setDayName(int day, int idx);                                                      // 80028F38
        void setTime(int hour, int amPmIdx, int hourIdx);                                       // 80028F4C
        void setBells(int value, int idx);                                                      // 80028FD4
        void setNumber(int value, int idx, int width, dScript::NumberFormat_e fmt);             // 8002907C
        void setNumber4(int value, int idx);                                                    // 80029090
        void setMinute(int minute, int idx);                                                    // 800290A4
        void formatNumber(int value, int idx, int width, dScript::NumberFormat_e fmt);          // 800290B8
        void setDecimal(f64 value, int idx, int precision);                                     // 800290CC
        void setUnitWord(int kind, int idx);                                                    // 800290D4
        void clearWord(int idx);                                                                // 80029184
        BOOL fn_8002918C() const;                                                                 // 8002918C
        void execute();                                                                         // 800291A8
        int getPlayerNo() const;                                                                // 800291E8
        void setPartner(dAcNpc_c *npc);                                                         // 80029220
        dAcNpc_c *getNpc(int idx);                                                              // 80029284
        dAcNpc_c *getNpc(int idx) const;                                                        // 800292B4
        virtual void setSpeakerPlayer();                                                              // 800292E4
        virtual void setSpeakerNpc();                                                              // 800292F0
        virtual void setSpeakerPartner();                                                              // 800292FC
        virtual void speakerLookAtPlayer();                                                              // 80029308
        virtual void speakerTurnToPlayer();                                                              // 80029374
        void playerLookAt(dAcNpc_c *target);                                                    // 80029434
        void playerTurnTo(dAcNpc_c *target);                                                    // 800294D4
        virtual void speakerLookAtNpc();                                                              // 800295B0
        virtual void speakerTurnToNpc();                                                              // 8002966C
        virtual void speakerLookAtPartner();                                                              // 8002975C
        virtual void speakerTurnToPartner();                                                              // 80029824
        virtual void changeSpeaker();                                                              // 80029920
        static BOOL isMenuInvalid();                                                            // 80029928
        static void *getMenuWork();                                                             // 80029940
        static int getMenuSelSlot();                                                            // 80029944
        static dItem::Item *getMenuSelItems(int *count);                                        // 8002997C
        static BOOL removeMenuSelItems();                                                       // 800299E0
        static u16 getMenuItem();                                                               // 80029A18
        static u32 fn_80029A50();                                                               // 80029A50
        static u32 fn_80029A84();                                                               // 80029A84
        static u32 fn_80029AB8();                                                               // 80029AB8
        static u32 fn_80029AEC();                                                               // 80029AEC
        static u32 fn_80029B20();                                                               // 80029B20
        static bool isMenuSel01();                                                              // 80029B54
        static bool isMenuSel20();                                                              // 80029B94
        static int countMenuSelSlots();                                                         // 80029BD4
        static bool isMenuSelSlot(int slot);                                                    // 80029C08
        static bool isMenuSel40();                                                              // 80029C58
        static bool isMenuSel80();                                                              // 80029C98
        static dTime_c getMenuResult();                                                         // 80029CD8: the time entered in the time menu (reqMenu27)
        static u32 fn_80029CDC();                                                               // 80029CDC
        static u16 fn_80029CE0();                                                               // 80029CE0
        void setFeel(u32 feel, u32 who, f32 frame = cNpcMorphFrames);                                             // 80029CE4
        virtual void setSpeakerFeel(u32 feel);                                                      // 80029D80
        void clearRequest();                                                                    // 80029D8C
        void initReqParam(reqParam_s *param);                                                   // 80029DA8
        BOOL setRequest(int kind);                                                              // 80029E24
        void execRequest();                                                                     // 80029E60
        BOOL setRequest1(int arg);                                                              // 80029F3C
        BOOL req1Start();                                                                       // 80029F90
        BOOL req1Wait();                                                                        // 80029FB4
        int procReq01();                                                                        // 8002A030
        BOOL reqMsgClose();                                                                     // 8002A07C
        int procMsgClose();                                                                     // 8002A084
        BOOL reqMenu(int menuKind);                                                             // 8002A104
        BOOL reqSelectItem(u16 item, u16 msgParam, int keepMsg);                                // 8002A158
        BOOL reqSelectItem(u16 item, u16 msgParam, u8 menuNo, int keepMsg);                     // 8002A1E8
        BOOL reqSelectItem(itemFilterFn filter, u16 msgParam, u8 menuNo, int keepMsg);          // 8002A268
        BOOL reqSelectItem2(itemFilterFn filter, u16 msgParam, int keepMsg);                    // 8002A2E8
        BOOL reqMailMenu(const dMail_c *mail, int keepMsg);                                     // 8002A370
        BOOL reqMenu05(int keepMsg);                                                            // 8002A3F8
        BOOL reqMenu06(int keepMsg);                                                            // 8002A44C
        BOOL reqMenu08(int keepMsg);                                                            // 8002A4A0
        BOOL reqMenu09(int keepMsg);                                                            // 8002A4F4
        BOOL reqMenu0A(int keepMsg);                                                            // 8002A548
        BOOL reqMenu0B(int keepMsg);                                                            // 8002A59C
        BOOL reqMenu0C(int keepMsg);                                                            // 8002A5F0
        BOOL reqMenu0D(int menuNo, int keepMsg);                                                // 8002A644
        BOOL reqMenu0E(int keepMsg);                                                            // 8002A6A8
        BOOL reqMenu0F(int keepMsg);                                                            // 8002A6FC
        BOOL reqMenu10(int keepMsg);                                                            // 8002A750
        BOOL reqMenu11(int keepMsg);                                                            // 8002A7A4
        BOOL reqMenu12(int keepMsg);                                                            // 8002A7F8
        BOOL reqMenu13(int keepMsg);                                                            // 8002A84C
        BOOL reqMenu14(int keepMsg);                                                            // 8002A8A0
        BOOL reqMenu15(int keepMsg);                                                            // 8002A8F4
        BOOL reqMenu16(int keepMsg);                                                            // 8002A948
        BOOL reqMenu17(int keepMsg);                                                            // 8002A99C
        BOOL reqMenu18(int keepMsg);                                                            // 8002A9F0
        BOOL reqMenu19(int keepMsg);                                                            // 8002AA44
        BOOL reqMenu1A(int keepMsg);                                                            // 8002AA98
        BOOL reqMenu1B(int keepMsg);                                                            // 8002AAEC
        BOOL reqMenu1C(int keepMsg);                                                            // 8002AB40
        BOOL reqMenu1D(int keepMsg);                                                            // 8002AB94
        BOOL reqMenu1E(int keepMsg);                                                            // 8002ABE8
        BOOL reqMenu1F(int keepMsg);                                                            // 8002AC3C
        BOOL reqMenu20(int keepMsg);                                                            // 8002AC90
        BOOL reqMenu21(int keepMsg);                                                            // 8002ACE4
        BOOL reqMenu22(int keepMsg);                                                            // 8002AD38
        BOOL reqMenu23(int keepMsg);                                                            // 8002AD8C
        BOOL reqMenu24(int keepMsg);                                                            // 8002ADE0
        BOOL reqMenu25(int keepMsg);                                                            // 8002AE34
        BOOL reqMenu26(int keepMsg);                                                            // 8002AE88
        BOOL reqMenu27(int keepMsg);                                                            // 8002AEDC
        BOOL reqMenu28(int keepMsg);                                                            // 8002AF30
        BOOL reqMenu29(u32 arg, int keepMsg);                                                   // 8002AF84
        BOOL reqMenu2A(int keepMsg);                                                            // 8002AFE8
        BOOL reqMenu2B(int keepMsg);                                                            // 8002B03C
        BOOL reqMenu2C(int keepMsg);                                                            // 8002B090
        BOOL reqMenu2D(int keepMsg);                                                            // 8002B0E4
        BOOL reqMenu2E(int keepMsg);                                                            // 8002B138
        BOOL reqTextInput2F(u16 *buf, int keepMsg);                                             // 8002B18C
        BOOL reqMenu30(int keepMsg);                                                            // 8002B1F0
        BOOL reqMenu31(u32 arg, int keepMsg);                                                   // 8002B244
        BOOL reqMenu32(int keepMsg);                                                            // 8002B2A8
        BOOL reqTextInput33(u16 *buf, int keepMsg);                                             // 8002B2FC
        BOOL reqTextInput34(u16 *buf, int keepMsg);                                             // 8002B360
        int stepMenuInit();                                                                     // 8002B3C4
        int stepMenuOpen();                                                                     // 8002B3E4
        int stepMenuWait();                                                                     // 8002B9AC
        int stepMenuEnd();                                                                      // 8002BA18
        int procMenu();                                                                         // 8002BA78
        BOOL requestItemAct(const dItem::Item *item, int a, int b);                             // 8002BAC4
        int stepItemActStart();                                                                 // 8002BB90
        int stepItemActWait();                                                                  // 8002BC4C
        int procItemAct();                                                                      // 8002BCC0
        BOOL requestItemActEx(int a4, const dItem::Item *item, int a0, int a8, int ac, int c0); // 8002BD0C
        int stepItemActExStart();                                                               // 8002BDE0
        int stepItemActExWait();                                                                // 8002BEA4
        int procItemActEx();                                                                    // 8002BF18
        BOOL requestHandActC();                                                                 // 8002BF64
        int stepHandActCStart();                                                                // 8002BF6C
        int stepHandActCWait();                                                                 // 8002C00C
        int procHandActC();                                                                     // 8002C080
        BOOL requestHandActD();                                                                 // 8002C0CC
        int stepHandActDStart();                                                                // 8002C0D4
        int stepHandActDWait();                                                                 // 8002C174
        int procHandActD();                                                                     // 8002C1E8
        BOOL requestHandActE();                                                                 // 8002C234
        int stepHandActEStart();                                                                // 8002C23C
        int stepHandActEWait();                                                                 // 8002C2DC
        int procHandActE();                                                                     // 8002C350
        BOOL requestHandActF();                                                                 // 8002C39C
        int stepHandActFStart();                                                                // 8002C3A4
        int stepHandActFWait();                                                                 // 8002C440
        int procHandActF();                                                                     // 8002C4AC
        BOOL requestHandAct10(int param);                                                       // 8002C4F8
        int stepHandAct10Start();                                                               // 8002C54C
        int stepHandAct10Wait();                                                                // 8002C5F0
        int procHandAct10();                                                                    // 8002C664
        BOOL requestHandAct11();                                                                // 8002C6B0
        int stepHandAct11Start();                                                               // 8002C6B8
        int stepHandAct11Wait();                                                                // 8002C758
        int procHandAct11();                                                                    // 8002C7CC
        BOOL requestEquip(const dEquip_c *equip, int mode, int kind);                           // 8002C818
        int stepEquipStart();                                                                   // 8002C894
        int stepEquipWait();                                                                    // 8002C900
        int procEquip();                                                                        // 8002C95C
        BOOL requestHairChange(u8 hair, u8 color);                                              // 8002C9A8
        int stepHairChange();                                                                   // 8002CA0C
        int procHairChange();                                                                   // 8002CA68
        BOOL requestChangeSpeaker(u8 moveCamera);                                               // 8002CAB4
        int stepChangeSpeakerInit();                                                            // 8002CB08
        int stepChangeSpeakerCamera();                                                          // 8002CB40
        int stepChangeSpeakerTalk();                                                            // 8002CBEC
        int procChangeSpeaker();                                                                // 8002CD74
        BOOL requestAct09(int b0, f32 b8, f32 b4, int bc);                                      // 8002CDC0
        int stepAct09Start();                                                                   // 8002CE44
        int stepAct09Wait();                                                                    // 8002CF04
        int procAct09();                                                                        // 8002CF78
        bool requestPlayerTurn();                                                               // 8002CFC4
        int stepPlayerTurnStart();                                                              // 8002CFF4
        int stepPlayerTurnWait();                                                               // 8002D048
        int procPlayerTurn();                                                                   // 8002D0A4
        int procNone();                                                                         // 8002D0F0
        // new virtuals, in vtable order
        virtual void preExecute() {}                   // +0x94 (weak 8002EA08; called by execute)
        virtual void onTalkEnd() {}                         // +0x98 (weak 8002EA14; message end)
        virtual void getMsgInfo(msgInfo_s *info) = 0;  // +0x9C (talk_c::startTalk)
        virtual void onRequestEnd(int kind);           // +0xA0 80029F38
        virtual int getTurnFrame();                    // +0xA4 800295A8 (500)

        /* 0x64 */ dActor_c *mpActActor; // target actor passed to the npc's hand-item actions
        /* 0x68 */ dAcNpc_c *mpNpc;      // owner (getNpc(0)); not cleared by init
        /* 0x6C */ dAcNpc_c *mpPartner;  // getNpc(1)
        /* 0x70 */ int mTalkIdx;         // which of npc/partner talks (0/1)
        /* 0x74 */ int mSpeaker;         // 0 npc, 1 partner, 2 player (hooks 4C/50/54)
        /* 0x78 */ u8 mActing;           // 1 while a hand/item action runs; setFeel ignores the npc then
        /* 0x79 */ u8 mDemoFlagSaved;
        /* 0x7A */ u8 mDemoFlag8;
        /* 0x7C */ int mReqKind;         // 0..17, 18 = none
        /* 0x80 */ BOOL mReqPending;
        /* 0x84 */ reqParam_s mReq;
        /* 0xE8 */ u8 mReqStep;
        /* 0xE9 */ u8 mLangFlag;          // language flag index (init 6)
    }; // size 0xEC

    // Abstract npc resource manager interface (implemented by dAcNpcNml_c::resMng_c ...). RTTI
    // dAcNpc_c::resBase_c, vtable 804A0330.
    class resBase_c {
    public:
        resBase_c();          // 8002DDF0
        virtual ~resBase_c(); // 8002DE00
        virtual BOOL create(dAcNpc_c *npc) = 0; // FALSE -> not ready
        virtual void *getMdlRes() = 0;        // the model ResFile data (getMdlResFile)
        virtual void *getTexRes() = 0;        // the texture/face ResFile data (getTexResFile)
    }; // size 0x4

    // Abstract clothing interface (implemented in d_a_npc_nml). RTTI dAcNpc_c::clothBase_c,
    // vtable 804A02F0.
    class clothBase_c {
    public:
        clothBase_c();          // 8002DE40
        virtual ~clothBase_c(); // 8002DE50
        virtual BOOL requestNpcCloth(dAcNpc_c *npc) = 0; // request the wearer's current cloth
        virtual BOOL requestCloth(const dItem::Item *item, dDesign_c *design) = 0; // request a cloth
        virtual BOOL create(dAcNpc_c *npc) = 0;            // create: FALSE -> not ready
        virtual void bindCloth(nw4r::g3d::ResMdl mdl) = 0;    // create
        virtual void swapCloth(nw4r::g3d::ResMdl mdl) = 0;    // change anm frame 24: model swap
        virtual BOOL execute(dAcNpc_c *npc) = 0;            // execute (result unused by d_a_npc)
        virtual BOOL isLoaded(const dAcNpc_c *npc) const = 0; // ready to change clothes? (const: d_a_npc_nml 80030848 / 8003004C)
    }; // size 0x4

    // ---- vtable 804A0200 (0xC4: 2 header words + 47 slots) ----
    // fBase_c / dActor_c / dDemoActor_c overrides (dtor slot +0x48: 8002EA18, implicit)
    virtual ~dAcNpc_c() {}                          // 8002EA18 (weak, after __sinit)
    virtual int create();                           // 80015C2C
    virtual int preCreate();                        // 80015808
    virtual void postCreate(MAIN_STATE_e state);    // 80015E50
    virtual int doDelete();                         // 80015EEC
    virtual int execute();                          // 80015F70
    virtual int draw();                             // 8001665C
    virtual bool canInteract(dDemoActor_c *actor);  // 80012FC0 (+0x4C)
    virtual void demoHook50();                      // 80012FE4 (+0x50)
    virtual mVec3_c *demoHook58();                  // 800134E8 (+0x58) look/talk target position
    virtual int demoHook64(mVec3_c *pos);           // 80016770 (+0x64)
    // dAcNpc_c's own virtuals (+0x68..+0xC0)
    virtual int vt68();                             // 8001407C (+0x68)
    virtual nw4r::g3d::ResFile getMdlResFile();                  // 80012914 (+0x6C)
    virtual nw4r::g3d::ResFile getTexResFile();                  // 80012938 (+0x70)
    virtual model_c *getModel() { return &mModel; }        // 8002E9A0 (+0x74)
    virtual clothBase_c *getCloth() { return NULL; }       // 8002E990 (+0x78)
    virtual toolBase_c *getTool() { return NULL; }         // 8002E9A8 (+0x7C)
    virtual void vt80() {}                                 // 8002EA0C (+0x80) (recept_c::setPartner)
    virtual void vt84() {}                                 // 8002EA10 (+0x84) (talk_c::finish, on the partner)
    virtual void getName(dHmnName::Word_c *name, int len) = 0; // (+0x88)
    virtual u8 getNameKind() = 0;                          // (+0x8C)
    virtual f32 vt90() const { return 0.0f; }                    // 8002E9F8 (+0x90)
    virtual f32 vt94() const { return 0.0f; }                    // 8002EA00 (+0x94)
    virtual int getEarType() = 0;                          // (+0x98) 0..59, 60 = no ears
    virtual const dItem::Item *getHoldItem() { return NULL; } // 8002E9B0 (+0x9C)
    virtual int getSoundId();                              // 800156F0 (+0xA0)
    virtual void playSound();                              // 800156F8 (+0xA4)
    virtual int getVoiceType() const = 0;                                // (+0xA8) -> Rcpt_c::mVoiceType
    virtual void getManpuOfs(mVec3_c *ofs, mVec3_c *ofsL, mVec3_c *ofsR, u8 type); // 80015734 (+0xAC)
    virtual int vtB0() { return 1; }                       // 8002E998 (+0xB0) execute hook
    virtual u32 getHeapSize() = 0;                         // (+0xB4)
    virtual u32 addToNpcList() = 0;                                // (+0xB8) stored at _DC
    virtual void removeFromNpcList() = 0;                               // (+0xBC) doDelete hook
    virtual int getFaceType() = 0;                          // (+0xC0)

    BOOL loadRes(void **pData, const char *path);                                                        // 800128A0
    f32 calcDistXZ(const dActor_c *actor, const mVec3_c *pos) const;                                     // 8001295C
    f32 calcPlayerDistXZ(int player, const mVec3_c *pos) const;                                          // 800129BC
    int searchPlayer(u32 player, BOOL checkGround, int exclude, BOOL nearest, const mVec3_c *pos, f32 range) const; // 80012A08
    BOOL isPlayerNear(u32 player, BOOL checkGround, const mVec3_c *pos, f32 range) const;                // 80012B7C
    mAng getAngleYTo(const dActor_c *actor) const;                                                       // 80012BB8
    mAng getAngleYToPlayer(int player) const;                                                            // 80012BF4
    BOOL calcOfsPos(mVec3_c *out, const mVec3_c *ofs);                                                   // 80012C30 non-const (scheduling)
    int getSidePos(mVec3_c *out);                                                                        // 80012CFC
    void setRecept(recept_c *recept);                                                                    // 80012DA8
    mVec3_c getBodyPos() const;                                                                          // 80012DBC
    mVec3_c getNode0EPos() const;                                                                        // 80012E50
    mVec3_c getNode13Pos() const;                                                                        // 80012EAC
    mVec3_c getFootPos0() const;                                                                         // 80012F08
    mVec3_c getFootPos1() const;                                                                         // 80012F64
    int requestPermit(int type, u32 player, BOOL arg);                                                   // 80012FE8
    void setPermitOwner(u32 player);                                                                     // 8001318C
    BOOL setPermitState(u32 player);                                                                     // 80013204
    BOOL setPermitStateSelf();                                                                           // 800132AC
    BOOL setPermitFlagSelf();                                                                            // 80013348
    BOOL isPermitSelf() const;                                                                           // 800133E0
    void fn_80013474();                                                                                  // 80013474
    bool checkLocalOwner() const;                                                                        // 80013478
    void fn_800134A4();                                                                                  // 800134A4
    BOOL isLocalOwnerHost() const;                                                                       // 800134A8
    static BOOL isNetActive();                                                                           // 8001354C
    static BOOL isMultiPlay();                                                                           // 80013550
    BOOL isLocalOwner() const;                                                                           // 80013598
    void saveDaub();                                                                                     // 80013630
    bool isNetSync() const;                                                                              // 8001371C
    void clearNetSync();                                                                                 // 80013724
    BOOL getPermitPlayers(int *owner, int *player) const;                                                // 80013730
    BOOL isInUse(int *player) const;                                                                     // 800137C8
    BOOL getDaubPos(mVec3_c *pos, mAng *angleY) const;                                                   // 80013864
    BOOL getDaubHeadAngle(mAng *headY, mAng *headX) const;                                               // 80013918
    BOOL getDaubData(void *dst, u32 size) const;                                                         // 800139D0
    BOOL setDaubData(const void *src, u32 size);                                                         // 80013A88
    BOOL getDaubState(u32 *state, u32 *sub) const;                                                       // 80013B04
    BOOL getDaubMove(mVec3_c *from, mVec3_c *to, u16 *angle, f32 *speed) const;                          // 80013BD8
    BOOL getDaubTalk(u16 *a, u16 *b, f32 *c) const;                                                      // 80013C98
    BOOL getDaubAct(u32 *a, u8 *b, f32 *c, f32 *d) const;                                                // 80013DA4
    BOOL recvDaub();                                                                                     // 80013E64
    static BOOL isPlayerFromThisTown();                                                                  // 80013F1C
    static BOOL checkSpotFunc(int ux, int uz, int arg);                                                  // 80013F98
    static BOOL searchSpot(mVec3_c *out, int arg, const mVec3_c *exclude, f32 dist);                     // 80014008
    static BOOL searchSpotAny(mVec3_c *out, const mVec3_c *exclude, f32 dist);                           // 80014020
    static BOOL checkSpotFunc2(int x, int z, int arg);                                                   // 80014030
    static BOOL searchSpot2(mVec3_c *out, int arg, const mVec3_c *exclude, u32 type, BOOL skipTrees, BOOL allowFg94, f32 dist); // 8001404C
    static BOOL searchFreeSideAngle(mAng *pAngle, const mVec3_c *pCenter, BOOL twoPass);                 // 80014084
    static int getSingleDir(int dirBits);                                                                // 800142CC
    static u32 countDirBits(int dirBits);                                                                // 8001436C
    static u32 dirToBit(u32 dir);                                                                        // 800143C4
    static int getFreeDirToward(int tgtX, int tgtZ, int unitX, int unitZ);                               // 800143E0
    static u32 getFreeDirBitsToward(int tgtX, int tgtZ, int unitX, int unitZ);                           // 8001468C
    static BOOL searchPosNear(mVec3_c *pPos, const mVec3_c *pCenter, f32 minDist, f32 maxDist);          // 8001481C
    static BOOL isFreeBeyond(const mVec3_c *pPos, const mVec3_c *pTarget, BOOL checkA);                  // 80014A80
    static BOOL searchPosInCircle(mVec3_c *pPos, const mVec3_c *pBase, const mVec3_c *pTarget, f32 radius, f32 minDist, f32 maxDist); // 80014B3C
    static BOOL searchPosInRect(mVec3_c *pPos, const mVec3_c *pTarget, f32 x0, f32 z0, f32 x1, f32 z1, f32 minDist, f32 maxDist); // 80014E70
    BOOL checkTargetAngle(mAng *pAngle, const mVec3_c *pTarget, const mAng *pRange) const;               // 800151DC
    u32 selectFreeSide();                                                                          // 80015260
    static BOOL getPointedUnit(int *pUnitX, int *pUnitZ, const mAng *pAngle, int snapToCardinal, f32 range, f32 dist); // 8001537C
    void stopManpu();                                                                                    // 800155C8
    BOOL checkWarpHome();                                                                                // 800155D0
    void setGroundEffect(const char *sandName, const char *otherName, const mVec3_c *pPos, const mAng3_c *pAngle); // 80015738

    // Plays a body animation and the face textures of it (the recurring block of the actions).
    void setAnm(int anmId, m3d::playMode_e playMode, f32 frame, f32 rate, const f32 &blend, BOOL force) {
        if (getModel()->setAnm(anmId, playMode, frame, rate, blend, force) && mFace.isValid()) {
            mFace.setAnmTex(this, anmId, m3d::PLAYMODE_INHERIT);
        }
    }

    // ---- data (offsets from the dtor 8002EA18 and preCreate 80015808) ----
    /* 0x00BC */ EGG::FrmHeap *m_heap_p;      // "dAcNpc_c::m_heap_p : NPC actor heap"
    /* 0x00C0 */ mAllocator_c mAllocator;
    /* 0x00DC */ u32 _DC;                     // = addToNpcList() (preCreate)
    /* 0x00E0 */ dItem::Item mNpcItem;        // the npc's id as an item code (0xE000|i, 0x8011 ...)
    /* 0x00E2 */ u8 _E2[2];
    /* 0x00E4 */ resBase_c *mpRes;
    /* 0x00E8 */ anmSet_c mAnmSet;
    /* 0x00F4 */ f32 mAnmRate;                // base animation rate (action_c::calcAnmRate)
    /* 0x00F8 */ dBGCF::acch_c mAcch;
    /* 0x0140 */ model_c mModel;
    /* 0x023C */ face_c mFace;
    /* 0x02C0 */ manpuMgr_c mManpu;
    /* 0x16C0 */ action_c mAction;
    /* 0x18E0 */ lookAt_c mLookAt;
    /* 0x195C */ earCtrl_c mEar;
    /* 0x19CC */ talk_c mTalk;
    /* 0x19E4 */ viewArea_c mViewArea;
    /* 0x19FC */ frontChk_c mFrontChk;
    /* 0x1A08 */ mMtx_c mNodeMtx01;           // world mtx of model node 1 (body; identity = not computed yet)
    /* 0x1A38 */ mMtx_c mNodeMtx12;           // node 0x12 (manpu matrix)
    /* 0x1A68 */ mMtx_c mNodeMtx0B;           // node 0xB
    /* 0x1A98 */ mMtx_c mNodeMtx0E;           // node 0xE (hand: tool matrix)
    /* 0x1AC8 */ mMtx_c mNodeMtx13;           // node 0x13 (mouth: effects)
    /* 0x1AF8 */ mMtx_c mNodeMtx04;           // node 4 (foot, footmark side 1)
    /* 0x1B28 */ mMtx_c mNodeMtx07;           // node 7 (foot, footmark side 0)
    /* 0x1B58 */ recept_c *mpRecept;
    /* 0x1B5C */ f32 mWarpDist;               // warp home when farther (0 disables)
    /* 0x1B60 */ f32 mShadowSize;
    /* 0x1B64 */ f32 _1B64;                   // shadow (fn_801C8518 f1)
    /* 0x1B68 */ f32 _1B68;                   // shadow (fn_801C8518 f2)
    /* 0x1B6C */ u32 mPushTimer;              // frames pushed by mObjc while moving (saturates)
    /* 0x1B70 */ int _1B70;                   // preCreate = 4
    /* 0x1B74 */ dDvd::loader_c mLoader;
    /* 0x1B88 */ dAudioObjNpc_c mAudioObj;
    /* 0x1BD8 */ mVec3_c mLookTargetPos;      // demoHook58 fallback: mPos + (0, 16, 0)
    /* 0x1BE4 */ footstep_c mFootstep;
    /* 0x1BF0 */ objc_c mObjc;
    /* 0x1C38 */ u8 mIsActive;                // preCreate 1; draw/push/ground/footsteps only when set; canInteract
    /* 0x1C39 */ u8 mCheckGround;             // ground snap / acch
    /* 0x1C3A */ u8 mDrawShadow;
    /* 0x1C3B */ bool mNetSync;               // takes part in the net daub/permit sync
    /* 0x1C3C */ u8 _1C3C;
    /* 0x1C3D */ u8 mAcceptMoveReq;           // follow the net (daub) move / look requests
    /* 0x1C3E */ u8 _1C3E[2];
    /* 0x1C40 */ int mPermitState;            // 1 force local owner, 2 force remote, 0 ask permit; 3 after postCreate
    /* 0x1C44 */ u8 mRecvDaubAsOwner;                    // receive the daub even as local owner
    /* 0x1C45 */ u8 _1C45[3];
}; // size 0x1C48

// ---- top-of-file data of d_a_npc.cpp used by other TUs (d_a_npc_nml, d_a_npc_sp); placeholder names ----
extern const dAcNpc_c::posTable33_s l_80466730[6];
extern const dAcNpc_c::posTable33_s l_80467090[3];
extern const dAcNpc_c::posTable33_s l_80467540[3];
extern const dAcNpc_c::posTable97_s l_804679F0[6];
extern const dAcNpc_c::posTable97_s l_80469550[3];
extern const dAcNpc_c::posTable97_s l_8046A300[3];
extern const int l_8074FE38; // 6: number of entries of l_80466730
extern const int l_8074FE3C; // 3
extern const int l_8074FE40; // 3
extern const int l_8074FE44; // 6
extern const int l_8074FE48; // 3
extern const int l_8074FE4C; // 3
extern const f32 l_8074FE50; // 32.0f (d_a_npc_sp)
extern const f32 l_8074FE54; // 64.0f (d_a_npc_sp)
extern const int l_8074FEE0; // 13
extern const int l_8074FEE4; // 13
extern const int l_8074FEE8; // 18
extern const int l_8074FEEC; // 7
// Globals initialized by __sinit (in this order; several are used by d_a_npc_sp, d_npc_talk_quest_q08,
// d_npc_talk_fmarket):
extern dAcNpc_c::moveParam_c l_moveParamStop; // 80564B6C (0, 0, 0)
extern mAng l_walkTurnSpeed;                  // 8074E150 0x170
extern dAcNpc_c::moveParam_c l_moveParamWalk; // 80564B84 (0.4, 0.04, 0.07)
extern mAng l_runTurnSpeed;                   // 8074E154 0x270
extern dAcNpc_c::moveParam_c l_moveParamRun;  // 80564B9C (0.6, 0.14, 0.2)
extern mAng l_turnSpeed;                      // 8074E158 0x300
extern mAng l_8074E15C;                       // 0x2DFF look yaw max
extern mAng l_8074E160;                       // 0x37FF look fov
extern mAng l_8074E164;                       // 0x17F look yaw step
extern mAng l_8074E168;                       // 0xFFF look pitch max
extern mAng l_8074E16C;                       // 0x7F look pitch step
extern mVec3_c l_80564BC0[2];                 // {(10, 0, 16), (-10, 0, 16)} getSidePos
extern mAng l_8074E170;                       // 0x4000 "in front" half angle
extern dAcNpc_c::viewArea_c l_80564BE8;       // ctor 80028330
extern mAng l_8074E174;                       // 0x4000

// Shared with the NPC RELs (d_a_npc_out etc.).
struct emotionData_s {
    /* 0x00 */ dAcNpc_c::manpuMgr_c::startData_s mStart; // mAnmId 0: the wait animation of the npc
    /* 0x08 */ dAcNpc_c::manpuMgr_c::startData_s mLoop;  // mAnmId 0x1BC: none (end)
    /* 0x10 */ u32 mMode;                                // m3d::playMode_e of mStart
}; // size 0x14
const emotionData_s *getEmotionData(u32 idx);       // 80020B2C
BOOL isQuickHandItem(const dItem::Item &item);       // 800220BC
