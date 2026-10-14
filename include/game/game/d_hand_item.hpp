#pragma once

// d_hand_item (DOL TU d_hand_item.cpp, .text 80191500..80195290): dHandItem_c (named by its RTTI), the process that
// animates an item passed between two actors (players and npcs): handed over, caught, held up, put on, eaten...
// One instance (dHandItem_c::sInstance); the rest of the game uses the dHandItem functions, which check the
// instance and forward to it. Member and function names are inferred from behaviour.

#include <types.h>
#include <game/framework/f_base.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_snd_obj.hpp>
#include <game/mLib/m_vec.hpp>

class dActor_c;
class dDemoActor_c;

class dHandItem_c : public fBase_c {
public:
    typedef void (dHandItem_c::*StateFunc)();

    // The init / exec functions of a state for one variant.
    struct stateFunc_c {
        StateFunc mInit;
        StateFunc mExec;
    };

    // The states (mState, request): what the item does. Names are inferred from the motion and the callers. An
    // exchange (d_a_npc action_c catch, dPlayerMgr::giveItem / receiveItem): the giver loads (fish) and takes the
    // item out (APPEAR); by hand, the receiver TAKEs and HOLDs it, then uses it as its use type says (mUseType,
    // the same values: SHRINK, KEEP, FOLLOW, EAT, SHOW...). Through the ground, the giver puts it down (FALL),
    // the receiver becomes the holder (swapActors) and picks it up (STOW).
    enum State_e {
        STATE_LOAD = 0,     // loads the fish model (dPlayerMgr::receiveItem, the npcs' catch)
        STATE_APPEAR = 1,   // the giver takes it out: it grows at the hand (ground: then drops to the drop spot)
        STATE_FALL = 2,     // ground: falls to the drop spot in front of the actor
        STATE_STOW = 3,     // ground: the receiver picks it up; it moves to the hand and shrinks away
        STATE_TAKE = 4,     // the receiver takes it: moves to the hand
        STATE_HOLD = 5,     // held at the hand for a while
        STATE_SHRINK = 6,   // shrinks away beside the actor (put away)
        STATE_KEEP = 7,     // stays at the hand until the next state
        STATE_FOLLOW = 8,   // stays at the hand for a while (d_a_npc plays animation 0x5B / 0x66 backwards)
        STATE_CHANGE = 9,   // no state functions; a use type: the npc changes clothes or takes medicine (d_a_npc)
        STATE_EAT = 10,     // moves to the mouth, then shrinks
        STATE_UNWRAP = 11,  // the present box opens (af_itm_presentbox): the item kind becomes KIND_ITEM
        STATE_SHOW = 12,    // held up for a while
        STATE_NUM = 13,
        STATE_NONE = 13,
    };

    // The variant (mVariant) picks the state functions: handed between two actors, or through the ground.
    enum Variant_e {
        VARIANT_HAND = 0,
        VARIANT_GROUND = 1, // checked with a wider range (isInRange)
        VARIANT_NUM = 2,
    };

    // What the item looks like (mKind): wrapped kinds draw the present box (item index 12).
    enum Kind_e {
        KIND_ITEM = 0,
        KIND_WRAPPED = 1,
        KIND_PRESENT = 2, // also flagged to the fg object callback (fgMngProc_callFgObjCallback)
        KIND_NONE = 7,
    };

    enum {
        ACTOR_NUM = 2, // 0: the actor holding the item, 1: its partner
    };

    dHandItem_c();  // 80191530
    virtual ~dHandItem_c(); // 80191580

    void reset(); // 801915EC
    BOOL start(const dItem::Item *item, int kind, int useType, int variant, dDemoActor_c *actor,
               dDemoActor_c *partner, f32 ofsY0, f32 ofsY1, f32 fishOfsY0, f32 fishOfsY1, f32 ofsZ0,
               f32 ofsZ1); // 80191664
    void release(); // 801917E8
    BOOL isActor(dDemoActor_c *actor, u32 idx); // 80191840
    BOOL swapActors(dDemoActor_c *actor); // 801918AC
    BOOL request(int state, dDemoActor_c *actor, f32 reach); // 801919F0
    BOOL isState(int state) const; // 80191A58
    BOOL isReceiving() const; // 80191A8C
    BOOL setUseType(int type, dDemoActor_c *actor); // 80191AD8
    BOOL isWaiting() const; // 80191B48
    void setActor(dDemoActor_c *actor, u32 idx); // 80191BC0
    dDemoActor_c *getActor(u32 idx); // 80191BD8
    void setOfsY(u32 idx, f32 value); // 80191BF8
    f32 getOfsY(u32 idx); // 80191C10
    void setFishOfsY(u32 idx, f32 value); // 80191C30
    f32 getFishOfsY(u32 idx); // 80191C48
    void setOfsZ(u32 idx, f32 value); // 80191C68
    f32 getOfsZ(u32 idx); // 80191C80
    void getHandPos(mVec3_c *out, u32 idx); // 80191CA0
    void toWorld(mVec3_c *out, const mVec3_c *local, u32 idx); // 80191D0C
    mVec3_c *getOfs(u32 idx); // 80191D9C
    const mVec3_c *getFishOfs(dItem::Item *item, u32 idx); // 80191E4C

    virtual int create();   // 80191F04
    virtual int doDelete(); // 80191F38
    virtual int execute();  // 80191F64
    virtual int draw();     // 80192128

    // State functions: init* run once when a state starts, exec* every frame. The exec functions dispatch on
    // mStep through a table of the steps below; a step ends at its frame threshold (l_*End[mStep]) and sets up
    // the next one. "Track" steps keep the item at mOffset from the hand.
    void initLoad();               // 80192258
    void execLoad();               // 80192264: requests the fish model, then ends the state
    void initAppearHand();         // 801922F4: the target in front of the hand (mReach ahead), scale from 0
    void appearHandGrow();         // 80192554: grows (with an overshoot) while rising from the hand to the target
    void appearHandGrowFish();     // 80192654: a fish grows while held at its offset from the hand
    void execAppearHand();         // 80192818
    void initFallHand();           // 80192858: nothing to do in the hand variant
    void initTakeHand();           // 80192864: the target at the receiver's hand
    void takeHandMove();           // 80192AE8: moves to the hand in 10 frames
    void execTakeHand();           // 80192BAC
    void initHoldHand();           // 80192BEC
    void holdHandMoveTo();         // 80192C2C: moves to mTarget (x/z), keeping the hand's height (not used: init starts at the next step)
    void holdHandTrack();          // 80192CB8: 24 frames at the offset from the hand
    void execHoldHand();           // 80192D3C
    void initKeepHand();           // 80192D7C
    void keepHandTrack();          // 80192DE0: stays at the offset from the hand until the next state
    void execKeepHand();           // 80192E44
    void initShrinkHand();         // 80192E84: the target beside the actor (l_shrinkHandTarget)
    void shrinkHandAway();         // 80192F9C: moves there and shrinks to nothing in 18 frames
    void execShrinkHand();         // 80193068
    void initFollowHand();         // 801930A8
    void followHandTrack();        // 8019310C: 24 frames at the offset from the hand
    void execFollowHand();         // 80193194
    void initUnwrapHand();         // 801931D4: the present box effect and sound; the item is no longer wrapped
    void unwrapHandTrack();        // 80193274: 30 frames at the offset from the hand
    void execUnwrapHand();         // 801932F8
    void initEatHand();            // 80193338: the target at the mouth (the appear target)
    void eatHandToMouth();         // 801934D4: moves to the mouth (x/z), keeping its height above the hand
    void eatHandTrack();           // 8019358C: stays at the mouth
    void eatHandShrink();          // 80193650: shrinks to nothing (eaten)
    void execEatHand();            // 801936FC
    void initShowHand();           // 8019373C
    void showHandTrack();          // 801937A0: 160 frames at the offset from the hand
    void execShowHand();           // 80193828
    void initAppearGround();       // 80193868: players start with appearGroundWait, npcs with appearGroundWaitNpc
    void appearGroundWait();       // 801938FC: a player: waits until frame 33, then sets up the rise
    void appearGroundRise();       // 80193A0C: rises from the hand to the target while growing (steps 1-4)
    void getDropPos(mVec3_c *out); // 80193B70: the unit centre in front of the actor (fixed in the fortune shop), raised by 16
    void appearGroundDrop();       // 80193C20: drops to the drop position
    void appearGroundWaitNpc();    // 80193CCC: an npc: waits until frame 64, then sets up the pop
    void appearGroundPop();        // 80193E00: pops out of the npc's hand (l_appearGroundPopOfs) while growing
    void execAppearGround();       // 80193F28
    void initFallGround();         // 80193F68: the target at the drop position
    void fallGroundMove();         // 80193FF8: falls there in 12 frames
    void execFallGround();         // 80194074
    void initTakeGround();         // 801940B4: the target at the hand (l_groundHandOfs)
    void takeGroundMove();         // 801941C0: moves there in 10 frames
    void execTakeGround();         // 8019423C
    void initHoldGround();         // 8019427C
    void holdGroundTrack();        // 80194288: 14 frames at the hand (l_groundHandOfs)
    void execHoldGround();         // 80194358
    void initShrinkGround();       // 80194398: the target beside the actor (l_shrinkGroundTarget)
    void shrinkGroundAway();       // 801944B0: moves there and shrinks to nothing in 18 frames
    void execShrinkGround();       // 8019457C
    void initFollowGround();       // 801945BC
    void execFollowGround();       // 801945C8: the hand variant's hold steps
    void initStowGround();         // 801945CC
    void stowGroundWait();         // 801945D8: waits until frame 18, then sets up the move to the hand
    void stowGroundToHand();       // 80194730: moves to the hand (l_stowGroundOfs in front of it)
    void stowGroundShrink();       // 801948D0: shrinks into the hand
    void execStowGround();         // 801949EC

    /* 0x064 */ const stateFunc_c *mpStateFunc;
    /* 0x068 */ dDemoActor_c *mActors[ACTOR_NUM];
    /* 0x070 */ int mState;
    /* 0x074 */ int mUseType;
    /* 0x078 */ int mVariant;
    /* 0x07C */ int mNextState;
    /* 0x080 */ int mKind;
    /* 0x084 */ mVec3_c mPos;
    /* 0x090 */ mVec3_c mTarget;
    /* 0x09C */ mVec3_c mSpeed;
    /* 0x0A8 */ mVec3_c mOffset;
    /* 0x0B4 */ dSndObjSimple_c mSound;
    /* 0x0E4 */ f32 mScale;
    /* 0x0E8 */ f32 mScaleSpeed;
    /* 0x0EC */ f32 mRange;
    /* 0x0F0 */ f32 mReach;
    /* 0x0F4 */ f32 mOfsY[ACTOR_NUM];
    /* 0x0FC */ f32 mFishOfsY[ACTOR_NUM];
    /* 0x104 */ f32 mOfsZ[ACTOR_NUM];
    /* 0x10C */ int mTimer;
    /* 0x110 */ dItem::Item mItem;
    /* 0x112 */ u8 mFrame;
    /* 0x113 */ u8 mStep;
    /* 0x114 */ u8 mDone;
    /* 0x115 */ u8 mVisible;

    static dHandItem_c *sInstance; // 8074E9F8
}; // size 0x118

namespace dHandItem {

extern const f32 cDefaultReach; // 80751600: 16.0f, the default reach of request (d_a_npc)


BOOL start(const dItem::Item *item, int kind, int useType, int variant, dDemoActor_c *actor, dDemoActor_c *partner,
           f32 ofsY0, f32 ofsY1, f32 fishOfsY0, f32 fishOfsY1, f32 ofsZ0, f32 ofsZ1); // 80194A2C
BOOL request(int state, dDemoActor_c *actor, f32 reach); // 80194AA0
BOOL isHolder(dDemoActor_c *actor);                       // 80194AC8
BOOL isActive();                                          // 80194AEC
BOOL isState(int state);                                  // 80194B10
int getUseType();                                         // 80194B30
BOOL setUseType(int type, dDemoActor_c *actor);           // 80194B4C
BOOL isReceiving();                                       // 80194B74
BOOL swapActors(dDemoActor_c *actor);                     // 80194B8C
dDemoActor_c *getActor(u32 idx);                          // 80194BAC
BOOL isInRange(dDemoActor_c *actor);                      // 80194BCC
void release(dDemoActor_c *actor);                        // 80194CB0
BOOL getPos(mVec3_c *pos);                                // 80194CEC
void getItem(dItem::Item *item);                          // 80194D20

} // namespace dHandItem
