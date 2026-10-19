#pragma once

// The city's structures (REL d_a_str_townNP, profiles STR_TOWN..STR_CHEST):
// the shops and buildings along the city's street (Shampoodle, the HRA, Crazy
// Redd's, Katrina's, GracieGrace, the auction house, the theater), its
// fountain, the reset building, the ATM, the street lamps, the organ grinder
// and the bulletin board. All are structures (daStrTown_c, a daSTR_c with a
// texture animation that slows down while the building is closed). Source:
// src/d_a_str_townNP/d_a_str_town.cpp. Names are inferred.

// The class and include order set the vtable / RTTI order in .data (reverse
// class completion) and the weak inline order: dScript::Word_c completes
// before dSTR::alwaysAc_c, and dItem::name_c between dSTR::alwaysAc_c and
// dMsg::Rcpt_c, so d_script.hpp / d_item.hpp go into d_a_str.hpp's include
// list around d_str.hpp.
#include <game/mLib/m_3d.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_bgc.hpp>
#include <game/game/d_script.hpp>
#include <game/game/d_str.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_a_str.hpp>

// The base of the city's structures: the model's texture animation runs at
// full speed while the door's message is DOOR_MSG_NONE (open) and slows to a
// stop otherwise.
class daStrTown_c : public daSTR_c {
  public:
    // isNight: the windows are lit from 18:00 to 23:00.
    enum {
        LIGHT_ON_HOUR = 18,
        LIGHT_OFF_HOUR = 23,
    };

    virtual int preDelete();
    virtual int preExecute();
    virtual void afterCreateMdl();
    virtual BOOL isCheckCamera() {
        return FALSE;
    }
    virtual BOOL isCheckViewArea() {
        return FALSE;
    }
    virtual int getCheckMsg() {
        return DOOR_MSG_NONE;
    }
    virtual BOOL isNight();

    /* 0x498 */ m3d::anmTexSrt_c mAnmTexSrt;
    /* 0x4C4 */ f32 mAnmRate; // eases to 1 (open) or 0 (closed)
}; // size 0x4C8

// Shampoodle (the barber).
class daStrBarber_c : public daStrTown_c {
  public:
    // Open from 8:00 to 21:00 (isNight, getCheckMsg); the outside lights
    // (isEtcLightOn) from closing to 23:00.
    enum {
        OPEN_HOUR = 8,
        CLOSE_HOUR = 21,
        ETC_LIGHT_OFF_HOUR = 23,
    };

    virtual BOOL hasKnockSe() {
        return FALSE;
    }
    virtual int getDoorOpenSe() {
        return SE_TownGlassDoorOpen;
    }
    virtual int getDoorCreakOpenSe() {
        return SE_TownGlassDoorOpenKishimu;
    }
    virtual int getDoorCloseSe() {
        return SE_TownGlassDoorClose;
    }
    virtual int getDoorCreakCloseSe() {
        return SE_TownGlassDoorCloseKishimu;
    }
    virtual int getKnockSe() {
        return SE_TownGlassDoorKnock;
    }
    virtual int getDoorEndSe() {
        return DOOR_MELODY_BARBER;
    }
    virtual int getCheckMsg();
    virtual BOOL isNight();
    virtual BOOL isEtcLightOn();
}; // size 0x4C8

// The Happy Room Academy.
class daStrHappy_c : public daStrTown_c {
  public:
    // Open from 9:00 to 21:00 (isNight); the outside lights (isEtcLightOn)
    // from 18:00 to closing.
    enum {
        OPEN_HOUR = 9,
        CLOSE_HOUR = 21,
        ETC_LIGHT_ON_HOUR = 18,
    };

    virtual BOOL isNight();
    virtual BOOL isEtcLightOn();
}; // size 0x4C8

// Crazy Redd's (npc_SP/NPC_tunekichi): members only; others need an answered
// invitation (dPrivateData_c::findAnsweredInvite).
class daStrBroker_c : public daStrTown_c {
  public:
    // Open from 10:00 to 1:00 (isNight; npc_SP/NPC_tunekichi MSG_CLOSED).
    enum {
        OPEN_HOUR = 10,
        CLOSE_HOUR = 1,
    };

    // npc_SP/NPC_tunekichi: the door's messages (getCheckMsg / getLockedMsg).
    enum {
        MSG_SOLD_OUT = 0x29, // "Ol' Redd's gone and sold all his merchandise!"
        MSG_CLOSED = 0x2A,   // "Aw, shoot! We're closed!"
        MSG_NOT_MEMBER =
            0x2B, // "Ol' Redd doesn't recognize ya!" (the first time)
        MSG_NOT_MEMBER_AGAIN = 0x2C, // "Ah, no! It's the fuzz!"
        MSG_INVITED = 0x2D,          // a player's invitation, the first time
        MSG_INVITED_AGAIN = 0x2E,    // "Welcome back, friend!"
        MSG_NO_INVITE = 0x2F,        // after a talk about members (getFlagB)
        MSG_NO_INVITE_AGAIN =
            0x30, // "I just can't letcha in without an invitation"
        MSG_INVITED_REFUSED = 0x31,
        MSG_NO_INVITE_REFUSED = 0x32,
        MSG_INVITED_REFUSED_AGAIN = 0x3B,
        MSG_ANIMAL_INVITED =
            0x47, // a villager's invitation: as 0x2D / 0x31 / 0x3B
        MSG_ANIMAL_INVITED_REFUSED = 0x48,
        MSG_ANIMAL_INVITED_REFUSED_AGAIN = 0x49,
    };

    // sys_STRING/STR_SPNpc_name: "?????" (setReddName before the first visit).
    enum {
        NAME_UNKNOWN = 0x34,
    };

    virtual int doDelete();
    virtual int getVoiceMode();
    virtual void onMessageStart(int arg);
    virtual void onMessageEnd(int kind);
    virtual void onTalkStart(int kind, dMsg::Comp_c *comp);
    virtual void afterCreateMdl();
    virtual BOOL isWalkThrough() {
        return TRUE;
    }
    virtual int getDoorOpenSe() {
        return SE_TownSteelDoorOpen;
    }
    virtual int getDoorCreakOpenSe() {
        return SE_TownSteelNormalOpenKishimu;
    }
    virtual int getDoorCloseSe() {
        return SE_TownSteelDoorClose;
    }
    virtual int getDoorCreakCloseSe() {
        return SE_TownSteelDoorCloseKishimu;
    }
    virtual int getKnockSe() {
        return SE_TownSteelDoorKnock;
    }
    virtual int getDoorEndSe() {
        return DOOR_MELODY_BROKER;
    }
    virtual int getCheckMsg();
    virtual int getLockedMsg();
    virtual BOOL isNight();
    virtual BOOL isWindowLit();

    BOOL hasStock();
    BOOL isOpen();
    BOOL isMember();
    BOOL hasInvite();
    static BOOL isReddIn(int msg);
    void setInviteWords();
    void setReddName();

    /* 0x4C8 */ u8 _4C8[0x4D0 - 0x4C8];
}; // size 0x4D0

// Katrina's (the fortune teller).
class daStrFortune_c : public daStrTown_c {
  public:
    // The windows are lit from 18:00 to midnight (isNight); open from 10:00
    // (getCheckMsg).
    enum {
        LIGHT_ON_HOUR = 18,
        LIGHT_OFF_HOUR = 24,
        OPEN_HOUR = 10,
    };

    virtual BOOL hasKnockSe() {
        return FALSE;
    }
    virtual int getDoorOpenSe() {
        return SE_TownNormalDoorOpen;
    }
    virtual int getDoorCreakOpenSe() {
        return SE_TownNormalDoorOpenKishimu;
    }
    virtual int getDoorCloseSe() {
        return SE_TownNormalDoorClose;
    }
    virtual int getDoorCreakCloseSe() {
        return SE_TownNormalDoorCloseKishimu;
    }
    virtual int getKnockSe() {
        return SE_TownNormalDoorKnock;
    }
    virtual int getDoorEndSe() {
        return DOOR_MELODY_FORTUNE;
    }
    virtual int getCheckMsg();
    virtual BOOL isNight();
}; // size 0x4C8

// GracieGrace.
class daStrGrace_c : public daStrTown_c {
  public:
    // Open from 9:00 to 21:00 (isNight, getCheckMsg); the outside lights
    // (isEtcLightOn) from 18:00 to closing.
    enum {
        OPEN_HOUR = 9,
        CLOSE_HOUR = 21,
        ETC_LIGHT_ON_HOUR = 18,
    };

    virtual BOOL isDoorCentered() const {
        return FALSE;
    }
    virtual int getAutoDoorOpenSe() {
        return SE_jidou_door_open_GraceShop;
    }
    virtual int getAutoDoorCloseSe() {
        return SE_jidou_door_close__GraceShop;
    }
    virtual int getCheckMsg();
    virtual BOOL isNight();
    virtual BOOL isEtcLightOn();
}; // size 0x4C8

// The auction house.
class daStrAuction_c : public daStrTown_c {
  public:
    virtual BOOL isNight() {
        return TRUE;
    }
}; // size 0x4C8

// The theater (its neon sign).
class daStrTheater_c : public daStrTown_c {
  public:
    // The windows are lit from 18:00 (isNight) and the neon sign from 10:00
    // (isNeonOn), both to 23:00; the outside lights (isEtcLightOn) from 23:00
    // to 1:00.
    enum {
        LIGHT_ON_HOUR = 18,
        NEON_ON_HOUR = 10,
        LIGHT_OFF_HOUR = 23,
        ETC_LIGHT_OFF_HOUR = 1,
    };

    virtual BOOL isNight();
    virtual BOOL isNeonOn();
    virtual BOOL isEtcLightOn();
}; // size 0x4C8

// The city's fountain.
class daStrTwFountain_c : public daStrTown_c {
  public:
    virtual int doDelete();
    virtual int execute();
    virtual void afterCreateMdl();

    /* 0x4C8 */ m3d::anmChr_c mAnmChr;
    /* 0x500 */ m3d::anmMatClr_c mAnmClr;
    /* 0x52C */ m3d::anmTexPat_c mAnmTexPat;
    /* 0x558 */ m3d::anmTexSrt_c mFountainAnmTexSrt;
}; // size 0x584

// The reset building: open on some nights (SAVE_FLAG_RESET_OPEN, from 20:00).
class daStrReset_c : public daStrTown_c {
  public:
    // Open from 20:00 on the days with SAVE_FLAG_RESET_OPEN.
    enum {
        OPEN_HOUR = 20,
    };

    virtual int execute();
    virtual void afterCreateMdl();
    virtual f32 getMdlScale() {
        return 0.93f;
    }
    virtual BOOL isNight();

    /* 0x4C8 */ u8 mIsOpen;
}; // size 0x4CC

// The ATM.
class daStrAtm_c : public daStrTown_c {
  public:
    virtual BOOL isNight();
}; // size 0x4C8

// A street lamp: lit at night and in bad weather.
class daStrLamp_c : public daStrTown_c {
  public:
    // Lit from 18:00 to 6:00 (isNight).
    enum {
        LIGHT_ON_HOUR = 18,
        LIGHT_OFF_HOUR = 6,
    };

    virtual int doDelete();
    virtual int execute();
    virtual void afterCreateMdl();
    virtual void drawMdl();
    virtual f32 getRoundShadowSize() {
        return 16.0f;
    }
    virtual BOOL isNight();

    /* 0x4C8 */ dObjc::ac_c mObjc;
}; // size 0x50C

// The organ grinder's stand: creates the balloons (ORGAN_BALLOON) and places
// the town BGM's music source.
class daStrOrgan_c : public daStrTown_c {
  public:
    virtual int doDelete();
    virtual int execute();
    virtual void afterCreateMdl();
    virtual f32 getRoundShadowSize() {
        return 24.0f;
    }
}; // size 0x4C8

// The city's bulletin board (as the town's daStrBbs_c, without the bird).
class daStrChest_c : public daStrTown_c {
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

    BOOL setState(int state);
    void executeState();

    BOOL initWait();
    void executeWait();
    BOOL initMenu();
    void executeMenu();
    BOOL initMenuWait();
    void executeMenuWait();

    /* 0x4C8 */ bool mInFront; // the player stands in front of the board
    /* 0x4CC */ int mChestState;
}; // size 0x4D0
