#pragma once

// Field object tasks of the FG manager: pending changes to field units (planting, digging,
// cross-breeding, ...), each a 0x10-byte record in a fixed list of 32, plus the per-player state
// and the 2-bit unit grid of the usable field. The code is in include/game/game/d_fg_mng_task.inc,
// which d_field_assessment.cpp #includes (built with -sym on, so it lands in the TU's tail,
// 800A2220..). All names are inferred.

#include <types.h>
#include <game/game/d_field_info.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_fg_item.hpp>

#define FG_MNG_TASK_NUM 32

// The scene value that matches any scene (dFgMngTask_c::clear sets it, isScene accepts it,
// fgMngTask_procGroupAt searches with it).
#define FG_MNG_SCENE_ANY SCENE_NUM

// A packed unit position (x << 8) | z that is unset (task / link / command / drop positions).
#define FG_MNG_POS_NONE 0xFFFF

// The field commands (80589E90, sFgMngCmds) and their retry bits (8074E348, one bit each).
#define FG_MNG_CMD_NUM 0x40

// The action of a field request / command / task / player state. dFgMngTask_c::mKind,
// dFgMngReq_c::mKind and dFgMngCmd_c::mKind are 5-bit fields; dFgMngPlayerState_c::mKind holds
// the same value (fn_111_8C30 stores it, fn_111_8D8C copies it into the request).
// The "_FULL" kinds are the base kind + 1: fn_111_8C30 bumps KIND_PICK_UP / KIND_DIG_UP /
// KIND_PICK_UP_SPECIAL when dPrivateData_c::findEmptyPocket fails (and the item is not a
// design), and refuses when the player already has an active task. Their tasks wait
// (dFgMngTask_c::update skips them, cancel keeps them active) until the player swaps
// (KIND_SWAP for PICK_UP_FULL / PICK_UP_SPECIAL_FULL, KIND_BURY with an item for DIG_UP_FULL;
// fgMngState_canAct, fgMngProc_isChangeRequestValid, fgMngTask_replace).
enum dFgMngKind_e {
    FG_MNG_KIND_DROP,                 // 0x00 an item dropped beside the target (fruit / shake item / stone money: planFruit, planShakeItem, dFgMngStone_c::drop, the drops of dFgMngCmd_c::exec); 0 also means "none" in dFgMngPlayerState_c (clearPlayer, fgobj fn_111_A548)
    FG_MNG_KIND_SHAKE_TREE,           // 0x01 fgobj fn_111_9274 (tool type 0): tree of stage > 1; plan() plans the drops (planFruit / planShakeItem); procGroup / getGroupNum
    FG_MNG_KIND_SHAKE_TREE_SPECIAL,   // 0x02 as SHAKE_TREE, chosen instead for trees 0x17 / 0x54 when fgMngProc_canEditField (offline only; what drops is handled outside plan(), hedged)
    FG_MNG_KIND_PICK_UP,              // 0x03 fgobj fn_111_9DFC / fn_111_A088: pick up an item / flower / dandelion / design fg 0x74..0x93; new item NONE
    FG_MNG_KIND_PICK_UP_FULL,         // 0x04 PICK_UP with a full pocket (waits for KIND_SWAP)
    FG_MNG_KIND_PULL,                 // 0x05 fn_111_9DFC: weeds 0x57..0x5A, clover 0xE0, wilted flowers; proc counts weeds (fn_800C77D0, stat 2)
    FG_MNG_KIND_CHOP_TREE,            // 0x06 fgobj fn_111_94AC (tool type 1, axe): unit flag = hits so far, felled into the stump (FgInfo::_00) at a per-stage hit count; proc counts felled trees (fn_800C77B8, stat 1)
    FG_MNG_KIND_CHOP_TREE_SPECIAL,    // 0x07 as CHOP_TREE for trees 0x17 / 0x54 when fgMngProc_canEditField (hedged, see 0x02)
    FG_MNG_KIND_DIG_HOLE,             // 0x08 fgobj fn_111_9880 (tool type 2, shovel): empty diggable ground, weeds, flowers -> hole fg 0x94
    FG_MNG_KIND_DIG_UP,               // 0x09 fn_111_9880: a buried item (dFdBase_c::isFlagA) or a planted red turnip 0x95..0x9D (gives ITEM_IDX_RED_TURNIP_00 + n)
    FG_MNG_KIND_DIG_UP_FULL,          // 0x0A DIG_UP with a full pocket (waits for KIND_BURY with an item)
    FG_MNG_KIND_DIG_OUT,              // 0x0B fn_111_9880: stumps 0x05..0x10, dead trees 0x01..0x04, 0x95 / 0x9D, saplings (FgInfo stage 0)
    FG_MNG_KIND_BLOCKED,              // 0x0C tool blocked: an item (BITM / furniture) or another player on the unit, undiggable ground, rafflesia; no field change (exec / Req::proc return TRUE)
    FG_MNG_KIND_HIT_ROCK,             // 0x0D axe / shovel on a rock 0x5B..0x5F, an ext object 0xD000.., another player's money rock; no field change
    FG_MNG_KIND_HIT_MONEY_ROCK,       // 0x0E own money rock 0x60..0x73 ((id - 0x60) / 5 == fn_801017B8()), offline: dFgMngReq_c::proc -> dFgMngStone_c::hit
    FG_MNG_KIND_WATER,                // 0x0F fgobj fn_111_9CE8 (tool type 4): watering; dFgMngReq_c::proc / send -> fgMngProc_waterAround
    FG_MNG_KIND_PUT,                  // 0x10 fgobj fn_111_A878 / fn_111_A994: put an item down on the unit in front
    FG_MNG_KIND_SWAP,                 // 0x11 fgobj fn_111_A8D4: put the held item down at the active (pocket-full) task's unit
    FG_MNG_KIND_PUT_PENDING,          // 0x12 fn_111_A994 when the player has an active task: put down elsewhere; proc() then finishes the linked task (mLinkPos)
    FG_MNG_KIND_BURY,                 // 0x13 fn_111_9880 on a hole 0x94 (also d_menu_item00): bury / plant the held item (fgMngProc_getBuryFg), or fill the hole when none; Req::proc maps an fg to getPlantedFg
    FG_MNG_KIND_FILL_HOLE,            // 0x14 fgobj fn_111_A204: hole 0x94 -> NONE (stage 1)
    FG_MNG_KIND_PICK_UP_SPECIAL,      // 0x15 fn_111_9DFC: fg 0xE1 or a KIND_MUSH_FTR item; proc counts 0xE1 (fn_800C7818, stat 5); hedged name
    FG_MNG_KIND_PICK_UP_SPECIAL_FULL, // 0x16 PICK_UP_SPECIAL with a full pocket
    FG_MNG_KIND_MISS,                 // 0x17 a tool used on nothing it can affect (empty unit / other fg); no field change
    FG_MNG_KIND_PLANT,                // 0x18 fgobj fn_111_A788(kind 0x18) (player, d_menu_item00): plant the item in front (fgMngProc_getPlantedFg); fgobj fn_111_6C84 effect
    FG_MNG_KIND_PITFALL,              // 0x19 fgobj fn_111_A300 / fn_111_A424 (player, d_a_npc_out): a buried ITEM_IDX_PITFALL_SEED -> hole 0x94
    FG_MNG_KIND_PUT_DESIGN,           // 0x1A fgobj fn_111_AA00 (d_menu_desi00): fg 0x74 + player * 8 + slot; fn_111_A878 for a design item; fn_111_6D64 effect; fgMngTask_canAdd lets it be replaced
    FG_MNG_KIND_BALLOON,              // 0x1B fgobj fn_111_6E80 (called only by d_a_balloonNP): an item lands on the field (player 0); hedged name
    FG_MNG_KIND_NONE,                 // 0x1C dFgMngCmd_c::init (a free command)
};

// Net packet ids passed to fn_800DD588 (handlers in the table 80473E44, by id).
enum dFgMngPacket_e {
    FG_MNG_PACKET_REQ = 0x28,       // dFgMngReq_c to the host (fgobj fn_111_8D8C); handler fn_800DF7A0 (isChangeRequestValid + send)
    FG_MNG_PACKET_CMD = 0x29,       // dFgMngCmd_c (dFgMngReq_c::send, fgMngCmd_apply); handler fn_800DF804 (fgMngCmd_apply)
    FG_MNG_PACKET_TASK_DONE = 0x2A, // dFgMngTaskMsg_c (dFgMngTask_c::finish); handler fn_800DF860 (clears the lock's pending bit)
    FG_MNG_PACKET_UNIT_FG = 0x2B,   // fgMngProc_setUnitFgSync; handler fn_800DF8E4 (fgMngProc_recvUnitFg)
};

// fn_800DD588's destination meaning "every member" (members are 0..3; the per-member sends
// pass a member number, finish sends to member 0). Hedged.
#define FG_MNG_NET_ALL 4

// A unit position packed as (x << 8) | z, built byte by byte.
struct dFgMngTaskPos_c {
    dFgMngTaskPos_c(u16 pos) : mPos(pos) {}
    dFgMngTaskPos_c(const dFdAsPos_c &pos) {
        ((u8 *)&mPos)[0] = pos.mX;
        ((u8 *)&mPos)[1] = pos.mZ;
    }

    /* 0x0 */ u16 mPos;
}; // size 0x2

// The message sent when a task finishes on a client (fn_800DD518, 4 bytes).
struct dFgMngTaskMsg_c {
    /* 0x0 */ u16 mPos;
    /* 0x2 */ u8 mPlayer : 2;
    /* 0x2 */ u8 mLayer : 1;
    /* 0x2 */ u8 _2_low : 5;
    /* 0x3 */ u8 mScene;
}; // size 0x4

// One field task (0x10).
class dFgMngTask_c {
public:
    enum {
        PLAYER_NONE = 7,   // mPlayer of a cleared task (clear)
        DROP_IDX_NUM = 3,  // drops 0..2 of a group (exec / planFruit; findGroup; searchSpot tests mDropIdx > 2)
        DROP_IDX_STONE = 3, // the money a hit rock drops (dFgMngStone_c::drop); hedged
        DROP_IDX_MAIN = 4, // the command's own task (dFgMngCmd_c::exec)
        DROP_IDX_NONE = 7, // clear
    };

    void reset();                                                        // 800A2220
    void update();                                                       // 800A2224
    void set(int player, u8 scene, u16 pos, u16 pos2, u16 item, u16 item2, int kind, u8 stage, u8 b2,
             u8 idx, u8 layer, s8 day);                                  // 800A22C4
    void proc();                                                         // 800A233C
    void procGroup();                                                    // 800A2584
    void cancel();                                                       // 800A2718
    void clear();                                                        // 800A27AC
    void finish();                                                       // 800A2800
    void apply();                                                        // 800A2918
    void notify(BOOL keep);                                              // 800A29D0
    void release();                                                      // 800A314C

    BOOL isOwner(u32 player) const { return player == mPlayer && mActive; }
    BOOL isAt(const dFdAsPos_c *pos) const { return pos->mX == getX() && pos->mZ == getZ(); }
    BOOL isAtLayer(const dFdAsPos_c *pos, u32 layer) const { return isAt(pos) && layer == mLayer; }
    BOOL isScene(u32 scene) const { return scene == mScene || scene == FG_MNG_SCENE_ANY; }
    BOOL isAtScene(u32 scene, const dFdAsPos_c *pos, u32 layer) const {
        return isAtLayer(pos, layer) && isScene(scene);
    }
    BOOL isOwnAtX(int player, const dFdAsPos_c *pos) const { return player == mPlayer && pos->mX == getX(); }
    BOOL isOwnAt(int player, const dFdAsPos_c *pos) const { return isOwnAtX(player, pos) && pos->mZ == getZ(); }
    BOOL isOwnAtScene0(int player, u32 scene, const dFdAsPos_c *pos) const {
        return isOwnAt(player, pos) && scene == mScene;
    }
    BOOL isOwnAtScene(int player, u32 scene, const dFdAsPos_c *pos, u32 layer) const {
        return isOwnAtScene0(player, scene, pos) && layer == mLayer;
    }
    BOOL isOwnerAtXP(u32 player, const dFgMngTaskPos_c *pos) const { return isOwner(player) && mPos >> 8 == pos->mPos >> 8; }
    BOOL isOwnerAtP(u32 player, const dFgMngTaskPos_c *pos) const {
        return isOwnerAtXP(player, pos) && (mPos & 0xFF) == (pos->mPos & 0xFF);
    }
    BOOL isOwnerOf(const dFgMngTask_c *other) const { return mPlayer == other->mPlayer && mActive; }
    BOOL isAtXOf(const dFgMngTask_c *other) const { return isOwnerOf(other) && mPos >> 8 == other->mLinkPos >> 8; }
    BOOL isLinkOf(const dFgMngTask_c *other) const { return isAtXOf(other) && (mPos & 0xFF) == (other->mLinkPos & 0xFF); }
    BOOL isFree() const { return getX() == 0xFF && getZ() == 0xFF; }

    int getX() const { return mPos >> 8; }
    int getZ() const { return mPos & 0xFF; }

    /* 0x0 */ u8 mPlayer : 3; // 7: none
    /* 0x0 */ u8 mKind : 5;
    /* 0x1 */ u8 mStage : 2;
    /* 0x1 */ u8 mDropIdx : 3;
    /* 0x1 */ u8 mB2 : 2;
    /* 0x1 */ u8 mActive : 1;
    /* 0x2 */ u8 mSet : 1;
    /* 0x2 */ u8 mLayer : 1;
    /* 0x2 */ s8 mDay : 4;
    /* 0x2 */ u8 mHold : 1;
    /* 0x2 */ u8 _02_7 : 1;
    /* 0x3 */ u8 _03;
    /* 0x4 */ u16 mPos;    // (x << 8) | z; 0xFFFF: none
    /* 0x6 */ u16 mLinkPos;
    /* 0x8 */ u16 mNewItem;
    /* 0xA */ u16 mPrevItem;
    /* 0xC */ u8 mScene;
    /* 0xD */ u8 _0D[3];
}; // size 0x10

// The task list (80589B7C).
class dFgMngTaskList_c {
public:
    dFgMngTaskList_c() : mInit(0) {}

    void init();   // 800A2A5C
    void reset();  // 800A2ABC
    void update(); // 800A2B08 (empty)

    /* 0x000 */ u8 mInit;
    /* 0x001 */ u8 mBusy;
    /* 0x002 */ u8 _002[2];
    /* 0x004 */ dFgMngTask_c mTasks[FG_MNG_TASK_NUM];
}; // size 0x204

extern dFgMngTaskList_c sFgMngTaskList; // 80589B7C

BOOL fgMngTask_add(u8 player, u8 scene, dFdAsPos_c pos, dFdAsPos_c pos2, u16 item, u16 item2, int kind,
                   u8 stage, u8 b2, u8 idx, u8 layer, s8 day);                            // 800A2B0C
BOOL fgMngTask_replace(int player, u8 scene, dFdAsPos_c pos, dFdAsPos_c pos2, u16 item, u16 item2, int kind,
                       u8 stage, u8 b2, u8 idx, u8 layer, s8 day);                        // 800A2C98
void fgMngTask_procAt(u8 scene, const dFdAsPos_c *pos, u32 layer);                        // 800A2F58
void fgMngTask_procAt(int x, int z, u32 layer);                                           // 800A2F7C
void fgMngTask_proc(int idx);                                                             // 800A2FC0
void fgMngTask_procOwn(u8 scene, const dFdAsPos_c *pos, u32 player, u32 layer);           // 800A2FE4
void fgMngTask_procGroupAt(u8 x, u8 z, u8 layer);                                         // 800A3044
void fgMngTask_releaseAt(u8 scene, const dFdAsPos_c *pos, u32 layer);                     // 800A3094
void fgMngTask_releaseAt(int x, int z, u32 layer);                                        // 800A30D4
void fgMngTask_releaseAt(u8 scene, int x, int z, u32 layer);                              // 800A3118
void fgMngTask_finishActive(u8 player);                                                    // 800A3200
void fgMngTask_finishAt(int player, dFgMngTaskPos_c pos);                                 // 800A3240
int fgMngTask_findMarked(u8 scene, const dFdAsPos_c *pos, u32 layer);                     // 800A338C
void fgMngTask_remove(int player, const dFdAsPos_c *pos);                                 // 800A33F0
void fgMngTask_setBusy();                                                                 // 800A3468
void fgMngTask_updateAll();                                                               // 800A347C
int fgMngTask_find(u32 scene, const dFdAsPos_c *pos, u32 layer);                           // 800A34E0
int fgMngTask_findOwn(int player, u32 scene, const dFdAsPos_c *pos, u32 layer);           // 800A35B0
int fgMngTask_findFree();                                                                 // 800A3694
int fgMngTask_findActive(u8 player);                                                      // 800A379C
int fgMngTask_findGroup(u32 player, u8 idx, BOOL notHeld);                                // 800A3868
dFgMngTask_c *fgMngTask_get(int idx);                                                     // 800A39E8
u16 *fgMngTask_getActive(int player); // 800A3A18: &task->mPos
void fgMngTask_set(int idx, int player, u8 scene, dFdAsPos_c pos, dFdAsPos_c pos2, u16 item, u16 item2,
                   int kind, u8 stage, u8 b2, u8 idx2, u8 layer, s8 day);                 // 800A3B14
BOOL fgMngTask_new(int player, u8 scene, dFdAsPos_c pos, dFdAsPos_c pos2, u16 item, u16 item2, int kind,
                   u8 stage, u8 b2, u8 idx, u8 layer, s8 day);                            // 800A3BAC
BOOL fgMngTask_canAdd(u8 scene, dFdAsPos_c pos);                                          // 800A3C9C

// Per-player state (0x24) of dFgMngState_c.
struct dFgMngPlayerState_c {
    enum State_e {
        STATE_NONE,     // 0 clearPlayer; fgobj fn_111_A548 returns 2 (nothing pending)
        STATE_WAIT,     // 1 fgobj fn_111_8C30: a request was made, no answer yet (fn_111_A548 returns 0)
        STATE_ACCEPTED, // 2 dFgMngReq_c::send (local), fgMngCmd_apply (accepted), fgobj offline path
        STATE_REJECTED, // 3 dFgMngReq_c::send (refused), fgMngCmd_apply (!mLocal), fgobj failures (fn_111_A548 returns 2)
    };

    /* 0x00 */ u32 mPlayer;  // low 2 bits
    /* 0x04 */ u32 _04;
    /* 0x08 */ int mState;
    /* 0x0C */ int mKind;
    /* 0x10 */ u16 mItem;
    /* 0x14 */ dFdAsPos_c mPos;
    /* 0x1C */ u8 mLayer;
    /* 0x1D */ u8 _1D;
    /* 0x1E */ s8 mDay;
    /* 0x20 */ u32 _20;
}; // size 0x24

// A rock: hits and the respawn countdown (dFgMngState_c + 0x6AC).
struct dFgMngStone_c {
    enum {
        HIT_MAX = 8,        // hit(): hit count cap (index into drop()'s sMoney / sMoneyGold, 10 entries)
        HIT_MAX_BONUS = 9,  // hit(): cap when town flag 0x100 is set and the player's bit is in townFlags[0x11] (first hit counts twice); hedged
        TIMER_NONE = -1,    // init / update: no respawn pending (hit() only arms the timer when mTimer < 0)
    };

    void reset();                                       // 800A3D0C
    void init();                                        // 800A3D10
    void update();                                      // 800A3D24
    void hit(dFdAsPos_c pos);                           // 800A3E30
    void spawn(const dFdAsPos_c *pos);                  // 800A3F20
    BOOL drop(dFdBase_c *fd, const dFdAsPos_c *pos);    // 800A4000

    /* 0x0 */ u16 mPos;
    /* 0x2 */ s16 mHit;
    /* 0x4 */ s16 mTimer;
}; // size 0x6

// 805894C8 (0x6B4).
class dFgMngState_c {
public:
    void clearPlayer(int player);                          // 800A41A8
    void init();                                           // 800A41C0
    void updateStone();                                    // 800A4214
    u32 getUnitFlag(const dFdAsPos_c *pos);                // 800A421C
    void setUnitFlag(const dFdAsPos_c *pos, u32 flag);     // 800A426C

    /* 0x000 */ dFgMngPlayerState_c mPlayers[3];
    /* 0x06C */ u32 mUnitFlags[5 * 80]; // 2 bits per unit of the usable 80 x 80 units
    /* 0x6AC */ dFgMngStone_c mStone;
}; // size 0x6B4

extern dFgMngState_c sFgMngState; // 805894C8

void fgMngState_clearUnitFlags(dFgMngState_c *state);              // 800A42D0 (state unused)
dFgMngPlayerState_c *fgMngState_getPlayer(int player);              // 800A4394
u32 fgMngState_getUnitFlag(int x, int z);                           // 800A43A8
BOOL fgMngState_isPlayerLeftOf(int player, const f32 *dist);        // 800A43DC
BOOL fgMngState_isFreeOfPlayers(int x, int z, int exclude);         // 800A4478
BOOL fgMngState_isFreeOfNpcs(int x, int z);                         // 800A4550
BOOL fgMngState_isFree(int x, int z);                               // 800A46B4
BOOL fgMngState_canAct(int player, int x, int z);                   // 800A46FC

// A field request (from the player's action).
struct dFgMngReq_c {
    BOOL proc(dFgMngPlayerState_c *st);                 // 800A4868
    void send(BOOL local, int player, u8 scene);        // 800A4B78

    /* 0x0 */ u8 mMember : 2; // the requesting net member
    /* 0x0 */ u8 mPlayer : 2;
    /* 0x0 */ u8 mStage : 2;
    /* 0x0 */ u8 mSend : 1;
    /* 0x0 */ u8 _00_0 : 1;
    /* 0x1 */ u8 mKind : 5;
    /* 0x1 */ u8 mB2 : 2;
    /* 0x1 */ u8 mLayer : 1;
    /* 0x2 */ u16 mPos;
    /* 0x4 */ u16 mTarget; // the tree or flower acted on
    /* 0x6 */ u16 mItem;
    /* 0x8 */ u16 mItem2;
};

// A field command (sent to the other consoles, 0x12).
struct dFgMngCmd_c {
    enum {
        ITEM_FREE = 0xFFFF,     // mNewItem of a free command (init, fgMngCmd_find, fgMngCmd_findFree)
        SEND_SIZE_SHORT = 0xA,  // send(): bytes up to mDropPos (offsetof(dFgMngCmd_c, mDropPos))
        SEND_SIZE_FULL = 0x12,  // send() with drops / a link; = sizeof(dFgMngCmd_c); fgMngCmd_apply's arg limit
    };
    // searchSpot result
    enum Spot_e {
        SPOT_NONE,    // 0 no free spot (planFruit / planShakeItem leave mDropPos[i] = FG_MNG_POS_NONE)
        SPOT_FOUND,   // 1 *pos is the spot
        SPOT_BLOCKED, // 2 a locked neighbour that isn't a drop: the planners fail
    };
    // plan result (dFgMngReq_c::send switches on it)
    enum Plan_e {
        PLAN_NO_DROPS, // 0 nothing to drop (17 / 18 then send the link position)
        PLAN_DROPS,    // 1 mDropPos / mDropItem planned (mHasDrops, SEND_SIZE_FULL)
        PLAN_FAILED,   // 2 no room: the request is refused (mLocal = 0)
    };

    void init();                                                // 800A4948
    BOOL exec();                                                // 800A4974
    int searchSpot(dFdAsPos_c *pos, int dir, dFdBase_c *fd);    // 800A4E50
    u16 getFruit(u16 id);                                       // 800A5278
    BOOL planFruit(int player, u16 id);                         // 800A52BC
    BOOL planShakeItem(dFgMngReq_c *req, int player);           // 800A54B0
    int plan(dFgMngReq_c *req, int player);                     // 800A5738
    BOOL isPlanned(int x, int z);                               // 800A588C

    union {
        u16 mRaw; // copied as one halfword
        struct {
            /* 0x00 */ u8 mHasDrops : 1;
            /* 0x00 */ u8 mPlayer : 2;
            /* 0x00 */ u8 mKind : 5;
            /* 0x01 */ u8 mReqPlayer : 2;
            /* 0x01 */ u8 mLocal : 1;
            /* 0x01 */ u8 mStage : 2;
            /* 0x01 */ u8 mB2 : 2;
            /* 0x01 */ u8 mLayer : 1;
        };
    };
    /* 0x02 */ u16 mPos;
    /* 0x04 */ u16 mNewItem; // 0xFFFF: a free command
    /* 0x06 */ u16 mItem;
    /* 0x08 */ u8 mScene;
    /* 0x0A */ u16 mDropPos[3];
    /* 0x10 */ u16 mDropItem;
}; // size 0x12

// 80589E90: the 0x40 field commands (destroyed by the __arraydtor 800A5D20).
struct dFgMngCmdEntry_c : public dFgMngCmd_c {
    ~dFgMngCmdEntry_c(); // 800A4E10
}; // size 0x12

int fn_800A597C(); // 800A597C

// Unit locks and field commands (d_field_assessment.cpp 80093238..800938F0). A lock (80589D80)
// holds a unit while the players in mPendingPlayers still have to finish its task; a command
// (80589E90) whose exec() failed is retried by d_fgobj_managerNP while its retry bit is set.
void fgMngSync_reset();                                                     // 80093238: all locks and commands
int fgMngLock_find(u32 scene, dFdAsPos_c *pos, u32 layer);                  // 80093388: -1 if none
int fgMngCmd_find(int player, u32 kind, u32 scene, dFdAsPos_c *pos, u32 layer); // 80093490: -1 if none
int fgMngLock_findFree();                                                   // 800935A8: -1 if none
dFgMngLock_c *fgMngLock_get(int idx);                                       // 800935FC
void fgMngCmd_apply(dFgMngCmd_c *cmd, u32 arg);                             // 80093610: runs it, rebroadcasts it
dFgMngCmd_c *fgMngCmd_get(int idx);                                         // 80093778
void fgMngLock_add(u32 scene, dFdAsPos_c *pos, u32 layer);                  // 8009378C: a bit per online member
void fgMngCmd_setRetry(int idx);                                            // 8009387C
void fgMngCmd_clearRetry(int idx);                                          // 800938A0
BOOL fgMngCmd_isRetry(int idx);                                             // 800938C4
int fgMngCmd_findFree();                                                    // 800938F0: 0x3F if none
