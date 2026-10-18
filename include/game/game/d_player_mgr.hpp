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
    STATE_RETURN_HOME = 0x0B,        // collapse outdoors / leave by the door; then to the house (d_a_player)
    STATE_WALK_THROUGH_DOOR = 0x0C,  // d_a_str
    STATE_NET_SWING = 0x0D,          // d_a_player: HMN_ANM_NET_SWING1, the net's objc cylinders
    STATE_NET_CATCH = 0x0E,          // d_a_player: shows the caught insect (catalog, activity log)
    STATE_NET_POCKET = 0x0F,         // d_a_player: the insect shrinks into the pocket
    STATE_AXE_SWING = 0x10,          // d_a_player: HMN_ANM_AXE_SWING1, the axe's objc cylinder; checkAxeHit
    STATE_AXE_STRIKE = 0x11,         // HMN_ANM_AXE_SWING_WAIT1: dFgobjManager::useTool, then 0x12..0x15 / 0x17
    STATE_AXE_CHOP = 0x12, // af_itm_axe_woodchip
    STATE_AXE_HIT = 0x13,            // HMN_ANM_AXE_HANE1: dFgobjManager::procChopResult
    STATE_AXE_MISS = 0x14,           // HMN_ANM_AXE_SUKA1
    STATE_AXE_BREAK = 0x15, // af_itm_axe_break; then 0x16
    STATE_AXE_BREAK_MSG = 0x16,      // HMN_ANM_AXE_BROKE2: SYS_Player 0x10
    STATE_AXE_HIT_OBJECT = 0x17,     // the axe hits an objc (mObjcPos): HMN_ANM_AXE_HANE1 (else 0x11), steps back 8 (d_a_player)
    STATE_FISHING_CAST_MISS = 0x18,  // d_a_player: HMN_ANM_POLE_SUKA1, setPoleSuka (the cast misses the water)
    STATE_FISHING_CAST = 0x19,       // d_a_player: HMN_ANM_POLE_SWING1, setPoleSwing; STATE_FISHING_WAIT in water
    STATE_FISHING_WAIT = 0x1A,       // the bobber is in the water
    STATE_FISHING_HOOKED = 0x1B,     // d_a_player: a fish bit (dFishFldShadow_c::pull), setPoleHit
    STATE_FISHING_FAIL = 0x1C,       // the fish got away (setPoleFail) (d_a_player)
    STATE_FISHING_PUT_BACK = 0x1D,
    STATE_FISHING_LAND = 0x1E,       // the hooked fish is pulled out (setPoleGet1) (d_a_player)
    STATE_FISHING_GET = 0x1F,        // the fish is shown: SYS_Get_fish (d_a_player)
    STATE_FISHING_PUT_AWAY = 0x20,   // the fish goes into the pockets (setPolePutaway) (d_a_player)
    STATE_TAKE_OUT_TOOL = 0x21, // HMN_ANM_TOOL_PUTAWAY1 reversed; the tool scale grows (d_a_player)
    STATE_PUT_AWAY_TOOL = 0x22,      // before a talk
    STATE_FTR_GRAB = 0x23,           // take hold of furniture (dFTR::holdList_c::startHold) (d_a_player)
    STATE_FTR_HOLD = 0x24,           // holding furniture; the stick picks the move
    STATE_FTR_ROTATE = 0x25,
    STATE_FTR_PUSH_START = 0x26,
    STATE_FTR_PUSH = 0x27,
    STATE_FTR_PULL_START = 0x28,
    STATE_FTR_PULL = 0x29,
    STATE_LIE_DOWN = 0x2A,
    STATE_GET_INTO_BED = 0x2B,       // after the STATE_LIE_DOWN request (d_a_player)
    STATE_GET_UP = 0x2C,
    STATE_GET_OUT_OF_BED = 0x2D,     // after the STATE_GET_UP request (d_a_player)
    STATE_BED_WAIT = 0x2E,
    STATE_BED_ROLL = 0x2F,           // ask to roll over in the bed (dFTR::requestMove) (d_a_player)
    STATE_BED_ROLL_OVER = 0x30,
    STATE_BED_ROLL_BLOCKED = 0x31,
    STATE_CHEST_OPEN = 0x32,
    STATE_CHEST_WAIT = 0x33,
    STATE_CHEST_CLOSE = 0x34,
    STATE_SIT_DOWN = 0x35,
    STATE_SIT_DOWN_FRONT = 0x36,     // after the STATE_SIT_DOWN request (d_a_player)
    STATE_SIT_DOWN_SIDE = 0x37,
    STATE_CHAIR_WAIT = 0x38,
    STATE_CHAIR_POSE = 0x39,         // changeSeatedPose
    STATE_CHAIR_STAND_UP = 0x3A, // HMN_ANM_CHAIR_STANDUPF1 (d_a_player)
    STATE_CHAIR_FACE_FRONT = 0x3B, // HMN_ANM_CHAIR_STANDUPL1 / HMN_ANM_CHAIR_STANDUPR1 by mSeatPose; then STATE_CHAIR_STAND_UP
    STATE_SHAKE_TREE = 0x3C,         // dFgobjManager::startShakeTree
    STATE_SHAKE_TREE_END = 0x3D,
    STATE_SHOVEL_START = 0x3E, // the swing; the result in _2208
    STATE_SHOVEL_WAIT = 0x3F, // dFgobjManager::useTool; dispatches on the task result
    STATE_SHOVEL_DIG = 0x40, // spawnDigUp
    STATE_SHOVEL_HIT = 0x41, // HMN_ANM_SCOOP_HANE1
    STATE_SHOVEL_MISS = 0x42, // HMN_ANM_SCOOP_CANCEL1
    STATE_SHOVEL_SWING_LOW = 0x43, // HMN_ANM_SCOOP_SUKA1
    STATE_PUT_ITEM_IN_HOLE = 0x44,
    STATE_SHOVEL_BURY = 0x45, // HMN_ANM_SCOOP_BURY1
    STATE_SHOVEL_DIG_UP_ITEM = 0x46, // HMN_ANM_SCOOP_GET1, af_ply_dig_b_*
    STATE_SHOW_DUG_ITEM = 0x47,      // d_a_player: HMN_ANM_SCOOP_PUTAWAY1, the dug-up item shrinks into the pockets
    STATE_SHOVEL_HIT_OBJECT = 0x48,  // the shovel hits an objc (mObjcPos / mObjcPos2): HMN_ANM_SCOOP_HANE1, a hit villager reacts (vt68) (d_a_player)
    STATE_EXIT_ROOM = 0x49,          // requestExit
    STATE_ENTER_ROOM = 0x4A,         // at scene start
    STATE_MII_MASK_IMMEDIATE = 0x4B,
    STATE_MII_MASK = 0x4C,
    STATE_CHANGE_DESIGN = 0x4D,      // changeEquipment
    STATE_CHANGE_EQUIPMENT_QUICK = 0x4E, // changeEquipment mode 0
    STATE_CHANGE_HAIR = 0x4F,
    STATE_CHANGE_MII_MASK = 0x50,
    STATE_PICK_UP_REACH = 0x51,      // d_a_player: HMN_ANM_PICKUP_WAIT1, waits for the fgMngTask (_229C)
    STATE_PICK_UP_ITEM = 0x52,       // HMN_ANM_PICKUP1: the field item flies to the body (fgMngTask_procAt)
    STATE_PICK_UP_ITEM_MSG = 0x53,   // HMN_ANM_PICKUP_GET1: lift it up, SYS_Player 0xF / 0x2C / 0x2E / 0x2F
    STATE_SHOW_PICKED_ITEM = 0x54,   // HMN_ANM_PICKUP_GET2
    STATE_PULL_WEED = 0x55,          // HMN_ANM_PICKUP_WEEDS1: weeds / clovers (playGrassEffect)
    STATE_FILL_HOLE_WAIT = 0x56,     // d_a_player: hole_bury_wait1, turns to the hole, waits for dFgobjManager::fillHole
    STATE_STAMP_HOLE = 0x57, // af_ply_footbury_*, spawnFillHole
    STATE_PITFALL_TRIGGER = 0x58,    // waits for the buried seed's fgMngTask (_229C)
    STATE_PITFALL_FALL = 0x59, // af_ply_pitfall_*
    STATE_PITFALL_STRUGGLE = 0x5A,   // HMN_ANM_PITFALL_FALL3: stick / buttons / dPad::isShakeHeld
    STATE_PITFALL_CLIMB_OUT = 0x5B, // af_ply_pitout_*
    STATE_POCKET_FULL = 0x5C,        // HMN_ANM_PUTAWAY_WAIT1: SYS_Player 0xD, then the menus 0x25..0x28
    STATE_PICK_UP = 0x5D,            // the item goes into an empty pocket
    STATE_SWAP_ITEM = 0x5E,          // d_a_player: dFgobjManager::swapItem (_218C), then STATE_IDLE
    STATE_SHOW_CATCH = 0x5F,
    STATE_SHOW_CATCH_AT = 0x60,
    STATE_SURPRISED = 0x61,          // turns to a target, then an emotion bubble
    STATE_BEE_STING = 0x62,
    STATE_RECEIVE_ITEM = 0x63,
    STATE_PASS_ITEM = 0x64,          // d_a_player: trans_pass3; dHandItem STATE_FALL
    STATE_PASS_ITEM_WAIT = 0x65,     // trans_pass_wait1, until the partner holds the item
    STATE_HAND_ITEM_REACH = 0x66,    // dHandItem
    STATE_HAND_ITEM_TAKE = 0x67,
    STATE_HAND_ITEM_HOLD = 0x68,     // trans_receive2; dHandItem STATE_SHRINK
    STATE_HAND_ITEM_PUT_AWAY = 0x69, // trans_putaway1 (gold net / rod: mReactionKind)
    STATE_GIVE_ITEM = 0x6A,
    STATE_MAIL_RECEIVE = 0x6B,       // d_a_player: mail_receive1
    STATE_MAIL_RECEIVE_END = 0x6C,   // mail_receive2
    STATE_MAIL_PUT_AWAY = 0x6D,      // mail_putaway1
    STATE_HAND_ITEM_RETURN = 0x6E,
    STATE_INSECT_BITE = 0x6F,
    STATE_CHAT = 0x70,               // d_menu_chat
    STATE_EMOTION = 0x71,
    STATE_MAILBOX_OPEN = 0x72,
    STATE_MAILBOX_CLOSE = 0x73,
    STATE_FAINT = 0x74,              // af_ply_kizetsu
    STATE_FAINT_GET_UP = 0x75,       // sting_up1 at the house after STATE_FAINT (requestHomeScene)
    STATE_TURN_TO_TALKER = 0x76,
    STATE_TRIP = 0x77,               // bad luck: a random fall while walking fast (d_a_player)
    STATE_EAT = 0x78,
    STATE_EAT_REMOVE_MASK = 0x79,
    STATE_CATCH_POSE = 0x7A,         // holds up the netted insect / hooked fish (HMN_ANM_NET_COMPLETE1 / HMN_ANM_POLE_COMPLETE1), the jingle (d_a_player)
    STATE_OPEN_UMBRELLA = 0x7B,      // HMN_ANM_UMB_OPEN1 (d_a_player)
    STATE_CLOSE_UMBRELLA = 0x7C,
    STATE_SHAKE_UMBRELLA = 0x7D,     // HMN_ANM_UMB_ROTATE1, "af_itm_umbrella" in the rain (d_a_player)
    STATE_PICK_UP_UNIT_ITEM = 0x7E,  // the target unit's item, ground (HMN_ANM_PICKUP1) or upper layer (0x70) (d_a_player)
    STATE_STAIRS_TURN = 0x7F,        // turn to mTargetAngle, then 0x80 / 0x82 by mExitType (d_a_player)
    STATE_GO_UPSTAIRS = 0x80,        // stair_up1, then dScene requestExit (d_a_player)
    STATE_ARRIVE_UPSTAIRS = 0x81,    // stair_up2 (fade in)
    STATE_GO_DOWNSTAIRS = 0x82,      // stair_down1
    STATE_ARRIVE_DOWNSTAIRS = 0x83,  // stair_down2
    STATE_REACT = 0x84,              // dAcPlayer_c::mReactionKind (REACTION_*)
    STATE_PHONE_PICK_UP = 0x85,      // d_a_rmobj_tel
    STATE_PHONE_TALK = 0x86,
    STATE_PHONE_HANG_UP = 0x87,
    STATE_FLINCH = 0x88,             // Resetti
    STATE_WATER = 0x89,              // d_a_player: jyoro_raise1, af_itm_jyoro_water
    STATE_BLOW_DANDELION = 0x8A,     // tanpopo_fly1, af_itm_tanpopo
    STATE_TRICK_CHANGE = 0x8B,       // EQUIPMENT_CHANGE_TRICK (af_ply_trick)
    STATE_SCENE_START = 0x8C,
    STATE_FIREWORKS = 0x8D,          // fire_setoff1, af_itm_firework (d_a_player)
    STATE_CRACKER = 0x8E,            // cracker_setoff1, af_itm_cracker
    STATE_BUS_BOARD = 0x8F,          // daStrBus_c
    STATE_BUS_BOARDED = 0x90,
    STATE_BUS_RIDE = 0x91,
    STATE_BUS_GET_OFF = 0x92,
    STATE_BUS_GOT_OFF = 0x93,
    STATE_BUS_LEAVE = 0x94,
    STATE_GODDESS_ITEM = 0x95,       // the fountain goddess returns the axe
    STATE_RELEASE_BALLOON = 0x96,    // the held balloon flies off; the player watches it (after STATE_TRIP) (d_a_player)
    STATE_EAT_SPECIAL = 0x97,
    STATE_CHANGE_HELD_ITEM_DEMO = 0x98,
    STATE_SLINGSHOT_AIM = 0x99,      // HMN_ANM_SLING_SHOOT1, setSlingShoot1/2, turns to 0x8000, camera mode 0x17; STATE_SLINGSHOT_SHOOT on release (d_a_player)
    STATE_SLINGSHOT_SHOOT = 0x9A,
    STATE_SLINGSHOT_WATCH = 0x9B,    // a shot balloon / UFO falls
    STATE_DRINK_COFFEE = 0x9C,
    STATE_FINISH_COFFEE = 0x9D,
    STATE_TURN_FRONT = 0x9E,
    STATE_BED_HAIR = 0x9F,
    STATE_CHANGE_OUTFIT_AND_HELD = 0xA0,
    STATE_LISTEN_KK = 0xA1,
    STATE_BLOW_BUBBLES = 0xA2,       // tanpopo_fly1, af_itm_syabon (d_a_player)
    STATE_PUT_HELD_ITEM = 0xA3,      // d_a_player: dFgobjManager::putHeldItem (mActionItem), then STATE_IDLE
    STATE_FORTUNE_HIT = 0xA4,        // af_ply_fortune_hit
    STATE_FORTUNE_FAINT = 0xA5,
    STATE_SHOE_SHINE_START = 0xA6,
    STATE_SHOE_SHINE = 0xA7,
    STATE_SHOE_SHINE_END = 0xA8,
    STATE_PUT_AWAY_TOOL_FULL = 0xA9, // HMN_ANM_TOOL_PUTAWAY1 put-away, HMN_ANM_PUTAWAY_WAIT1 + SYS_Player 0x13 (pockets full), the tool comes back out (d_a_player)
    STATE_SCENE_START_HIDDEN = 0xAA, // as STATE_SCENE_START, but the end keeps ACTOR_FLAG_DISABLED (d_a_player)
    STATE_NONE = 0xAB,
    STATE_COUNT = 0xAB,
    STATE_FROM_REMOTE = 0xAD, // start state (actor param) only: take the remote player's synced state
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

// dAcPlayer_c::mActorFlags.
enum ActorFlag {
    ACTOR_FLAG_DEMO = 0x1,                    // cleared by clearDemoFlags
    ACTOR_FLAG_HAIR_MDL = 0x2,                // mHairMdl is loaded
    ACTOR_FLAG_HEAD_MDL = 0x4,                // mHeadMdl is loaded (the face animations need it)
    ACTOR_FLAG_ACCESSORY_MDL = 0x8,           // mAccessoryMdl is loaded
    ACTOR_FLAG_TOOL_OUT = 0x10,               // a tool is held ready
    ACTOR_FLAG_TOOL_ACTIVE = 0x20,            // the tool is in use (canFinishToolAction)
    ACTOR_FLAG_LOOK_AT = 0x40,
    ACTOR_FLAG_FTR_ROTATE_LEFT = 0x80,        // STATE_FTR_ROTATE turns the held furniture to the left
    ACTOR_FLAG_KEEP_ANIMATION = 0x100,        // skip the idle animation reset; cleared on a state change
    ACTOR_FLAG_FTR_STICK_NEUTRAL = 0x200,     // furniture hold: the stick went back to neutral since the last move
    ACTOR_FLAG_MII_MASK_OK = 0x400,           // canUseMiiMask
    ACTOR_FLAG_MII_FACE = 0x800,              // a Mii face can be made (d_a_player checkMiiFace / createMiiFace)
    ACTOR_FLAG_MII_MASK = 0x1000,             // the Mii mask is worn: mFace replaces the head models (putOnMiiMask)
    ACTOR_FLAG_NOISY = 0x2000,                // tool swings, digging, tumbling, fireworks (isMakingNoise)
    ACTOR_FLAG_POCKETS_FULL = 0x4000,         // the picked-up / caught item has no pocket left
    ACTOR_FLAG_INSECT = 0x8000,
    ACTOR_FLAG_CATALOG_COMPLETE = 0x10000,    // the catch's catalog (insects / fish) is complete
    ACTOR_FLAG_LEFT_WATER = 0x20000,          // walked out of STATE_WATER (the walk starts from the wait animation)
    ACTOR_FLAG_FIELD_ACTION_PENDING = 0x40000, // the state's field change (chop, pick up, dig, ...) is not applied yet
    ACTOR_FLAG_EFFECT_PENDING = 0x80000,      // the state's one-shot effect (sound, model update) is not played yet
    ACTOR_FLAG_SCRIPTED_MOVE = 0x100000,      // an NPC walks the player (setScriptedMove)
    ACTOR_FLAG_RESUME_BGM = 0x200000,         // isSceneReadyForBgm
    ACTOR_FLAG_LOCAL_AUDIO = 0x400000,        // sounds go through mAudioObj (the local player), else mVoiceObj
    ACTOR_FLAG_WINTER = 0x800000,             // created in a winter season
    ACTOR_FLAG_MENU_ACTION = 0x1000000,       // the action was chosen in the pocket menu (show / drop / bury an item)
    ACTOR_FLAG_DISABLED = 0x2000000,
    ACTOR_FLAG_BEE_STUNG = 0x4000000,         // the bee-stung face (getFaceNo)
    ACTOR_FLAG_LOCAL_PLAYER = 0x8000000,      // created as the local player
    ACTOR_FLAG_NO_OBJC = 0x10000000,          // the collision is not registered (doors, stairs, scene start)
    ACTOR_FLAG_HIDE_TOOL = 0x20000000,        // the tool model is not drawn (held item change, broken axe)
    ACTOR_FLAG_POINTER_WALK = 0x40000000,     // cleared every input frame; isPointerWalking
    ACTOR_FLAG_HOLE_FROM_TASK = 0x80000000,   // putItemInHole was called by a field task: the item is not put back
};

// dAcPlayer_c::mActorFlags2. Unnamed bits are still literals.
enum ActorFlag2 {
    ACTOR_FLAG2_POINTER_NO_TALK = 0x2,        // the pointer was used this frame: no talk input
    ACTOR_FLAG2_STOP_FOR_SAVE = 0x4,          // canStartNetSave: the waiting state returns to STATE_IDLE
    ACTOR_FLAG2_NOT_ALONE = 0x8,              // other players are in the structure (dSTR::isAlone)
    ACTOR_FLAG2_POINTER_NO_USE = 0x10,        // the pointer aims at a target: no tool use input
    ACTOR_FLAG2_EQUIP_FROM_MENU = 0x20,       // the held item / outfit change comes from the equipment menu
    ACTOR_FLAG2_EFFECT = 0x40,                // mEffect is playing (water, umbrella, bubbles)
    ACTOR_FLAG2_UNIT_ACTION = 0x80,           // isActingOnUnit
    ACTOR_FLAG2_SHOT_BALLOON = 0x100,         // watchShotBalloon
    ACTOR_FLAG2_SHOT_UFO = 0x200,             // watchShotUfo
    ACTOR_FLAG2_COCKROACH = 0x400,            // set with markCockroachNoticed; STATE_SURPRISED at the door
    ACTOR_FLAG2_POINTER = 0x800,              // the pointer gives the action direction
    ACTOR_FLAG2_POINTER_TURN = 0x1000,        // turning to the pointer direction before the action
    ACTOR_FLAG2_NET_SPAWNED_INSECT = 0x2000,  // the net swing spawned an insect (fn_116_1CDC)
    ACTOR_FLAG2_POINTER_FTR = 0x4000,         // the furniture was grabbed with the pointer
    ACTOR_FLAG2_ESCORTED = 0x8000,            // moveToEscorted
    ACTOR_FLAG2_FROM_BUS = 0x10000,           // created after a bus demo scene (no BGM resume at the scene start)
    ACTOR_FLAG2_PUSHED = 0x40000,             // an objc pushed the idle player
    ACTOR_FLAG2_FTR_BLOCKED = 0x80000,        // the held furniture cannot move
    ACTOR_FLAG2_FTR_BLOCKED_SE = 0x100000,    // SE_CanNot was played for the blocked furniture
    ACTOR_FLAG2_EXIT_REQUESTED = 0x200000,    // the scene exit was requested (rooms, stairs)
    ACTOR_FLAG2_DOOR_SE = 0x400000,           // the room door sound was played
};

// dAcPlayer_c::mReactionKind: the STATE_REACT variant.
enum Reaction {
    REACTION_DEBT_PAID = 0,  // reactDebtPaid
    REACTION_JOB_DONE = 1,   // Nook's part-time job is done
    REACTION_GOLD_NET = 2,           // d_a_player: after STATE_HAND_ITEM_PUT_AWAY with a gold net
    REACTION_GOLD_ROD = 3,           // after STATE_HAND_ITEM_PUT_AWAY with a gold rod
    REACTION_GOLD_WATERING = 4,      // after STATE_HAND_ITEM_RETURN with a gold watering can
    REACTION_GOLD_AXE = 5,           // d_a_player: the goddess gave the golden axe (fountain)
    REACTION_COCKROACH = 6,  // reactToCockroaches
    REACTION_SKIP = 7,               // d_a_player: set in STATE_RECEIVE_ITEM (gold net / rod): no reaction after the put away
    REACTION_NONE = 8,               // endReact
};

// dAcPlayer_c::mEquipFlags: the head to load for the hat (by its hide bone, setHatHideBone) and the
// accessory / cloth changes.
enum EquipmentVisibility {
    EQUIPMENT_HAT_CAP = 0x1,        // hide bones 0 / 1: loadCapHead
    EQUIPMENT_HAT_FULL_MASK = 0x2,  // hide bone 2: loadFullMask
    EQUIPMENT_HAT_MASK = 0x4,       // hide bones 3 / 4: loadMaskHead
    EQUIPMENT_HAT_MASK_2 = 0x8,     // hide bones 5 / 6: loadMaskHead
    EQUIPMENT_HAT_OVER_HEAD = 0x10, // hide bone 7: loadHeadAndCap
    EQUIPMENT_HAT_NONE = 0x20,      // no hat (HIDE_BONE_NONE): loadHead
    EQUIPMENT_HAT_ANY = 0x1F,       // the hat styles
    EQUIPMENT_HAT_STYLE = 0x3F,     // the hat styles and EQUIPMENT_HAT_NONE
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
extern u8 sSnowEffectWearThreshold; // 8074E5F8?: snow wear above it counts as snow (surface effects)


// Output of RFLGetCharInfo; the create ID goes to dPrivateData_c::mMiiId.
struct MiiData_c {
    /* 0x00 */ u8 _00[0x2C];
    /* 0x2C */ u8 mCreateID[8];
    /* 0x34 */ u32 mPersonalInfo;
    /* 0x38 */ GXColor mFavoriteColor; // d_a_player: the "b0" material's TEV KColor (putOnMiiMask)
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
BOOL enterDoor(f32 x, f32 z, s16 angle, int type,
               int target); // 800FE39C: STATE_ENTER_DOOR,
                            // STATE_WALK_THROUGH_DOOR with a target (d_a_str)
BOOL walkThroughDoor(f32 x, f32 z, s16 angle,
                     int target); // 800FE484: STATE_WALK_THROUGH_DOOR (d_a_str)
BOOL enterDoorDirect(f32 x, f32 z, s16 angle, int type,
                     BOOL walkThrough); // 800FE550: STATE_ENTER_DOOR /
                                        // STATE_WALK_THROUGH_DOOR (d_a_str)
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
BOOL isFacingPosition(f32 maxDist, const mVec3_c *target, int maxAngle); // 800FFDC0
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
u32 requestInsectBite(); // 80100860: STATE_INSECT_BITE (d_insect_field, d_insect_museum)
bool isUsingNet(int idx); // 801008E4
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
bool isSafeFromInsects(); // 80100D44: demo, pitfall or ACTOR_FLAG_DEMO: insects leave the player alone
BOOL isScaringCockroaches(int idx); // 80100DD0: walking, running, pushing or pulling (d_a_cockroach)
BOOL boardBus(s16 angle); // 80100E3C: STATE_BUS_BOARD (daStrBusStop_c; the angle is not used)
BOOL isOnBus(); // 80100E7C: STATE_BUS_BOARDED
BOOL rideBus(); // 80100EC0: STATE_BUS_RIDE
BOOL getOffBus(s16 angle, f32 frame); // 80100F00: STATE_BUS_GET_OFF
BOOL isOffBus(); // 80100F98: STATE_BUS_GOT_OFF
BOOL leaveBus(); // 80100FDC: STATE_BUS_LEAVE
BOOL isAxeInFountain(); // 8010101C: Mgr_c::mAxeInFountain (the goddess appears)
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
BOOL isCockroachNoticed(); // 8010152C: Mgr_c::mCockroachNoticed (BOOL: the player REL compares it with cmpwi)
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
