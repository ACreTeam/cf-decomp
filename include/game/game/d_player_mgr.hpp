#pragma once

#include <types.h>
#include <game/game/d_actor.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_player_mngr.hpp>
#include <game/mLib/m_3d/fanm.hpp>
#include <lib/nw4r/ef/ef_particle.h>
#include <lib/nw4r/ef/ef_particlemanager.h>

class dHmnToolBank_c;

// Player manager. Source: src/dol/game/d_player_mgr.cpp (.text 800FBB58..8010263C).
// Notes: notes/TUs/d_player_mgr.txt. Names are inferred from behavior and callers.
//
// Player index arguments: 0-3 are local controllers, 4 means "the current one"
// (getMyNo). Player slots (mSlots): 0-3 town players (dSaveData_c::mPlayers),
// 4-6 visiting players (GuestPlayers_c), 7 none.

// The player actor (lives in the d_a_player REL; its constructor runs dDemoActor_c's). Only the fields this TU
// uses.
class dPlayerActor_c : public dDemoActor_c {
public:
    using dActor_c::mPos;
    using dActor_c::mAngle;

    f32 getMoveSpeed() const { return _22EC; }

    m3d::fanm_c *getAnm() { return (m3d::fanm_c *)_03C4; }

    /* 0x00BA */ u8 _00BA[0x30A]; // dDemoActor_c ends at 0xBA (mDemoFlags); MWCC packs into its tail padding
    /* 0x03C4 */ u8 _03C4[0x8];
    /* 0x03CC */ u8 _03CC[0x240];
    /* 0x060C */ u8 _060C[0x18];
    /* 0x0624 */ u8 _0624[0x30];
    /* 0x0654 */ mVec3_c _0654;
    /* 0x0660 */ u8 _0660[0x90];
    /* 0x06F0 */ u8 _06F0[0x4];
    /* 0x06F4 */ u8 _06F4[0x540];
    /* 0x0C34 */ dHmnToolBank_c *_0C34;
    /* 0x0C38 */ u8 _0C38[0x1430];
    /* 0x2068 */ mVec3_c _2068;
    /* 0x2074 */ mVec3_c _2074;
    /* 0x2080 */ u8 _2080[0xC];
    /* 0x208C */ f32 _208C;
    /* 0x2090 */ u8 _2090[0x5C];
    /* 0x20EC */ mVec3_c _20EC;
    /* 0x20F8 */ u8 _20F8[0x92];
    /* 0x218A */ u16 _218A;
    /* 0x218C */ u8 _218C[0x4];
    /* 0x2190 */ int _2190;
    /* 0x2194 */ int _2194;
    /* 0x2198 */ u8 _2198[0x24];
    /* 0x21BC */ f32 _21BC;
    /* 0x21C0 */ u8 _21C0[0x60];
    /* 0x2220 */ int mAnmId;
    /* 0x2224 */ u8 _2224[0x8];
    /* 0x222C */ int mState;
    /* 0x2230 */ int _2230;
    /* 0x2234 */ int _2234;
    /* 0x2238 */ int _2238;
    /* 0x223C */ int _223C;
    /* 0x2240 */ u8 _2240[0x4];
    /* 0x2244 */ int _2244;
    /* 0x2248 */ int _2248;
    /* 0x224C */ int _224C;
    /* 0x2250 */ u8 _2250[0x4];
    /* 0x2254 */ int _2254;
    /* 0x2258 */ u8 _2258[0x14];
    /* 0x226C */ int _226C;
    /* 0x2270 */ u32 _2270;
    /* 0x2274 */ u32 _2274;
    /* 0x2278 */ u8 _2278[0x8];
    /* 0x2280 */ int _2280;
    /* 0x2284 */ u32 _2284;
    /* 0x2288 */ u32 _2288;
    /* 0x228C */ int _228C;
    /* 0x2290 */ int _2290;
    /* 0x2294 */ u8 _2294[0x4];
    /* 0x2298 */ int _2298;
    /* 0x229C */ u8 _229C[0x18];
    /* 0x22B4 */ u16 _22B4;
    /* 0x22B6 */ u16 _22B6;
    /* 0x22B8 */ u8 _22B8[0xC];
    /* 0x22C4 */ s16 _22C4;
    /* 0x22C6 */ s16 _22C6;
    /* 0x22C8 */ s16 _22C8;
    /* 0x22CA */ s16 _22CA;
    /* 0x22CC */ s16 _22CC;
    /* 0x22CE */ u8 _22CE[0x2];
    /* 0x22D0 */ s16 _22D0;
    /* 0x22D2 */ u8 _22D2[0x8];
    /* 0x22DA */ u8 _22DA;
    /* 0x22DB */ u8 _22DB[0x2];
    /* 0x22DD */ u8 _22DD;
    /* 0x22DE */ u8 _22DE;
    /* 0x22DF */ u8 _22DF[0x5];
    /* 0x22E4 */ u8 _22E4;
    /* 0x22E5 */ u8 _22E5[0x7];
    /* 0x22EC */ f32 _22EC;
    /* 0x22F0 */ u8 _22F0[0x8];
    /* 0x22F8 */ s16 _22F8;
    /* 0x22FA */ u8 _22FA[0xE];
    /* 0x2308 */ dPlayerMngr_c::actionParam_c mRemoteActionParams;
    /* 0x2314 */ u8 _2314[0x4];
}; // size >= 0x2318

namespace dPlayerMgr {

// State IDs shared with the d_a_player REL. Unidentified states keep their numeric IDs.
// Function arguments and actor storage remain int, as used by the state callback ABI.
enum State {
    STATE_IDLE = 0x00,
    STATE_WALK = 0x01,
    STATE_RUN = 0x02,
    STATE_MOVE_TO = 0x03,
    STATE_TURN_TO = 0x04,
    STATE_CHANGE_EQUIPMENT = 0x05,
    STATE_CHANGE_HELD_ITEM = 0x06,
    STATE_DEMO_START = 0x07,
    STATE_DEMO_WAIT = 0x08,
    STATE_LIE_DOWN = 0x2A,
    STATE_GET_UP = 0x2C,
    STATE_BED_WAIT = 0x2E,
    STATE_SIT_DOWN = 0x35,
    STATE_CHAIR_WAIT = 0x38,
    STATE_MII_MASK_IMMEDIATE = 0x4B,
    STATE_MII_MASK = 0x4C,
    STATE_CHANGE_HAIR = 0x4F,
    STATE_CHANGE_MII_MASK = 0x50,
    STATE_SHOW_CATCH = 0x5F,
    STATE_SHOW_CATCH_AT = 0x60,
    STATE_RECEIVE_ITEM = 0x63,
    STATE_GIVE_ITEM = 0x6A,
    STATE_EMOTION = 0x71,
    STATE_EAT = 0x78,
    STATE_EAT_REMOVE_MASK = 0x79,
    STATE_EAT_SPECIAL = 0x97,
    STATE_CHANGE_HELD_ITEM_DEMO = 0x98,
    STATE_DRINK_COFFEE = 0x9C,
    STATE_FINISH_COFFEE = 0x9D,
    STATE_BED_HAIR = 0x9F,
    STATE_CHANGE_OUTFIT_AND_HELD = 0xA0,
    STATE_SHOE_SHINE_START = 0xA6,
    STATE_SHOE_SHINE = 0xA7,
    STATE_SHOE_SHINE_END = 0xA8,
    STATE_NONE = 0xAB,
    STATE_COUNT = 0xAB,
};

enum PlayerIndex {
    PLAYER_INDEX_CURRENT = 4,
    PLAYER_INDEX_COUNT = 4,
};

enum PlayerSlot {
    PLAYER_SLOT_FIRST_GUEST = 4,
    PLAYER_SLOT_NONE = 7,
    PLAYER_SLOT_TOWN_COUNT = 4,
    PLAYER_SLOT_GUEST_COUNT = 3,
};

enum EquipmentChangeKind {
    EQUIPMENT_CHANGE_NORMAL = 0,
    EQUIPMENT_CHANGE_PUT_ON_MII_MASK = 1,
    EQUIPMENT_CHANGE_TAKE_OFF_MII_MASK = 2,
};

enum EquipmentChangeMode {
    EQUIPMENT_CHANGE_SHIRT = 1,
    EQUIPMENT_CHANGE_HAT = 2,
    EQUIPMENT_CHANGE_ACCESSORY = 3,
    EQUIPMENT_CHANGE_DESIGN = 5,
};

enum AppearanceChange {
    APPEARANCE_CLEAR_BEE_STING = 0x10, // clear PRIVATE_FLAG0_BEE_STUNG (removeMaskForEating)
    APPEARANCE_MII_MASK_ON = 0x11,
    APPEARANCE_MII_MASK_OFF = 0x12,
};

enum ActorFlag {
    ACTOR_FLAG_LOOK_AT = 0x40,
    ACTOR_FLAG_INSECT = 0x8000,
    ACTOR_FLAG_DISABLED = 0x2000000,
};

enum EquipmentVisibility {
    EQUIPMENT_SHOW_ACCESSORY = 0x40,
    EQUIPMENT_HIDE_ACCESSORY = 0x80,
    EQUIPMENT_CHANGE_CLOTH = 0x100,
};

enum HeldItemChange {
    HELD_ITEM_CHANGE_EQUIP = 1,
    HELD_ITEM_CHANGE_REMOVE = 2,
};

// Indices into the surface effect name arrays supplied by players and NPCs.
enum SurfaceEffectKind {
    SURFACE_EFFECT_NORMAL = 0,
    SURFACE_EFFECT_SEA = 1,
    SURFACE_EFFECT_RAIN = 2,
    SURFACE_EFFECT_SNOW = 3,
    SURFACE_EFFECT_SAND = 4,
};

// 0x19440 at Mgr_c+0x60: data for up to three visiting players (slots 4-6).
class GuestPlayers_c {
public:
    GuestPlayers_c();  // 80101AEC
    ~GuestPlayers_c(); // 80101B38
    void clear();                    // 80101BA0
    int getIndex(int slot);          // 80101BF0: slot 4-6 -> 0-2, else -1
    dPrivateData_c *get(int slot);   // 80101C1C
    dPrivateData_c *getRaw(int slot); // 80101C68

    dPrivateData_c mPlayers[PLAYER_SLOT_GUEST_COUNT];
};

// Manager storage. 0x194C0.
class Mgr_c {
public:
    typedef void (*ChangeStateFn)(int idx, int state);

    Mgr_c();  // 800FBB58
    ~Mgr_c(); // 800FBBEC

    // Player save data by player index: 0-3 town players, 4-6 visitors online.
    static dPrivateData_c *getPlayer(int player);                 // 80101624
    static dPrivateData_c *getPlayer(const dPersonalID_c *pid);   // 801016B8
    static dPrivateData_c *getPlayerRaw(int player);              // 801016DC
    // By network member index (0-3, 4 = this console), through mSlots.
    static dPrivateData_c *getNetPlayer(int member);              // 80101694
    static dPrivateData_c *getNetPlayerRaw(int member);           // 8010174C
    // This console's player.
    static dPrivateData_c *getCurrentPlayer();                    // 80101770
    static dPrivateData_c *getCurrentPlayerRaw();                 // 80101794

    /* 0x00000 */ ChangeStateFn mChangeState[PLAYER_INDEX_COUNT]; // set by setChangeStateCallback, called by changeState
    /* 0x00010 */ int _10[PLAYER_INDEX_COUNT];
    /* 0x00020 */ int _20;
    /* 0x00024 */ dPlayerActor_c *mActors[PLAYER_INDEX_COUNT];    // set by setActor
    /* 0x00034 */ int mSlots[PLAYER_INDEX_COUNT];                 // player index per network member (getPlayerSlot), 7 = none
    /* 0x00044 */ u8 _44[0x1C];
    /* 0x00060 */ GuestPlayers_c mGuests;
    /* 0x194A0 */ u32 mTimer;                    // 0xD2F0 (resetTimer)
    /* 0x194A4 */ u8 mDailyHairUpdated;
    /* 0x194A5 */ u8 mBedHairPending;
    /* 0x194A6 */ u8 mDailyTanUpdated;
    /* 0x194A7 */ u8 _194A7;
    /* 0x194A8 */ u8 _194A8;
    /* 0x194A9 */ u8 mLocked;                    // lock / unlock
    /* 0x194AA */ u8 mToolActionHeld;
    /* 0x194AB */ u8 mSaveRequestFlagsCleared;
    /* 0x194AC */ u8 _194AC[0x14];
}; // size 0x194C0

extern Mgr_c gManager;
// Shared player tuning constants also referenced by REL actors.
extern const u8 sRemoteActionDelayFrames;
extern const u8 sRemoteActionAlternateDelayFrames;
extern const f32 sRemoteMoveSpeed;
extern const f32 sDefaultMoveSpeed;
extern const f32 sPlayerValue80750A7C; // Player REL use; precise meaning unresolved.

extern s16 sLookPitchMax; // 8074E600: 10 degrees
extern s16 sLookYawMax;   // 8074E602: 45 degrees


// Output of RFLGetCharInfo; the create ID goes to dPrivateData_c::mMiiId.
struct MiiData_c {
    /* 0x00 */ u8 _00[0x2C];
    /* 0x2C */ u8 mCreateID[8];
    /* 0x34 */ u32 mPersonalInfo;
    /* 0x38 */ u8 mCreatorID[4];
};

mVec3_c *getPosition(int idx); // 800FBC48
dPlayerActor_c *getPlayerActor(int idx); // 800FBC7C
dPlayerActor_c *getPlayerActorRaw(int idx); // 800FBC80
dPlayerActor_c *getNpcTargetActor(int idx); // 800FBC84
mVec3_c *getSoundPosition(int idx); // 800FBC88
BOOL getMoveAngle(s16 *out); // 800FBCBC
s16 getYaw(int idx); // 800FBD28
BOOL getUnitCoordinates(u32 *a, u32 *b, int idx); // 800FBD5C
mVec3_c *getHandPosition(int idx); // 800FBDD4
void *getHandTransform(int idx); // 800FBE08
BOOL canStartDemo(); // 800FBE3C
BOOL canStartNetSave(); // 800FBE98
BOOL isDemoStartState(int state); // 800FBF84
void startDemoWait(); // 800FBFCC
BOOL requestDemoWait(); // 800FC010
void fn_800FC064(BOOL on, int idx); // 800FC064: Player action/flag helper; purpose unresolved.
u32 fn_800FC0C0(int idx); // 800FC0C0: Player action/flag helper; purpose unresolved.
BOOL fn_800FC0F8(); // 800FC0F8: Requests player state 0x9B; action meaning unresolved.
BOOL fn_800FC194(const mVec3_c *target); // 800FC194: Requests player state 0x0, 0x61; action meaning unresolved.
BOOL fn_800FC2A4(); // 800FC2A4: Requests player state 0x9B; action meaning unresolved.
BOOL fn_800FC340(); // 800FC340: Requests player state 0x0; action meaning unresolved.
BOOL fn_800FC3B0(); // 800FC3B0: Requests player state 0xA1; action meaning unresolved.
void refreshEquippedUmbrellaDesign(); // 800FC404
BOOL fn_800FC4C0(); // 800FC4C0: Requests player state 0xA4; action meaning unresolved.
BOOL fn_800FC514(); // 800FC514: Requests player state 0xA5; action meaning unresolved.
void lock(); // 800FC568
void unlock(); // 800FC580
void holdToolAction(); // 800FC598
void clearToolActionHold(); // 800FC604
BOOL isToolActionHeld(); // 800FC61C
BOOL enableCurrentActor(); // 800FC630
BOOL refreshEquippedDesign(int slot); // 800FC674
BOOL canStartIdleDemo(); // 800FC7E8
BOOL hasCurrentActor(); // 800FC858
BOOL isNetSaveWaitState(); // 800FC888
void setDemoWait(BOOL a); // 800FC8EC
BOOL fn_800FC8FC(); // 800FC8FC: Requests player state 0x88; action meaning unresolved.
BOOL fn_800FC958(); // 800FC958: Requests player state 0x9E; action meaning unresolved.
BOOL fn_800FC9AC(); // 800FC9AC: Tests player action states; action meaning unresolved.
BOOL beginDemoWait(); // 800FCA04
BOOL canStartMovementDemo(); // 800FCA28
BOOL requestDemoStart(); // 800FCAA0
BOOL showCatch(const dItem::Item *item); // 800FCAF4
BOOL finishShowCatch(); // 800FCBA8
BOOL isShowCatchFinished(); // 800FCBFC
BOOL showCatchAt(const dItem::Item *item, const mVec3_c *pos); // 800FCC74
BOOL fn_800FCD60(u32 a, u32 b, BOOL c); // 800FCD60: Requests player state 0x44; action meaning unresolved.
BOOL changeOutfitAndHeldItem(const dItem::Item *shirt, const dItem::Item *hat, const dItem::Item *held); // 800FCDFC
BOOL eatItem(const dItem::Item *item); // 800FD058
BOOL removeMaskForEating(); // 800FD130
BOOL endDemoWait(); // 800FD17C
BOOL getChairPosition(mVec3_c *out, int idx); // 800FD510
// sitOnChair type: the side of the seat (dFTR::SIDE_*) the player sits down from.
enum {
    SIT_FROM_FRONT,
    SIT_FROM_RIGHT,
    SIT_FROM_LEFT,
};

// lieOnBed type: the side of the bed the player lies down from.
enum {
    LIE_FROM_BACK = 1,
    LIE_FROM_FRONT,
};

BOOL sitOnChair(s16 angle, int type, f32 x, f32 z); // 800FD588
BOOL isChairAnimation(); // 800FD664
BOOL getBedAnimationPosition(mVec3_c *out, int idx); // 800FD6A8
BOOL getBedPosition(mVec3_c *out, int idx); // 800FD720
BOOL lieOnBed(s16 angle, int type, f32 x, f32 z, int arg); // 800FD798; last argument unused in this build
BOOL getUpFromBed(int type, int arg); // 800FD88C; second argument unused in this build
BOOL changeSeatedPose(int type); // 800FD924
BOOL fn_800FD9E0(int type, s16 angle, f32 x, f32 z); // 800FD9E0: Requests player state 0x32; action meaning unresolved.
BOOL fn_800FDAA4(int type); // 800FDAA4: Requests player state 0x34; action meaning unresolved.
BOOL hasActionActor(); // 800FDB10
void fn_800FDB40(); // 800FDB40: Player action/flag helper; purpose unresolved.
BOOL fn_800FDB78(int type); // 800FDB78: Requests player state 0xA; action meaning unresolved.
void fn_800FDBDC(u32 mode); // 800FDBDC: Requests player state 0x7C, 0x22; action meaning unresolved.
BOOL canFinishToolAction(); // 800FDCC0
BOOL isSceneTransitionState(); // 800FDD04
BOOL changeHair(u32 hair, u32 color); // 800FDD8C
BOOL drinkCoffee(); // 800FDE3C
BOOL isCoffeeAnimationFinished(); // 800FDE7C
BOOL finishCoffee(); // 800FDEE8
f32 getCoffeeAnimationFrame(); // 800FDF28
BOOL prepareMiiMask(); // 800FDF7C
BOOL putOnMiiMask(); // 800FE094
BOOL takeOffMiiMask(); // 800FE0D0
BOOL putOnMiiMaskImmediately(); // 800FE140
BOOL changeMiiMask(u16 mii); // 800FE17C
BOOL takeOffMiiMaskImmediately(); // 800FE2D4
u32 canUseMiiMask(); // 800FE360
BOOL fn_800FE39C(s16 angle, int type, int target, f32 x, f32 z); // 800FE39C: Requests player state 0x9, 0xC; action meaning unresolved.
BOOL fn_800FE484(s16 angle, int target, f32 x, f32 z); // 800FE484: Requests player state 0xC; action meaning unresolved.
BOOL fn_800FE550(s16 angle, int type, BOOL useC, f32 x, f32 z); // 800FE550: Requests player state 0xC, 0x9; action meaning unresolved.
BOOL fn_800FE634(); // 800FE634: Tests player animation IDs; animation meaning unresolved.
void getDemoWaitPose(int *outState, mVec3_c *outPos, s16 *outAngle); // 800FE688
BOOL fn_800FE74C(const mVec3_c *pos, int idx, f32 speed); // 800FE74C: moveTo with flag 0x8000 set on _2274; flag meaning unresolved.
BOOL moveTo(const mVec3_c *pos, int idx, f32 speed); // 800FE7E0
BOOL turnTo(s16 angle, int idx); // 800FE88C
BOOL turnToInFrames(s16 angle, int frames, int idx); // 800FE89C
BOOL lookAt(const mVec3_c *target, int a, BOOL pitch, int c); // 800FE938
BOOL isMovingTo(int idx); // 800FEA9C
BOOL isTurningTo(int idx); // 800FEAA8
BOOL isIdle(int idx); // 800FEAB4
BOOL isWalking(int idx); // 800FEAC0
BOOL isWaitingForDemo(int idx); // 800FEACC
BOOL isStartingDemo(int idx); // 800FEAD8
BOOL isState(int state, int idx); // 800FEAE4
int getState(int idx); // 800FEB34
BOOL changeShirt(const dItem::Item *shirt); // 800FEB68
BOOL changeHat(const dItem::Item *hat); // 800FEBE4
BOOL changeAccessory(const dItem::Item *acc); // 800FEC60
BOOL changeHeldItem(const dItem::Item *held); // 800FECF8
BOOL isChangingEquipment(); // 800FEEB0
mVec3_c *getToolOrPlayerPosition(int idx); // 800FEFF4
mVec3_c *getCurrentSoundPositionRaw(); // 800FF048
BOOL getGroundPositionAhead(mVec3_c *out); // 800FF080
u32 fn_800FF150(int idx); // 800FF150: Player action/flag helper; purpose unresolved.
u32 fn_800FF188(int idx); // 800FF188: Player action/flag helper; purpose unresolved.
int getActionUnitAndKind(int *x, int *z, int idx); // 800FF1C0
BOOL fn_800FF354(int *outState); // 800FF354: Requests player state 0x1D, 0x62; action meaning unresolved.
BOOL changeEquipment(const dEquip_c *equip, int mode, int kind); // 800FF444: mode selects the animation; kind selects normal/Mii-mask changes.
BOOL isSceneReadyForBgm(); // 800FF840
BOOL canHoldEquippedItem(); // 800FF8B0
BOOL hasHairFeature(u8 hair, int bit); // 800FF94C
BOOL currentHairHasFeature(int bit); // 800FF978
u8 getShoeColor(u32 idx, u32 gender); // 800FF9C4
BOOL fn_800FFA50(const mVec3_c *a, int b, f32 f); // 800FFA50: Player action/flag helper; purpose unresolved.
BOOL fn_800FFAC0(); // 800FFAC0: Tests player action states; action meaning unresolved.
BOOL fn_800FFB18(); // 800FFB18: Tests player action states; action meaning unresolved.
BOOL fn_800FFBE4(); // 800FFBE4: Tests player action states; action meaning unresolved.
BOOL fn_800FFC38(); // 800FFC38: Tests player action states; action meaning unresolved.
BOOL fn_800FFCA8(); // 800FFCA8: Tests player action states; action meaning unresolved.
BOOL isActionItemState(); // 800FFD10
u16 getActiveActionItem(); // 800FFD64
BOOL isFacingPosition(const mVec3_c *target, int maxAngle, f32 maxDist); // 800FFDC0
void makeCurvedYawMatrix(mMtx_c *mtx, const mVec3_c *position, const mAng *angle); // 800FFEAC
void makeCurvedAngleMatrix(mMtx_c *mtx, const mVec3_c *position, const mAng3_c *angle); // 800FFF2C
u16 getRandomShirt(int gender); // 800FFFCC
void clearEquipmentManagers(); // 8010003C
void clearPlayerEquipmentManagers(int value); // 80100068
BOOL isDesignEquipped(int slot); // 801000AC
BOOL getActionAnimationFrame(f32 *out, int idx); // 80100128
f32 getIntermediateGroundHeight(const mVec3_c *a, const mVec3_c *b, const mVec3_c *c); // 80100194
dActor_c *createActor(int member, int remote, u32 state, const mVec3_c *pos, const mAng3_c *angle); // 80100234
void setPlayerSlot(u32 idx, int value); // 80100238
void clearPlayerSlot(u32 idx); // 80100258
void clearPlayers(); // 8010027C
void clearSaveRequestFlags(); // 801002BC
int getPlayerSlot(u32 idx); // 80100330
BOOL setHatHideBone(u32 bone, dPlayerActor_c *player); // 80100358
void clearDemoFlags(dPlayerActor_c *player); // 80100410
void getActionUnitCoordinates(dPlayerActor_c *player, int *x, int *z); // 8010042C
BOOL receiveItem(const dItem::Item *item, int b, int c, int d, dActor_c *partner, f32 f1, f32 f2, f32 f3); // 80100440
f32 getItemExchangeOffsetY(); // 80100548
BOOL giveItem(const dItem::Item *item, int b, int c, int d, dActor_c *partner, f32 f1, f32 f2); // 80100550
BOOL fn_80100644(BOOL a); // 80100644: Requests player state 0x66, 0x67; action meaning unresolved.
BOOL fn_801006A8(); // 801006A8: Requests player state 0x6E; action meaning unresolved.
BOOL setShoeColor(u8 shoeColor, u8 b); // 80100714
BOOL fn_801007AC(); // 801007AC: Requests player state 0x85; action meaning unresolved.
BOOL fn_80100804(); // 80100804: Requests player state 0x87; action meaning unresolved.
BOOL fn_80100860(); // 80100860: Requests player state 0x1D, 0x6F; action meaning unresolved.
BOOL isUsingNet(int idx); // 801008E4
BOOL isRunAnimation(int idx); // 8010094C
BOOL fn_8010098C(); // 8010098C: Requests player state 0x70; action meaning unresolved.
BOOL playEmotion(u32 type); // 801009E0
BOOL fn_80100A54(); // 80100A54: Requests player state 0x84; action meaning unresolved.
BOOL fn_80100AB0(); // 80100AB0: Tests player animation IDs; animation meaning unresolved.
BOOL fn_80100B24(); // 80100B24: Requests player state 0x84; action meaning unresolved.
u32 fn_80100B80(); // 80100B80: Player action/flag helper; purpose unresolved.
BOOL fn_80100B84(f32 x, f32 z); // 80100B84: Requests player state 0x72; action meaning unresolved.
BOOL fn_80100C28(); // 80100C28: Requests player state 0x73; action meaning unresolved.
BOOL fn_80100C84(BOOL a); // 80100C84: Requests player state 0x74; action meaning unresolved.
BOOL fn_80100D44(); // 80100D44: Tests player action states; action meaning unresolved.
BOOL fn_80100DD0(int idx); // 80100DD0: Tests player action states; action meaning unresolved.
BOOL fn_80100E3C(); // 80100E3C: Requests player state 0x8F; action meaning unresolved.
BOOL fn_80100E7C(); // 80100E7C: Tests player action states; action meaning unresolved.
BOOL fn_80100EC0(); // 80100EC0: Requests player state 0x91; action meaning unresolved.
BOOL fn_80100F00(s16 angle, f32 frame); // 80100F00: Requests player state 0x92; action meaning unresolved.
BOOL fn_80100F98(); // 80100F98: Tests player action states; action meaning unresolved.
BOOL fn_80100FDC(); // 80100FDC: Requests player state 0x94; action meaning unresolved.
u8 fn_8010101C(); // 8010101C: Accesses the manager flag at 194A8; purpose unresolved.
void fn_80101030(); // 80101030: Accesses the manager flag at 194A8; purpose unresolved.
void fn_80101048(); // 80101048: Accesses the manager flag at 194A8; purpose unresolved.
BOOL changeHeldItemForDemo(const dItem::Item *held); // 80101060
BOOL fn_80101168(); // 80101168: Requests player state 0x95; action meaning unresolved.
BOOL fn_801011BC(); // 801011BC: Constant stub; intended use unresolved.
BOOL getScriptedActionPosition(mVec3_c *out); // 801011C4
BOOL startShoeShine(s16 frames, f32 speed); // 8010125C
BOOL finishShoeShine(); // 801012FC
BOOL fn_80101350(); // 80101350: Requests player state 0x76; action meaning unresolved.
BOOL beginDemo(); // 801013B4
u32 *getTimer(); // 801013F4
void resetTimer(); // 80101408
void reset(); // 80101424
BOOL isDailyHairUpdated(); // 80101490
void markDailyHairUpdated(); // 801014A4
BOOL isBedHairPending(); // 801014BC
void setBedHairPending(); // 801014D0
void clearBedHairPending(); // 801014E8
BOOL isDailyTanUpdated(); // 80101500
void markDailyTanUpdated(); // 80101514
u8 fn_8010152C(); // 8010152C: Accesses the manager flag at 194A7; purpose unresolved.
void fn_80101540(); // 80101540: Accesses the manager flag at 194A7; purpose unresolved.
BOOL areSaveRequestFlagsCleared(); // 80101558
void markSaveRequestFlagsCleared(); // 8010156C
BOOL isDesignSlot(int slot, const dItem::Item *item); // 80101584
u32 getHeapSize(); // 801015F4
u32 fn_80101600(); // 80101600: Constant stub; intended use unresolved.
u32 fn_80101608(); // 80101608: Constant stub; intended use unresolved.
u32 fn_80101610(); // 80101610: Constant stub; intended use unresolved.
u32 fn_80101618(); // 80101618: Constant stub; intended use unresolved.
int getCurrentPlayerSlot(); // 801017B8
void clearGuests(); // 801017DC
int findPersonalIDSlot(const dPersonalID_c *pid); // 801017EC
int findPlayerIDSlot(const dPlayerID_c *id); // 801018DC
void setChangeStateCallback(Mgr_c::ChangeStateFn fn, int idx); // 80101990
void setActor(dPlayerActor_c *actor, u32 idx); // 801019A4
dPlayerActor_c *getActor(int idx); // 801019C4
dPlayerActor_c *getActorRaw(int idx); // 80101A28
BOOL changeState(int state, int idx); // 80101A74
void receiveEquipment(int idx, int part, int item); // 80101CB4
void receiveAppearance(int idx, int kind); // 80101CB8
void queueRemoteEquipment(int idx, int part, int item); // 80101CBC
void setRemoteActionParams(int idx, const dPlayerMngr_c::actionParam_c *value); // 80101CC0
void setRemoteActionSync(int idx, int a, int b); // 80101D0C
void setRemoteSyncValue(int idx, int value); // 80101D70
void cacheRemoteState(int idx, int state); // 80101DA8
void cacheRemoteParams(int idx, const u8 *value); // 80101DAC
BOOL changeNetPlayerState(int idx, int state); // 80101DB0
BOOL getRemoteTransform(void *position, void *angle, void *flags, u32 member); // 80101DC0
BOOL getRemoteStateInfo(void *info, u8 *scene, u8 *b, u32 idx); // 80101DC4
void cacheUnavailableActorAction(int idx, u8 b, int state, const u8 *param); // 80101DC8
void cacheOtherSceneAction(int idx, u8 b, int state, const u8 *param); // 80101E3C
BOOL updateActionFieldTasks(int idx, u8 b, int state, const u8 *param); // 80101EB0
BOOL isSyncValueAhead(int a, int b); // 80102138
BOOL isActionParamLocked(u32 state); // 801021B0
int getWearSound(const dItem::Item *a, const dItem::Item *b); // 801021D0
void onParticleCollision(nw4r::ef::Particle *obj, u32 kind); // 801022B8
int getSurfaceEffectKind(const mVec3_c *pos); // 80102434
void createSurfaceEffects(const char *const *names, const mVec3_c *pos, const mAng3_c *angle,
                          const char *wetEffect, const char *snowEffect); // 801024F8
BOOL fn_801025B8(); // 801025B8: Player action/flag helper; purpose unresolved.
} // namespace dPlayerMgr
