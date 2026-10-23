#pragma once

// City Folk's textinput (software keyboard) library as the game sees it: only what d_kbd uses.
// The lib itself (lib/keyboard, .text 802E5xxx..80316xxx) is not decompiled; lib/keyboard/include has
// the Wii menu (IPL) version of these headers, whose vtables differ from City Folk's. Names follow the
// IPL headers where the City Folk slot does the same thing; vt_0xNN are slots of unknown purpose.

#include <types.h>
#include <revolution/mem.h>
#include <nw4r/ut.h>

namespace textinput {

enum Language {
    JP,
    USA,
    UK,
    FR,
    DE,
    IT,
    SP,
    NL,
    CN,
    KR,
    LanguageEnd,
};

enum Destination {
    DST_JP,
    DST_US,
    DST_EU,
    DST_CN,
    DST_KR,
    DST_Last,
};

namespace sound {
// The sound requests EventObserver::onSE gets (IPL names).
enum SE {
    SE_0,
    SE_1,
    SE_2,
    SE_LANGUAGE_SWITCHING,
    SE_SELECT,
    SE_CHAR_CURSOR,
    SE_CHAR_CURSOR_FIX,
    SE_CHAR_DELETE,
    SE_CHAR_DELETE_ERROR,
    SE_CHAR_DECIDE,
    SE_CHAR_INPUT,
    SE_LINE_SCROLL,
    SE_12,
    SE_KETAI_MODE_SWITCHING,
    SE_SYMBOL_PAGE_OPEN,
    SE_15,
    SE_PREDICT_ON,
    SE_PREDICT_OFF,
    SE_18,
    SE_LANGUAGE_PAGE_OPEN,
    SE_CHAR_LARGER,
    SE_CHAR_LOWER,
    SWITCH_TO_KEYBOARD,
    SWITCH_TO_KETAI,
    SE_BT_TARGETTING,
    SE_DECIDE,
    SE_MESSAGE_SCROLL,
    SE_OPEN,
    SE_APPEARED,
    SE_DISAPPEARD,
    SE_Last,
};
} // namespace sound

class Base {
public:
    virtual ~Base() {}                               // 0x08
    virtual void create(MEMAllocator *allocator) {}  // 0x0C
    virtual void init() {}                           // 0x10
};

class CommandSender;

class CommandReceiver : public Base {
public:
    enum INPUT_COMMAND {
        INPUT_COMMAND_0 = 0,
        // A character from a key: data is a KeyCommand.
        INPUT_COMMAND_KEY = 5,
    };

    struct Scroll {
        /* 0x0 */ bool absY;
        /* 0x4 */ f32 x;
        /* 0x8 */ f32 y;
    }; // size 0xC

    CommandReceiver() { clearSender(); }

    virtual ~CommandReceiver() {}                                   // 0x08
    virtual void clearSender();                                     // 0x14
    virtual void onCommand(INPUT_COMMAND command, void *data);      // 0x18
    virtual void addSender(CommandSender *sender) {                 // 0x1C
        nw4r::ut::List_Append(&mSenderList, sender);
    }

private:
    /* 0x04 */ nw4r::ut::List mSenderList;
}; // size 0x10

class CommandSender : public Base {
public:
    virtual void setCommandReceiver(CommandReceiver *receiver); // 0x14
    virtual void sendCommand(u32 command, void *data) {         // 0x18
        if (mpCommandReceiver != NULL) {
            mpCommandReceiver->onCommand((CommandReceiver::INPUT_COMMAND)command, data);
        }
    }
    virtual void updateFromReceiver(u32, void *); // 0x1C

    /* 0x04 */ nw4r::ut::Link mLink;

private:
    /* 0x0C */ CommandReceiver *mpCommandReceiver;
}; // size 0x10

} // namespace textinput
