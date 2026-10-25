#include <game/game/d_menu_notice.hpp>
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_notice.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_region.hpp>
#include <game/game/d_script.hpp>
#include <game/game/d_string.hpp>
#include <stdio.h>
#include <string.h>

namespace dMenuNotice {
fProf::fBaseProfile_c g_profile_MENU_NOTICE = {classInit, fProf::MENU_NOTICE,
                                               fProf::DRAW_ORDER::MENU_NOTICE};

// .text:0x5C
void *classInit() {
    return new dMenuNotice_c;
}
} // namespace dMenuNotice

static const char *l_allPane = "N_all";

// The menu number of each OPEN_MENU_e.
static const u8 l_openMenu[] = {dMenuMng_c::MENU_BBS_NEWEST,
                                dMenuMng_c::MENU_BBS_LIST,
                                dMenuMng_c::MENU_BBS_WRITE};

dMenuNotice_c *dMenuNotice_c::sInstance;

// .text:0xB8
int dMenuNotice_c::create() {
    if (!mRes.load("Layout/board.arc")) {
        return NOT_READY;
    }
    mNotice.build(&mRes);
    setState(STATE_MAIN);
    setSubState(SUB_OPEN);
    mMenuIdx = OPEN_NEWEST;
    sInstance = this;
    return SUCCEEDED;
}

// .text:0x144
int dMenuNotice_c::doDelete() {
    if (!mRes.remove()) {
        return NOT_READY;
    }
    sInstance = nullptr;
    return SUCCEEDED;
}

// .text:0x188
int dMenuNotice_c::stateMain() {
    executeSubState();
    return SUCCEEDED;
}

// .text:0x1AC
int dMenuNotice_c::stateAnm() {
    return dMenuBase_c::stateAnm();
}

// .text:0x1B0
int dMenuNotice_c::stateClose() {
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x1D4
int dMenuNotice_c::execute() {
    return dMenuBase_c::execute();
}

// .text:0x1D8
int dMenuNotice_c::draw() {
    if (isFlag(FLAG_DRAW) && !isFlag(FLAG_HIDE)) {
        mNotice.draw();
    }
    return SUCCEEDED;
}

// .text:0x230
void dMenuNotice_c::subStateIdle() {
}

// .text:0x234
void dMenuNotice_c::subStateOpen() {
    if (dMenuMng_c::openSub(l_openMenu[mMenuIdx])) {
        setSubState(SUB_IDLE);
    }
}

// .text:0x290
void dMenuNotice_c::subStateWaitClose() {
    if (mNode.getNext() == nullptr) {
        setState(STATE_CLOSE);
    }
}

// .text:0x2A8
void dMenuNotice_c::subStateWaitReopen() {
    if (mNode.getNext() == nullptr) {
        setSubState(SUB_OPEN);
    }
}

// .text:0x2CC
void dMenuNotice_c::executeSubState() {
    static SubStateProc procs[SUB_NUM] = {
        &dMenuNotice_c::subStateIdle,
        &dMenuNotice_c::subStateOpen,
        &dMenuNotice_c::subStateWaitClose,
        &dMenuNotice_c::subStateWaitReopen,
    };
    (this->*procs[mSubState])();
}

// .text:0x304
BOOL dMenuNotice_c::requestClose() {
    if (sInstance->mSubState != SUB_IDLE) {
        return FALSE;
    }
    sInstance->setSubState(SUB_WAIT_CLOSE);
    return TRUE;
}

// .text:0x354
BOOL dMenuNotice_c::requestOpenMenu(int idx) {
    if (sInstance->mSubState != SUB_IDLE) {
        return FALSE;
    }
    sInstance->mMenuIdx = idx;
    sInstance->setSubState(SUB_WAIT_REOPEN);
    return TRUE;
}

// .text:0x3AC
m2d::ResAccLoader_c *dMenuNotice_c::getResAcc() {
    return &sInstance->mRes;
}

// .text:0x3BC
void dMenuNotice_c::copyPos(m2d::SimpleEx_c *lyt, const char *pane) {
    sInstance->mNotice.copyPos(lyt, pane);
    sInstance->onFlag(FLAG_DRAW);
}

// .text:0x404
void dMenuNotice_c::setNotice(const dNoticeData_c *notice) {
    sInstance->mNotice.setNotice(notice);
}

// .text:0x418
void dMenuNotice_c::setPriority(u8 priority) {
    sInstance->mNotice.setPriority(priority);
}

// .text:0x42C
BOOL dMenuNotice_c::setText(const wchar_t *text) {
    return sInstance->mNotice.setText(text, FALSE);
}

// .text:0x444
void dMenuNotice_c::updateText(BOOL fixedSpaces) {
    sInstance->mNotice.updateText(fixedSpaces);
}

// .text:0x458
void dMenuNotice_c::setSelect(int start, int length) {
    sInstance->mNotice.setSelect(start, length);
}

// .text:0x470
void dMenuNotice_c::initNew() {
    sInstance->mNotice.initNew();
}

// .text:0x480
mVec3_c dMenuNotice_c::getPos(int pos) {
    return sInstance->mNotice.getPos(pos);
}

// .text:0x494
int dMenuNotice_c::getLineEnd(mVec3_c *out, int line) {
    return sInstance->mNotice.getLineEnd(out, line);
}

// .text:0x4AC
int dMenuNotice_c::getLine(int pos) {
    return sInstance->mNotice.getLine(pos, nullptr);
}

// .text:0x4C4
int dMenuNotice_c::hitCheck(const mVec2_c *pos) {
    return sInstance->mNotice.hitCheck(pos);
}

// .text:0x4D8
int dMenuNotice_c::getPosAtScreenX(int line, f32 x) {
    return sInstance->mNotice.getPosAtScreenX(line, x);
}

// .text:0x4EC
void dMenuNotice_c::setPos(f32 x, f32 y) {
    sInstance->mNotice.setPos(x, y);
}

// .text:0x4FC
void dMenuNotice_c::setPosX(f32 x) {
    sInstance->mNotice.setPosX(x);
}

// .text:0x50C
f32 dMenuNotice_c::getLineY(int line) {
    return sInstance->mNotice.getLineY(line);
}

// .text:0x520
void dMenuNotice_c::post() {
    sInstance->mNotice.post();
}

// .text:0x530
void dMenuNotice_c::removeNotice(int idx) {
    dNotice::removeNotice(idx);
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        dNotice::sendRemoveNotice(idx);
    }
}

// .text:0x580
int dMenuNotice_c::getLineNum() {
    return sInstance->mNotice.getLineNum();
}

// .text:0x590
BOOL dMenuNotice_c::isUpdated() {
    return sInstance->mNotice.isUpdated();
}

// .text:0x5B0
void dMenuNotice_c::setHidden(BOOL hidden) {
    if (hidden) {
        sInstance->onFlag(FLAG_HIDE);
    } else {
        sInstance->offFlag(FLAG_HIDE);
    }
}

// .text:0x5D8
void dMenuNotice_c::setNextAnm(int anm) {
    sInstance->mNotice.startAnm(anm, FALSE);
}

// .text:0x5F0
BOOL dMenuNotice_c::isAnmEnd() {
    return sInstance->mNotice.isAnmEnd();
}

// .text:0x600
dNotice_c::dNotice_c() {
    mTextWhite.mPacked = nw4r::ut::Color::WHITE;
    mTextBlack.mPacked = nw4r::ut::Color::WHITE;
    mSelectWhite.mPacked = nw4r::ut::Color::WHITE;
    mSelectBlack.mPacked = nw4r::ut::Color::WHITE;
    mpRes = nullptr;
}

// .text:0x6B4
dNotice_c::~dNotice_c() {
}

// .text:0x75C
void dNotice_c::build(m2d::ResAccIf_c *res) {
    mSelectWhite.mBytes[0] = 0;
    mSelectWhite.mBytes[1] = 0xFF;
    mSelectWhite.mBytes[2] = 0;
    mSelectWhite.mBytes[3] = 0xFF;
    mSelectBlack.mBytes[3] = 0;
    mTextWhite.mBytes[0] = 0;
    mTextWhite.mBytes[1] = 0;
    mTextWhite.mBytes[2] = 0;
    mTextWhite.mBytes[3] = 0xFF;
    mTextBlack.mBytes[3] = 0;
    clear();
    mpRes = res;
#if VERSION == VERSION_RUUK01_01
    mLyt.build(res, "board_win_paper_kr_00.brlyt");
#else
    mLyt.build(res, "board_win_paper_00.brlyt");
#endif
    mLyt.calc();
    mLyt.mPriority = LYT_PRIORITY;
    for (int i = 0; i < LINE_NUM; ++i) {
        char name[0x20];
        sprintf(name, "T_msg_%02d", i + 1);
        mWords[i].setTextBox(mLyt.findTextBox(name));
    }
    mSenderWord.setTextBox(mLyt.findTextBox("T_msg_07"));
    mDateWord.setTextBox(mLyt.findTextBox("T_msg_00"));
    initAnm();
}

// .text:0x888
void dNotice_c::draw() {
    if (mAnm.play()) {
        mFlags |= FLAG_UPDATE;
    }
    if (mFlags & FLAG_UPDATE) {
        mFlags &= ~FLAG_UPDATE;
        mLyt.calc();
        mFlags |= FLAG_UPDATED;
    } else {
        mFlags &= ~FLAG_UPDATED;
    }
    mLyt.entry();
}

// .text:0x914
void dNotice_c::copyPos(m2d::SimpleEx_c *lyt, const char *pane) {
    lyt->copyPos(&mLyt, pane, l_allPane);
    mFlags |= FLAG_UPDATE;
}

// .text:0x95C
BOOL dNotice_c::setText(const wchar_t *text, BOOL truncate) {
    int line;
    const wchar_t *cursor = text;
    int lengths[LINE_NUM];
    u32 newLines = 0;
    for (line = 0; line < LINE_NUM; ++line) {
        int length = dScript::getLineLength((const u16 *)cursor, 0);
        if (length > MAX_LINE_LENGTH) {
            length = MAX_LINE_LENGTH;
        }
        if (length > 0) {
            nw4r::lyt::TextBox *box = mWords[line].mpTextBox;
            for (;;) {
                BOOL tooWide = m2d::calcTextWidth(box, cursor, length) >
                               box->GetSize().width;
                if (tooWide == FALSE) {
                    break;
                }
                if (line == LINE_NUM - 1) {
                    return FALSE;
                }
                --length;
            }
        }
        lengths[line] = length;
        cursor += length;
        if (*cursor == L'\n') {
            newLines |= 1 << line;
            ++lengths[line];
            if (line != LINE_NUM - 1) {
                ++cursor;
            }
        }
    }
    BOOL fits = TRUE;
    if (*cursor != 0) {
        if (truncate) {
            fits = FALSE;
        } else {
            return FALSE;
        }
    }
    mLineNum = 1;
    for (int i = 0; i < LINE_NUM; ++i) {
        mLineLengths[i] = lengths[i];
        if (lengths[i] > 0) {
            mLineNum = i + 1;
        }
        if (newLines & (1 << i)) {
            mLineNum = i + 2;
        }
    }
    mNewLineBits = newLines;
    mNotice.setText(text);
    return fits;
}

// .text:0xB4C
void dNotice_c::setNotice(const dNoticeData_c *notice) {
    mNotice = *notice;
    updateAlign();
    updateAll();
}

// .text:0xB8C
void dNotice_c::updateAlign() {
    u8 align;
    if (getRegion() == CONSOLE_REGION_NA || getRegion() == CONSOLE_REGION_EU) {
        align = 0;
    } else {
        align = mNotice.isFlag20() != 0;
    }
    for (int i = 0; i < LINE_NUM; ++i) {
        char name[0x28];
        sprintf(name, "T_msg_%02d", i + 1);
        mLyt.findTextBox(name)->SetTextPositionH(align);
    }
}

// .text:0xC50
void dNotice_c::updateAll() {
    updateDate();
    setText(mNotice.getText(), TRUE);
    updateText(FALSE);
    updateSender();
}

// .text:0xCA8
void dNotice_c::updateDate() {
    mDateWord.clear();
    dString::Word_c word;
    dScript::formatNumber(&word, mNotice.getYear(), 4,
                          dScript::NUM_FORMAT_PLAIN, 0);
    dString::setWord(DATE_YEAR, &word);
    word.clear();
    dScript::formatNumber(&word, mNotice.getMonth() + 1, 2,
                          dScript::NUM_FORMAT_PLAIN, 0);
    dString::setWord(DATE_MONTH, &word);
    word.clear();
    dScript::formatNumber(&word, mNotice.getDay(), 2, dScript::NUM_FORMAT_PLAIN,
                          0);
    dString::setWord(DATE_DAY, &word);
    dString::loadString(&mDateWord, MSG_DATE, "sys_2D/SYS_2D_menu");
    mDateWord.update();
}

// .text:0xDA0
void dNotice_c::updateSender() {
    const wchar_t *sender = mNotice.getSender();
    mSenderWord.clear();
    mSenderWord.set(sender, 0);
    mSenderWord.update();
}

// .text:0xDFC
void dNotice_c::updateText(BOOL fixedSpaces) {
    const wchar_t *text = mNotice.getText();
    static wchar_t lineText[LINE_BUFFER_LENGTH];
    for (int line = 0; line < LINE_NUM; ++line) {
        int length = mLineLengths[line];
        if (length > 0) {
            memcpy(lineText, text, length * sizeof(wchar_t));
        }
        lineText[length] = 0;
        mWords[line].clear();
        mWords[line].set(lineText, 0);
        text += length;
        if (length > 0 && mSelectLength > 0) {
            int colorStart;
            int start = mSelectStart;
            int end = start + mSelectLength;
            int before = 0;
            for (int i = 0; i < line; ++i) {
                before += mLineLengths[i];
            }
            if (start >= before && start < before + mLineLengths[line]) {
                colorStart = start - before;
            } else {
                colorStart = -1;
            }
            if (end > before && end <= before + mLineLengths[line]) {
                end -= before;
                if (colorStart == -1) {
                    colorStart = 0;
                }
            }
            if (colorStart > -1) {
                mWords[line].insertColor(colorStart + 1, mSelectWhite.mBytes,
                                         mSelectBlack.mBytes, TRUE);
            }
            if (end > -1) {
                mWords[line].insertColor(end, mTextWhite.mBytes,
                                         mTextBlack.mBytes, FALSE);
            }
        }
        if (fixedSpaces) {
            mWords[line].toFixedSpaces();
        }
        mWords[line].update();
        if (fixedSpaces) {
            mWords[line].fromFixedSpaces();
        }
    }
}

// .text:0x1048
void dNotice_c::clear() {
    mNotice.clear();
    mSelectLength = 0;
    mSelectStart = 0;
}

// .text:0x1084
void dNotice_c::setPriority(u8 priority) {
    mLyt.mPriority = priority;
}

// .text:0x108C
void dNotice_c::setSelect(int start, int length) {
    mSelectLength = length;
    mSelectStart = start;
}

// .text:0x1098
void dNotice_c::initNew() {
    mNotice.init(dTime_c::getCurrent());
    mNotice.setSender(dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID.player.mName);
    updateAlign();
    updateAll();
}

// .text:0x10F4
int dNotice_c::getLine(int pos, int *linePos) {
    int line = 0;
    while (pos >= mLineLengths[line] && line < LINE_NUM) {
        if (pos == mLineLengths[line]) {
            if (!(mNewLineBits & (1 << line))) {
                break;
            }
        }
        pos -= mLineLengths[line];
        ++line;
    }
    if (line >= LINE_NUM) {
        return NO_LINE;
    }
    if (linePos != nullptr) {
        *linePos = pos;
    }
    return line;
}

// .text:0x1160
mVec3_c dNotice_c::getPos(int pos) {
    int linePos;
    int line = getLine(pos, &linePos);
    mVec3_c screen;
    m2d::getScreenPos(&screen, mWords[line].mpTextBox);
    mVec3_c result;
    result = screen;
    result.x = mWords[line].getScreenX(linePos);
    return result;
}

// .text:0x11EC
int dNotice_c::getLineEnd(mVec3_c *out, int line) {
    if (mLineLengths[line] == 0 && line > 0 &&
        !(mNewLineBits & (1 << (line - 1)))) {
        return LINE_END_NONE;
    }
    mVec3_c screen;
    m2d::getScreenPos(&screen, mWords[line].mpTextBox);
    *out = screen;
    out->x = mWords[line].getScreenX(mLineLengths[line]);
    if (mNewLineBits & (1 << line)) {
        return LINE_END_NEWLINE;
    }
    if (line == LINE_NUM - 1) {
        return LINE_END_TEXT;
    }
    if (line < LINE_NUM - 1 && mLineLengths[line + 1] == 0) {
        return LINE_END_TEXT;
    }
    return LINE_END_NONE;
}

// .text:0x12F4
int dNotice_c::hitCheck(const mVec2_c *pos) {
    for (int i = 0; i < LINE_NUM; ++i) {
        if (m2d::isInPaneRect(mWords[i].mpTextBox, 5.0f, pos)) {
            return i;
        }
    }
    return NO_LINE;
}

// .text:0x1374
int dNotice_c::getPosAtScreenX(int line, f32 x) {
    int pos = mWords[line].getPosAtScreenX(x);
    for (int i = line - 1; i >= 0; --i) {
        pos += mLineLengths[i];
    }
    return pos;
}

// .text:0x14B0
void dNotice_c::setPos(f32 x, f32 y) {
    nw4r::lyt::Pane *pane = mLyt.findPane("N_all");
    mVec3_c pos = pane->GetTranslate();
    pos.x = x;
    pos.y = y;
    pane->SetTranslate(*static_cast<nw4r::math::VEC3 *>(pos));
    mFlags |= FLAG_UPDATE;
}

// .text:0x1540
void dNotice_c::setPosX(f32 x) {
    nw4r::lyt::Pane *pane = mLyt.findPane("N_all");
    mVec3_c pos = pane->GetTranslate();
    pos.x = x;
    pane->SetTranslate(*static_cast<nw4r::math::VEC3 *>(pos));
    mFlags |= FLAG_UPDATE;
}

// .text:0x15BC
f32 dNotice_c::getLineY(int line) {
    nw4r::lyt::TextBox *box = mWords[line].mpTextBox;
    if (box == nullptr) {
        return 0.0f;
    }
    mVec3_c pos;
    m2d::getScreenPos(&pos, box);
    return pos.y;
}

// .text:0x1604
void dNotice_c::post() {
    dNotice::addNotice(&mNotice);
    dNotice::setNoticeRead(dNotice::getNoticeCount() - 1);
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        dNotice::sendAddNotice(&mNotice);
    }
}

// .text:0x1660
int dNotice_c::getLineNum() {
    return mLineNum;
}

// .text:0x1668
void dNotice_c::initAnm() {
    static const char *names[ANM_NUM] = {
#if VERSION == VERSION_RUUK01_01
        "board_win_paper_kr_00_ppr_R_Out.brlan",
        "board_win_paper_kr_00_ppr_R_In.brlan",
        "board_win_paper_kr_00_ppr_L_Out.brlan",
        "board_win_paper_kr_00_ppr_L_In.brlan",
#else
        "board_win_paper_00_ppr_R_Out.brlan",
        "board_win_paper_00_ppr_R_In.brlan",
        "board_win_paper_00_ppr_L_Out.brlan",
        "board_win_paper_00_ppr_L_In.brlan",
#endif
    };
    mAnm.init(&mLyt, mpRes, names, ANM_NUM);
    resetAnm();
}

// .text:0x16B4
void dNotice_c::resetAnm() {
    mAnm.start(ANM_R_IN);
    mAnm.setToEnd();
    mFlags |= FLAG_UPDATE;
}

// .text:0x16FC
void dNotice_c::startAnm(int anm, BOOL now) {
    if (now) {
        mAnm.start(anm);
    } else {
        mAnm.setNext(anm);
    }
}

// .text:0x1714
BOOL dNotice_c::isAnmEnd() {
    BOOL end = mAnm.isEnd();
    if (end) {
        resetAnm();
    }
    return end;
}

// .text:0x1764
dNotice_c::Word_c::Word_c() {
    clear();
}

// .text:0x17A8
dNotice_c::Word_c::~Word_c() {
}

// .text:0x1800
u32 dNotice_c::Word_c::getBufferSize() {
    return sizeof(mBuffer);
}

// .text:0x1808
wchar_t *dNotice_c::Word_c::getBuffer() {
    return mBuffer;
}
