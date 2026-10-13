#ifndef GAME_D_OBJ_STAFFROLL_HPP
#define GAME_D_OBJ_STAFFROLL_HPP

#include <game/framework/f_base.hpp>
#include <game/game/d_m2d.hpp>

// Only the process name is recovered from RTTI; other names are inferred.
namespace dObjStaffroll {
enum { MAX_LINES = 250, MAX_ENTRIES = 20 };
enum {
    NAME_LINE_HEIGHT = 40, TITLE_LINE_HEIGHT = 28, LINE_SPACING = 5,
    GROUP_SPACING = 25, SCROLL_HEIGHT = 456, SCROLL_FRAMES = 5850,
    ENTRY_START_Y = -228, ENTRY_END_Y = 278,
    FIRST_HOLD_FRAMES = 110, BETWEEN_MESSAGES_FRAMES = 10,
    SECOND_HOLD_FRAMES = 150, FINAL_WAIT_FRAMES = 75,
    ENDING_MESSAGE = 0x150, ANIMATION_COUNT = 2, INVALID_LINE = -1
};
enum Flag_e {
    REQUEST_START = 1, REQUEST_STOP = 2, SCROLLING = 4,
    REQUEST_ENDING = 8, SHOW_ENDING = 16, ENDING_DONE = 32
};
enum Animation_e { ANM_IN, ANM_OUT };
enum EndingState_e {
    WAIT_FIRST_IN, HOLD_FIRST, WAIT_FIRST_OUT, GAP_BEFORE_SECOND,
    WAIT_SECOND_IN, HOLD_SECOND, WAIT_SECOND_OUT, GAP_AFTER_SECOND, FINISHED
};

struct Line_c {
    void clear(); // 801B5BD4
    /* 0x0 */ f32 mY;
    /* 0x4 */ int mMessage;
    /* 0x8 */ int mLine;
};

class Entry_c {
public:
    enum Flag_e { ACTIVE = 1 };
    enum { STOP_AT_NEWLINE = 1 };
    Entry_c(); // 801B4D64
    ~Entry_c(); // 801B4DB0
    void build(m2d::ResAccIf_c *res); // 801B5BF0
    void calc(f32 speed); // 801B5C68
    void draw(); // 801B5CD8
    void set(const wchar_t *text, int title, f32 y); // 801B5CEC
    void clear() { mFlags = 0; }
    bool isActive() { return (mFlags & ACTIVE) != 0; }

    /* 0x000 */ m2d::SimpleEx_c mLyt;
    /* 0x098 */ m2d::stringWord_c mName;
    /* 0x18C */ m2d::stringWord_c mTitle;
    /* 0x280 */ f32 mY;
    /* 0x284 */ u16 mFlags;
}; // size 0x288

int getLineLength(const wchar_t *text); // 801B5308
const wchar_t *getNextLine(const wchar_t *text); // 801B5340
int getLineCount(const wchar_t *text); // 801B5384
const wchar_t *getLine(const wchar_t *text, int line); // 801B53C8
void start(); // 801B5B98
void stop(); // 801B5BA8
void startEnding(); // 801B5BB8
BOOL isEndingDone(); // 801B5BC8
}

// Global scope is established by the RTTI string "dObjStaffroll_c".
class dObjStaffroll_c : public fBase_c {
public:
    virtual int create(); // 801B4E2C
    virtual int doDelete(); // 801B4E90
    virtual int execute(); // 801B4EC0
    virtual int draw(); // 801B4FE4
    virtual ~dObjStaffroll_c(); // 801B5DDC

    void buildEnding(); // 801B5028
    void calcEnding(); // 801B50D8
    void buildLines(); // 801B5410
    void buildEntries(); // 801B5874
    void calcEntries(); // 801B58D0
    void drawEntries(); // 801B592C
    void clearEntries(); // 801B5978
    BOOL addEntry(const dObjStaffroll::Line_c *line); // 801B59D0
    int findFreeEntry(); // 801B5A8C

    /* 0x0064 */ m2d::ResAccLoader_c mRes;
    /* 0x0138 */ dObjStaffroll::Line_c mLines[dObjStaffroll::MAX_LINES];
    /* 0x0CF0 */ dObjStaffroll::Entry_c mEntries[dObjStaffroll::MAX_ENTRIES];
    /* 0x3F90 */ int mLineCount;
    /* 0x3F94 */ f32 mTotalY;
    /* 0x3F98 */ int mNextLine;
    /* 0x3F9C */ f32 mScrollY;
    /* 0x3FA0 */ m2d::SimpleEx_c mEndingLyt;
    /* 0x4038 */ m2d::AnmEx_c mFirstAnm;
    /* 0x4074 */ m2d::AnmEx_c mSecondAnm;
    /* 0x40B0 */ int mEndingTimer;
    /* 0x40B4 */ int mEndingState;
    /* 0x40B8 */ f32 mScrollSpeed;
}; // size 0x40BC

#endif
