#pragma once

// The fish swimming in the town: their shadows in the rivers, ponds and sea. REL d_fish_fieldNP
// (profile 0xB2), one TU d_fish_fieldNP/d_fish_field.cpp (REL .text 0x5C..0x7E3C). Class names are
// the game's (RTTI); everything else is inferred. Addresses are REL .text offsets. See
// notes/d_fish_field.txt and notes/d_fish_field_symbols.txt.
//
// dFishField_c (fBase_c)          the manager: 8 town fish + 4 fish held by players, each a
//                                 dFishFldShadow_c placed in its own child heap
//   dFishFldShadow_c              one fish shadow: model + anim, state machine (sProcs)
//     dFishShadowMdl_c            the shadow model (m3d::smdl_c) with a world callback
// dSearchFishPos                  units of one acre a fish can spawn on
// dBkAttrSearchCand_c             the field's acres with a block flag
//
// In net play the 8 town fish are shared through 8 sync records (mRecs, d_play_util's
// "dPlaySyncHost_c": lbl_8074E840 is the dFishField_c instance).

#include <types.h>
#include <nw4r/g3d.h>
#include <game/mLib/m_3d/smdl.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/framework/f_base.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_fd_bk_search_cand.hpp> // before d_search_cand.hpp: sets the weak data order
#include <game/game/d_search_cand.hpp>
#include <game/game/d_play_util.hpp>
#include <game/game/d_fish_info.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_weather.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_snd_obj.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/mLib/m_vec.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_allocator.hpp>
#include <lib/egg/core/eggFrmHeap.h>

class dFishFldShadow_c;
struct dEffectTarget_c;

// Per fish type (sFishParam, data 0x8, [77]; copied to dFishFldShadow_c::mParam).
struct dFishParam_c {
    /* 0x0 */ u8 mSize; // sSizeParams index (rodata 0x90)
    /* 0x1 */ u8 _1;    // searchFloat: sTurnParam0/1 index
    /* 0x2 */ u8 _2;    // initBite: sTimeParam0/1 index
}; // size 0x3

// Per size class (sSizeParams, rodata 0x90, [8]; by dFishParam_c::mSize).
struct dFishSizeParam_c {
    /* 0x00 */ f32 mScale[3];    // the shadow model's scale
    /* 0x0C */ f32 _0C;
    /* 0x10 */ s16 mHookTimeMin; // initHooked: frames before the catch (cM::rndRange<s16>)
    /* 0x12 */ s16 mHookTimeMax;
}; // size 0x14

// A player's fishing float (DOL, not split yet; fn_801710A4(player, fish) returns it while it is
// free for that fish, fn_801710BC(player, fish) reserves it). Only what this TU uses.
struct dFishingFloat_c {
    /* 0x000 */ u8 _000[0x270];
    /* 0x270 */ mVec3_c mPos;
    /* 0x27C */ u8 _27C[0x446 - 0x27C];
    /* 0x446 */ u8 mCastState; // 1, 2: in the water (2: cast long, a longer bite window)
    /* 0x447 */ u8 _447;       // 9: the fish escapes instead of biting (executeNibble)
};

// What the players hold up (DOL manager lbl_8074E9E8, not split yet, around 80190970):
// fn_80190970 requests player idx's held item model, fn_801909D4 releases it. Only what this TU uses.
struct dHoldItemMgr_c {
    struct Req_c {
        /* 0x00 */ u8 _00[0x64];   // a dDvd::bank_c (ctor 80190A0C)
        /* 0x64 */ int mState;     // 3: loaded
        /* 0x68 */ int _68;
        /* 0x6C */ u8 mReady;
    }; // size 0x70
    struct Mdl_c {
        /* 0x00 */ u8 _00[0x8];
        /* 0x08 */ m3d::bmdl_c mMdl;
        /* 0x14 */ u8 _14[0x94 - 0x14];
    }; // size 0x94

    // The model of request idx once it is loaded, NULL before.
    Mdl_c *getMdl(int idx) {
        Req_c *req = &mReqs[idx];
        if (req->mState == 3 && req->mReady) {
            return &mMdls[idx];
        }
        return NULL;
    }

    /* 0x000 */ u8 _000[0x64];
    /* 0x064 */ Req_c mReqs[4];
    /* 0x224 */ u8 _224[0x10];
    /* 0x234 */ Mdl_c mMdls[4];
};
extern dHoldItemMgr_c *lbl_8074E9E8;

#define FISH_FIELD_FISH_NUM 8   // town fish (mFish), one sync record each
#define FISH_FIELD_PLAYER_NUM 4 // fish held by the players (mPlayerFish)

// Fish kinds with a fin above the water (ocean sunfish, hammerhead, shark): the fsh_shadow01 model.
BOOL isFinFish(int type); // 0x250C

// Spawn place checks of an acre, by dFishSpawn_c::Place_e (sPlaceChecks).
typedef dFdBkSearchCandCb_c::Func dFishPlaceCheck; // user: the dFishSpawn_c
BOOL isRiverBlock(int blockX, int blockZ, void *user);      // 0x134: BLOCK_KIND_FLAG_RIVER
BOOL isPoolBlock(int blockX, int blockZ, void *user);       // 0x188: BLOCK_KIND_FLAG_RACCO
BOOL isWaterfallBlock(int blockX, int blockZ, void *user);  // 0x1D8: BLOCK_KIND_FLAG_FALL
BOOL isPondBlock(int blockX, int blockZ, void *user);       // 0x228: dFdBlock_c::fn_80080FC0
BOOL isRiverMouthBlock(int blockX, int blockZ, void *user); // 0x274: river S0/S1/S2 on the beach
BOOL isOffingBlock(int blockX, int blockZ, void *user);     // 0x2F4: beach, in rainy/snowy weather only
BOOL isSeaBlock(int blockX, int blockZ, void *user);        // 0x39C: BLOCK_KIND_FLAG_BEACH
dFishPlaceCheck getPlaceCheck(int place);       // 0x3EC

// dBkUnitSearchCandCb_c filter: units of the waterfall basin (unit attribute 0x76..0x78).
BOOL isWaterfallUnit(int blockX, int blockZ, int unitX, int unitZ, void *user); // 0x400
// All 5 units around pos are open water (attribute 2).
BOOL isOpenWater(const mVec3_c *pos); // 0x450

dItem::Item getFishItem(int type);             // 0x8C: fish, trash or key item of a fish type
int getLostItemLikeIdx();                      // 0xF0: local player's lost item villager, -1 if none


// A sub-object of dFishFldShadow_c (at 0xB8), only set up for frogs (mKind == FISH_FROG): croaks
// while a player is near and not running.
struct dFishFrog_c {
    enum State_e {
        STATE_AWAY,  // no player within 128
        STATE_NEAR,  // a player within 128
        STATE_CROAK, // croaking
    };

    dFishFrog_c() : mCroakTimer(0), mCroakWait(0), mAlarm(0), mState(0) {}

    void init();                            // 0x69EC
    void execute(dFishFldShadow_c *fish);   // 0x6A04: by mState
    void execAway(dFishFldShadow_c *fish);  // 0x6A3C
    void execNear(dFishFldShadow_c *fish);  // 0x6B14
    void execCroak(dFishFldShadow_c *fish); // 0x6C44

    /* 0x0 */ u16 mCroakTimer; // croak frames left
    /* 0x2 */ u16 mCroakWait;  // frames before the next croak
    /* 0x4 */ u8 mAlarm;       // the running player raises it (max 200); croaks when <= 150
    /* 0x5 */ u8 mState;       // State_e
}; // size 0x6 (padded to 0x8)

// The fish's surface ripples (dFishFldShadow_c::mRipple): bursts of af_fsh_hamon.
struct dFishRipple_c {
    dFishRipple_c(dFishFldShadow_c *owner) : mOwner(owner), mFirst(0) {}

    void init();    // 0x6D10
    void execute(); // 0x6D5C

    /* 0x0 */ dFishFldShadow_c *mOwner;
    /* 0x4 */ s16 mCount; // ripples left in the burst
    /* 0x6 */ s16 mTimer; // frames to the next ripple
    /* 0x8 */ u8 mFirst;  // first burst: shorter intervals
}; // size 0xC

// The shadow model. The callback turns the root by mAngle (ExecCallbackB).
class dFishShadowMdl_c : public m3d::smdl_c {
public:
    class mdlCallback_c : public nw4r::g3d::ICalcWorldCallback {
    public:
        mdlCallback_c(dFishShadowMdl_c *owner); // 0x71A0
        virtual ~mdlCallback_c();               // 0x71B4
        virtual void ExecCallbackB(nw4r::g3d::WorldMtxManip *manip, nw4r::g3d::ResMdl mdl,
                                   nw4r::g3d::FuncObjCalcWorld *funcObj); // 0x71F4

        /* 0x4 */ dFishShadowMdl_c *mOwner;
    }; // size 0x8

    dFishShadowMdl_c();               // 0x6F8C
    virtual ~dFishShadowMdl_c();      // 0x6FF8

    BOOL create(nw4r::g3d::ResMdl mdl, mAllocator_c *allocator, ulong bufferOption, int viewCount,
                size_t *objSize);     // 0x7060
    void calcRoot(nw4r::g3d::WorldMtxManip *manip, u16 nodeID); // 0x7128: turns the root by mAngle

    /* 0x0C */ mdlCallback_c mCallback;
    /* 0x14 */ int _14;
    /* 0x18 */ int _18;
    /* 0x1C */ mAng mAngle;
    /* 0x20 */ mVec3_c _20;
}; // size 0x2C

// A node of dFishFldShadow_c::mLinks (someone that refers to the fish; unlinked on deletion).
struct dFishLink_c {
    /* 0x0 */ dFishFldShadow_c *mFish;
    /* 0x4 */ dFishLink_c *mNext;
};

// One fish shadow. Allocated in its slot's child heap (operator new, 0x7200) and set up by
// create(ResFile). States (mState, sProcs[11]: init / execute / end).
class dFishFldShadow_c {
public:
    typedef void (dFishFldShadow_c::*ProcFunc)();
    struct Proc {
        ProcFunc mInit;
        ProcFunc mExecute;
        ProcFunc mEnd;
    };

    enum State_e {
        STATE_SWIM = 0,      // swims around its home
        STATE_APPROACH = 1,  // goes for mPlayer's float
        STATE_NIBBLE = 2,    // nibbles at the float
        STATE_BITE = 3,      // bitten: the player can pull
        STATE_ESCAPE = 4,    // swims away and fades out, then mDead
        STATE_HOOKED = 5,    // hooked: follows the float
        STATE_CATCH = 6,     // lifted out of the water to the player
        STATE_RELEASE = 7,   // thrown back into the water, swims away
        STATE_HOLD = 8,      // a player fish held
        STATE_SHOW_HOLD = 9, // a player fish shown (held up) by its player
        STATE_SHOW = 10,
        STATE_NONE = 11,
    };

    static void *operator new(size_t size, EGG::Heap *heap); // 0x7200: zeroed

    dFishFldShadow_c(const dFishParam_c *param);              // 0x2624: param = sFishParam[type]
    virtual ~dFishFldShadow_c();                              // 0x2788
    virtual BOOL create(const nw4r::g3d::ResFile &res);       // 0x283C
    virtual int execute();                                    // 0x641C
    virtual int draw();                                       // 0x67A8
    virtual BOOL isDeleteOk();                                // 0x68CC

    void calcMtx();                   // 0x2970
    BOOL isMine();                    // 0x2BB4: player fish, or dFishField_c::isRecMine(mSlot)
    int searchFloat();                // 0x2BDC: the player whose fishing float it goes for, -1
    BOOL checkScared();               // 0x2EC0: scareFish, or a running player within 80
    BOOL isOpenWater();               // 0x2F98: ::isOpenWater(&mPos)
    dPlaySyncRecBuf_c *getRec() const; // 0x2FA0: its net record (NULL for a player fish)
    void calcRemote();                // 0x2FD0: follow the record's position / direction
    void updateRec();                 // 0x31A4: write the position / direction to the record

    // States (sProcs, State_e; text order 0-6, 9, 10, 8, 7). setState(STATE_SWIM) on create;
    // addPlayerFish: STATE_HOLD or STATE_SHOW_HOLD.
    void initSwim();                  // 0x3314: also picks a new course while swimming
    void executeSwim();               // 0x38D4
    void initApproach();              // 0x3C28
    void executeApproach();           // 0x3D00
    void endApproach();               // 0x4130
    void initNibble();                // 0x41B0: tests the PTMF isMine (data 0x398 / 0x3A4)
    void executeNibble();             // 0x42A0
    void initBite();                  // 0x4858: window sTimeParam0/1 by mParam._2
    void executeBite();               // 0x4934
    void initEscape();                // 0x49E8: a fin fish keeps its fin up
    void executeEscape();             // 0x4AA0
    void initHooked();                // 0x4CA4: sSizeParams hook time, then catch
    void executeHooked();             // 0x4D70
    void initCatch();                 // 0x4E64: 45 frames from the float to the player
    void executeCatch();              // 0x4F4C
    void initShowHold();              // 0x50DC
    void executeShowHold();           // 0x513C
    void initShow();                  // 0x51A0
    void executeShow();               // 0x51BC
    void initHold();                  // 0x5228
    void executeHold();               // 0x5280
    void initRelease();               // 0x5318
    void executeRelease();            // 0x547C

    BOOL isFinFish() const;           // 0x5998: ::isFinFish(mKind)
    int getCatchKind() const;         // 0x59A0: mKind, all keys as the first key kind
    BOOL isSeaSlot() const;           // 0x59D0: mSlot outside the river slots 0-3
    dItem::Item getItem() const;      // 0x59F4: getFishItem(mKind)
    BOOL pull();                      // 0x59FC: the player pulls: TRUE if hooked, else escape
    BOOL isCatching() const;          // 0x5B8C: STATE_CATCH
    BOOL isCaught() const;            // 0x5BA0: STATE_CATCH done
    void throwBack(const mVec3_c *pos); // 0x5BC8: thrown back into the water at pos (STATE_RELEASE)
    void requestDelete();             // 0x5C44: let go (float, held model), then delete
    BOOL getMouthPos(mVec3_c *pos) const;   // 0x5CE0: FALSE while held
    mAng getTargetAngleY() const { return mTargetAngle.y; }
    int getKind() const { return mKind; }
    u8 getRecId() const { return mRecId; }
    void setHeldPos(const mVec3_c *pos);     // 0x5E28
    void setHeldScale(const mVec3_c *scale); // 0x5E50
    void calcBgMove();                // 0x5EC4: wall check of the move from mTargetPos to mPos
    static BOOL isInFallBasin(const mVec3_c *pos); // 0x5FE8: called by dSearchFishPos::check
    BOOL pushOutOfFall();             // 0x6084: out of the waterfall basin's back
    m3d::bmdl_c *getMdl();            // 0x61B8: the model to draw (the held model of a held fish)
    void setState(int state);         // 0x622C
    void recvRec();                   // 0x62D8
    void addLink(dFishLink_c *link);    // 0x698C
    void removeLink(dFishLink_c *link); // 0x69B8
    static void splashCallback(dEffectTarget_c *target, u32 kind); // 0x7260: watersplash / hamon
    void setSplashEffect();           // 0x73BC: af_fsh_watercolumn_* / af_hny_mizutama_*
    void setEscapeEffect();           // 0x74C0: af_fsh_escape_hamon

    static Proc sProcs[11];           // data 0x1EC

    /* 0x000 vtable */
    /* 0x004 */ dFishShadowMdl_c mMdl;
    /* 0x030 */ m3d::anmChr_c mAnm;
    /* 0x068 */ dFishParam_c mParam;
    /* 0x06C */ mAllocator_c mAllocator;
    /* 0x088 */ dSndObjSimple_c mSound; // mSound.setPos(&mPos) every frame
    /* 0x0B8 */ dFishFrog_c mFrog;     // set up when mKind == FISH_FROG (0xB)
    /* 0x0C0 */ dBGCF::acch_c mWallCheck;
    /* 0x108 */ dLevelEffect_c mEffect; // af_fsh_shadow / af_fsh_sebire_moya (execute)
    /* 0x19C */ int mState;           // State_e
    /* 0x1A0 */ mVec3_c mHomePos;
    /* 0x1AC */ mVec3_c mPos;
    /* 0x1B8 */ mVec3_c mStartPos;    // release: where the throw starts
    /* 0x1C4 */ mVec3_c mDestPos;     // release: where it lands; net: the record's position
    /* 0x1D0 */ mVec3_c mTargetPos;
    /* 0x1DC */ mVec3_c _1DC;         // the move of this frame
    /* 0x1E8 */ mVec3_c _1E8;
    /* 0x1F4 */ mVec3_c mScale;
    /* 0x200 */ mAng3_c mAngle;
    /* 0x206 */ mAng3_c mTargetAngle;
    /* 0x20C */ void **_20C;          // cleared (*_20C = NULL) on deletion
    /* 0x210 */ f32 mSpeed;
    /* 0x214 */ f32 mAnmRate;         // the swim animation's rate
    /* 0x218 */ int mNibbleNum;       // nibbles so far (executeNibble)
    /* 0x21C */ int mPlayer;          // the player whose float it is on / holding it, -1
    /* 0x220 */ int mHoldPlayer;      // the player of the held model request (lbl_8074E9E8), -1
    /* 0x224 */ s16 mTimer;           // state timer
    /* 0x226 */ s16 mTurnTimer;       // swim: turn again in open water; searchFloat: wait
    /* 0x228 */ dFishRipple_c mRipple;
    /* 0x234 */ s16 mSlowTimer;       // swim: slow down when 0
    /* 0x236 */ mAng mCourse;         // the angle mTargetAngle.y turns to
    /* 0x238 */ u8 mAlpha;            // escape: fading out
    /* 0x239 */ u8 _239;
    /* 0x23A */ u8 mNibbling;         // approach / nibble: going for the float (record dir)
    /* 0x23B */ u8 mHeld;             // held by a player
    /* 0x23C */ u8 mScared;           // dFishField_c::scareFish
    /* 0x23D */ u8 _23D[0x240 - 0x23D];
    /* 0x240 */ int mSlot;            // index in dFishField_c::mFish / mPlayerFish
    /* 0x244 */ int mIsPlayerFish;
    /* 0x248 */ int mKind;            // dFishType_e
    /* 0x24C */ int mLifeTimer;       // 3600: frames out of the players' view before deletion
    /* 0x250 */ dFishLink_c *mLinks;
    /* 0x254 */ u8 mRecId;            // 7-bit id of the fish in its record
    /* 0x255 */ u8 mHideHeld;         // the held model is not shown yet (hold / show hold)
    /* 0x256 */ u8 mDead;
}; // size 0x258

class dFishField_c : public fBase_c {
public:
    dFishField_c();                   // 0x564
    virtual ~dFishField_c();          // 0x610

    virtual int create();             // 0x6B4
    virtual int preCreate() { return SUCCEEDED; } // 0x7E34 (weak)
    virtual int doDelete();           // 0xCFC
    virtual int execute();            // 0xAEC
    virtual void postExecute(MAIN_STATE_e state) {} // 0x7E30 (weak)
    virtual int draw();               // 0xC04
    virtual void postDraw(MAIN_STATE_e state) {}    // 0x7E2C (weak)
    virtual void deleteReady() {}     // 0x7E28 (weak)
    virtual bool createHeap() { return true; }      // 0x7E20 (weak)
    // Scares the town fish within radius (default 80) of pos; TRUE if any. Called by the player.
    virtual BOOL scareFish(f32 radius, const mVec3_c *pos); // 0x2530

    BOOL canSpawnKey() const;                 // 0xE0C: a villager lost an item and no key is out
    void calcSpawn();                         // 0x10D0: every 300 frames, refill the empty slots
    void respawnFromRecs();                   // 0x11EC: net: the slots the records hold a fish for
    void checkFish();                         // 0x1274: delete the fish that left the players' view
    BOOL deleteFish(int idx);                 // 0x13A8
    BOOL deletePlayerFish(int idx);           // 0x14C8
    void deleteAll();                         // 0x1568
    BOOL isRecChanged(int idx);               // 0x1600: the record no longer matches the fish
    BOOL addFish(int type, int water, const mVec3_c *pos, const mAng3_c *angle); // 0x16BC
    BOOL addFishFromRec(int type, int idx);   // 0x1A50
    dFishFldShadow_c *addPlayerFish(int type, int mode, const mVec3_c *pos, int player); // 0x1C70
    void initSpawn();                         // 0x1DF4
    void spawn(int riverNum, int seaNum, bool awayFromPlayers); // 0x1F0C
    BOOL isRecMine(int idx);                  // 0x7518: this machine controls record idx
    void clearRecs();                         // 0x75C4
    void sendRecs();                          // 0x7698
    void recvRecs();                          // 0x7788
    void initRecs();                          // 0x7848

    static BOOL isInView(const mVec3_c *center, const mVec3_c *pos); // 0x2304
    static BOOL isAwayFromPlayers(int blockX, int blockZ, int unitX, int unitZ); // 0x23AC

    /* 0x000 fBase_c (vtable at 0x060) */
    /* 0x064 */ dDvd::brresBank_c mShadowRes;            // /Fish/fsh_shadow.brres
    /* 0x0BC */ dDvd::brresBank_c mRes2;                 // unused
    /* 0x114 */ EGG::FrmHeap *m_ResHeap;
    /* 0x118 */ EGG::FrmHeap *m_childHeap[FISH_FIELD_FISH_NUM];
    /* 0x138 */ dFishFldShadow_c *mFish[FISH_FIELD_FISH_NUM];
    /* 0x158 */ EGG::FrmHeap *m_childHeapForPlayer[FISH_FIELD_PLAYER_NUM];
    /* 0x168 */ dFishFldShadow_c *mPlayerFish[FISH_FIELD_PLAYER_NUM];
    /* 0x178 */ dPlaySyncRecBuf_c mRecs[FISH_FIELD_FISH_NUM];
    /* 0x1F8 */ int mSpawnTimer;
    /* 0x1FC */ mVec3_c mFallPos;                        // the waterfall basin's center
    /* 0x208 */ mVec3_c mFallNormal;
    /* 0x214 */ u8 mKeyOut;                              // a key fish is in the water
}; // size 0x218

// The units of acre (mBlockX, mBlockZ) a fish of mWater / mPlace can spawn on.
class dSearchFishPos : public dSearchCandXZ_c<16, 16> {
public:
    virtual BOOL check(int x, int z); // 0xE58

    /* 0x3C */ int mBlockX;
    /* 0x40 */ int mBlockZ;
    /* 0x44 */ u8 mAwayFromPlayers; // also require the unit outside every player's view (0x23AC)
    /* 0x48 */ int mWater;          // dFishWater_e
    /* 0x4C */ int mPlace;          // dFishSpawn_c::Place_e
}; // size 0x50

// The field's acres with a block flag (dFdBlock_c::hasFlag(mMask)).
class dBkAttrSearchCand_c : public dFdBkSearchCand_c {
public:
    virtual BOOL check(int x, int z); // 0x1074

    /* 0x24 */ int mMask;
}; // size 0x28



// The manager instance is d_play_util's lbl_8074E840.
