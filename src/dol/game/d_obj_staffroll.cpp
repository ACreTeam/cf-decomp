#include <game/game/d_obj_staffroll.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_script.hpp>

using namespace dObjStaffroll;

namespace dObjStaffroll {
// 801B4CC0
CUSTOM_BASE_PROFILE(OBJ_STAFFROLL, dObjStaffroll_c, 0xF9, 0x128);

// Request helpers use the mask view; the process updates individual state bits.
union Flags_c {
    u32 mValue;
    struct {
        u32 unused : 26;
        u32 done : 1;
        u32 show : 1;
        u32 ending : 1;
        u32 scroll : 1;
        u32 stop : 1;
        u32 start : 1;
    } mBits;
};
static Flags_c l_flags;

// 801B4D64
Entry_c::Entry_c() {}
// 801B4DB0
Entry_c::~Entry_c() {}
}

// 801B4E2C
int dObjStaffroll_c::create() {
    if (mRes.load("Layout/staff.arc") == false) {
        return NOT_READY;
    }
    l_flags.mValue = 0;
    buildEntries();
    buildEnding();
    return SUCCEEDED;
}

// 801B4E90
int dObjStaffroll_c::doDelete() {
    return mRes.remove() != FALSE;
}

// 801B4EC0
int dObjStaffroll_c::execute() {
    if (l_flags.mValue & REQUEST_START) {
        l_flags.mBits.start = 0;
        buildLines();
        clearEntries();
        l_flags.mBits.scroll = 1;
    }
    if (l_flags.mValue & REQUEST_STOP) {
        l_flags.mBits.stop = 0;
        clearEntries();
        l_flags.mBits.scroll = 0;
    }
    f32 speed = mScrollSpeed;
    if (l_flags.mValue & SCROLLING) {
        Line_c *line = &mLines[mNextLine];
        if (speed + (mScrollY + line->mY) >= 0.0f) {
            if (addEntry(line)) {
                mNextLine++;
            }
        }
        mScrollY += speed;
        if (mNextLine >= mLineCount) {
            l_flags.mBits.scroll = 0;
        }
    }
    calcEntries();
    calcEnding();
    return SUCCEEDED;
}

// 801B4FE4
int dObjStaffroll_c::draw() {
    drawEntries();
    if (l_flags.mValue & SHOW_ENDING) {
        mEndingLyt.entry();
    }
    return SUCCEEDED;
}

// 801B5028
void dObjStaffroll_c::buildEnding() {
    static const char *l_firstAnms[] = {
        "All_Rights_00_msg00_In.brlan", "All_Rights_00_msg00_Out.brlan"
    };
    static const char *l_secondAnms[] = {
        "All_Rights_00_msg01_In.brlan", "All_Rights_00_msg01_Out.brlan"
    };
    mEndingLyt.build(&mRes, "All_Rights_00.brlyt");
    mFirstAnm.init(&mEndingLyt, &mRes, l_firstAnms, ANIMATION_COUNT);
    mFirstAnm.startEnd(ANM_OUT);
    mSecondAnm.init(&mEndingLyt, &mRes, l_secondAnms, ANIMATION_COUNT);
    mSecondAnm.startEnd(ANM_OUT);
    mEndingLyt.setMessage("T_msg_01", ENDING_MESSAGE, NULL);
    mEndingLyt.calc();
}

// 801B50D8
void dObjStaffroll_c::calcEnding() {
    if (l_flags.mValue & SHOW_ENDING) {
        switch (mEndingState) {
        case WAIT_FIRST_IN:
            if (mFirstAnm.isEnd()) { mEndingTimer = FIRST_HOLD_FRAMES; mEndingState++; }
            break;
        case HOLD_FIRST:
            if (mEndingTimer > 0) { mEndingTimer--; }
            else { mFirstAnm.setNext(ANM_OUT); mEndingState++; }
            break;
        case WAIT_FIRST_OUT:
            if (mFirstAnm.isEnd()) { mEndingTimer = BETWEEN_MESSAGES_FRAMES; mEndingState++; }
            break;
        case GAP_BEFORE_SECOND:
            if (mEndingTimer > 0) { mEndingTimer--; }
            else { mSecondAnm.setNext(ANM_IN); mEndingState++; }
            break;
        case WAIT_SECOND_IN:
            if (mSecondAnm.isEnd()) { mEndingTimer = SECOND_HOLD_FRAMES; mEndingState++; }
            break;
        case HOLD_SECOND:
            if (mEndingTimer > 0) { mEndingTimer--; }
            else { mSecondAnm.setNext(ANM_OUT); mEndingState++; }
            break;
        case WAIT_SECOND_OUT:
            if (mSecondAnm.isEnd()) { mEndingTimer = FINAL_WAIT_FRAMES; mEndingState++; }
            break;
        case GAP_AFTER_SECOND:
            if (mEndingTimer > 0) { mEndingTimer--; }
            else { mEndingState++; }
            break;
        default:
            l_flags.mBits.show = 0;
            l_flags.mBits.done = 1;
            break;
        }
    } else if (l_flags.mValue & REQUEST_ENDING) {
        l_flags.mBits.ending = 0;
        l_flags.mBits.done = 0;
        l_flags.mBits.show = 1;
        mFirstAnm.setNext(ANM_IN);
        mEndingState = WAIT_FIRST_IN;
    }
    bool first = mFirstAnm.play();
    bool second = mSecondAnm.play();
    if (first | second) { mEndingLyt.calc(); }
}

namespace dObjStaffroll {
// 801B5308
int getLineLength(const wchar_t *text) {
    int length = 0;
    while (*text) {
        if (*text != L'\n') {
            text++;
            length++;
        } else {
            return length;
        }
    }
    return length;
}
// 801B5340
const wchar_t *getNextLine(const wchar_t *text) {
    text += getLineLength(text);
    if (*text == L'\n') { text++; }
    return text;
}
// 801B5384
int getLineCount(const wchar_t *text) {
    int count = 0;
    while (*text) { text = getNextLine(text); count++; }
    return count;
}
// 801B53C8
const wchar_t *getLine(const wchar_t *text, int line) {
    for (int i = 0; i < line; i++) { text = getNextLine(text); }
    return text;
}
}

// 801B5410
void dObjStaffroll_c::buildLines() {
    for (int i = 0; i < MAX_LINES; i++) { mLines[i].clear(); }
    dScript::Res_c res(dScript::getBmgFile(dScript::getBank(), "sys_2D/SYS_Stafflist"));
    mLineCount = 0;
    mTotalY = 0.0f;
    for (u32 message = 1; message < res.getCount(); message++) {
        // Odd messages are titles; even messages hold the following staff names.
        f32 height = message & 1 ? (f32)TITLE_LINE_HEIGHT : (f32)NAME_LINE_HEIGHT;
        int count = getLineCount(res.getMessage(message));
        for (int line = 0; line < count; line++) {
            mLines[mLineCount].mY = mTotalY;
            mLines[mLineCount].mMessage = message;
            mLines[mLineCount].mLine = line;
            mTotalY -= (f32)LINE_SPACING + height;
            mLineCount++;
        }
        if (count != 0 && !(message & 1)) { mTotalY -= (f32)GROUP_SPACING; }
    }
    mNextLine = 0;
    mScrollY = 0.0f;
    mScrollSpeed = -((f32)SCROLL_HEIGHT + mTotalY) / (f32)SCROLL_FRAMES;
}

// 801B5874
void dObjStaffroll_c::buildEntries() {
    for (int i = 0; i < MAX_ENTRIES; i++) { mEntries[i].build(&mRes); }
}
// 801B58D0
void dObjStaffroll_c::calcEntries() {
    for (int i = 0; i < MAX_ENTRIES; i++) { mEntries[i].calc(mScrollSpeed); }
}
// 801B592C
void dObjStaffroll_c::drawEntries() {
    for (int i = 0; i < MAX_ENTRIES; i++) { mEntries[i].draw(); }
}
// 801B5978
void dObjStaffroll_c::clearEntries() {
    for (int i = 0; i < MAX_ENTRIES; i++) { mEntries[i].clear(); }
}
// 801B59D0
BOOL dObjStaffroll_c::addEntry(const Line_c *line) {
    int entry = findFreeEntry();
    if (entry == -1) { return FALSE; }
    dScript::Res_c res(dScript::getBmgFile(dScript::getBank(), "sys_2D/SYS_Stafflist"));
    const wchar_t *text = getLine(res.getMessage(line->mMessage), line->mLine);
    mEntries[entry].set(text, line->mMessage & 1, line->mY + mScrollY);
    return TRUE;
}
// 801B5A8C
int dObjStaffroll_c::findFreeEntry() {
    for (int i = 0; i < MAX_ENTRIES; i++) {
        if (!mEntries[i].isActive()) { return i; }
    }
    return -1;
}

namespace dObjStaffroll {
// 801B5B98
void start() { l_flags.mValue |= REQUEST_START; }
// 801B5BA8
void stop() { l_flags.mValue |= REQUEST_STOP; }
// 801B5BB8
void startEnding() { l_flags.mValue |= REQUEST_ENDING; }
// 801B5BC8
BOOL isEndingDone() { return l_flags.mBits.done; }
// 801B5BD4
void Line_c::clear() { mMessage = 0; mY = 0.0f; mLine = INVALID_LINE; }

// 801B5BF0
void Entry_c::build(m2d::ResAccIf_c *res) {
    mFlags = 0;
    mLyt.build(res, "staff_00.brlyt");
    mName.setTextBox(mLyt.findTextBox("T_name_00"));
    mTitle.setTextBox(mLyt.findTextBox("T_title_00"));
}
// 801B5C68
void Entry_c::calc(f32 speed) {
    if (mFlags & ACTIVE) {
        mY += speed;
        if (mY > (f32)ENTRY_END_Y) { mFlags &= ~ACTIVE; }
        else {
            mVec2_c pos(0.0f, mY);
            mLyt.mPos.y = pos.y;
            mLyt.mPos.x = pos.x;
            mLyt.calc();
        }
    }
}
// 801B5CD8
void Entry_c::draw() { if (mFlags & ACTIVE) { mLyt.entry(); } }
// 801B5CEC
void Entry_c::set(const wchar_t *text, int title, f32 y) {
    bool name = !title;
    mLyt.setVisible("T_name_00", name);
    mLyt.setVisible("T_title_00", !name);
    if (title) {
        mTitle.clear(); mTitle.set(text, STOP_AT_NEWLINE); mTitle.update();
    } else {
        mName.clear(); mName.set(text, STOP_AT_NEWLINE); mName.update();
    }
    mY = (f32)ENTRY_START_Y + y;
    mFlags |= ACTIVE;
}
}

// 801B5DDC
dObjStaffroll_c::~dObjStaffroll_c() {}
