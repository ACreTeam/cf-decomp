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

// The player actor: include/game/game/d_a_player.hpp.
class dAcPlayer_c;

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
    STATE_ENTER_DOOR = 0x09,         // d_a_str (door_knob_in)
    STATE_EXIT_DOOR = 0x0A,          // d_a_str at scene start (door_knob_out)
    STATE_WALK_THROUGH_DOOR = 0x0C,  // d_a_str
    STATE_FISHING_WAIT = 0x1A,       // the bobber is in the water
    STATE_FISHING_PUT_BACK = 0x1D,
    STATE_PUT_AWAY_TOOL = 0x22,      // before a talk
    STATE_LIE_DOWN = 0x2A,
    STATE_GET_UP = 0x2C,
    STATE_BED_WAIT = 0x2E,
    STATE_CHEST_OPEN = 0x32,
    STATE_CHEST_WAIT = 0x33,
    STATE_CHEST_CLOSE = 0x34,
    STATE_SIT_DOWN = 0x35,
    STATE_CHAIR_WAIT = 0x38,
    STATE_CHAIR_POSE = 0x39,         // changeSeatedPose
    STATE_SHAKE_TREE = 0x3C,         // dFgobjManager::startShakeTree
    STATE_SHAKE_TREE_END = 0x3D,
    STATE_PUT_ITEM_IN_HOLE = 0x44,
    STATE_EXIT_ROOM = 0x49,          // requestExit
    STATE_ENTER_ROOM = 0x4A,         // at scene start
    STATE_MII_MASK_IMMEDIATE = 0x4B,
    STATE_MII_MASK = 0x4C,
    STATE_CHANGE_DESIGN = 0x4D,      // changeEquipment
    STATE_CHANGE_EQUIPMENT_QUICK = 0x4E, // changeEquipment mode 0
    STATE_CHANGE_HAIR = 0x4F,
    STATE_CHANGE_MII_MASK = 0x50,
    STATE_PICK_UP = 0x5D,            // the item goes into an empty pocket
    STATE_SHOW_CATCH = 0x5F,
    STATE_SHOW_CATCH_AT = 0x60,
    STATE_SURPRISED = 0x61,          // turns to a target, then an emotion bubble
    STATE_BEE_STING = 0x62,
    STATE_RECEIVE_ITEM = 0x63,
    STATE_HAND_ITEM_REACH = 0x66,    // dHandItem
    STATE_HAND_ITEM_TAKE = 0x67,
    STATE_GIVE_ITEM = 0x6A,
    STATE_HAND_ITEM_RETURN = 0x6E,
    STATE_INSECT_BITE = 0x6F,
    STATE_CHAT = 0x70,               // d_menu_chat
    STATE_EMOTION = 0x71,
    STATE_MAILBOX_OPEN = 0x72,
    STATE_MAILBOX_CLOSE = 0x73,
    STATE_FAINT = 0x74,              // af_ply_kizetsu
    STATE_TURN_TO_TALKER = 0x76,
    STATE_EAT = 0x78,
    STATE_EAT_REMOVE_MASK = 0x79,
    STATE_CLOSE_UMBRELLA = 0x7C,
    STATE_REACT = 0x84,              // dAcPlayer_c::mReactionKind (REACTION_*)
    STATE_PHONE_PICK_UP = 0x85,      // d_a_rmobj_tel
    STATE_PHONE_TALK = 0x86,
    STATE_PHONE_HANG_UP = 0x87,
    STATE_FLINCH = 0x88,             // Resetti
    STATE_TRICK_CHANGE = 0x8B,       // EQUIPMENT_CHANGE_TRICK (af_ply_trick)
    STATE_SCENE_START = 0x8C,
    STATE_BUS_BOARD = 0x8F,          // daStrBus_c
    STATE_BUS_BOARDED = 0x90,
    STATE_BUS_RIDE = 0x91,
    STATE_BUS_GET_OFF = 0x92,
    STATE_BUS_GOT_OFF = 0x93,
    STATE_BUS_LEAVE = 0x94,
    STATE_GODDESS_ITEM = 0x95,       // the fountain goddess returns the axe
    STATE_EAT_SPECIAL = 0x97,
    STATE_CHANGE_HELD_ITEM_DEMO = 0x98,
    STATE_SLINGSHOT_SHOOT = 0x9A,
    STATE_SLINGSHOT_WATCH = 0x9B,    // a shot balloon / UFO falls
    STATE_DRINK_COFFEE = 0x9C,
    STATE_FINISH_COFFEE = 0x9D,
    STATE_TURN_FRONT = 0x9E,
    STATE_BED_HAIR = 0x9F,
    STATE_CHANGE_OUTFIT_AND_HELD = 0xA0,
    STATE_LISTEN_KK = 0xA1,
    STATE_FORTUNE_HIT = 0xA4,        // af_ply_fortune_hit
    STATE_FORTUNE_FAINT = 0xA5,
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
    EQUIPMENT_CHANGE_TRICK = 6, // a Halloween trick (STATE_TRICK_CHANGE)
};

enum AppearanceChange {
    APPEARANCE_CLEAR_BEE_STING = 0x10, // clear PRIVATE_FLAG0_BEE_STUNG (removeMaskForEating)
    APPEARANCE_MII_MASK_ON = 0x11,
    APPEARANCE_MII_MASK_OFF = 0x12,
};

// dAcPlayer_c::mActorFlags. Unnamed bits are still literals.
enum ActorFlag {
    ACTOR_FLAG_DEMO = 0x1,               // cleared by clearDemoFlags
    ACTOR_FLAG_TOOL_OUT = 0x10,          // a tool is held ready
    ACTOR_FLAG_TOOL_ACTIVE = 0x20,       // the tool is in use (canFinishToolAction)
    ACTOR_FLAG_LOOK_AT = 0x40,
    ACTOR_FLAG_KEEP_ANIMATION = 0x100,   // skip the idle animation reset; cleared on a state change
    ACTOR_FLAG_MII_MASK_OK = 0x400,      // canUseMiiMask
    ACTOR_FLAG_NOISY = 0x2000,           // tool swings, digging, tumbling, fireworks (isMakingNoise)
    ACTOR_FLAG_INSECT = 0x8000,
    ACTOR_FLAG_SCRIPTED_MOVE = 0x100000, // an NPC walks the player (setScriptedMove)
    ACTOR_FLAG_RESUME_BGM = 0x200000,    // isSceneReadyForBgm
    ACTOR_FLAG_DISABLED = 0x2000000,
    ACTOR_FLAG_POINTER_WALK = 0x40000000, // cleared every input frame; isPointerWalking
};

// dAcPlayer_c::mActorFlags2. Unnamed bits are still literals.
enum ActorFlag2 {
    ACTOR_FLAG2_UNIT_ACTION = 0x80,      // isActingOnUnit
    ACTOR_FLAG2_SHOT_BALLOON = 0x100,    // watchShotBalloon
    ACTOR_FLAG2_SHOT_UFO = 0x200,        // watchShotUfo
    ACTOR_FLAG2_COCKROACH = 0x400,       // set with markCockroachNoticed; STATE_SURPRISED at the door
    ACTOR_FLAG2_ESCORTED = 0x8000,       // moveToEscorted
};

// dAcPlayer_c::mReactionKind: the STATE_REACT variant.
enum Reaction {
    REACTION_DEBT_PAID = 0,  // reactDebtPaid
    REACTION_JOB_DONE = 1,   // Nook's part-time job is done
    REACTION_COCKROACH = 6,  // reactToCockroaches
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
    /* 0x00024 */ dAcPlayer_c *mActors[PLAYER_INDEX_COUNT];    // set by setActor
    /* 0x00034 */ int mSlots[PLAYER_INDEX_COUNT];                 // player index per network member (getPlayerSlot), 7 = none
    /* 0x00044 */ u8 _44[0x1C];
    /* 0x00060 */ GuestPlayers_c mGuests;
    /* 0x194A0 */ u32 mTimer;                    // 0xD2F0 (resetTimer)
    /* 0x194A4 */ u8 mDailyHairUpdated;
    /* 0x194A5 */ u8 mBedHairPending;
    /* 0x194A6 */ u8 mDailyTanUpdated;
    /* 0x194A7 */ u8 mCockroachNoticed; // markCockroachNoticed
    /* 0x194A8 */ u8 mAxeInFountain;     // setAxeInFountain
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
dAcPlayer_c *getPlayerActor(int idx); // 800FBC7C
dAcPlayer_c *getPlayerActorRaw(int idx); // 800FBC80
dAcPlayer_c *getNpcTargetActor(int idx); // 800FBC84
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
void setScriptedMove(BOOL on, int idx); // 800FC064: ACTOR_FLAG_SCRIPTED_MOVE (the gatekeeper / the lost child walk the player)
u32 isScriptedMove(int idx); // 800FC0C0: d_net_sync ignores remote states meanwhile
BOOL watchShotBalloon(); // 800FC0F8: a shot balloon pops: STATE_SLINGSHOT_SHOOT -> STATE_SLINGSHOT_WATCH
BOOL landBalloonPresent(const mVec3_c *splashPos); // 800FC194: the present landed (d_fgobj_manager); STATE_SURPRISED at a water splash
BOOL watchShotUfo(); // 800FC2A4: a shot UFO falls (d_a_ufo)
BOOL finishWatchUfo(); // 800FC340: the UFO flew off: idle
BOOL listenToKK(); // 800FC3B0: STATE_LISTEN_KK (d_a_npc_sp_totakeke)
void refreshEquippedUmbrellaDesign(); // 800FC404
BOOL startFortuneHit(); // 800FC4C0: STATE_FORTUNE_HIT (d_a_npc_sp_fortune)
BOOL startFortuneFaint(); // 800FC514: STATE_FORTUNE_FAINT after STATE_FORTUNE_HIT
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
BOOL flinch(); // 800FC8FC: STATE_FLINCH (Resetti shouts)
BOOL turnToFront(); // 800FC958: STATE_TURN_FRONT (dAcNpc_c::recept_c::stepPlayerTurnStart)
BOOL isFacingFront(); // 800FC9AC: STATE_TURN_FRONT done (angle 0x8000)
BOOL beginDemoWait(); // 800FCA04
BOOL canStartMovementDemo(); // 800FCA28
BOOL requestDemoStart(); // 800FCAA0
BOOL showCatch(const dItem::Item *item); // 800FCAF4
BOOL finishShowCatch(); // 800FCBA8
BOOL isShowCatchFinished(); // 800FCBFC
BOOL showCatchAt(const dItem::Item *item, const mVec3_c *pos); // 800FCC74
BOOL putItemInHole(u32 unitX, u32 unitZ, BOOL fromTask); // 800FCD60: STATE_PUT_ITEM_IN_HOLE (d_menu_item00 / d_menu_item03)
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
BOOL openChest(int type, s16 angle, f32 x, f32 z); // 800FD9E0: STATE_CHEST_OPEN (daFtrChest_c)
BOOL closeChest(int type); // 800FDAA4: STATE_CHEST_CLOSE
BOOL hasActionActor(); // 800FDB10
void keepAnimation(); // 800FDB40: ACTOR_FLAG_KEEP_ANIMATION (furniture that starts a talk)
BOOL exitDoor(int type); // 800FDB78: STATE_EXIT_DOOR (d_a_str at scene start)
void putAwayToolForTalk(u32 mode); // 800FDBDC: dDemo::startTalk: STATE_PUT_AWAY_TOOL / STATE_CLOSE_UMBRELLA
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
BOOL enterDoor(s16 angle, int type, int target, f32 x, f32 z); // 800FE39C: STATE_ENTER_DOOR, STATE_WALK_THROUGH_DOOR with a target (d_a_str)
BOOL walkThroughDoor(s16 angle, int target, f32 x, f32 z); // 800FE484: STATE_WALK_THROUGH_DOOR (d_a_str)
BOOL enterDoorDirect(s16 angle, int type, BOOL walkThrough, f32 x, f32 z); // 800FE550: STATE_ENTER_DOOR / STATE_WALK_THROUGH_DOOR (d_a_str)
BOOL isDoorAnimation(); // 800FE634: animation 5 or 8 (d_a_str)
void getDemoWaitPose(int *outState, mVec3_c *outPos, s16 *outAngle); // 800FE688
BOOL moveToEscorted(const mVec3_c *pos, int idx, f32 speed); // 800FE74C: moveTo with ACTOR_FLAG2_ESCORTED (the gatekeeper)
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
u32 isMakingNoise(int idx); // 800FF150: ACTOR_FLAG_NOISY: scares insects and fish
u32 isActingOnUnit(int idx); // 800FF188: ACTOR_FLAG2_UNIT_ACTION (dInsectUtil::playerCheck_c)
int getActionUnitAndKind(int *x, int *z, int idx); // 800FF1C0
BOOL requestBeeSting(int *outState); // 800FF354: STATE_BEE_STING; ends fishing first (d_insect_field)
BOOL changeEquipment(const dEquip_c *equip, int mode, int kind); // 800FF444: mode selects the animation; kind selects normal/Mii-mask changes.
BOOL isSceneReadyForBgm(); // 800FF840
BOOL canHoldEquippedItem(); // 800FF8B0
BOOL hasHairFeature(u8 hair, int bit); // 800FF94C
BOOL currentHairHasFeature(int bit); // 800FF978
u8 getShoeColor(u32 idx, u32 gender); // 800FF9C4
BOOL isATriggeredFacing(const mVec3_c *pos, int maxAngle, f32 maxDist); // 800FFA50: A pressed while isFacingPosition
BOOL isPointerWalking(); // 800FFAC0: walking with ACTOR_FLAG_POINTER_WALK (d_obj_cursor, d_clock)
BOOL isToolState(); // 800FFB18: net, axe, rod, shovel, slingshot, dandelion, fireworks, bubbles
BOOL isShakingTree(); // 800FFBE4: STATE_SHAKE_TREE / STATE_SHAKE_TREE_END
BOOL isPickUpState(); // 800FFC38: STATE_PICK_UP and its variants
BOOL isFurnitureState(); // 800FFCA8: holding / moving furniture, bed and chair states
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
BOOL setHatHideBone(u32 bone, dAcPlayer_c *player); // 80100358
void clearDemoFlags(dAcPlayer_c *player); // 80100410
void getActionUnitCoordinates(dAcPlayer_c *player, int *x, int *z); // 8010042C
BOOL receiveItem(const dItem::Item *item, int b, int c, int d, dActor_c *partner, f32 f1, f32 f2, f32 f3); // 80100440
f32 getItemExchangeOffsetY(); // 80100548
BOOL giveItem(const dItem::Item *item, int b, int c, int d, dActor_c *partner, f32 f1, f32 f2); // 80100550
BOOL passHandItem(BOOL reach); // 80100644: STATE_HAND_ITEM_REACH / STATE_HAND_ITEM_TAKE (dAcNpc_c::action_c)
BOOL returnHandItem(); // 801006A8: STATE_HAND_ITEM_RETURN (dAcNpc_c::action_c)
BOOL setShoeColor(u8 shoeColor, u8 b); // 80100714
BOOL pickUpPhone(); // 801007AC: STATE_PHONE_PICK_UP (d_a_rmobj_tel)
BOOL hangUpPhone(); // 80100804: STATE_PHONE_HANG_UP after STATE_PHONE_TALK
BOOL requestInsectBite(); // 80100860: STATE_INSECT_BITE (d_insect_field, d_insect_museum)
BOOL isUsingNet(int idx); // 801008E4
BOOL isRunAnimation(int idx); // 8010094C
BOOL startChat(); // 8010098C: STATE_CHAT (d_menu_chat)
BOOL playEmotion(u32 type); // 801009E0
BOOL reactDebtPaid(); // 80100A54: STATE_REACT, REACTION_DEBT_PAID (daFtrAtm_c)
BOOL isReactionFinished(); // 80100AB0: STATE_REACT's animation (0x95) stopped
BOOL reactToCockroaches(); // 80100B24: STATE_REACT, REACTION_COCKROACH (d_cockroach_manager)
u32 isCockroachReactionFinished(); // 80100B80: isReactionFinished (d_cockroach_manager)
BOOL openMailbox(f32 x, f32 z); // 80100B84: STATE_MAILBOX_OPEN (daStrPost_c, before MENU_POST00)
BOOL closeMailbox(); // 80100C28: STATE_MAILBOX_CLOSE after the post menu
BOOL requestFaint(BOOL a); // 80100C84: STATE_FAINT (d_insect_field)
BOOL isSafeFromInsects(); // 80100D44: demo, pitfall or ACTOR_FLAG_DEMO: insects leave the player alone
BOOL isScaringCockroaches(int idx); // 80100DD0: walking, running, pushing or pulling (d_a_cockroach)
BOOL boardBus(); // 80100E3C: STATE_BUS_BOARD (daStrBus_c)
BOOL isOnBus(); // 80100E7C: STATE_BUS_BOARDED
BOOL rideBus(); // 80100EC0: STATE_BUS_RIDE
BOOL getOffBus(s16 angle, f32 frame); // 80100F00: STATE_BUS_GET_OFF
BOOL isOffBus(); // 80100F98: STATE_BUS_GOT_OFF
BOOL leaveBus(); // 80100FDC: STATE_BUS_LEAVE
u8 isAxeInFountain(); // 8010101C: Mgr_c::mAxeInFountain (the goddess appears)
void setAxeInFountain(); // 80101030: the player REL, when an axe drops into the fountain
void clearAxeInFountain(); // 80101048
BOOL changeHeldItemForDemo(const dItem::Item *held); // 80101060
BOOL receiveGoddessItem(); // 80101168: STATE_GODDESS_ITEM (d_a_npc_sp_goddess)
BOOL canGoddessLeave(); // 801011BC: always TRUE (d_a_npc_sp_goddess)
BOOL getScriptedActionPosition(mVec3_c *out); // 801011C4
BOOL startShoeShine(s16 frames, f32 speed); // 8010125C
BOOL finishShoeShine(); // 801012FC
BOOL turnToTalkActor(); // 80101350: STATE_TURN_TO_TALKER (d_a_npc_out)
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
u8 isCockroachNoticed(); // 8010152C: Mgr_c::mCockroachNoticed
void markCockroachNoticed(); // 80101540: the player REL, entering a house with cockroaches
BOOL areSaveRequestFlagsCleared(); // 80101558
void markSaveRequestFlagsCleared(); // 8010156C
BOOL isDesignSlot(int slot, const dItem::Item *item); // 80101584
u32 getHeapSize(); // 801015F4
u32 getGeneralHeapSize(); // 80101600: the player REL's general heap
u32 getHeadHeapSize(); // 80101608: the player REL's head heap
u32 getAccessoryHeapSize(); // 80101610: the player REL's accessory heap
u32 getMiiHeapSize(); // 80101618: dAcPlayer_c::m_heap_Mii_p
int getCurrentPlayerSlot(); // 801017B8
void clearGuests(); // 801017DC
int findPersonalIDSlot(const dPersonalID_c *pid); // 801017EC
int findPlayerIDSlot(const dPlayerID_c *id); // 801018DC
void setChangeStateCallback(Mgr_c::ChangeStateFn fn, int idx); // 80101990
void setActor(dAcPlayer_c *actor, u32 idx); // 801019A4
dAcPlayer_c *getActor(int idx); // 801019C4
dAcPlayer_c *getActorRaw(int idx); // 80101A28
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
BOOL isBusy(); // 801025B8: always FALSE (d_s_boot_static)
} // namespace dPlayerMgr
