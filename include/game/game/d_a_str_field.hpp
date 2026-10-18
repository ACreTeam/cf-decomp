#pragma once

// The town's structures (REL d_a_str_fieldNP, profiles STR_PL..STR_CURNIVAL):
// the gate, the players' houses and mailboxes, the construction site, the
// villagers' houses, Nook's store, the bulletin board, the museum, the tailor,
// the town hall, the tent, the harvest festival's tent, the public works
// (lighthouse, windmill, fountain), the crashed UFO and the carnival's stand.
// All are structures (daSTR_c). Source: src/d_a_str_fieldNP/d_a_str_field.cpp.
// Names are inferred.

// The class order sets the vtable / RTTI order in .data (reverse class
// completion) and the weak inline order.
#include <game/game/d_a_str.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_item.hpp>

// The gate (BUILDING_GATE): its flag shows the town flag's design (dFlag_c).
class daStrChkp_c : public daSTR_c {
  public:
    virtual int doDelete();
    virtual int execute();
    virtual void beforeCreateMdl();
    virtual void afterCreateMdl();
    virtual BOOL isDoorCentered() const {
        return FALSE;
    }
    virtual BOOL isCheckCamera() {
        return FALSE;
    }
    virtual const char *getFadeMdlName();

    /* 0x498 */ dItem::resLoader_c mFlagLoader; // the town flag's design
}; // size 0x508

// A player's house (BUILDING_PLAYER_HOUSE_0..3). Creates its mailbox.
class daStrPl_c : public daSTR_c {
  public:
    virtual int doDelete();
    virtual int execute();
    virtual void afterCreateMdl();
    virtual BOOL hasKnockSe() {
        return !mIsCurrent;
    }
    virtual int getDoorEndSe() {
        return DOOR_MELODY_HOUSE;
    }
    virtual int getCheckMsg();
    virtual void drawMdl();
    virtual BOOL isWindowLit();

    int getHouseNo();

    /* 0x498 */ m3d::anmMatClr_c mRoofAnm; // roof_color (obj_myhome_common)
    /* 0x4C4 */ m3d::anmVis_c mFlagAnm;    // the flag pole (a size 4 house)
    /* 0x4FC */ bool mIsCurrent;           // the current player's house
    /* 0x4FD */ u8
        mHasFlag; // dHome_c mHasFlag: the flag pole shows, with a dFlag_c
}; // size 0x500

// A player's mailbox (BUILDING_MAILBOX_0..3, created by the house with its
// building index as the param). The flag is up while there is mail; the door
// states (setDoorState) are its animations, synced to the other net members.
class daStrPost_c : public daSTR_c {
  public:
    // setDoorState / executeDoorState (mDoorState).
    enum {
        POST_STATE_EMPTY,
        POST_STATE_NEW_MAIL, // the flag goes up (obj_post_newmail)
        POST_STATE_MAIL,     // the flag is up and blinks (obj_post_blink)
        POST_STATE_OPEN,     // obj_post_open
        POST_STATE_OPENED,
        POST_STATE_CLOSE,
        POST_STATE_FLAG_DOWN,
        POST_STATE_NUM,
    };

    // setState / executeState.
    enum {
        STATE_WAIT,
        STATE_OPEN_BOX, // the player opens the mailbox
        STATE_MENU,     // the letter menu
        STATE_MENU_WAIT,
        STATE_CLOSE_BOX,
        STATE_NUM,
    };

    virtual int doDelete();
    virtual int execute();
    virtual bool canInteract(dDemoActor_c *actor);
    virtual void onDemoCmd(int cmd, u8 member);
    virtual BOOL isCopyCol() {
        return FALSE;
    }
    virtual BOOL isActive() {
        return FALSE;
    }
    virtual void afterCreateMdl();
    virtual BOOL setDoorState(int state);
    virtual void executeDoorState();
    virtual BOOL isDraw() {
        return FALSE;
    } // drawn by the house (daStrPl_c::drawMdl)

    BOOL setupAnm();
    BOOL hasNewMail();
    BOOL setState(int state);
    void executeState();
    void sendNetState(int state);
    u8 getNetState();

    BOOL initEmpty();
    void executeEmpty();
    BOOL initNewMail();
    void executeNewMail();
    BOOL initMail();
    void executeMail();
    BOOL initOpen();
    void executeOpen();
    BOOL initOpened();
    void executeOpened();
    BOOL initClose();
    void executeClose();
    BOOL initFlagDown();
    void executeFlagDown();

    BOOL initWait();
    void executeWait();
    BOOL initOpenBox();
    void executeOpenBox();
    BOOL initMenu();
    void executeMenu();
    BOOL initMenuWait();
    void executeMenuWait();
    BOOL initCloseBox();
    void executeCloseBox();

    /* 0x498 */ u8 mHouseNo;
    /* 0x499 */ bool mIsCurrent; // the current player's mailbox
    /* 0x49C */ m3d::anmMatClr_c mBlinkAnm;
    /* 0x4C8 */ nw4r::g3d::ResAnmChr mNewMailAnm;
    /* 0x4CC */ nw4r::g3d::ResAnmChr mOpenAnm;
    /* 0x4D0 */ nw4r::g3d::ResAnmClr mBlinkClr;
    /* 0x4D4 */ dObjc::ac_c mObjc;
    /* 0x518 */ int mPostState;
}; // size 0x51C

// The construction site (BUILDING_BUILD_SITE): its sign.
class daStrReserve_c : public daSTR_c {
  public:
    // sys_MESS/SYS_Board: the sign's sayings, picked by the site and the town.
    enum {
        BOARD_MSG_FIRST = 2, // "--The Fisherman's Creed--..."
        BOARD_MSG_NUM = 17,
    };

    virtual u16 getDemoCode() const;
    virtual void onTalkStart(int kind, dMsg::Comp_c *comp);
    virtual BOOL isCopyCol() {
        return FALSE;
    }
    virtual BOOL isActive() {
        return FALSE;
    }
    virtual ulong getMdlBufferOption() {
        return 0;
    }
    virtual void afterCreateMdl();
    virtual void onMessageStart(int arg);

    int getBoardMsg();

    /* 0x498 */ u16 mSiteNo; // dStrm_c::sBuildSiteNo
}; // size 0x49C

// A villager's house. Creates its name plate.
class daStrNpc_c : public daSTR_c {
  public:
    virtual int doDelete();
    virtual int execute();
    virtual void onTalkStart(int kind, dMsg::Comp_c *comp);
    virtual void afterCreateMdl();
    virtual BOOL hasKnockSe() {
        return TRUE;
    }
    virtual int getDoorEndSe() {
        return DOOR_MELODY_HOUSE;
    }
    virtual int getCheckMsg();
    virtual void drawMdl();
    virtual BOOL isNight();
    virtual BOOL isWindowLit();

    int getNpcNo();
    int getHouseType();

    /* 0x498 */ m3d::anmTexPat_c mHouseAnm; // the region's house textures
    /* 0x4C4 */ m3d::mdl_c _4C4;
}; // size 0x504

// Nook's store (all four stages).
class daStrShop_c : public daSTR_c {
  public:
    virtual void beforeCreateMdl();
    virtual int getSceneOffset();
    virtual BOOL isNight() {
        return TRUE;
    }
    virtual int getDoorEndSe() {
        return isAutoDoor() ? DOOR_MELODY_SHOP : DOOR_MELODY_NONE;
    }
    virtual int getDoorType() const;
    virtual int getCheckMsg();

    static BOOL isOpNookHere();

    /* 0x498 */ u8 mStage; // SHOP_STAGE_*
}; // size 0x49C

// The museum.
class daStrMuseum_c : public daSTR_c {
  public:
    virtual const char *getFadeMdlName();
    virtual int getCheckMsg();
}; // size 0x498

// The tailor (Able Sisters): closed at night.
class daStrTailor_c : public daSTR_c {
  public:
    // The opening hours (getCheckMsg): 8:00 to 23:00.
    enum {
        OPEN_HOUR = 8,
        CLOSE_HOUR = 23,
    };

    virtual BOOL isNight() {
        return TRUE;
    }
    virtual int getDoorEndSe() {
        return DOOR_MELODY_TAILOR;
    }
    virtual int getCheckMsg();
}; // size 0x498

// The town hall.
class daStrOffice_c : public daSTR_c {
  public:
    virtual int getDoorEndSe() {
        return DOOR_MELODY_OFFICE;
    }
    virtual int getCheckMsg();
}; // size 0x498

// The tent.
class daStrTent_c : public daSTR_c {
  public:
}; // size 0x498

// The harvest festival's tent.
class daStrHarvest_c : public daSTR_c {
  public:
}; // size 0x498

// The bulletin board. Creates its bird (daBbsBird_c).
class daStrBbs_c : public daSTR_c {
  public:
    // setState / executeState.
    enum {
        STATE_WAIT,
        STATE_MENU,
        STATE_MENU_WAIT,
        STATE_NUM,
    };

    virtual int doDelete();
    virtual int execute();
    virtual bool canInteract(dDemoActor_c *actor);
    virtual void onDemoCmd(int cmd, u8 member);
    virtual void afterCreateMdl();
    virtual void drawMdl();

    BOOL setState(int state);
    void executeState();

    BOOL initWait();
    void executeWait();
    BOOL initMenu();
    void executeMenu();
    BOOL initMenuWait();
    void executeMenuWait();

    /* 0x498 */ bool mInFront; // the player stands in front of the board
    /* 0x49C */ int mBbsState;
}; // size 0x4A0

// The lighthouse: its light turns with the windows.
class daStrLightHs_c : public daSTR_c {
  public:
    virtual int doDelete();
    virtual int execute();
    virtual void afterCreateMdl();

    /* 0x498 */ m3d::anmMatClr_c mLightAnm;
}; // size 0x4C4

// The windmill.
class daStrWindmill_c : public daSTR_c {
  public:
    virtual int doDelete();
    virtual int execute();
    virtual void afterCreateMdl();
}; // size 0x498

// The crashed UFO.
class daStrUFO_c : public daSTR_c {
  public:
    virtual int doDelete();
    virtual int execute();
    virtual void afterCreateMdl();
    virtual BOOL isDraw() {
        return mDraw;
    }

    /* 0x498 */ u8 mDraw; // SAVE_FLAG_UFO_STR_DRAW
}; // size 0x49C

// The fountain: the goddess comes out for an axe thrown in.
class daStrFountain_c : public daSTR_c {
  public:
    // setState / executeState.
    enum {
        STATE_WAIT,
        STATE_ITEM,      // dPlayerMgr::receiveGoddessItem
        STATE_ITEM_WAIT, // waits for the player
        STATE_TALK,      // npc_SP/NPC_goddess
        STATE_TALK_END,
        STATE_NUM,
    };

    // npc_SP/NPC_goddess.
    enum {
        MSG_GODDESS_AWAY =
            0x01, // branches to 0x02..0x06: "I'm not here right now!"
    };

    virtual int doDelete();
    virtual int execute();
    virtual void onDemoCmd(int cmd, u8 member);
    virtual void afterCreateMdl();
    virtual int getVoiceMode();

    BOOL isGoddessHere() const;
    BOOL setState(int state);
    void executeState();

    BOOL initWait();
    void executeWait();
    BOOL initItem();
    void executeItem();
    BOOL initItemWait();
    void executeItemWait();
    BOOL initTalk();
    void executeTalk();
    BOOL initTalkEnd();
    void executeTalkEnd();

    /* 0x498 */ u8 _498[0x49C - 0x498];
    /* 0x49C */ m3d::anmMatClr_c mAnmClr;
    /* 0x4C8 */ m3d::anmTexPat_c mAnmTexPat;
    /* 0x4F4 */ m3d::anmTexSrt_c mAnmTexSrt;
    /* 0x520 */ dLevelEffect_c mEffect; // af_obj_fountain
    /* 0x5B4 */ u8 _5B4[0x5B8 - 0x5B4];
    /* 0x5B8 */ int mFountainState;
}; // size 0x5BC

// The carnival's stand.
class daStrCurnival_c : public daSTR_c {
  public:
    virtual int doDelete();
    virtual int execute();
    virtual void afterCreateMdl();
}; // size 0x498
