// See include/game/game/d_select.hpp.
#include <game/game/d_select.hpp>
#include <game/game/d_font.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_animal_id.hpp>
#include <game/game/d_prc_mng.hpp>
#include <game/game/d_region.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/mLib/m_pad.hpp>
#include <lib/revolution/SC/scapi.h>
#include <string.h>

namespace dSelect {

static const char *sTextNames[BComp_c::NUM_ANSWERS] = {"T_msg_00", "T_msg_01", "T_msg_02", "T_msg_03", "T_msg_04"};
static const char *sLineNames[BComp_c::NUM_ANSWERS] = {"line_00", "line_01", "line_02", "line_03", "line_04"};
static const char *sButtonNames[BComp_c::NUM_ANSWERS] = {"B_msg_00", "B_msg_01", "B_msg_02", "B_msg_03", "B_msg_04"};

// 8015B680
BOOL isDecideTrig() {
    return dPad::isConfirmTriggered();
}

// 8015B684
BOOL isCancelTrig() {
    return dPad::isCancelTriggered();
}

// 8015B688
BOOL isUpHold() {
    BOOL ret = TRUE;
    if (((dPad::getHeld() >> 3) & 1) == 0) {
        if (mPad::getCore()->down(8) == 0) {
            ret = FALSE;
        }
    }
    return ret;
}

// 8015B6E4
BOOL isDownHold() {
    BOOL ret = TRUE;
    if (((dPad::getHeld() >> 2) & 1) == 0) {
        if (mPad::getCore()->down(4) == 0) {
            ret = FALSE;
        }
    }
    return ret;
}

// 8015B740
void Separate_c::clear() {
    mMsgCode = 0;
    memset(mLabel, 0, sizeof(mLabel));
    mWord.clear();
    mIsCancel = 0;
    mAddPeriod = false;
}

// 8015B798
void Separate_c::setLabel(const char *label) {
    if (strlen(label)) {
        strncpy(mLabel, label, sizeof(mLabel) - 1);
    }
}

// 8015B7EC
void BaseComp_c::open() {}

// 8015B7F0
bool BaseComp_c::isClosed() {
    return false;
}

// 8015B7F8
void BaseComp_c::reset() {
    mCursor = 0xFF;
    mCode = 0;
    memset(mLabel, 0, sizeof(mLabel));
}

// 8015B818
void BaseComp_c::onOpen() {
    playSe(0x16C8);
}

// 8015B820
void BaseComp_c::onClose() {}

// 8015B824
BComp_c::BComp_c() {
    mRepeatTimer = 0;
}

// 8015B8E4
BComp_c::~BComp_c() {}

// 8015B998
BOOL BComp_c::load() {
    return mResources.load("Layout/chc_win.arc");
}

static const dState::mode_c<BComp_c>::Entry sModes[] = {
    {"\x94\xF1\x95\x5C\x8E\xA6", &BComp_c::initHidden, &BComp_c::executeHidden},
    {"\x8B\xD6\x8E\x7E\x95\xB6\x8E\x9A\x97\xF1\x92\xB2\x8D\xB8", &BComp_c::initCheck, &BComp_c::executeCheck},
    {"\x8B\xD6\x8E\x7E\x95\xB6\x8E\x9A\x97\xF1\x92\x75\x8A\xB7", &BComp_c::initReplace, &BComp_c::executeReplace},
    {"\x8F\x6F\x8C\xBB", &BComp_c::initIn, &BComp_c::executeIn},
    {"\x91\x49\x91\xF0\x92\x86\x81\x69\x83\x81\x83\x62\x83\x5A\x81\x5B\x83\x57\x97\x70\x81\x6A",
     &BComp_c::initSelectMsg, &BComp_c::executeSelectMsg},
    {"\x91\x49\x91\xF0\x92\x86\x82\x63\x82\x6F\x82\x63\x8E\x64\x97\x6C\x81\x69\x83\x81\x83\x6A\x83\x85\x81\x5B\x97"
     "\x70\x81\x6A",
     &BComp_c::initSelectDpd, &BComp_c::executeSelectDpd},
    {"\x91\x49\x91\xF0\x92\x86\x82\x6A\x82\x64\x82\x78\x8E\x64\x97\x6C\x81\x69\x83\x81\x83\x6A\x83\x85\x81\x5B\x97"
     "\x70\x81\x6A",
     &BComp_c::initSelectKey, &BComp_c::executeSelectKey},
    {"\x91\x49\x91\xF0\x8C\xF8\x89\xCA", &BComp_c::initDecide, &BComp_c::executeDecide},
    {"\x83\x8D\x83\x62\x83\x4E", &BComp_c::initLock, &BComp_c::executeLock},
    {"\x91\xDE\x8F\xEA", &BComp_c::initOut, &BComp_c::executeOut},
};

// 8015B9A8
void BComp_c::build() {
    mLayout.mpResAcc = &mResources;
    mLayout.build("chc_win_00.brlyt", NULL);
    mLayout.mPriority = 0x85;
    mpAll = mLayout.findPane("N_all");
    mpWindow = mLayout.findPane("N_chcWin_all");
    mpShadow = mLayout.findPane("N_chcWinSdw");
    mLayout.createAnm(&mWindowAnm, "chc_win_00_winYlw.brlan", NULL);
    mLayout.bind(&mWindowAnm);
    mWindowAnm.setFrame(0.0f);
    mLayout.calc();
    mLayout.setFont(&dFont::g_mainFont.mFont);
    dTagProcessor_c *proc = &g_tagProcessor;
    for (int i = 0; i < NUM_ANSWERS; i++) {
        nw4r::lyt::TextBox *text = mLayout.findTextBox(sTextNames[i]);
        text->SetTagProcessor(proc);
        text->AllocStringBuffer(101);
        mSeparates[i].mpTextBox = text;
    }
    mTextBoxWidth = mSeparates[0].mpTextBox->GetSize().width;
    mLineWidth = mLayout.findPane(sLineNames[0])->GetSize().width;
    mButtonWidth = mLayout.findPane(sButtonNames[0])->GetSize().width;
    mpCursor = mLayout.findPane("csr_00");
    initialize("dSelect::BComp_c", sModes, ARRAY_SIZE(sModes), false);
}

// 8015BB88
void BComp_c::setPinkWindow() {
    mLayout.changeAnm(&mWindowAnm, "chc_win_00_winPnk.brlan", NULL);
    mWindowAnm.setFrame(0.0f);
    mLayout.calc();
}

// 8015BBE8
BOOL BComp_c::create() {
    if (!load()) {
        return FALSE;
    }
    if (!mCreated) {
        build();
        mCreated = true;
    }
    return TRUE;
}

// 8015BC44
BOOL BComp_c::remove() {
    if (!mResources.remove()) {
        return FALSE;
    }
    if (mCreated == true) {
        for (int i = 0; i < NUM_ANSWERS; i++) {
            if (mSeparates[i].mpTextBox) {
                mSeparates[i].mpTextBox->FreeStringBuffer();
            }
        }
        mCreated = false;
    }
    return TRUE;
}

// 8015BCE0
BOOL BComp_c::update() {
    preExecute();
    execute();
    return TRUE;
}

// 8015BD44
BOOL BComp_c::draw() {
    mWindowAnm.play();
    if (!isMode(&BComp_c::executeHidden) && !isMode(&BComp_c::executeCheck) && !isMode(&BComp_c::executeReplace)) {
        mLayout.calc();
        mLayout.entry();
    }
    return TRUE;
}

// 8015BE70
void BComp_c::setWord(int idx, dScript::Word_c *word, u16 code, const char *label, int isCancel, int addPeriod) {
    Separate_c *separate = getSeparate(idx);
    separate->mWord.copy(word, 0);
    separate->mMsgCode = code;
    separate->mIsCancel = isCancel;
    if (addPeriod) {
        separate->mAddPeriod = true;
    }
    if (label) {
        separate->setLabel(label);
    }
}

// 8015BEF4
void BComp_c::setMessage(int idx, u16 index, const char *group, u16 code, const char *label, int isCancel,
                         int addPeriod) {
    dString::Word_c word(index, group);
    setWord(idx, &word, code, label, isCancel, addPeriod);
}

// 8015BF74
void BComp_c::setSysMessage(int idx, u16 index, u16 code, const char *label, int isCancel, int addPeriod) {
    setMessage(idx, index, "sys_SELECT/SYS_Select", code, label, isCancel, addPeriod);
}

// 8015BF9C
void BComp_c::setNpcMessage(int idx, u16 index, u16 code, const char *label, int isCancel, int addPeriod) {
    setMessage(idx, index, "sys_SELECT/SYS_SelectNPC", code, label, isCancel, addPeriod);
}

// 8015BFC4
void BComp_c::setPlayerName(int idx, const dPersonalID_c *id, u16 code, const char *label, int isCancel,
                            int addPeriod) {
    dHmnName::Word_c word;
    id->setWord(&word);
    switch (getRegion()) {
    case 1:
    case 2:
        word.capitalize();
        break;
    }
    setWord(idx, &word, code, label, isCancel, addPeriod);
}

// 8015C06C
void BComp_c::setAnimalName(int idx, dAnmPersonalID_c *id, u16 code, const char *label, int isCancel,
                            int addPeriod) {
    dHmnName::Word_c word;
    id->setWord(&word, 10);
    setWord(idx, &word, code, label, isCancel, addPeriod);
}

void BComp_c::setItemName(int idx, dItem::Item item, u16 code, const char *label, int isCancel, int addPeriod) {
    dItem::name_c word(item);
    setWord(idx, &word, code, label, isCancel, addPeriod);
}

// 8015C0F8
f32 BComp_c::getTextWidth(nw4r::lyt::TextBox *textBox) {
    return m2d::calcTextWidth(textBox, NULL, 0);
}

// 8015C108
void BComp_c::initHidden() {}

// 8015C10C
void BComp_c::executeHidden() {}

// 8015C110
void BComp_c::initCheck() {
    Word_c *word;
    Separate_c *separate = mSeparates;
    for (int i = 0; i < NUM_ANSWERS; i++, separate++) {
        word = &separate->mWord;
        u32 len = word->getLength(0);
        if (word->_22 != 0 && len != 0) {
            mCheckWords[mCheckNum] = word;
            mCheckTexts[mCheckNum] = (const u16 *)word->getBuffer();
            mCheckNum++;
        }
    }
}

// 8015C1CC
void BComp_c::executeCheck() {
    if (mCheckNum == 0) {
        changeMode(&BComp_c::executeReplace);
    } else if (dPrcMng_c::startCheck(mCheckTexts, mCheckNum, mCheckResults, &mCheckHits)) {
        changeMode(&BComp_c::executeReplace);
    }
}

// 8015C474
void BComp_c::initReplace() {}

// 8015C478
void BComp_c::executeReplace() {
    if (mCheckNum != 0) {
        if (!dPrcMng_c::isDone()) {
            return;
        }
        for (int i = 0; i < mCheckNum; i++) {
            if ((int)mCheckResults[i] == 1 && mCheckWords[i]->_23 != 0) {
                mCheckWords[i]->censor();
            }
        }
    }
    for (int i = 0; i < mNum; i++) {
        if (mSeparates[i].mAddPeriod) {
            Word_c *word = mSeparates[i].getWord();
            switch (getLanguage()) {
            case 1:
            case 3:
            case 4:
            case 6:
            case 7:
            case 8:
                word->appendChar(L'.');
                break;
            }
        }
    }
    changeMode(&BComp_c::executeIn);
}

// 8015C69C
void BComp_c::initIn() {
    mLayout.changeAnm(&mWindowAnm, "chc_win_00_chcWinIn.brlan", NULL);
    mWindowAnm.mFrameCtrl.mFlags = 1;
    mWindowAnm.setFrame(0.0f);
    onOpen();
    mLayout.calc();
}

// 8015C718
void BComp_c::executeIn() {
    mLayout.calc();
    if (mWindowAnm.mFrameCtrl.isStop()) {
        mCurrent = 0;
        changeMode(mNextMode);
    }
}

// 8015C89C
void BComp_c::initSelectMsg() {
    if (fn_8017FE28()) {
        setKeyMode(false);
    } else {
        setKeyMode(true);
    }
    updateCursor();
    mLayout.calc();
}

// 8015C908
void BComp_c::executeSelectMsg() {
    int prev = mCurrent;
    if (fn_8017FE28()) {
        if (mKeyMode == true) {
            setKeyMode(false);
            prev = -2;
        } else {
            updatePointer();
        }
    } else {
        if (mKeyMode == false) {
            setKeyMode(true);
            prev = -2;
        } else {
            updateKey();
        }
    }
    if (mCurrent != prev) {
        if (mCurrent != -1) {
            playSe(0x16BE);
        }
        updateCursor();
        mLayout.calc();
    }
}

// 8015C9D4
void BComp_c::initSelectDpd() {
    setKeyMode(false);
    updateCursor();
    mLayout.calc();
}

// 8015CA20
void BComp_c::executeSelectDpd() {
    int prev = mCurrent;
    if (!fn_8017FE28()) {
        mCurrent = -1;
    } else {
        updatePointer();
    }
    if (mPad::getCore()->downTrigger(0x800) && mCurrent == -1 && mCancel != -1) {
        cancel();
    } else if (!fn_8017FE28() && mCancel != -1 && dPad::isCancelTriggered()) {
        cancel();
    }
    if (mCurrent != prev) {
        if (mCurrent != -1) {
            playSe(0x16BE);
        }
        updateCursor();
        mLayout.calc();
    }
}

// 8015CB34
void BComp_c::initSelectKey() {
    setKeyMode(true);
    initSelectMsg();
}

// 8015CB6C
void BComp_c::executeSelectKey() {
    int prev = mCurrent;
    if (mCancel != -1 && fn_8017FE28()) {
        cancel();
    } else {
        updateKey();
    }
    if (mCurrent != prev) {
        updateCursor();
        mLayout.calc();
    }
}

// 8015CC00
void BComp_c::initDecide() {
    if (!mKeyMode) {
        changeTextAnm(mCursor, 3);
    } else {
        changeTextAnm(mCursor, 4);
    }
    Separate_c *separate = getSeparate(mCursor);
    mCode = separate->mMsgCode;
    setResultLabel(separate->mLabel);
}

// 8015CC74
void BComp_c::executeDecide() {
    if (mTextAnms[mCursor].mFrameCtrl.isStop()) {
        if (mLockMask & (1 << mCursor)) {
            changeMode(&BComp_c::executeLock);
        } else {
            changeMode(&BComp_c::executeOut);
        }
    } else {
        mTextAnms[mCursor].play();
        mLayout.calc();
    }
}

// 8015CF58
void BComp_c::initLock() {
    mpCursor->SetVisible(false);
}

// 8015CF6C
void BComp_c::executeLock() {}

// 8015CF70
void BComp_c::initOut() {
    mLayout.changeAnm(&mWindowAnm, "chc_win_00_chcWinOut.brlan", NULL);
    mWindowAnm.mFrameCtrl.mFlags = 1;
    mWindowAnm.setFrame(0.0f);
    onClose();
}

// 8015CFD8
void BComp_c::executeOut() {
    mLayout.calc();
    if (mWindowAnm.mFrameCtrl.isStop()) {
        changeMode(&BComp_c::executeHidden);
    }
}

// 8015D158
void BComp_c::reset() {
    BaseComp_c::reset();
    for (int i = 0; i < NUM_ANSWERS; i++) {
        mSeparates[i].clear();
    }
    mCurrent = -1;
    mNum = 2;
    mCancel = -1;
    mLockMask = 0;
    mCheckNum = 0;
    mCheckHits = 0;
    for (int i = 0; i < NUM_ANSWERS; i++) {
        mCheckResults[i] = 0;
        mCheckWords[i] = NULL;
        mCheckTexts[i] = NULL;
    }
}

// 8015D214
void BComp_c::unlock(bool close) {
    if (close) {
        changeMode(&BComp_c::executeOut);
    } else {
        changeMode(mNextMode);
        mpCursor->SetVisible(true);
    }
}

// 8015D4B0
Separate_c *BComp_c::getSeparate(int idx) {
    return &mSeparates[idx];
}

// 8015D4C0
void BComp_c::setResultLabel(const char *label) {
    if (strlen(label)) {
        strncpy(mLabel, label, sizeof(mLabel) - 1);
    }
}

// 8015D514
void BComp_c::updateVisible() {
    for (int i = 0; i < NUM_ANSWERS; i++) {
        nw4r::lyt::TextBox *text = mSeparates[i].mpTextBox;
        nw4r::lyt::Pane *line = mLayout.findPane(sLineNames[i]);
        nw4r::lyt::Pane *button = mLayout.findPane(sButtonNames[i]);
        if (i < mNum) {
            text->SetVisible(true);
            line->SetVisible(true);
            button->SetVisible(true);
        } else {
            text->SetVisible(false);
            line->SetVisible(false);
            button->SetVisible(false);
        }
    }
}

// 8015D5F8
f32 BComp_c::getMaxTextWidth() {
    f32 width = 30.0f;
    for (int i = 0; i < mNum; i++) {
        f32 w = getTextWidth(mSeparates[i].mpTextBox);
        if (width < w) {
            width = w;
        }
    }
    return width;
}

// 8015D680
f32 BComp_c::getTextHeight() {
    return mNum * 41;
}

// 8015D6B0
void BComp_c::calcWindowSize() {
    mTextWidth = getMaxTextWidth();
    mTextHeight = getTextHeight();
    f32 rateW = (mTextWidth - 30.0f) / 270.0f;
    f32 rateH = (mTextHeight - 82.0f) / 123.0f;
    mMarginLeft = 44.0f + 21.0f * rateW;
    mMarginRight = 27.0f + 19.0f * rateW;
    mMarginTop = 11.0f + 17.0f * rateH;
    mMarginBottom = 13.0f + 21.0f * rateH;
}

// 8015D760
void BComp_c::resizeWindow() {
    f32 sx = getWindowWidth() / 384.0f;
    f32 sy = getWindowHeight() / 293.0f;
    mVec2_c scale(sx, sy);
    mpWindow->SetScale(scale);
    mpShadow->SetScale(scale);
    nw4r::lyt::Pane *msg = mLayout.findPane("N_msg_all");
    nw4r::lyt::Pane *line = mLayout.findPane("N_line_all");
    nw4r::lyt::Pane *button = mLayout.findPane("N_b_msg_all");
    mVec3_c pos = mpWindow->GetTranslate();
    pos.y -= (NUM_ANSWERS - mNum) * 41 / 2;
    pos.y -= mMarginTop / 2.0f;
    pos.y += mMarginBottom / 2.0f;
    pos.x += -11.0f;
    pos.x -= mMarginRight / 2.0f;
    pos.x += mMarginLeft / 2.0f;
    mVec3_c linePos = pos;
    linePos.x -= 1.0f;
    linePos.x += 1.0f;
    line->SetTranslate(linePos);
    button->SetTranslate(pos);
    pos.x += mTextBoxWidth / 2.0f - mTextWidth / 2.0f;
    msg->SetTranslate(pos);
    line->SetScale(mVec2_c((2.0f + (2.0f + mTextWidth)) / mLineWidth, 1.0f));
    f32 buttonWidth = mTextWidth;
    button->SetScale(mVec2_c(buttonWidth / mButtonWidth, 1.0f));
}

// 8015D960
void BComp_c::updateCursor() {
    for (int i = 0; i < NUM_ANSWERS; i++) {
        if (i != mCurrent) {
            changeTextAnm(i, 0);
        } else if (!mKeyMode) {
            changeTextAnm(i, 1);
        } else {
            changeTextAnm(i, 2);
        }
    }
    if (mCurrent == -1) {
        mpCursor->SetVisible(false);
    } else {
        mpCursor->SetVisible(true);
        mVec3_c pos = mSeparates[mCurrent].mpTextBox->GetTranslate();
        pos.x += -5.0f - 0.5f * mTextWidth;
        pos.x += -11.0f;
        pos.x += mMarginLeft * 0.5f - mMarginRight * 0.5f;
        pos.y += -(0.5f * ((NUM_ANSWERS - mNum) * 41));
        pos.y += mMarginBottom * 0.5f - mMarginTop * 0.5f;
        mpCursor->SetTranslate(pos);
    }
}

// 8015DAEC
void BComp_c::changeTextAnm(int idx, int anm) {
    static const char *names[] = {
        "chc_win_00_txtBrwnGrn.brlan", "chc_win_00_txtGrnBrwn.brlan", "chc_win_00_txtBluBrwn.brlan",
        "chc_win_00_txtGrnPsh.brlan",  "chc_win_00_txtBluPsh.brlan",
    };
    m2d::Anm_c *a = &mTextAnms[idx];
    if (a->isBound()) {
        m2d::setAnmNumber(a, idx);
        mLayout.unbind(a);
    }
    a->create(names[anm], &mResources);
    a->mFrameCtrl.mFlags = 1;
    m2d::setAnmNumber(a, idx);
    mLayout.bind(a);
}

// 8015DB9C
void BComp_c::setKeyMode(bool keyMode) {
    static const char *names[] = {"chc_win_00_crsrBluGrn.brlan", "chc_win_00_crsrGrnBlu.brlan"};
    mKeyMode = keyMode;
    if (!mKeyMode) {
        mCurrent = -1;
    } else {
        mCurrent = 0;
    }
    mLayout.setAnm(&mCursorAnm, names[!keyMode], NULL);
}

// 8015DBDC
void BComp_c::decide() {
    static const int sDecideSe[] = {0x16D7, 0x1734, 0x1710, 0x16D2};
    mCursor = mCurrent;
    Separate_c *separate = getSeparate(mCursor);
    if (mCancel != -1 && mCancel == mCurrent) {
        playSe(0x16D8);
    } else {
        playSe(sDecideSe[separate->mIsCancel]);
    }
    if (separate->mIsCancel == 1) {
        dMsg::Comp_c::mInstance->mNoCloseSe = 1;
    }
    changeMode(&BComp_c::executeDecide);
}

// 8015DDA0
void BComp_c::updatePointer() {
    struct Hit {
        mVec2_c pos;
        f32 scale;
        nw4r::lyt::Pane *pane;
    } hit;
    hit.pane = NULL;
    hit.scale = 1.0f;
    hit.pos = dObjCursor_c::getPos();
    mLayout.patrolPane(m2d::isInPane, m2d::isRectButton, &hit);
    int idx = -1;
    if (hit.pane != NULL) {
        idx = m2d::getNameNumber(hit.pane->GetName(), 2);
        if (idx < mNum) {
            mCurrent = idx;
        }
    }
    mCurrent = idx;
    if (mPad::getCore()->downTrigger(0x800)) {
        if (mCurrent != -1) {
            decide();
        }
    } else if (mCancel != -1 && dPad::isCancelTriggered()) {
        cancel();
    }
}

// 8015DEAC
void BComp_c::updateKey() {
    if (mRepeatTimer != 0) {
        mRepeatTimer--;
    }
    if (mRepeatDir == 0 && (isUpHold() || isDownHold())) {
        int dir = isUpHold() ? -1 : 1;
        mRepeatDir = dir;
        mCurrent += dir;
        mRepeatTimer = 27;
    } else if ((mRepeatDir == 1 && isDownHold()) || (mRepeatDir == -1 && isUpHold())) {
        if (mRepeatTimer == 0) {
            mCurrent += mRepeatDir;
            mRepeatTimer = 9;
        }
    } else {
        mRepeatTimer = 0;
        mRepeatDir = 0;
    }
    int cur = mCurrent;
    int max = mNum - 1;
    if (cur < 0) {
        cur = 0;
    } else if (cur > max) {
        cur = max;
    }
    mCurrent = cur;
    if (isDecideTrig()) {
        decide();
    } else if (mCancel != -1 && isCancelTrig()) {
        cancel();
    }
}

// 8015E01C
void BComp_c::cancel() {
    mCursor = mCurrent = mCancel;
    playSe(0x16D8);
    changeMode(&BComp_c::executeDecide);
}

// 8015E18C
void BComp_c::onOpen() {
    BaseComp_c::onOpen();
    setTexts();
}

// 8015E1C0
void BComp_c::setTexts() {
    for (int i = 0; i < NUM_ANSWERS; i++) {
        mSeparates[i].applyText();
    }
}

// 8015E244
void BComp_c::onClose() {
    BaseComp_c::onClose();
}

// 8015E248
void BComp_c::preExecute() {}

// 8015E24C
void Comp_c::decide() {
    if (mDecideDelay == 0) {
        BComp_c::decide();
    }
}

// 8015E260
void Comp_c::cancel() {
    if (mCancelDelay == 0) {
        BComp_c::cancel();
    }
}

// 8015E274
bool Comp_c::create() {
    return BComp_c::create();
}

// 8015E2A0
bool Comp_c::remove() {
    return BComp_c::remove();
}

// 8015E2CC
void Comp_c::updateLayout() {
    updateVisible();
    calcWindowSize();
    resizeWindow();
    moveWindow();
}

// 8015E310
void Comp_c::moveWindow() {
    f32 w = getWindowWidth();
    f32 h = getWindowHeight();
    mVec3_c pos(0.0f, 0.0f, 0.0f);
    pos.x = 256.0f;
    pos.y = -175.0f;
    pos.x -= w / 2.0f;
    pos.y += h / 2.0f;
    pos.x += mMarginRight;
    pos.y -= mMarginBottom;
    mpAll->SetTranslate(pos);
}

// 8015E388
void Comp_c::onOpen() {
    BComp_c::onOpen();
    updateLayout();
    updateCursor();
    mDecideDelay = 15;
    mCancelDelay = 30;
}

// 8015E3D4
void Comp_c::onClose() {
    BComp_c::onClose();
}

// 8015E3D8
void Comp_c::preExecute() {
    if (mDecideDelay != 0) {
        mDecideDelay--;
    }
    if (mCancelDelay != 0) {
        mCancelDelay--;
    }
}

// 8015E404
void MComp_c::addItem(u16 index, u16 code, int lock, int isCancel) {
    dString::Word_c word(index, "sys_2D/SYS_2D_menu");
    setWord(mNum, &word, code, NULL, isCancel, 0);
    if (lock) {
        mLockMask |= 1 << mNum;
    }
    mNum++;
}

// 8015E4B8
void MComp_c::setCancelLast() {
    mCancel = mNum - 1;
}

// 8015E4C8
void MComp_c::placeAbove(mVec2_c pos) {
    updateLayout();
    if (SCGetAspectRatio() == 1) {
        pos.x += 0.730769f * (0.5f * getWindowWidth());
    } else {
        pos.x += 0.5f * getWindowWidth();
    }
    f32 x = pos.x;
    pos.y += 0.5f * getWindowHeight();
    mpAll->SetTranslate(mVec3_c(x, pos.y, 0.0f));
    fitScreen();
}

// 8015E5C0
void MComp_c::placeBelow(mVec2_c pos) {
    updateLayout();
    if (SCGetAspectRatio() == 1) {
        pos.x += 0.730769f * (0.5f * getWindowWidth());
    } else {
        pos.x += 0.5f * getWindowWidth();
    }
    f32 x = pos.x;
    pos.y -= 0.5f * getWindowHeight();
    mpAll->SetTranslate(mVec3_c(x, pos.y, 0.0f));
    fitScreen();
}

// 8015E6B8
void MComp_c::place(mVec2_c pos) {
    updateLayout();
    bool above = false;
    if (pos.y - (mTextHeight + mMarginTop) < -194.0f &&
        mMarginBottom + (mTextHeight + (41.0f + pos.y)) <= 194.0f) {
        above = true;
    }
    if (SCGetAspectRatio() == 1) {
        pos.x += 0.730769f * (0.5f * getWindowWidth());
    } else {
        pos.x += 0.5f * getWindowWidth();
    }
    if (above) {
        pos.y += 41.0f + 0.5f * getWindowHeight();
    } else {
        pos.y -= 0.5f * getWindowHeight();
    }

    mpAll->SetTranslate(mVec3_c(pos, 0.0f));
    fitScreen();
}

// 8015E844
void MComp_c::place(mVec2_c *below, mVec2_c *above) {
    updateLayout();
    bool isAbove = false;
    if (below->y - (mTextHeight + mMarginTop) < -194.0f && mMarginBottom + (above->y + mTextHeight) <= 194.0f) {
        isAbove = true;
    }
    mVec2_c pos;
    if (isAbove) {
        pos = *above;
    } else {
        pos = *below;
    }
    if (SCGetAspectRatio() == 1) {
        pos.x += 0.730769f * (0.5f * getWindowWidth());
    } else {
        pos.x += 0.5f * getWindowWidth();
    }
    if (isAbove) {
        pos.y += 0.5f * getWindowHeight();
    } else {
        pos.y -= 0.5f * getWindowHeight();
    }
    mpAll->SetTranslate(mVec3_c(pos, 0.0f));
    fitScreen();
}

// 8015E9FC
void MComp_c::updateLayout() {
    setTexts();
    updateVisible();
    calcWindowSize();
    resizeWindow();
}

// 8015EA40
void MComp_c::fitScreen() {
    clampToScreen();
    updateCursor();
}

// 8015EA74
f32 MComp_c::getLeft() {
    f32 x = mpAll->GetTranslate().x;
    return x - 0.5f * getWindowWidth();
}

// 8015EAA0
f32 MComp_c::getCursorY() {
    return m2d::getTotalPos(mSeparates[mCursor].mpTextBox).y - 20.5f;
}

// 8015EAE4
BOOL MComp_c::isReady() {
    if (isOut() || isIn()) {
        return FALSE;
    }
    isClosed();
    return TRUE;
}

// 8015EB94
void MComp_c::close() {
    requestClose();
}

// 8015ED08
void MComp_c::clampToScreen() {
    nw4r::lyt::TextBox *text = mSeparates[0].mpTextBox;
    mVec3_c pos = mpAll->GetTranslate();
    mVec3_c textPos = m2d::getTotalPos(text);
    f32 left = textPos.x - 0.5f * text->GetSize().width;
    f32 right = left + mTextWidth;
    left -= 24.0f;
    if (left < -276.0f) {
        pos.x += -276.0f - left;
    } else if (right > 276.0f) {
        pos.x -= right - 276.0f;
    }
    f32 top = textPos.y + 0.5f * text->GetSize().height;
    f32 bottom = top - mNum * text->GetSize().height;
    if (bottom < -194.0f) {
        pos.y += -194.0f - bottom;
    } else if (top > 194.0f) {
        pos.y -= top - 194.0f;
    }
    mpAll->SetTranslate(pos);
}

// 8015EE58
void MComp_c::onOpen() {
    BComp_c::onOpen();
}

// 8015EE5C
void MComp_c::onClose() {
    BComp_c::onClose();
}

// 8015EE60
void MComp_c::preExecute() {}

// 8015EE64
Word_c::Word_c() {
    clear();
}

// 8015EEA8
Word_c::~Word_c() {}

// 8015EF00
u32 Word_c::getBufferSize() {
    return sizeof(mBuffer);
}

// 8015EF08
wchar_t *Word_c::getBuffer() {
    return mBuffer;
}

// 8015EF10
int Word_c::procRubyTag(int pos) {
    return procRuby(pos);
}

} // namespace dSelect
