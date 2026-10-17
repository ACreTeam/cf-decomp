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
#include <game/game/d_hmn_body_mng.hpp>
#include <game/game/d_hmn_face_anm_mng.hpp>
#include <game/game/d_hmn_head_mng.hpp>
#include <game/game/d_hmn_palette_mng.hpp>
#include <game/game/d_m3d.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_objc.hpp>
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
namespace EGG {
class FrmHeap;
}

class dAcPlayer_c : public dDemoActor_c, public dMsg::Rcpt_c {
  public:
    using dActor_c::mAngle;
    using dActor_c::mPos;

    // The face (eyes / mouth textures). Size 0x1E0.
    class myFace_c : public m3d::faceEx_c {
      public:
        virtual ~myFace_c();

        u8 _faceEx[0x1E0 - sizeof(m3d::faceEx_c)];
    };

    // The calcWorld hook of the body model: curves the bones and stores the
    // node positions that dPlayerMgr hands out. Size 0x14C.
    class myMdlCallback_c : public m3d::mdlEx_c::callback_c {
      public:
        virtual ~myMdlCallback_c();

        /* 0x004 */ mVec3_c mSoundPos; // dPlayerMgr::getSoundPosition
        /* 0x010 */ mVec3_c _010;
        /* 0x01C */ mVec3_c mHandPos; // dPlayerMgr::getHandPosition
        /* 0x028 */ u8 _028[0x24];
        /* 0x04C */ mVec3_c mHeadPos; // dPlayerMgr::lookAt
        /* 0x058 */ u8 _058[0x90];
        /* 0x0E8 */ u8 mHandMtx[0x30]; // dPlayerMgr::getHandTransform
        /* 0x118 */ f32 _118;
        /* 0x11C */ f32 _11C;
        /* 0x120 */ u8 _120[0x2C];
    };

    f32 getMoveSpeed() const {
        return mMoveSpeed;
    }

    m3d::fanm_c *getAnm() {
        return &mAnm;
    }

    virtual ~dAcPlayer_c();

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
    /* 0x0304 */ m3d::mdl_c mMdl304;
    /* 0x0344 */ m3d::mdl_c mMdl344;
    /* 0x0384 */ m3d::mdl_c mMdl384;
    /* 0x03C4 */ m3d::anmChr_c mAnm; // the body animation (mAnmId)
    /* 0x03FC */ m3d::anmTexPat_c mTexPatAnm;
    /* 0x0428 */ myFace_c mFace;
    /* 0x0608 */ myMdlCallback_c mMdlCallback;
    /* 0x0754 */ dBGCF::acch_c mAcch;
    /* 0x079C */ dBGCF::acch_c mAcch2;
    /* 0x07E4 */ dObjc::ac_c mObjc;
    /* 0x0828 */ dObjc::acpos_c mObjcPos;
    /* 0x0878 */ dObjc::acpos_c mObjcPos2;
    /* 0x08C8 */ dLevelEffect_c mEffects[5];
    /* 0x0BAC */ dEffect_c mEffect;
    /* 0x0C28 */ dHmnHeadMng_c mHeadMng;
    /* 0x0C29 */ dHmnPaletteMng_c mPaletteMng;
    /* 0x0C2A */ dHmnBodyMng_c mBodyMng;
    /* 0x0C2C */ dHmnCloth_c *mpCloth;       // dPlayerMngr_c::getCloth
    /* 0x0C30 */ dHmnHeadItem_c *mpHeadItem; // dPlayerMngr_c::getHeadItem
    /* 0x0C34 */ dHmnToolBank_c *mpToolBank; // dPlayerMngr_c::getToolBank
    /* 0x0C38 */ dHmnFaceAnmMng_c mFaceAnmMng;
    /* 0x0C40 */ dAcNpc_c::manpuMgr_c mManpu;
    /* 0x2040 */ int _2040;
    /* 0x2044 */ int _2044;
    /* 0x2048 */ int _2048;
    /* 0x204C */ int _204C;
    /* 0x2050 */ s16 _2050;
    /* 0x2052 */ s16 _2052;
    /* 0x2054 */ s16 _2054;
    /* 0x2056 */ s16 _2056;
    /* 0x2058 */ s16 _2058;
    /* 0x205A */ s16 _205A;
    /* 0x205C */ mVec3_c _205C;
    /* 0x2068 */ mVec3_c mTargetPos; // moveTo / lookAt target
    /* 0x2074 */ mVec3_c mShowPos;   // showCatchAt
    /* 0x2080 */ mVec3_c _2080;
    /* 0x208C */ f32 _208C; // >= 1.0f: the tool is put away before a demo
    /* 0x2090 */ f32 _2090[0x17];
    /* 0x20EC */ mVec3_c
        mScriptedActionPos; // dPlayerMgr::getScriptedActionPosition
    /* 0x20F8 */ u8 _20F8[0x30];
    /* 0x2128 */ mMtx_c _2128;
    /* 0x2158 */ u8 _2158[0x30];
    /* 0x2188 */ u16 _2188; // ITEM_ID_NONE at construction
    /* 0x218A */ u16
        mActionItem; // the shown / held action item (getActiveActionItem)
    /* 0x218C */ u16 _218C;
    /* 0x2190 */ int mTargetUnitX; // getActionUnitCoordinates, putItemInHole
    /* 0x2194 */ int mTargetUnitZ;
    /* 0x2198 */ EGG::FrmHeap *mpHeap; // getGeneralHeapSize
    /* 0x219C */ EGG::FrmHeap
        *mpMiiHeap; // getMiiHeapSize ("dAcPlayer_c::m_heap_Mii_p")
    /* 0x21A0 */ EGG::FrmHeap *mpHeadHeap;      // getHeadHeapSize
    /* 0x21A4 */ EGG::FrmHeap *mpAccessoryHeap; // getAccessoryHeapSize
    /* 0x21A8 */ int _21A8;
    /* 0x21AC */ int _21AC;
    /* 0x21B0 */ int _21B0;
    /* 0x21B4 */ int _21B4;
    /* 0x21B8 */ f32 _21B8;
    /* 0x21BC */ f32 mTargetSpeed; // moveTo speed
    /* 0x21C0 */ f32 _21C0[0x12];
    /* 0x2208 */ int _2208;
    /* 0x220C */ int _220C;
    /* 0x2210 */ int _2210;
    /* 0x2214 */ int _2214;
    /* 0x2218 */ int _2218;
    /* 0x221C */ int _221C;
    /* 0x2220 */ int mAnmId;
    /* 0x2224 */ int _2224;
    /* 0x2228 */ int _2228;
    /* 0x222C */ int mState;       // dPlayerMgr::State
    /* 0x2230 */ int mRemoteSyncA; // setRemoteActionSync
    /* 0x2234 */ int mNextState;   // the state after the current action
    /* 0x2238 */ int _2238;        // 1 with mNextState
    /* 0x223C */ int _223C;
    /* 0x2240 */ int mPlayerNo; // the player / network member index
    /* 0x2244 */ int mDoorType; // enterDoor / exitDoor
    /* 0x2248 */ int mSeatPose; // getUpFromBed / changeSeatedPose
    /* 0x224C */ int mSeatSide; // SIT_FROM_* / LIE_FROM_*
    /* 0x2250 */ int _2250;
    /* 0x2254 */ int mChestType; // openChest / closeChest
    /* 0x2258 */ int _2258;
    /* 0x225C */ int _225C;
    /* 0x2260 */ u16 _2260;
    /* 0x2262 */ u16 _2262;
    /* 0x2264 */ u16 _2264;
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
    /* 0x2294 */ int _2294;
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
    /* 0x22DA */ u8 mHeldItemChange; // dPlayerMgr::HELD_ITEM_CHANGE_*
    /* 0x22DB */ u8 _22DB[2];
    /* 0x22DD */ u8 mReactionKind; // dPlayerMgr::REACTION_* (STATE_REACT)
    /* 0x22DE */ u8 mEmotion;      // playEmotion
    /* 0x22DF */ u8 _22DF[5];
    /* 0x22E4 */ u8 mEquipChangeMode; // changeEquipment mode
    /* 0x22E5 */ u8 _22E5[7];
    /* 0x22EC */ f32 mMoveSpeed;
    /* 0x22F0 */ f32 _22F0;
    /* 0x22F4 */ f32 _22F4;
    /* 0x22F8 */ s16 mMoveAngle; // getMoveAngle
    /* 0x22FA */ u8 _22FA[0xE];
    /* 0x2308 */ dPlayerMngr_c::actionParam_c mRemoteActionParams;
}; // size 0x2314
