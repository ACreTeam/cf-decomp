#pragma once

#include <types.h>
#include <game/framework/f_base.hpp>
#include <game/game/d_state_mode.hpp>
#include <game/game/d_m2d.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_string.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_analog_select.hpp>
#include <game/game/d_select.hpp>
#include <game/game/d_msg_seq.hpp>

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

// dMsg::Comp_c (TU d_msg.cpp, code in d_msg_comp.inc): the message controller (fBase_c profile MSG,
// dState::mode_c<dMsg::Comp_c>; Rcpt_c::mpController). Formerly dDemo_c in d_demo.hpp.
// The state machine has 10 modes (table 8047D920, names in CP932):
//   0 hidden, 1 load until shown, 2 load until the next page, 3 appear, 4 prepare,
//   5 replace (word check), 6 display, 7 complete, 8 exit, 9 suspend.

// Comp_c table sizes.
#define MSG_WORD_NUM 20            // Comp_c::mWords: the message's word slots (setWord)
#define MSG_NUM_WORD_NUM 10        // Comp_c::mNumWords: the number slots (setNumber)
#define MSG_RANDOM_WORD_KIND_NUM 7 // Comp_c::mRandomWords: drink, object, sweets, sports, music, food, hobby
#define MSG_CHECK_WORD_NUM 31      // Comp_c::mCheckWords: the words handed to the word checker
#define MSG_NICKNAME_NUM 10        // nickname candidates tried (mNicknames has MSG_CHECK_WORD_NUM entries)

// Comp_c::mVoiceType / mCurVoiceType: the voice type (which voice set the speech uses), 0..12, given to
// the sound system by updateVoiceType (8000FE18 -> 801CCA30, next to the voice mode and mood); Seq_c's
// voice type tag (procVoiceType) sets 5..12.
#define MSG_VOICE_TYPE_NONE 13 // no override: the receiver's Rcpt_c::getVoiceType
// Comp_c::mVoiceMood (Seq_c tags set 0..4).
#define MSG_VOICE_MOOD_NONE 5 // no override: the receiver's Rcpt_c::getVoiceMood
// Comp_c::mRandomNumbers.
#define MSG_RANDOM_NUMBER_NONE 0xFFFF // not rolled yet (Word_c::procRandomNumber)

namespace dMsg {

// Message codes with a meaning of their own (MsgInfo_s::mNextCode, Comp_c::mNextMsgCode).
enum MsgCode_e {
    MSG_CODE_NONE = 0,         // no next message: the window closes
    MSG_CODE_HOLD = 0xFFFE,    // the window stays (mLock) until its owner goes on (sys_MESS/SYS_NetworkError)
    MSG_CODE_INVALID = 0xFFFF, // in no ENG BMG
};

// Comp_c::mSuspendReq: what the window does when the message ends. Anything but SUSPEND_REQ_NONE closes it
// into the suspend state (executeSuspend), where the requester works and then calls requestStart.
// dAcNpc_c::recept_c::setRequest1 stores its argument here too (0 in every caller).
enum SuspendReq_e {
    SUSPEND_REQ_MENU = 1,           // recept_c::stepMenuInit (a menu opens)
    SUSPEND_REQ_CHANGE_SPEAKER = 2, // recept_c::stepChangeSpeakerInit
    SUSPEND_REQ_NONE = 4,           // the window closes and the talk ends (reset)
};

// setWindowColor / getWindowColor: the "msg_win_00_msgWin*.brlan" colour animations.
enum WindowColor_e {
    WINDOW_COLOR_GRAY = 0,
    WINDOW_COLOR_BROWN = 1,
    WINDOW_COLOR_YELLOW = 2,
    WINDOW_COLOR_PINK = 3,
    WINDOW_COLOR_ORANGE = 4,
    WINDOW_COLOR_GREEN = 5,
    WINDOW_COLOR_RED = 6,
    WINDOW_COLOR_NUM = 7,
};

} // namespace dMsg
namespace dMsg {

class Comp_c : public fBase_c, public dState::mode_c<Comp_c> {
public:
    typedef void (Comp_c::*StateFunc)();

    // A message's INF1 entry in the BMG (dScript::Res_c::getEntry).
    struct MsgInfo_s {
        /* 0x00 */ u32 mOffset;
        /* 0x04 */ u16 mNextCode; // the message that follows
        /* 0x06 */ u16 mMemoryCode; // memory code (Comp_c::mMemoryCode)
        /* 0x08 */ u8 mFeel; // feel: group 2 tag id put in front of every word (Comp_c::mFeel)
        /* 0x09 */ u8 mStartReq; // Rcpt_c::onMessageStart request (Comp_c::mStartReq)
        /* 0x0A */ u8 mEndReq; // Rcpt_c::onMessageEnd request (Comp_c::mEndReq)
        /* 0x0B */ u8 mWindow; // window kind: colour (getWindowColor), speed, align
        /* 0x0C */ u8 mFriendship; // friendship change (getFriendshipDelta index)
    };

    static void *createMgr(); // 801A2384 (g_profile_MSG)
    Comp_c(); // 801A23B8
    virtual ~Comp_c(); // 801A2560
    virtual int create(); // 801A2748
    virtual int doDelete(); // 801A29E0
    virtual int execute(); // 801A2AE0
    virtual int draw(); // 801A2CA0

    void attachActor(dMsg::Rcpt_c *rcpt);
    void detachActor();
    static Comp_c *mInstance;

    void updateVoiceType();                                // 801A26DC
    void reset();                                          // 801A2E00: back to the hidden state
    void clearCheckWords();                                // 801A2FE0
    void fn_801A3084();                                    // 801A3084: mForceNext = 0 (complete -> exit, suspend request)
    void fn_801A3090();                                    // 801A3090: mForceNext = 0 (complete -> next page)
    void offerNickname(dScript::Word_c *word, u8 looks);   // 801A309C
    dHmnName::Word_c *getNickname(int idx);                // 801A3158
    void requestStart(int mode);                           // 801A316C
    void setWindowColor(int color);                        // 801A3238
    void setVibration(int level);                          // 801A32A8
    void setVoiceChar(wchar_t c);                           // 801A3304: Seq_c passes the typed character (0x20 after tags)
    void setMouthOpen(BOOL open);                          // 801A331C
    void closeMouth();                                     // 801A333C
    void applyNextMessage();                               // 801A3350
    BOOL isValidMsgCode(u16 code);                         // 801A3978

    // States (mode table 8047D920).
    void initializeHidden();       // 801A3348
    void executeHidden();            // 801A334C: executeHidden (empty)
    void initializeLoadAppear();   // 801A33B0
    void executeLoadAppear();      // 801A3404
    void initializeLoadNextPage(); // 801A356C
    void executeLoadNextPage();    // 801A3570
    void initializeAppear();       // 801A36D8
    void executeAppear();          // 801A380C
    void initializePrepare();      // 801A39C8
    void executePrepare();         // 801A3D8C
    void initializeReplace();      // 801A4040
    void executeReplace();         // 801A4044
    void initializeDisplay();      // 801A42F8
    void executeDisplay();         // 801A42FC
    void initializeComplete();     // 801A44DC
    void executeComplete();            // 801A4518: executeComplete (message lock)
    void initializeExit();            // 801A4A24: initializeExit
    void executeExit();            // 801A4AD0: executeExit
    void initializeSuspend();            // 801A4D70: initializeSuspend
    void executeSuspend();            // 801A4D74: executeSuspend (idle/wait, empty)

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
    void setStringWord(int idx, u16 msgId, const char *group);       // 801A5874: word idx = BMG string msgId of group

    // Used by the states and the message setup.
    void notifyMessageStart();                  // 801A4D78: mpRcpt->onMessageStart(mStartReq)
    void notifyMessageEnd();                  // 801A4D94: mpRcpt->onMessageEnd(mEndReq) (mLock on 0xFFFE)
    void notifyAnswer();                  // 801A4DC4: mpRcpt->onAnswer(0)
    void setNextMsgLabel(const char *label); // 801A4E34: message label
    void setNextMsgCode(u16 code);          // 801A4E44: message code
    BOOL loadBmg();                  // 801A4E4C: loads the receiver's BMG (if needed) and selects it
    void setNumWord(int idx, const dScript::Word_c *word); // 801A4F74: mNumWords[idx] = word (idx < 10)
    void setRandomNumber(int idx, u16 id);   // 801A4FE0: mRandomNumbers[idx] = id
    u16 getRandomNumber(int idx);            // 801A4FF4: mRandomNumbers[idx]
    // 801A5728: word idx = word, flagged when pid is the current player
    void setPersonalWord(int idx, const dScript::Word_c *word, const dPersonalID_c *pid);
    BOOL isWordCleared(int idx);             // 801A5868: mWordCleared[idx]
    void clearWords();                  // 801A590C: clears the word slots
    void clearRandomWords();                  // 801A5978: clears mRandomWords
    void clearNumWords();                  // 801A598C: clears the number words
    void updateSpeakerName();                  // 801A5A98: shows the speaker's name
    void addFriendship(s8 delta);          // 801A5C00: the speaker's friendship with the player += delta
    void updateCursor();                  // 801A5CA8: shows the page cursor / wait icon
    void showWaitIcon();                  // 801A5DE8: shows the wait icon
    void showPageCursor();                  // 801A5E78: shows the page cursor
    void hideCursor();                  // 801A5F08: hides both
    s8 getFriendshipDelta(int idx);             // 801A5F30: friendship change of feel idx (table 8047DA44)
    int getWindowColor(u8 idx);             // 801A5F44: table 8047DA68
    int getTextSpeed(u8 idx);             // 801A5F58: table 8047DACC
    int getTextAlign(u8 idx);             // 801A5F6C: table 8047DB30
    void updateUnitWord();                  // 801A5FD4: sets mUnitWord from the player and speaker

    // Used by dMsg::Seq_c.
    void openSelect();                  // 801A5A00: opens the answer list (mSelect)
    void openAnalogSelect();                  // 801A5A4C: opens the two-way select (mAnalogSelect)

    // Used by dMsg::Word_c.
    dScript::Word_c *getNumWord(u8 idx);           // 801A4FD0: &mNumWords[idx]
    void setAnimalParam(dAnimal_c *animal, int idx);   // 801A5008: mAnimalParam[idx] = animal
    dAnimal_c *getAnimalParam(int idx);                // 801A501C: mAnimalParam[idx]
    dScript::Word_c *getWord(u8 idx);           // 801A5858: &mWords[idx]
    u16 getRandomWord(int kind, int idx);             // 801A58D4: random word of kind used for slot idx (+0x8A90)
    void setRandomWord(int kind, int idx, u16 msgId); // 801A58F0: sets it
    void setUnitWord(const dScript::Word_c *word);  // 801A5F80: mUnitWord = word

    bool isState(StateFunc state) const { return this->mCurrentMethod == state; }
    dMsgSelect_c *getSelect() { return &mSelect; }
    dMsgAnalogSelect_c *getAnalogSelect() { return &mAnalogSelect; }

    /* 0x0000 */ // fBase_c (vtable at +0x60)
    /* 0x0064 */ // dState::mode_c<Comp_c> (+0x70: the current state, mCurrentMethod)
    /* 0x0088 */ m2d::ResAccLoader_c mResAcc;          // "Layout/msg_win.arc"
    /* 0x015C */ m2d::SimpleEx_c mLyt;                 // "msg_win_00.brlyt"
    /* 0x01F4 */ m2d::Anm_c mAnmWin;                   // in / out / colour animations
    /* 0x0218 */ m2d::Anm_c mAnmWaitIcon;              // "msg_win_00_waitIcn_00.brlan"
    /* 0x023C */ m2d::Anm_c mAnmVib;                   // "msg_win_00_msgWinVib1..3.brlan"
    /* 0x0260 */ dDvd::loader_c mLoader;               // the message file
    /* 0x0274 */ dMsg::Seq_c mSeq;                     // the text (vtable 804FB538)
    /* 0x55F8 */ dMsg::Voice_c mVoice;                 // vtable 804FB6A0
    /* 0x5610 */ dMsgSelect_c mSelect;
    /* 0x5FC4 */ dMsgAnalogSelect_c mAnalogSelect;
    /* 0x6C60 */ int mSuspendReq;                         // SuspendReq_e (set by the npc requests)
    /* 0x6C64 */ StateFunc mNextStateFunc;          // set by requestStart, applied by execute
    /* 0x6C70 */ dMsgSelectBase0_c *mpCurSelect;                   // &mSelect or &mAnalogSelect (openSelect / openAnalogSelect)
    /* 0x6C74 */ dMsg::Rcpt_c *mpRcpt;
    /* 0x6C78 */ const EGG::MsgRes::MessageHeader *mpBmg;            // the message data (BMG)
    /* 0x6C7C */ u8 mWaitIcon; // complete state: wait icon + hold SE (dSvMgr_c::lockMessage, d_sv_demo)
    /* 0x6C7D */ u8 mWaitIconSe; // its SE: 0 0x16B7, 1 0x16B8 (the gatekeeper REL sets 1)
    /* 0x6C7E */ u8 mPageDone; // complete state: the page is confirmed (answer taken / key pressed)
    /* 0x6C7F */ u8 mLock;                          // 1 while an npc action / player turn runs
    /* 0x6C80 */ u8 mForceNext; // complete state: go on without the key (RELs set it)
    /* 0x6C81 */ u8 mMouthOpen;                     // lip sync (dAcNpc_c::recept_c::isMouthOpen)
    /* 0x6C82 */ u8 mNoCloseSe; // no close SE 0x16BD (dMsgSelect_c sets it, 8015DC5C)
    /* 0x6C83 */ u8 mBgmKeep; // VolStateTalk_c::end(keep) on exit (d_sv_demo sets it)
    /* 0x6C84 */ u16 mNextMsgCode;                  // queued message code (setNextMsgCode sets it, applyNextMessage hands
                                                    // it to mpRcpt->setMessageCode and clears it; 0xFFFE detach);
                                                    // the talk procedures wait until it is 0
    /* 0x6C86 */ u16 mMemoryCode;                         // BMG entry +0x6: memory code (talk_c::onMessageEnd -> rememberMsg)
    /* 0x6C88 */ u32 mStartReq;                         // BMG entry +0x9: Rcpt_c::onMessageStart request
    /* 0x6C8C */ u32 mEndReq;                         // BMG entry +0xA: Rcpt_c::onMessageEnd request
    /* 0x6C90 */ u32 mFeel;                         // BMG entry +0x8: feel (Word_c::getParseStart: group 2 tag)
    /* 0x6C94 */ u8 mWindowColor;                   // setWindowColor
    /* 0x6C95 */ u8 _6C95[3];
    /* 0x6C98 */ int mStartMode;                         // requestStart mode (only stored)
    /* 0x6C9C */ u8 mVoiceType;                     // Seq_c override of the receiver's type (MSG_VOICE_TYPE_NONE: none)
    /* 0x6C9D */ u8 mVoiceMood; // Seq_c override of Rcpt_c::getVoiceMood (MSG_VOICE_MOOD_NONE: none)
    /* 0x6C9E */ u8 _6C9E[2];
    /* 0x6CA0 */ dString::Word_c mWords[MSG_WORD_NUM];        // the message's word slots
    /* 0x7F60 */ u8 mWordCleared[MSG_WORD_NUM];            // clearWord: 1, setWord: 0
    /* 0x7F74 */ dString::Word_c mNumWords[MSG_NUM_WORD_NUM];     // number words
    /* 0x88D4 */ u16 mRandomNumbers[MSG_NUM_WORD_NUM]; // rolled values of the random number slots (MSG_RANDOM_NUMBER_NONE: not yet)
    /* 0x88E8 */ char mBmgLabel[0x3D];             // label of the loaded BMG (loadBmg)
    /* 0x8925 */ char mNextMsgLabel[0x3D];          // queued message label (setNextMsgLabel)
    /* 0x8962 */ u8 _8962[2];
    /* 0x8964 */ dString::Word_c mUnitWord;         // counter unit of the last named villager (setUnitWord)
    /* 0x8A54 */ dAnimal_c *mAnimalA;               // best-compatibility villager (dMsg::Word_c::pickAnimalA)
    /* 0x8A58 */ dAnimal_c *mAnimalB;               // worst-compatibility villager (pickAnimalB)
    /* 0x8A5C */ dAnimal_c *mAnimalParam[6];        // villager slots (setAnimalParam / getAnimalParam)
    /* 0x8A74 */ nw4r::lyt::Pane *mpPageCursor;     // "pageCrsr_00"
    /* 0x8A78 */ nw4r::lyt::Pane *mpWaitIcon;       // "waitIcn_00"
    /* 0x8A7C */ nw4r::lyt::Pane *mpNamePane;       // "N_msgName"
    /* 0x8A80 */ nw4r::lyt::TextBox *mpNameText;    // "T_msgName"
    /* 0x8A84 */ nw4r::lyt::Pane *mpNameFrame[3];   // "msgName_frm_00..02"
    /* 0x8A90 */ u16 mRandomWords[MSG_RANDOM_WORD_KIND_NUM][MSG_WORD_NUM]; // random word msg ids per kind and slot
                                                 // (Word_c::procRandomWordTag; getRandomWord / setRandomWord)
    // The words of the message that the word checker (dPrcMng_c::startCheck) looks at.
    /* 0x8BA8 */ dScript::Word_c *mCheckWords[MSG_CHECK_WORD_NUM];
    /* 0x8C24 */ const wchar_t *mCheckStrs[MSG_CHECK_WORD_NUM];
    /* 0x8CA0 */ s8 mCheckResults[MSG_CHECK_WORD_NUM];
    /* 0x8CBF */ u8 _8CBF;
    /* 0x8CC0 */ int mCheckNum;
    /* 0x8CC4 */ int mCheckHits;
    /* 0x8CC8 */ u8 mCurVoiceType;                  // the type last given to the sound system (updateVoiceType)
    /* 0x8CC9 */ u8 _8CC9[3];
    /* 0x8CCC */ dHmnName::Word_c mNicknames[MSG_CHECK_WORD_NUM]; // nickname candidates (MSG_NICKNAME_NUM used)
    /* 0x9394 */ u8 mNicknameReq;                   // offerNickname
    /* 0x9396 */ u16 mNicknameSuffix;
    /* 0x9398 */ dHmnName::Word_c mNicknameWord;
    /* 0x93D0 */ int mNicknameCheckNum;             // mCheckNum before the nicknames
    /* 0x93D4 */ dHabitWord_c mCatchphrase;         // the speaker's catchphrase (procCatchphrase)
    /* 0x9410 */ int mCatchphraseCheckNum;          // mCheckNum before the catchphrase
}; // size 0x9414

} // namespace dMsg
