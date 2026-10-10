#pragma once

#include <types.h>
#include <game/game/d_analog_select.hpp>
#include <game/game/d_select.hpp>

class dDemoActor_c;
class dLandID_c;
class dPersonalID_c;
class dPlayerID_c;
class dAnmPersonalID_c;
namespace dItem {
class Item;
}
namespace dScript {
class Word_c;
}
namespace dMsg {
class Rcpt_c;
}

// Actor binding and message-state callbacks of the message controller (TU d_msg.cpp). RTTI names the
// 0x9414-byte controller dMsg::Comp_c (Rcpt_c::mpController); the older dDemo_c name stays for these
// methods because Matching units reference them (see notes/d_msg_rcpt.txt). Layout mostly unknown.
class dDemo_c {
public:
    void attachActor(dMsg::Rcpt_c *rcpt);
    void detachActor();
    void fn_801A4518(); // Message-lock state.
    void fn_801A334C(); // Message-end state.
    void fn_801A4D74(); // 801A4D74: idle/wait state (empty)
    static dDemo_c *mInstance;

    // Message word setters (slot idx of the message's 20 words; names provisional).
    void setNumber(int idx, int value, int width, int format);    // 801A5030
    void setNumber4(int idx, int value);                           // 801A50A8
    void setNumber2(int idx, int value);                           // 801A5118
    void setMinute(int idx, int value);                            // 801A5188
    void formatNumber(int idx, int value, int width, int format); // 801A51F8
    void setDecimal(int idx, int precision, double value);         // 801A5274
    void setMonthName(int idx, int month);                         // 801A52F4
    void setDayName(int idx, int day);                             // 801A5364
    void setPersonalName(int idx, const dPersonalID_c *id);        // 801A53D4
    void setPlayerName(int idx, const dPlayerID_c *id);            // 801A548C
    void setLandName(int idx, const dLandID_c *id);                // 801A5544
    void setAnmPersonalName(int idx, const dAnmPersonalID_c *id);  // 801A55F8
    void setItemName(int idx, const dItem::Item *item);            // 801A566C
    void clearWord(int idx);                                       // 801A56D0
    void setWord(int idx, const dScript::Word_c *word);            // 801A57D8
    void fn_801A5874(int idx, u16 msgId, const char *group);       // 801A5874: word idx = BMG string msgId of group

    typedef void (dDemo_c::*StateFunc)();
    bool isState(StateFunc state) const { return mStateFunc == state; }
    dMsgSelect_c *getSelect() { return &mSelect; }
    dMsgAnalogSelect_c *getAnalogSelect() { return &mAnalogSelect; }

    /* 0x0000 */ u8 _0000[0x70];
    /* 0x0070 */ void (dDemo_c::*mStateFunc)();     // current state (fn_801A334C, fn_801A4D74 ...)
    /* 0x007C */ u8 _007C[0x5610 - 0x7C];
    /* 0x5610 */ dMsgSelect_c mSelect;
    /* 0x5FC4 */ dMsgAnalogSelect_c mAnalogSelect;
    /* 0x5FCC */ u8 _5FCC[0x6C60 - 0x5FCC];
    /* 0x6C60 */ int _6C60;                         // request (1 menu, 2 change speaker, ...)
    /* 0x6C64 */ void (dDemo_c::*mNextStateFunc)(); // set by fn_801A316C
    /* 0x6C70 */ void *mpCurSelect;                 // &mSelect or &mAnalogSelect (fn_801A5A00 / fn_801A5A4C)
    /* 0x6C74 */ dMsg::Rcpt_c *mpRcpt;
    /* 0x6C78 */ u8 _6C78[0x6C7F - 0x6C78];
    /* 0x6C7F */ u8 mLock;                          // 1 while an npc action / player turn runs
    /* 0x6C80 */ u8 _6C80;
    /* 0x6C81 */ u8 mMouthOpen;                     // lip sync (dAcNpc_c::recept_c::isMouthOpen)
    /* 0x6C82 */ u8 _6C82[2];
    /* 0x6C84 */ u16 _6C84;
    /* 0x6C86 */ u16 _6C86;                         // message code to remember (talk_c::onMessageEnd -> rememberMsg)
    /* 0x6C88 */ u8 _6C88[0x6C98 - 0x6C88];
    /* 0x6C98 */ int _6C98;                         // fn_801A316C mode
    /* 0x6C9C */ u8 _6C9C[0x9414 - 0x6C9C];
}; // size 0x9414

extern "C" {
BOOL fn_8018F438(int type); // 8018F438 (d_demo): a demo of this type is running
}
