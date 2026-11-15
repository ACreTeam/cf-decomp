#pragma once

// The insects in the town, part 1: the shared helpers (namespace dInsectField),
// dInsFldBank_c, dInsFldBase_c and dInsFldShow_c. The manager dInsectField_c is
// in d_insect_field.hpp (its weak virtuals are the last .text of the REL, after
// every other weak function: a header of its own). The rest of this comment
// describes the whole REL.
//
// REL d_insect_fieldNP (profile INSECT_FIELD 0xB3),
// one TU d_insect_fieldNP/d_insect_field.cpp (REL .text 0xCC..0x270B8). Class
// names are the game's (RTTI, global scope); everything else is inferred.
// Addresses are REL .text offsets.
//
// dInsectField_c (fBase_c, 0x7D0)    the manager; its instance is the DOL's
// g_playActorMng (the DOL
//                                    sees it as dInsectField_c,
//                                    d_play_util.hpp)
//   dInsFldBank_c [15]               the insect .brres files
//   (/Insect/<model>.brres) dInsFldBase_c                    one insect: model
//   + anims, bg check, scare, hold, net slot
//     dInsFldShow_c                  an insect a player shows (only the held
//     model) dInsFldButterfly_c ...         one class per insect kind
//     (d_ins_fld_*.hpp, .inc per class)
//
// Class (and .text) order: dInsFldBank_c, dInsFldBase_c, dInsFldShow_c,
// dInsectField_c, then the kinds in .text order (Butterfly, Beetle,
// Grasshopper, Flower, Cicada, Dragonfly, Moth, Bee, Hornet, Ant, Strider,
// DivingBeetle, MoleCricket, TumbleBug, Firefly, Flea, PillBug, Mosquito,
// Spider, Tarantula, Stump, Leaf): the strong vtables in .data are in exactly
// the reverse order, so the kind headers must be included in that order after
// this one.

#include <types.h>
#include <nw4r/g3d.h>
#include <game/mLib/m_3d.hpp>
#include <game/framework/f_base.hpp> // after m_3d.hpp (fBase_c RTTI order)
#include <game/framework/f_profile.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_fd_bk_search_cand.hpp>
#include <game/game/d_bk_unit_search_cand.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_snd_obj.hpp>
#include <game/game/d_objc.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_insect_info.hpp>
#include <game/game/d_insect_util.hpp>
#include <game/game/d_play_util.hpp>
#include <game/game/d_fi_mdl_mgr.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_vec.hpp>
#include <game/snd/snd_id.hpp>
#include <lib/egg/core/eggHeap.h>
#include <lib/egg/core/eggFrmHeap.h>

class dSearchCandXZCore_c;
class dInsFldBase_c;
class dInsectField_c;
struct dInsectLink_c;
namespace nw4r {
namespace ef {
class Particle;
}
} // namespace nw4r

// ---------------------------------------------------------------------------------------------
// The player (d_a_playerNP) and d_fgobj_managerNP call the manager through
// g_playActorMng (dInsectField_c::getInstance) and the insects through
// dInsFldBase_c (addHoneycombBees, addFlea, getFlea, addPlayerInsect,
// cancelHold, setHeldPos, setHeldScale, initHold, tryCatch, release,
// dInsectField::getObjcRadiusAdd).

namespace dInsectField {
void addNpcLink(void *insect,
                void *link); // .text 6584: appends link to insect's mLinks
void removeNpcLink(void *insect, void *link); // .text 65B0
f32 getObjcRadiusAdd(); // .text 5B1C: 4.0 (the player's net reach is shortened
                        // by it)
} // namespace dInsectField

// A node of an insect's link list (someone who refers to the insect; unlinked
// on deletion). Same layout as dANpcOut::insectLink_s (d_a_npc_out.hpp).
struct dInsectLink_c {
    ~dInsectLink_c() {
        if (mpInsect) {
            dInsectField::removeNpcLink(mpInsect, this);
        }
    }
    // Moves the link to insect (none: just unlinks).
    void set(void *insect) {
        if (mpInsect) {
            dInsectField::removeNpcLink(mpInsect, this);
        }
        if (insect) {
            dInsectField::addNpcLink(insect, this);
        }
    }
    /* 0x0 */ void *mpInsect;
    /* 0x4 */ dInsectLink_c *mpNext;
}; // size 0x8

// ---------------------------------------------------------------------------------------------

#define INSECT_FIELD_BANK_NUM 15   // dInsectField_c::mBanks
#define INSECT_FIELD_SEARCH_NUM 17 // dInsectField_c::mSearch
// dInsectField_c::m_objHeapPermanent (honeycomb bees, ants, the flea): its
// insects use the banks after the INSECT_NET_SLOT_NUM net slot ones.
#define INSECT_FIELD_PERMANENT_NUM 3
#define INSECT_FIELD_PLAYER_NONE                                               \
    4 // dInsFldBase_c::mPlayer: not held by a player
// Added to the angle towards a player or NPC to flee from it: one degree short
// of turning right around (0x7F4A).
#define INSECT_FIELD_FLEE_ANGLE (DEG_TO_ANGLE(180) - ANGLE_PER_DEGREE)

namespace dInsectField {

// The per-type insect data (.data 0x1C7C, 0x1C each; sParamTable at .data
// 0x2380 points to the one of each dInsectType_e). getParam() returns it.
struct param_c {
    /* 0x00 */ dInsFldBase_c *(*mCreate)(
        EGG::Heap *heap);         // the factory (.text 26374..27040)
    /* 0x04 */ const char *mName; // "ins_monsiro", ...
    /* 0x08 */ int mScareLimit;   // mScare.setLimit
    /* 0x0C */ int mEscapeLimit;  // mScare.setEscapeLimit
    /* 0x10 */ f32 mScareRadius;  // -> dInsFldBase_c::mScareRadius
    /* 0x14 */ f32 mRadius;       // -> mBaseRadius, mBgRadius
    /* 0x18 */ f32 mShadowSize;   // -> mShadowSize
}; // size 0x1C

extern const param_c *sParamTable[INSECT_NUM]; // .data 0x2380

// A fixed set of child heaps for insects (dInsectField_c::m_objHeap /
// m_objHeapPermanent / m_objHeapPlayer); an insect takes a free one (mUsed) and
// keeps its index in mHeapIdx.
struct heapList_c {
    void create(ulong size, EGG::Heap *parent, int num,
                const char *name); // 0x108
    void destroy();                // 0x1C0

    int getFreeNum() const {
        int num = 0;
        for (u32 i = 0; i < mNum; i++) {
            if (!mUsed[i]) {
                num++;
            }
        }
        return num;
    }
    // The first free heap.
    bool getFree(int *idx) const {
        for (u32 i = 0; i < mNum; i++) {
            if (!mUsed[i]) {
                *idx = i;
                return true;
            }
        }
        return false;
    }

    /* 0x0 */ u32 mNum;
    /* 0x4 */ EGG::FrmHeap **mHeaps;
    /* 0x8 */ u8 *mUsed;
}; // size 0xC

// Shared helpers (manager region, called by the insect kinds).
// No player's view (units -10..10 x, -16..6 z around each player, or the
// camera) covers the unit.
BOOL isAwayFromPlayers(int blockX, int blockZ, int unitX, int unitZ); // 0x504C
// The offset an insect sits at on an item (flowers, trees, stumps, rocks, ...).
void getItemOffset(mVec3_c *out, const dItem::Item *item); // 0x55B4
// No other insect (active or waiting for its model) is on the unit.
BOOL isUnitFree(int blockX, int blockZ, int unitX, int unitZ); // 0x59F0

// .bss 0x40: the flea (set by dInsFldFlea_c::create, cleared by its doDelete;
// getFlea / addFlea). Where it is defined (and so its exact scope) is settled
// in phase 2.
extern dInsFldBase_c *sFlea;

// The camera / player view tests.
BOOL isInView(const mVec3_c *center, const mVec3_c *pos); // 0x4F58
BOOL isOutOfView(const mVec3_c *pos);                     // 0x5004

// Spawn places: each insect type has one (getPlace); sPlaceFinders[place]
// looks for a spawn position of the type.
// The place also indexes dInsectField_c::mSearch (acres left to try).
enum Place_e {
    PLACE_GROUND,        // 0x1DCC
    PLACE_FLOWER,        // 0x20F0
    PLACE_MANTIS_FLOWER, // 0x23F0
    PLACE_GRASS,         // 0x26B4
    PLACE_RIVER,         // 0x299C
    PLACE_WATER,         // 0x2C24
    PLACE_ANT,           // 0x2EB4
    PLACE_FLY,           // 0x31C4
    PLACE_SOIL,          // 0x34A8
    PLACE_ROCK,          // 0x3840
    PLACE_HOUSE_LIGHT,   // 0x3AD4
    PLACE_STUMP,         // 0x4B70
    PLACE_SNOWMAN,       // 0x4E10
    PLACE_TREE,          // 0x3FFC
    PLACE_PALM,          // 0x42FC
    PLACE_TREE_TRUNK,    // 0x45CC
    PLACE_GROWN_TREE,    // 0x48C8
    PLACE_NUM,
};
typedef BOOL (*PlaceFinder)(int type, mVec3_c *pos, BOOL awayFromPlayers);
int getPlace(int type);                                        // 0x4EA8
extern PlaceFinder sPlaceFinders[PLACE_NUM];                   // .data 0x1BD8
BOOL findAntPos(int type, mVec3_c *pos, BOOL awayFromPlayers); // 0x2FF4
BOOL findFlyPos(int type, mVec3_c *pos, BOOL awayFromPlayers); // 0x32D8
// 0x3AD4: next to a lit villager house window at night (the moth); sets
// sLightHouse / sLightHouseFound (daSTR_c::isWindowLit), or next to the
// town hall (BUILDING_TOWN_HALL).
BOOL checkHouseLight(int blockX, int blockZ, int unitX, int unitZ, void *user);
// .bss 0x10 / 0xC: the villager house checkHouseLight found (dInsFldMoth_c).
extern int sLightHouse;
extern int sLightHouseFound;

// dInsectCatchData_c::getReq / setReq: the owner's state of the insect (the
// kinds' State_e), or this: held in a player's net (dInsFldBase_c::tryCatch).
#define INSECT_FIELD_REQ_HELD 0xF

// The NET_PACKET_INSECT_FIELD message (4 bytes, bits packed from the low bit
// of byte 0 up): bits 0..1 type, 2..5 net slot, 6..12 insect type, 13..15
// member, 16..24 serial.
struct catchMsg_c {
    enum Type_e {
        TYPE_REQUEST = 0, // to the scene owner: a player's net reaches it
        TYPE_CAUGHT = 1,  // the owner's answers
        TYPE_MISSED = 2,
        TYPE_RESET = 3, // the default handler's answer (no insect field)
    };

    catchMsg_c() : b0(0), b1(0), b2(0), b3(0) {
    }

    u32 getType() const {
        return b0 & 3;
    }
    u32 getIdx() const {
        return (b0 >> 2) & 0xF;
    }
    u32 getValue() const {
        return ((b0 >> 6) & 3) + (b1 & 0x1F) * 4;
    }
    u32 getMember() const {
        return (b1 >> 5) & 7;
    }
    u16 getSerial() const {
        return b2 + (b3 & 1) * 0x100;
    }
    void setType(u32 type) {
        b0 &= ~0x03;
        b0 |= type & 3;
    }
    void setIdx(u32 idx) {
        b0 &= ~0x3C;
        b0 |= (idx & 0xF) << 2;
    }
    void setValue(u32 value) {
        b0 &= ~0xC0;
        b0 |= (value & 3) << 6;
        b1 &= ~0x1F;
        b1 |= (value >> 2) & 0x1F;
    }
    void setSerial(const u16 serial) {
        b2 = serial;
        b3 &= ~1;
        b3 |= (serial >> 8) & 1;
    }
    void setMember(int member) {
        b1 &= ~0xE0;
        b1 |= (member << 5) & 0xE0;
    }

    /* 0x0 */ u8 b0;
    /* 0x1 */ u8 b1;
    /* 0x2 */ u8 b2;
    /* 0x3 */ u8 b3;
}; // size 0x4

// findAnyInsect (d_play_util) callbacks on the insects (arg: a catchMsg_c).
BOOL tryCatchCb(dInsFldBase_c *actor, void *arg);   // 0x52F8
BOOL resetCatchCb(dInsFldBase_c *actor, void *arg); // 0x5340: CATCH_STATE_NONE
BOOL caughtCb(dInsFldBase_c *actor, void *arg);     // 0x536C: CATCH_STATE_CAUGHT
BOOL missedCb(dInsFldBase_c *actor, void *arg);     // 0x5398: CATCH_STATE_MISSED
// 0x53C4: the NET_PACKET_INSECT_FIELD handler (lbl_8074E510).
void recvCatchMsg(int member, const u8 *data, u32 size, u32 arg);

} // namespace dInsectField

// One insect .brres (/Insect/<dInsectInfo::getModelName(type)>.brres), shared
// by the insects of a type. In dInsectField_c::mBanks[15] (constructed with an
// InitObjects array).
class dInsFldBank_c : public dDvd::brresBank_c {
  public:
    enum State_e {
        STATE_LOADING = 0, // load() requested, not done yet
        STATE_LOADED = 1,
        STATE_SHARED = 3, // waits for mShared to load, then copies its file
        STATE_FREE = 4,
    };

    dInsFldBank_c()
        : mState(STATE_FREE), mSharers(NULL), mShared(NULL), mType(-1) {
    } // 0x2612C (weak)

    virtual void freeData(); // 0x5D0C
    virtual void onLoaded(); // 0x5D34: copies the file to the sharers

    static dInsFldBank_c *find(int type);   // 0x5B28: the bank of type (loading
                                            // or loaded)
    BOOL request(int type);                 // 0x5B74: TRUE once loaded
    BOOL release();                         // 0x5C94
    void setHeap(EGG::FrmHeap *heap);       // 0x5D04
    void dummy();                           // 0x5D30: empty, unreferenced
    void addSharer(dInsFldBank_c *bank);    // 0x5DAC
    void removeSharer(dInsFldBank_c *bank); // 0x5DCC
    void copyFrom(dInsFldBank_c *bank);     // 0x5E08: a copy of its file

    static dInsFldBank_c *sBanks; // .bss 0x44: dInsectField_c::mBanks
    static u32 sBankNum;          // .data 0x1C1C: INSECT_FIELD_BANK_NUM
    static char sPath[0x20];      // .bss 0x48: request's file path

    /* 0x58 */ EGG::FrmHeap *mpBankHeap;
    /* 0x5C */ int mState;              // State_e
    /* 0x60 */ dInsFldBank_c *mSharers; // the next bank waiting for this one
    /* 0x64 */ dInsFldBank_c *mShared;  // STATE_SHARED: the bank it waits for
    /* 0x68 */ int mType;               // dInsectType_e, -1 = none
}; // size 0x6C

// One insect. Allocated zeroed in a child heap of a heapList_c (operator new)
// by the per-type factory, then created (create(ResFile)) once its bank is
// loaded.
//
// The manager drives it every frame: mSound.setPos(&mPos), then mHoldExecute if
// set, else execute() while mActive and not deleting; draw: mHoldDraw if set,
// else draw(). A kind's own behaviour goes in the hooks: initState (end of
// create), executeState (in execute), onRelease (released by a player),
// calcScare (end of execute).
class dInsFldBase_c {
  public:
    typedef void (dInsFldBase_c::*ProcFunc)();

    enum CatchResult_e {
        CATCH_PENDING = 0,
        CATCH_MISSED = 1,
        CATCH_CAUGHT = 2
    };

    // mCatchState: a net catch of an insect another console runs.
    enum CatchState_e {
        CATCH_STATE_REQUESTED = 0, // tryCatch sent TYPE_REQUEST to the owner
        CATCH_STATE_CAUGHT = 1,    // caughtCb
        CATCH_STATE_MISSED = 2,    // missedCb
        CATCH_STATE_NONE = INSECT_NET_STATE_NORMAL, // no catch (resetCatchCb)
    };

    enum {
        LIFE_TIME = 3600, // mLifeTimer: frames out of view before it goes away
    };

    // mSpawnKind
    enum SpawnKind_e {
        SPAWN_FIELD = 2,  // addInsect: spawned in the town
        SPAWN_PLAYER = 3, // addPlayerInsect: held / shown by a player
    };

    static void *operator new(size_t size, EGG::Heap *heap); // 0x7FF4: zeroed

    dInsFldBase_c();          // 0x5EBC
    virtual ~dInsFldBase_c(); // 0x60B4

    virtual void calcMtx(); // 0x6240: setMtxFromAngle()
    virtual BOOL create(const nw4r::g3d::ResFile &res); // 0x6198
    virtual int execute();                              // 0x6244
    virtual int draw();                                 // 0x6378
    virtual BOOL doDelete();      // 0x6470: FALSE while a hold proc runs
    virtual BOOL isActive();      // 0x65E4: TRUE (dInsFldBase_c::isActive)
    virtual void onBecomeOwner(); // 0x65EC: this console now runs the scene
    virtual void onHold();        // 0x66E8: first frame held by a player
    virtual void initParam();     // 0x6D98: from getParam()
    virtual void setGroundPos();  // 0x6E3C: mPos.y += ground height
    virtual void initState() {
    } // 0x26184: end of create
    virtual void executeState() {
    } // 0x26188: in execute
    virtual void onRelease() {
    } // 0x2618C: release()
    virtual void vf40() {
    } // 0x26190: never overridden or called here
    virtual void calcScare(); // 0x7740: players / NPCs nearby

    void cancelHold();                   // 0x675C: the holding player goes away
    void setHeldPos(const mVec3_c *pos); // 0x66EC: mPos = *pos
    void setHeldScale(const mVec3_c *scale); // 0x6708
    void release();                          // 0x67F8: the player lets it go
    void initHold();                         // 0x6900: caught in the net
    void initHoldMdl();    // 0x6918: reset the angle and start the held model
    void waitHoldMdl();    // 0x6948: hold proc: request the held model
    void startHold();      // 0x69B0
    void executeHold();    // 0x69F4: hold proc (mHoldExecute)
    void drawHold();       // 0x6C94: hold proc (mHoldDraw)
    void clearHoldProcs(); // 0x6D60
    void createMdl(const nw4r::g3d::ResFile
                       &res); // 0x6E74: model, "wait"/"fly"/"stop" anims
    void setAnmChr(const nw4r::g3d::ResAnmChr &anm, f32 blend); // 0x708C
    void setAnmVis(const nw4r::g3d::ResAnmVis &anm);            // 0x70F4
    void setAnmRate(f32 rate);                                  // 0x7158
    void setAnmFrame(f32 frame);                                // 0x7204
    void setAlpha(u8 alpha); // 0x7270: TEV KColor 1 alpha of material "m1"
    bool isUnderGround();    // 0x7398: mPos.y < getGroundY()
    bool isUnderWater();     // 0x73D4
    f32 getGroundY();        // 0x7434: dBGCF::getGroundY(&mPos, 0)
    void fadeOut();          // 0x7440: lowers mAlpha after 24 frames
    BOOL isHeld() const;     // 0x74C8: mPlayer != none
    void moveForward(f32 speed,
                     f32 accelY); // 0x74E0: walk along mAngle.y, fade, delete
    BOOL isUnitTaken(
        const mVec3_c *pos); // 0x75F0: another active insect is on pos's unit
    bool isCurrentUnitFree() {
        mVec3_c pos;
        pos.set(mPos);
        return !isUnitTaken(&pos);
    }
    bool isCurrentUnitTaken() {
        mVec3_c pos;
        pos.set(mPos);
        return isUnitTaken(&pos) == TRUE;
    }
    bool isSceneChanging(); // 0x76EC (ignores this)
    dItem::Item getItem();  // 0x7840: the item at mPos
    void setMtxFromAngle(); // 0x78A4: mPos, mAngle (X, Y, Z)
    void setMtxFromAxis();  // 0x795C: mPos, mAxis
    void fly(f32 lift, f32 accel, f32 drag,
             f32 turn); // 0x7A24: around mHomePos with mAxis
    const dInsectField::param_c *getParam() const; // 0x7C30
    void requestDelete();                          // 0x7C48
    BOOL isDeleteRequested() const;                // 0x7D14
    BOOL isDeleteReady();                          // 0x7D3C
    void
    registObjc(const mVec3_c *pos, f32 radius,
               BOOL enable); // 0x7DA8: mObjc.regist(.., radius + 4, this, ..)
    int getInsectType() const {
        return mType;
    }
    u16 getSerial() const {
        return mSerial;
    }
    int tryCatch(int player);        // 0x7DC4: 0 pending, 1 missed, 2 caught
    void setHeld(int player);        // 0x7F60: requests the held model
    void attachToPlayer(int player); // 0x7FD0
    static void
    checkPtclLanding(nw4r::ef::Particle *ptcl,
                     u32 arg); // 0x8054: retire / af_hny_hamon_S01 on ground or
                               // water (dEffect::create callback; arg unused)
    dInsectCatchData_c *
    getCatchData() const; // 0x8188: its net slot (field insects)

    // Gives the held model back (to dFIMdlMgr_c and the manager's net insect).
    inline void releasePlayer();

    // (agent B) helpers of the kinds' own draw() / state code.
    static bool isZero(f32 v) {
        return std::fabs(v) <= FLT_EPSILON;
    }
    bool isWallHit() const {
        return mAcch.isWallHit();
    }
    // .rodata 0xE84 (0.5): the round shadow's alpha scale in the kinds' draw()
    // (a named object: loaded by its own symbol, not from the literal pool).
    static const f32 sShadowAlphaScale;

    /* 0x000 vtable */
    /* 0x004 */ m3d::mdl_c mMdl;
    /* 0x044 */ m3d::anmChr_c mAnmChr;
    /* 0x07C */ m3d::anmVis_c mAnmVis;
    /* 0x0B4 */ mAllocator_c mAllocator; // attached to the insect's child heap
    /* 0x0D0 */ dLevelEffect_c mEffect;
    /* 0x164 */ dSndObjInsect_c mSound;
    /* 0x194 */ mVec3_c mInitPos;
    /* 0x1A0 */ mVec3_c mPos;
    /* 0x1AC */ mVec3_c mOldPos; // mPos at the start of execute
    /* 0x1B8 */ mAng3_c mInitAngle;
    /* 0x1BE */ mAng3_c mAngle;
    /* 0x1C4 */ dObjc::ins_c mObjc;
    /* 0x1E8 */ int mFrame;           // +1 every execute
    /* 0x1EC */ dInsFldBase_c *mNext; // in the manager's lists
    /* 0x1F0 */ u8 _1F0[2];
    /* 0x1F2 */ u8 mAlpha;      // 0xFF
    /* 0x1F3 */ u8 mCatchState; // CatchState_e (tryCatch)
    /* 0x1F4 */ nw4r::g3d::ResAnmChr mAnmChrWait;
    /* 0x1F8 */ nw4r::g3d::ResAnmChr mAnmChrFly;
    /* 0x1FC */ nw4r::g3d::ResAnmChr mAnmChrStop;
    /* 0x200 */ nw4r::g3d::ResAnmVis mAnmVisWait;
    /* 0x204 */ nw4r::g3d::ResAnmVis mAnmVisFly;
    /* 0x208 */ nw4r::g3d::ResAnmVis mAnmVisStop;
    /* 0x20C */ mVec3_c mVelocity;
    /* 0x218 */ dBGCF::acch_c mAcch;
    /* 0x260 */ dInsectUtil::axis_c mAxis;
    /* 0x284 */ mVec3_c mHomePos; // mPos at create
    /* 0x290 */ dInsectUtil::scare_c mScare;
    /* 0x29C */ dInsectUtil::npcCheck_c mNpcCheck;
    /* 0x2AC */ dInsectUtil::playerCheck_c mPlayerCheck;
    /* 0x2B8 */ f32
        mShadowSize; // draw: a round shadow when larger than a minimum
    /* 0x2BC */ f32 mScareRadius;
    /* 0x2C0 */ f32 mBaseRadius;
    /* 0x2C4 */ f32 mBgRadius; // mAcch.checkSteps radius
    /* 0x2C8 */ ProcFunc
        mHoldExecute; // replaces execute() while set (__ptmf_null: none)
    /* 0x2D4 */ ProcFunc mHoldDraw;    // replaces draw() while set
    /* 0x2E0 */ dInsFldBase_c **mpRef; // cleared (*mpRef = NULL) on deletion
    /* 0x2E4 */ dInsectField::heapList_c *mHeapList;
    /* 0x2E8 */ dInsectLink_c *mLinks;
    /* 0x2EC */ int
        mHeapIdx; // index in mHeapList (= the net slot for m_objHeap insects)
    /* 0x2F0 */ int mType;      // dInsectType_e
    /* 0x2F4 */ int mLifeTimer; // LIFE_TIME
    /* 0x2F8 */ int mSpawnKind; // SpawnKind_e
    /* 0x2FC */ int mId;        // dInsectField_c::mNextId when activated
    /* 0x300 */ u8 _300[4];
    /* 0x304 */ u16 mSerial; // the net slot's serial
    /* 0x306 */ u8 _306;
    /* 0x307 */ u8 mDeleteReq;
    /* 0x308 */ u8 mRemoteDeleteReq; // net: deleted on the owner's word
    /* 0x309 */ u8 mReleased;
    /* 0x30A */ u8 mHoldStarted; // onHold() was called
    /* 0x30B */ u8 mActive;      // execute() / draw() run (1)
    /* 0x30C */ u8 mBgCheck;     // execute: mAcch.checkSteps
    /* 0x30D */ u8 mMdlCreated;
    /* 0x30E */ u8 _30E[2];
    /* 0x310 */ int mPlayer; // the holding player, INSECT_FIELD_PLAYER_NONE (4)
    /* 0x314 */ u32 mBgFlags; // checkSteps flags (dBGCF::checkFlags_e)
}; // size 0x318

// An insect shown by a player (dInsectField_c::addPlayerInsect mode 0): only
// the held model.
class dInsFldShow_c : public dInsFldBase_c {
  public:
    dInsFldShow_c() {
    }
    virtual ~dInsFldShow_c(); // 0x8210
    virtual BOOL
    create(const nw4r::g3d::ResFile &res); // 0x81BC: initParam, initHold
    virtual int execute();                 // 0x8200: TRUE
    virtual int draw();                    // 0x8208: TRUE
}; // size 0x318
