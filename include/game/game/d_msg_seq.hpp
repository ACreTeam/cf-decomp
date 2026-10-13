#pragma once

// dMsg::Seq_c (TU d_msg.cpp): types the message text out character by character into the window's
// text box, runs the message tags (waits, pages, answers, random/conditional jumps) and drives the
// receiver hooks. Owned by the controller (dMsg::Comp_c). Names from RTTI:
// dMsg::Seq_c (vtable 804FB538), dMsg::RndJump_c (804FB56C), dMsg::RndJumpData_c (804FB588),
// dMsg::Ruby_c (804FB550).

#include <types.h>
#include <game/game/d_msg_word.hpp>

namespace nw4r {
namespace lyt {
class TextBox;
}
} // namespace nw4r

namespace dMsg {
class Comp_c;
}

namespace dMsg {

// 801A20D8 / 801A2120: key checks used by Seq_c::execute (names provisional).
BOOL isNextTrig(); // 801A20D8: the next-page / key-wait trigger (pad 800FA748 or 800FA760)
BOOL isSkipTrig(); // 801A2120: the skip trigger: types the rest of the page (pad 800FA760 or 800FA76C)

#define SEQ_JUMP_CHOICE_MAX 8 // message codes in a jump tag
#define SEQ_ANSWER_CODE_CANCEL 0xFFFF // answer code: the cancel answer, no next message (SYS_Save_and_Load)
#define MSG_SE_NEXT 0x16C3 // the key advances the text (Seq_c::execute, Comp_c)
#define RND_JUMP_NUM 8 // random jumps remembered

// Seq_c::start's speed (from the BMG entry's window byte).
enum SeqSpeed_e {
    SEQ_SPEED_NORMAL,
    SEQ_SPEED_NO_WAIT, // the whole text at once
    SEQ_SPEED_NO_SKIP, // the skip key doesn't speed it up
};

// Seq_c::start's horizontal alignment.
enum SeqAlign_e {
    SEQ_ALIGN_LEFT,
    SEQ_ALIGN_CENTER,
};

// TAG_JUMP_HELD_ITEM's parameter: the tool the player holds (choice 0 when held).
enum JumpHeldItem_e {
    JUMP_HELD_UMBRELLA,
    JUMP_HELD_FISHINGROD,
    JUMP_HELD_NET,
    JUMP_HELD_SCOOP,
    JUMP_HELD_WATERING,
    JUMP_HELD_AXE,
    JUMP_HELD_PACHINKO,
    JUMP_HELD_CRACKER,
    JUMP_HELD_SYABON,
    JUMP_HELD_WINDMILL,
    JUMP_HELD_SPARKLER,
    JUMP_HELD_ROMAN_CANDLE,
    JUMP_HELD_BALLOON,
    JUMP_HELD_FLOWER,
};

// TAG_JUMP_BUILDING's parameter.
enum JumpBuilding_e {
    JUMP_BUILDING_BRIDGE,
    JUMP_BUILDING_FOUNTAIN,
    JUMP_BUILDING_WINDMILL,
    JUMP_BUILDING_LIGHTHOUSE,
};

// One remembered random jump: the choice last taken by message (code, label).
class RndJumpData_c {
public:
    RndJumpData_c(); // 8019FCB8 (inline, body in d_msg.cpp)
    virtual ~RndJumpData_c(); // 8019FCC8 (inline, body in d_msg.cpp)

    void clear(); // 8019C414
    void set(u16 msgCode, const char *label, u8 choice); // 8019C45C
    int get(u16 msgCode, const char *label); // 8019C4A8: the choice, -1 if this isn't the message

    /* 0x04 */ u16 mMsgCode;
    /* 0x06 */ char mLabel[61];
    /* 0x43 */ u8 mChoice;
}; // size 0x44

// The last 8 random jumps, most recent first.
class RndJump_c {
public:
    virtual ~RndJump_c(); // 8019FD08 (inline, body in d_msg.cpp)

    void clear(); // 8019C4FC
    void add(u16 msgCode, const char *label, u8 choice); // 8019C548: moves (or inserts) it to the front
    int get(u16 msgCode, const char *label); // 8019C61C: the choice, -1 if not remembered

    /* 0x004 */ RndJumpData_c mData[RND_JUMP_NUM];
}; // size 0x224

// Ruby (furigana, tag 0xFF:2): while it runs, the voice reads the ruby text instead of the base characters.
// Each step voices one ruby character; the base text advances one character per mStep ruby characters.
// 8019F9B8..8019FAB0.
class Ruby_c {
public:
    Ruby_c() { clear(); } // inlined into the controller ctor (801A23B8)
    virtual ~Ruby_c(); // 8019FD70 (inline, body in d_msg.cpp)

    void clear(); // 8019F9B8: clear
    void set(Comp_c *comp, int baseLen, int len, const wchar_t *text); // 8019F9E0
    BOOL stepBaseChar(); // 8019FA2C: TRUE when the base text should advance a character now
    BOOL isEnd(); // 8019FA94: all ruby characters voiced (mPos >= mLen)
    void procChar(); // 8019FAB0: voices the next ruby character

    /* 0x04 */ Comp_c *mpComp;
    /* 0x08 */ const wchar_t *mpText;
    /* 0x0C */ int mPos;
    /* 0x10 */ int mBasePos; // base characters released
    /* 0x14 */ int mBaseLen; // base characters under the ruby
    /* 0x18 */ int mLen;
    /* 0x1C */ int mStep; // ruby characters per base character (>= 1)
}; // size 0x20

// Seq_c states (setState; each calls a receiver hook).
enum SeqState_e {
    SEQ_STATE_NONE,
    SEQ_STATE_TYPE,      // 1: typing (Rcpt_c::onTypeStart)
    SEQ_STATE_PAGE_WAIT, // 2: page full, waits for a key (onPageWait)
    SEQ_STATE_END,       // 3: text end (onTextEnd)
    SEQ_STATE_KEY_WAIT,  // 4: tag 6:1, waits for a key (onKeyWait)
    SEQ_STATE_SELECT,    // 5: answer select (onSelectStart)
    SEQ_STATE_SELECTING, // 6: the controller opened the select (open), waits for the answer
};

class Seq_c {
public:
    typedef void (Seq_c::*TagFunc)();

    // Explicit (not the implicit ctor): the order of the inline copies at 8019FCB8 depends on it.
    Seq_c() {}
    virtual ~Seq_c(); // 8019FDB0 (inline, body in d_msg.cpp)

    // 8019C698..8019EC88
    BOOL create(Comp_c *comp, nw4r::lyt::TextBox *textBox); // 8019C698
    BOOL remove(); // 8019C700
    int calcTimer(); // 8019C73C: counts mTimer down; -1 when off
    void setState(int state); // 8019C778
    void execute(); // 8019C844
    void clearText(); // 8019CAB4
    void start(int speed, int align); // 8019CB0C
    void setText(const wchar_t *text); // 8019CCB0
    void nextPage(); // 8019CD24
    BOOL procChar(); // 8019CD88: FALSE when typing stops (page full, end)
    wchar_t getChar(const wchar_t *p); // 8019CFA4: the first character at p after any tags
    BOOL isTypeReady(); // 8019CFEC: FALSE while waiting
    void procTag(); // 8019D0C8: dispatch by tag group
    void procFeel(); // 8019D240: group 2: Rcpt_c::setSpeakerFeel(id)
    void procBlockTag(); // 8019D28C: group 6 (display control)
    void procSelectTag(); // 8019D3AC: group 7 (answers)
    void procJumpTag(); // 8019D428: group 8 (jumps)
    void procMoodTag(); // 8019D4A4
    void procSpeakerTag(); // 8019D520
    void procSoundTag(); // 8019D59C
    void procSystemTag(); // 8019D618: group 0xFF
    void procWait(); // 8019D6CC: 6:0
    void procKeyWait(); // 8019D74C: 6:1
    void procNoWaitOn(); // 8019D78C: 6:2
    void procNoWaitOff(); // 8019D7A0: 6:3
    void procNoSkipOn(); // 8019D7B4: 6:4
    void procNoSkipOff(); // 8019D7C8: 6:5
    void procTimer(); // 8019D7D4: 6:10
    void procTimerAuto(); // 8019D83C: 6:11
    void procVibration1(); // 8019D8A0: 6:12: Comp_c::setVibration(0) (msgWinVib1)
    void procVibration2(); // 8019D8AC: 6:13
    void procVibration3(); // 8019D8B8: 6:14
    void procPay(); // 8019D8C4: 6:15: pays the money in the parameter, if the player has it
    void procCustom0(); // 8019D92C: 6:21
    void procCustom1(); // 8019D948: 6:22
    void procCustom2(); // 8019D964: 6:23
    void procCustom3(); // 8019D980: 6:24
    void procCustom4(); // 8019D99C: 6:25
    void procPageEnd(); // 8019D9B8: 6:26
    void procSelect(); // 8019D9F4: 7:0-7: answer list
    void procAnalogSelect(); // 8019DB88: 7:8: two-way select
    int selectRndJump(int choice, int num); // 8019DCE4
    void procRndJump(int num); // 8019DCEC
    void procRndJump2(); // 8019DE08: 8:0
    void procRndJump3(); // 8019DE10: 8:1
    void procFriendshipJump(); // 8019DE18: 8:2
    void procPlayerGenderJump(); // 8019DEFC: 8:3
    void procHouseSizeJump(); // 8019DF78: 8:4: house size (dPrivateData_c::getHouseSize)
    void procInsectCatalogJump(); // 8019DFEC: 8:5
    void procFishCatalogJump(); // 8019E0F0: 8:6
    void procAnimalNumJump(); // 8019E1F4: 8:7
    void procTimeOfDayJump(); // 8019E284: 8:8
    void procWeatherJump(); // 8019E340: 8:9
    void procPlayerNumJump(); // 8019E3B8: 8:10
    void procWeekdayJump(); // 8019E430: 8:11
    void procVisitorJump(); // 8019E4F8: 8:12
    void procSpeciesJump(); // 8019E5C4: 8:13
    void procMoneyJump(); // 8019E678: 8:14
    void procPocketFullJump(); // 8019E71C: 8:15
    void procTanJump(); // 8019E7A8: 8:16
    void procOnlineJump(); // 8019E820: 8:17
    void procMemberNumJump(); // 8019E888: 8:18
    void procRndJump4(); // 8019E8F8: 8:19
    void procRndJump5(); // 8019E900: 8:20
    void procRndJump6(); // 8019E908: 8:21
    void procRndJump7(); // 8019E910: 8:22
    void procRndJump8(); // 8019E918: 8:23
    void procSeasonJump(); // 8019E920: 8:24
    void procShopStageJump(); // 8019E984: 8:25
    void procMiiMaskJump(); // 8019E9EC: 8:26: Mii mask (dPrivateData_c::mMiiMask, inferred)
    void procBeeStingJump(); // 8019EA68: 8:27: bee-stung face (flag0 2)
    void procSceneJump(); // 8019EAE4: 8:28
    void procHourJump(); // 8019EB54: 8:29

    bool isTimerAuto() const { return mTimerAuto; } // Comp_c::updateCursor
    BOOL isTimer(int timer) const { return mTimer == timer; } // Comp_c::executeDisplay

    // 8019EC88..8019F9B8: the remaining tag handlers
    void procHeldItemJump(); // 8:30
    void procHairJump(); // 8:31
    void procBuildingJump(); // 8:32
    void procKicksJump(); // 8:33
    void procPhineasJump(); // 8:34
    void procFlag0x28Jump(); // 8:35
    u16 getJumpCode(const u8 *params, int choice, int num); // 8019F378: message code of the choice
    void procMoodReset(); // 9:0
    void procMood1(); // 9:1
    void procMood2(); // 9:2
    void procMood3(); // 9:3
    void procMood4(); // 9:4
    void procSpeakerPlayer(); // 10:0
    void procSpeakerNpc(); // 10:1
    void procSpeakerPartner(); // 10:2
    void procLookAtPlayer(); // 10:3
    void procTurnToPlayer(); // 10:4
    void procLookAtNpc(); // 10:5
    void procTurnToNpc(); // 10:6
    void procLookAtPartner(); // 10:7
    void procTurnToPartner(); // 10:8
    void procChangeSpeaker(); // 10:9
    void procColor(); // 0xFF:0
    void procScale(); // 0xFF:1
    void procRuby(); // 0xFF:2
    void procSe(); // 11:0
    void fn_8019F80C(); // 11:1
    void fn_8019F810(); // 11:2
    void procTownTune(); // 11:3
    void procVoiceType(); // 11:4
    void procVoiceTypeReset(); // 11:5
    void procVoiceMood(); // 11:6
    void procVoiceMoodReset(); // 11:7

    /* 0x0004 */ nw4r::lyt::TextBox *mpTextBox;
    /* 0x0008 */ int mState;          // SeqState_e
    /* 0x000C */ int mPos;            // characters typed on this page
    /* 0x0010 */ u16 mWait;           // frames to the next character
    /* 0x0014 */ int mTimer;          // auto advance (tags 6:10/11), -1 off
    /* 0x0018 */ u16 mPageStart;      // the page's start in mWord
    /* 0x001A */ u16 mPageLines;      // lines on this page (4 = full)
    /* 0x001C */ u16 mLine;           // line on this page (tag processor +7)
    /* 0x001E */ u16 mLineTotal;      // lines so far
    /* 0x0020 */ u16 mTextLen;        // mWord's text length
    /* 0x0024 */ Comp_c *mpComp;
    /* 0x0028 */ u8 _0028[0x30 - 0x28]; // never accessed in the DOL
    /* 0x0030 */ const wchar_t *mpChar; // the character being typed
    /* 0x0034 */ u8 mFast;            // skip pressed: no waits
    /* 0x0035 */ u8 mNoWait;          // tags 6:2/6:3: the whole text at once
    /* 0x0036 */ u8 mNoSkip;          // tags 6:4/6:5
    /* 0x0037 */ u8 mTimerAuto;       // tag 6:11
    /* 0x0038 */ u8 mTyped;           // a character was typed
    /* 0x003C */ RndJump_c mRndJump;
    /* 0x0260 */ Word_c mWord;
    /* 0x2B60 */ wchar_t mText[0x1401]; // what the text box shows
    /* 0x5364 */ Ruby_c mRuby;
}; // size 0x5384

// dMsg::Voice_c is in d_msg_voice.hpp (included by d_msg_word.hpp: its RTTI follows dMsg::Word_c's).
} // namespace dMsg
