#pragma once

// The software keyboard: the game side of the textinput library
// (src/dol/game/d_kbd.cpp). Class names from the RTTI (dKbd::myManager_c,
// dKbd::ResLoader_c, dKbd::Event_c, dKbd::Listener_c, dKbd::Mng_c,
// dKbd::Cur_c); member and function names are inferred. See
// notes/TUs/d_kbd.txt.

#include <types.h>
#include <revolution/MEM.h>
#include <game/game/d_dvd.hpp>
#include <game/game/d_m2d.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/mLib/m_vec.hpp>
#include <lib/textinput/MyTiManager.h>
#include <nw4r/lyt.h>

namespace dKbd {

// The textinput manager: the memo manager with the game's drawing order.
class myManager_c : public textinput::extend::memo::Manager {
  public:
    myManager_c(MEMAllocator *allocator,
                nw4r::lyt::MultiArcResourceAccessor *multiArc,
                textinput::EventObserver *event)
        : textinput::extend::memo::Manager(allocator, multiArc, event) {
    }
    virtual ~myManager_c(); // 800C9230
    virtual void draw();    // 800C7D64
}; // size 0x78

// Loads the keyboard archive "Layout/sofkeybd.arc" into its own heap and
// registers the game's font.
class ResLoader_c {
  public:
    ResLoader_c();          // 800C7EA4
    virtual ~ResLoader_c(); // 800C7F30

    BOOL load();   // 800C7FBC: TRUE once loaded
    BOOL remove(); // 800C8074: FALSE while the loader can't be released yet
    BOOL createHeap(EGG::Heap *parent, size_t size); // 800C8118

    /* 0x004 */ void *mpArc;
    /* 0x008 */ mHeapAllocator_c mAllocator;
    /* 0x024 */ dDvd::loader_c mLoader;
    /* 0x038 */ nw4r::lyt::MultiArcResourceAccessor mResAcc;
    /* 0x054 */ nw4r::lyt::ArcResourceLink mArcLink;
    /* 0x0F8 */ nw4r::lyt::FontRefLink mFontLink;
    /* 0x184 */ bool mHeapCreated;
}; // size 0x188

// The keyboard's callbacks: collects what happened this frame (EVENT_*).
class Event_c : public textinput::EventObserver {
  public:
    enum Event_e {
        EVENT_OK = 0x1,
        EVENT_CANCEL = 0x2,
        EVENT_CHAR_INPUT = 0x4,
        EVENT_CHAR_DELETE = 0x8,
        EVENT_DECIDE = 0x10,
        EVENT_CURSOR_FIX = 0x20,
        EVENT_CURSOR = 0x40,
    };

    Event_c();          // 800C816C
    virtual ~Event_c(); // 800C8184

    virtual void onOK();                          // 800C81C4
    virtual void onCancel();                      // 800C81D4
    virtual void onSE(textinput::sound::SE seId); // 800C81E4

    /* 0x4 */ u16 mEvents;
}; // size 0x8

// The USB keyboard's keys the game handles itself (the emotion keys, send /
// clear, digits).
class Listener_c : public textinput::input::KBDListener {
  public:
    enum Request_e {
        REQUEST_NONE,
        REQUEST_CLEAR,
        REQUEST_SEND,
        REQUEST_EMOTION_0, // .. REQUEST_EMOTION_0 + 3
    };

    enum Key_e {
        KEY_REPEAT = 0xF001,
        KEY_EMOTION_0 = 0xF065, // .. 0xF068
        KEY_SEND = 0xF10D,
        KEY_PAD_0 = 0xF130, // .. 0xF139: the keypad's digits
        KEY_DIGIT_11 = 0xF1AE,
        KEY_SPECIAL_4 = 0xF1B2,
        KEY_IGNORED_0 = 0xF1B3,
        KEY_SPECIAL_1 = 0xF1B4,
        KEY_IGNORED_1 = 0xF1B5,
        KEY_SPECIAL_2 = 0xF1B6,
        KEY_IGNORED_2 = 0xF1B7,
        KEY_SPECIAL_3 = 0xF1B8,
        KEY_DIGIT_10 = 0xF1C8,
        KEY_SEND_2 = 0xF1CD,
        KEY_CLEAR = 0xF1DB,
    };

    enum {
        KEY_HOLD_FRAMES = 5,
        DIGIT_NONE = 0xFF,
        DIGIT_10 = 10,          // KEY_DIGIT_10
        DIGIT_11 = 11,          // KEY_DIGIT_11
        DIGIT_RESULT_NONE = -1, // getDigit() without a digit
    };

    virtual void vt_0x0C();                                // 800C82E8
    virtual void vt_0x10();                                // 800C82EC
    virtual void onKey(textinput::input::KeyEvent *event); // 800C82F0

    BOOL isKeyHeld(); // 800C84C8
    void
    calc(); // 800C84DC: counts the hold time down, then forgets the request
    void clear(); // 800C8504
    void
    calcClear(); // 800C8520: counts the hold time down, then clears everything
    int getDigit();     // 800C8540: 0..11, -1 if none
    u8 getSpecialKey(); // 800C8570: 1..4, 0 if none
    int getEmotion();   // 800C8578: 0..3, -1 if none

    /* 0x0C */ u8 mHoldTimer;
    /* 0x0D */ u8 mRequest;
    /* 0x0E */ u8 mDigit;
    /* 0x0F */ u8 mSpecialKey;
}; // size 0x10

// The keyboard and its text input (a textinput manager in its own heap).
class Mng_c : public m2d::Base_c {
  public:
    enum Flag_e {
        FLAG_INIT = 0x1,
        FLAG_CELL_PHONE =
            0x4, // the cell phone keypad instead of the PC keyboard
        FLAG_CANDIDATE = 0x8, // a keypad key is being cycled
        FLAG_LAYOUT = 0x10,   // the keyboard base layout is built
        FLAG_CHANGED = 0x40,  // something was input this frame
        FLAG_OPEN = 0x80,
    };

    enum CreateStep_e {
        CREATE_STEP_START,
        CREATE_STEP_MANAGER, // the manager is being created in a thread
        CREATE_STEP_SETUP,   // the manager is being set up in a thread
        CREATE_STEP_DONE,
    };

    enum SetupMode_e {
        SETUP_MODE_0,
        SETUP_MODE_1,
    };

    Mng_c();             // 800C87D0
    virtual ~Mng_c();    // 800C8890
    virtual void draw(); // 800C9980

    // init modes: the menus' number pad follows the key SE option (cell-phone
    // keypad), the on-screen keyboard always uses the full keyboard.
    enum InitMode_e {
        INIT_MODE_PAD,
        INIT_MODE_KEYBOARD,
    };
    // remove modes: what is saved for the next use.
    enum RemoveMode_e {
        REMOVE_MODE_SAVE_SETTING,
        REMOVE_MODE_SAVE_INPUT_MODE,
    };

    BOOL init(int mode);     // 800C8954: InitMode_e
    void setup(int mode);    // 800C8A44
    BOOL loadRes();          // 800C8B94
    void createManager();    // 800C8BE4
    void initManager();      // 800C8CA0
    void createDirect();     // 800C8F68: create() without the threads
    BOOL create();           // 800C8FB4: TRUE once done
    void buildLayout();      // 800C906C
    BOOL remove(int mode);   // 800C90F8: RemoveMode_e
    void calc();             // 800C9288
    void calcClear();        // 800C9330
    void updatePointerOff(); // 800C9368
    BOOL updatePointer();    // 800C9434
    BOOL updateKeyboard();   // 800C966C
    void open(int noPointer, int noUpdate); // 800C96EC
    BOOL reopen();                          // 800C97C4
    void updateNoInput();                   // 800C983C
    BOOL isTextChanged();                   // 800C9868
    BOOL isSendReq();                       // 800C9938
    BOOL isClearReq();                      // 800C994C
    int getDigit();                         // 800C9960
    int getEmotion();         // 800C9968: the chosen emotion slot, -1 if none
    u8 getSpecialKey();       // 800C9970
    void clearEmotion();      // 800C9978
    void postCalc();          // 800C99DC
    bool isOK();              // 800C9A1C
    bool isCancel();          // 800C9A34
    bool isCharInput();       // 800C9A4C
    bool isCharDelete();      // 800C9A64
    bool isDecide();          // 800C9A7C
    bool isCursorFixed();     // 800C9A94
    bool isCursorMoved();     // 800C9AAC
    void setMemoMode1();      // 800C9AC4
    void setMemoMode2();      // 800C9ADC
    BOOL isMemoEmpty();       // 800C9AF4
    void setPos(mVec3_c pos); // 800C9B3C
    const wchar_t *getText(); // 800C9CD8
    void setText(const wchar_t *str); // 800C9CEC
    void setMaxLength(int len);       // 800C9D00
    int getInputPos();                // 800C9D14
    int getCursorPos();               // 800C9D48
    int getInputLength();             // 800C9D8C
    int getTextLength();              // 800C9DB0
    void setCursorPos(int pos);       // 800C9E0C
    void saveCursorPos();             // 800C9E84
    void resetCursor();               // 800C9EB4
    void setKeyboardMode(int mode);   // 800C9F20
    void hideToolBar();               // 800C9FFC
    void setupLayout();               // 800CA04C
    const wchar_t *getInputText();    // 800CA1BC: the text being composed
    BOOL fixInput();                  // 800CA29C
    BOOL getComposedText(wchar_t *buf, int size); // 800CA318
    BOOL isKanaInput();                           // 800CA40C
    void setABC();                                // 800CA488
    BOOL isSignWindowOpen();                      // 800CA4F0
    void closeSignWindow();                       // 800CA530
    BOOL isPCKeyboardVisible();                   // 800CA564
    int getShiftState();                          // 800CA5C4
    BOOL isUpdated();                             // 800CA64C
    int getKeypadState(); // unused (stripped): the keypad's slot 0x40
    void updateInput();   // 800CA6A0
    m2d::ResAccLoader_c *getResAcc(); // 800CA7F0
    void saveSetting();               // 800CA7F8
    void saveInputMode();             // 800CA8BC

    /* 0x010 */ ResLoader_c mResLoader;
    /* 0x198 */ Event_c mEvent;
    /* 0x1A0 */ Listener_c mListener;
    /* 0x1B0 */ u8 _1B0[0x1B4 - 0x1B0];
    /* 0x1B4 */ myManager_c *mpManager;
    /* 0x1B8 */ u8 _1B8[0x1C0 - 0x1B8];
    /* 0x1C0 */ u16 mFlags;
    /* 0x1C4 */ u32 mCursorPos;
    /* 0x1C8 */ m2d::SimpleEx_c mLyt;
    /* 0x260 */ m2d::Anm_c mAnm;
    /* 0x284 */ m2d::ResAccLoader_c mResAcc;
    /* 0x358 */ mHeapAllocator_c mAllocator;
    /* 0x374 */ int mCreateStep;
}; // size 0x378

// The text cursor.
class Cur_c {
  public:
    Cur_c();          // 800CA958
    virtual ~Cur_c(); // 800CA9C8

    BOOL load();                     // 800CAA4C
    BOOL remove();                   // 800CAA5C
    void build();                    // 800CAA64
    void draw();                     // 800CAAF8
    void setPos(const mVec3_c *pos); // 800CAB44
    const mVec3_c &getPos();         // 800CAB8C

    /* 0x004 */ m2d::SimpleEx_c mLyt;
    /* 0x09C */ m2d::Anm_c mBlinkAnm;
    /* 0x0C0 */ m2d::Anm_c mColorAnm;
    /* 0x0E4 */ m2d::ResAccLoader_c mRes;
}; // size 0x1B8

textinput::input::HKBManager *getHKBManager(); // 800CABCC
void initHKB();      // 800CABD8: from the system setup
void calcHKB();      // 800CABFC: every frame, from the pad update
void resetSetting(); // 800CAC20: the keyboard settings to their defaults
u8 getSavedByte();   // 800CAC80
void setSavedByte(u8 value);  // 800CAC88
BOOL getSavedFlag();          // 800CAC90
void setSavedFlag(BOOL flag); // 800CACA4

void playSeCanNotInput(); // 800CB0BC (d_letter.cpp)
void playSeSubmit();      // 800CB0C4 (d_letter.cpp)

} // namespace dKbd
