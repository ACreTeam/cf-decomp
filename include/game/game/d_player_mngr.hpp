#pragma once

// d_player_mngr (DOL TU d_player_mngr.cpp, .text 801B76BC..801B96A4):
// dPlayerMngr_c (named by its RTTI), the process that owns the four players'
// equipment model managers (cloth, head item, tool bank), creates the player
// actors of house owners and visiting network members, and keeps their
// equipment in sync over the network (packets 0x24..0x26, shared records
// NET_REC_MEMBER_A/B/C). The rest of the game reaches it through dPlayerMgr
// (d_player_mgr.cpp) and the d_a_player REL. Member and function names are
// inferred from behaviour.

#include <types.h>
#include <game/framework/f_base.hpp>
#include <game/game/d_hmn_cloth_mng.hpp>
#include <game/game/d_hmn_head_item_mng.hpp>
#include <game/game/d_hmn_tool_mng.hpp>

class dActor_c;
class mVec3_c;
class mAng3_c;

class dPlayerMngr_c : public fBase_c {
  public:
    enum {
        MEMBER_NUM = 4, // network members (player indices 0-3)
        PART_NUM = 4,   // dEquip_c::Part: shirt, hat, accessory, held item
    };

    // The player actor (d_a_player REL) and its construction parameter: the
    // start state in the low bits.
    enum {
        PLAYER_PROFILE = fProf::A_PLAYER,
        ACTOR_PARAM_REMOTE = 0x20000000, // a remote member's actor: set from
                                         // the network, not the pad
        ACTOR_PARAM_MEMBER_SHIFT = 30,   // bits 30-31: the network member
    };

    // Equipment load state of one part of one player (equip_c::mState).
    enum EquipState_e {
        EQUIP_STATE_NONE = 0,
        EQUIP_STATE_LOAD = 1,   // load mItem, then EQUIP_STATE_LOADED
        EQUIP_STATE_CHANGE = 2, // load mItem, then EQUIP_STATE_CHANGED
        EQUIP_STATE_LOADED = 3,
        EQUIP_STATE_CHANGED = 4,
    };

    // The four players' equipment of one part (sEquip[part]).
    class equip_c {
      public:
        equip_c(); // 801B76EC

        /* 0x00 */ u16 mSyncItem[MEMBER_NUM]; // the item received while joining
                                              // a session (syncEquipment)
        /* 0x08 */ u16 mItem[MEMBER_NUM]; // the item the model managers load
        /* 0x10 */ u8
            mSyncState[MEMBER_NUM]; // EQUIP_STATE_CHANGE / EQUIP_STATE_CHANGED
                                    // while joining
        /* 0x14 */ u8 mState[MEMBER_NUM]; // EquipState_e
    }; // size 0x18

    // The last action parameters received for a remote player
    // (dPlayerMgr::cacheRemoteParams; read back by the d_a_player REL). Copied
    // as three words.
    struct actionParam_c {
        u32 mData[3];
    }; // size 0xC

    dPlayerMngr_c();          // 801B773C
    virtual ~dPlayerMngr_c(); // 801B77F4
    virtual BOOL create();    // 801B789C
    virtual BOOL doDelete();  // 801B7A08
    virtual BOOL draw();      // 801B7B60
    virtual BOOL execute();   // 801B7B68

    static u32 makeActorParam(int member, u32 state);      // 801B7BF0
    static u32 setActorParamRemote(int remote, u32 param); // 801B7BFC
    static dActor_c *createActor(int member, int remote, u32 state,
                                 const mVec3_c *pos,
                                 const mAng3_c *angle);    // 801B7C10
    void createVisitorActor();                             // 801B7C94
    void updateRemoteActors();                             // 801B8048
    static void queueEquip(int idx, u32 part, int item);   // 801B835C
    BOOL requestEquip(int idx, u8 part, u16 item);         // 801B8428
    void executeEquip(int idx, u8 part);                   // 801B8520
    static BOOL isEquipReady(u32 idx, int part);           // 801B85C0
    static BOOL checkEquip(u32 idx, int part);             // 801B8620
    static void setLocalEquip(int idx, u8 part, u16 item); // 801B8898
    static int updateEquip(int idx, int part);             // 801B8918
    static BOOL isEquipLoaded(int idx, u8 part);           // 801B8AC4
    static u8 isEquipChecked(int idx);                     // 801B8BB8
    static void clearEquipChecked(int idx);                // 801B8BC4
    static dHmnCloth_c *getCloth(u32 idx);                 // 801B8BD4
    static dHmnHeadItem_c *getHeadItem(u32 idx);           // 801B8C04
    static dHmnToolBank_c *getToolBank(u32 idx);           // 801B8C34
    dHmnToolBank_c *getPlayerToolBank(u32 idx) {
        return &mToolBank[idx];
    } // doDelete
    static void resetEquipStates(int idx);                        // 801B8C68
    static void resetEquipState(int idx, int part);               // 801B8CC8
    BOOL syncLoad();                                              // 801B8D08
    BOOL syncEquipment();                                         // 801B8DAC
    static void setLocalAppearance(int idx, int kind);            // 801B8FD8
    static void applyAppearance(int idx, int kind);               // 801B902C
    static void receiveAppearance(int idx, int kind);             // 801B90C0
    static void setLocalPlayerEquip(int idx, int part, u16 item); // 801B910C
    static void receiveEquip(int idx, int part, int item);        // 801B91B8
    static BOOL getRemoteTransform(f32 *x, f32 *z, s16 *angle,
                                   u32 member); // 801B9258
    static BOOL getRemoteStateInfo(u32 *info, u8 *scene, u8 *state,
                                   u32 member);                      // 801B9304
    static void sendEquip(int part, int item);                       // 801B93B8
    static void sendEquipQueue(int part, int item);                  // 801B9424
    static void sendAppearance(int kind);                            // 801B9490
    static void setRemoteState(u32 idx, int state);                  // 801B94E8
    static u8 getRemoteState(u32 idx);                               // 801B94FC
    static void setRemoteParam(u32 idx, const actionParam_c *param); // 801B9518
    static BOOL getRemoteParam(u32 idx, actionParam_c *param);       // 801B9548
    static u8 isRemotePresent(u32 idx);                              // 801B9588
    int getRemoteCreateState(int state);                             // 801B95B0
    static void requestSync();                                       // 801B9610
    static void clearAllEquip();                                     // 801B961C
    static void clearEquip(int idx);                                 // 801B9658

    /* 0x0064 */ dHmnCloth_c mCloth[MEMBER_NUM];
    /* 0x0068 */ dHmnHeadItem_c mHeadItem[MEMBER_NUM];
    /* 0x006C */ dHmnToolBank_c mToolBank[MEMBER_NUM];
    /* 0x1CDC */ int mSavedSlot; // player slot of member 1 before a house
                                 // owner's actor took it
    /* 0x1CE0 */ u8 _1CE0[4];
    /* 0x1CE4 */ u8 mCreated[MEMBER_NUM]; // the player's actor was created here
    /* 0x1CE8 */ u8 mPresent[MEMBER_NUM]; // the remote player was in this scene
                                          // before the scene change
    /* 0x1CEC */ u8 mSynced[MEMBER_NUM];  // syncEquipment started loading the
                                          // player's equipment
    /* 0x1CF0 */ u8
        mSlotOverridden; // createVisitorActor changed member 1's player slot

    static dPlayerMngr_c *sInstance;               // 8074EB78
    static u8 sEquipChecked[MEMBER_NUM];           // 8074EB7C
    static u8 sSyncRequest;                        // 8074EB80
    static u8 sRemoteScene[MEMBER_NUM];            // 8074C320
    static u8 sRemoteState[MEMBER_NUM];            // 8074C324
    static equip_c sEquip[PART_NUM];               // 80625940
    static actionParam_c sRemoteParam[MEMBER_NUM]; // 806259A0
    static u32 sRemoteInfo[MEMBER_NUM];            // 806259D0
}; // size 0x1CF4
