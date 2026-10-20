#include <game/game/d_menu_melody.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_obj_tutorial.hpp>
#include <game/game/d_post_office.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/mLib/m_pad.hpp>
#include <stdio.h>

namespace dMenuMelody {
// .text:0x5C
BASE_PROFILE(MENU_MELODY, dMenuMelody_c)
} // namespace dMenuMelody

static const char *l_allPane = "N_all";

// .text:0x1FC
int dMenuMelody_c::create() {
    int loaded = mRes.load("Layout/melody.arc");
    loaded &= mBtn.load();
    if (!loaded) {
        return NOT_READY;
    }
    if (!createNextNote()) {
        return NOT_READY;
    }
    loadTune();
    mLyt.build(&mRes, "muramero_win_00.brlyt");
    mBtn.create();
    initButtons();
    initAnms();
    mLyt.calc();
    updateNotes();
    dPostOffice::clearMelodySync();
    onFlag(FLAG_DRAW);
    startAnm(&mLyt, l_allPane, ANM_WIN_IN, STATE_MAIN, TRUE);
    setSubState(SUB_MAIN);
    return SUCCEEDED;
}

// .text:0x310
int dMenuMelody_c::doDelete() {
    if (mBtn.remove() == FALSE) {
        return NOT_READY;
    }
    return mRes.remove() != 0;
}

// .text:0x364
int dMenuMelody_c::stateMain() {
    executeSubState();
    return SUCCEEDED;
}

// .text:0x388
int dMenuMelody_c::stateAnm() {
    int result = dMenuBase_c::stateAnm();
    mBtn.setPos(&mLyt, l_allPane);
    onFlag(FLAG_NOTES_POS);
    return result;
}

// .text:0x3E4
int dMenuMelody_c::stateClose() {
    if (!dPostOffice::isMelodySyncDone()) {
        return NOT_READY;
    }
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x42C
int dMenuMelody_c::execute() {
    int prevCursor = mCursor;
    int result = dMenuBase_c::execute();
    if (mCursor != CURSOR_NONE && mCursor != prevCursor) {
        playSeCursor();
    }
    mBtn.execute();
    calcLeafAnm();
    return result;
}

// .text:0x49C
int dMenuMelody_c::draw() {
    if (isFlag(FLAG_CALC)) {
        offFlag(FLAG_CALC);
        mLyt.calc();
    }
    if (isFlag(FLAG_NOTES_POS)) {
        offFlag(FLAG_NOTES_POS);
        initNotePos();
    }
    if (isFlag(FLAG_NOTES_CALC)) {
        offFlag(FLAG_NOTES_CALC);
        calcNotes();
    }
    if (isFlag(FLAG_DRAW)) {
        mLyt.entry();
        drawNotes();
        mBtn.draw();
    }
    return SUCCEEDED;
}

// .text:0x574
void dMenuMelody_c::subStateMain() {
    setSelect(mCursor, FALSE);
    dObjTutorial_c::hide();
    int btn = mBtn.checkPad(TRUE, TRUE);
    if (btn != CURSOR_NONE) {
        mCursor = btn;
        setSelect(btn, TRUE);
        push();
        return;
    }
    int slot = hitCheck();
    mCursor = slot;
    if (slot != CURSOR_NONE) {
        setSelect(slot, TRUE);
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            push();
        } else if (mCursor == CURSOR_PLAY) {
            showPlayTutorial();
        }
    }
}

// .text:0x64C
void dMenuMelody_c::subStatePush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0x688
void dMenuMelody_c::subStatePlay() {
    int pos = getTownTuneState();
    if (pos != mPlayPos) {
        if (mPlayPos >= 0) {
            mNoteLyt[mPlayPos].setIdle(true);
        }
        mPlayPos = pos;
        if (pos == -1) {
            setSubState(SUB_MAIN);
        } else {
            mNoteLyt[pos].setIdle(false);
        }
    }
}

// .text:0x724
void dMenuMelody_c::subStateGrab() {
    if (!mPad::g_currentCore->down(WPAD_BUTTON_A)) {
        setSubState(SUB_MAIN);
        dObjCursor_c::show();
        return;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    int row = mGrabRow + (int)((pos.y - mGrabY) / NOTE_HEIGHT);
    if (row < 0) {
        row = 0;
    } else if (row >= NOTE_NUM) {
        row = NOTE_NUM - 1;
    }
    int idx = mCursor - CURSOR_NOTE_FIRST;
    u8 note = getNote(row);
    if (note != mTune[idx]) {
        playMelodyNote(note);
        mTune[idx] = note;
        updateNotes();
        setSelect(mCursor, TRUE);
    }
}

// .text:0x83C
void dMenuMelody_c::executeSubState() {
    typedef void (dMenuMelody_c::*Proc)();
    static Proc subStateProcs[] = {
        &dMenuMelody_c::subStateMain, &dMenuMelody_c::subStatePush,
        &dMenuMelody_c::subStatePlay, &dMenuMelody_c::subStateGrab};
    (this->*subStateProcs[mSubState])();
}

// .text:0x874
void dMenuMelody_c::setSelect(int slot, BOOL select) {
    if (isBtn(slot)) {
        mBtn.setSelect(slot, select, FALSE);
    }
    if (slot == CURSOR_PLAY) {
        mLeafAnm.setNext(!select);
    }
    if (isNote(slot)) {
        mNoteLyt[slot - CURSOR_NOTE_FIRST].setSelected(select);
    }
}

// .text:0x938
int dMenuMelody_c::hitCheck() {
    if (!dObjCursor_c::isVisible()) {
        return CURSOR_NONE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    int btn = mBtn.hitCheck(&pos);
    if (btn != CURSOR_NONE) {
        return btn;
    }
    mVec2_c hitPos = pos;
    int hit = m2d::hitButton(&hitPos, &mLyt, CURSOR_NOTE_FIRST, CURSOR_PLAY,
                             CURSOR_NONE, CURSOR_NONE, 1.0f);
    if (hit != CURSOR_NONE) {
        return hit;
    }
    return CURSOR_NONE;
}

// .text:0x9D8
void dMenuMelody_c::push() {
    int slot = mCursor;
    if (isBtn(slot)) {
        mBtn.push(slot, FALSE);
    } else if (isNote(slot)) {
        startGrab();
        playMelodyNote(mTune[slot - CURSOR_NOTE_FIRST]);
        return;
    } else if (slot == CURSOR_PLAY) {
        mLeafAnm.setNext(LEAF_ANM_PUSH);
    } else {
        return;
    }
    setSubState(SUB_PUSH);
}

// .text:0xAAC
BOOL dMenuMelody_c::isPushEnd() {
    int slot = mCursor;
    if (isBtn(slot)) {
        return !mBtn.isPushing();
    }
    if (isNote(slot)) {
        return mNoteLyt[slot - CURSOR_NOTE_FIRST].isPushEnd();
    }
    if (slot == CURSOR_PLAY) {
        return mLeafAnm.isEnd();
    }
    return true;
}

// .text:0xB50
void dMenuMelody_c::finishPush() {
    int slot = mCursor;
    if (isBtn(slot)) {
        switch (slot) {
        case BTN_CANCEL:
            startCloseAnm();
            break;
        case BTN_UNUSED:
            break;
        case BTN_OK:
            startCloseAnm();
            saveTune();
            dMenuMng_c::setResult(1);
            break;
        }
    } else if (isNote(slot)) {
        setSubState(SUB_MAIN);
    } else if (slot == CURSOR_PLAY) {
        mPlayPos = -1;
        playMelody(mTune);
        setSubState(SUB_PLAY);
    } else {
        setSubState(SUB_MAIN);
    }
}

// .text:0xC70
void dMenuMelody_c::startGrab() {
    mGrabRow = mNoteLyt[mCursor - CURSOR_NOTE_FIRST].mRow;
    mGrabY = dObjCursor_c::getPos().y;
    setSubState(SUB_GRAB);
    dObjCursor_c::hold();
}

// .text:0xCDC
void dMenuMelody_c::startCloseAnm() {
    startAnm(&mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, TRUE);
    dMenuMng_c::setResult(0);
}

// .text:0xD1C
void dMenuMelody_c::initButtons() {
    mBtn.setText(BTN_CANCEL, MSG_QUIT, TRUE);
    mBtn.setVisible(BTN_UNUSED, false);
    mBtn.setText(BTN_OK, MSG_CONFIRM, FALSE);
}

// .text:0xD7C
bool dMenuMelody_c::createNextNote() {
    if (mLoadCnt < NOTE_NUM) {
        mNoteLyt[mLoadCnt].create(&mRes);
        mLoadCnt++;
    }
    return mLoadCnt >= NOTE_NUM;
}

// .text:0xDE8
void dMenuMelody_c::drawNotes() {
    for (int i = 0; i < NOTE_NUM; i++) {
        mNoteLyt[i].draw();
    }
}

// .text:0xE34
void dMenuMelody_c::calcNotes() {
    for (int i = 0; i < NOTE_NUM; i++) {
        mNoteLyt[i].requestCalc();
    }
}

// .text:0xE80
void dMenuMelody_c::initNotePos() {
    for (int i = 0; i < NOTE_NUM; i++) {
        char name[0x28];
        sprintf(name, "N_note_%02d\0", i);
        mNoteLyt[i].setPos(&mLyt, name);
    }
    onFlag(FLAG_NOTES_CALC);
}

// .text:0xF0C
void dMenuMelody_c::loadTune() {
    dSaveData_c::getTown()->mVillageMelody.get(mTune);
}

// .text:0xF48
void dMenuMelody_c::saveTune() {
    dSaveData_c::getTown()->mVillageMelody.set(mTune);
    dPostOffice::broadcastMelody();
}

// .text:0xF88
void dMenuMelody_c::initAnms() {
    static const char *leafAnmNames[LEAF_ANM_NUM] = {
        "muramero_win_00_leafBtn_In.brlan",
        "muramero_win_00_leafBtn_Out.brlan",
        "muramero_win_00_leafBtn_Psh.brlan",
    };
    for (int i = 0; i < NOTE_NUM; i++) {
        mToneAnm[i].create("muramero_win_00_tone_00.brlan", &mRes);
        m2d::setAnmNumber(&mToneAnm[i], i);
        mLyt.bind(&mToneAnm[i]);
    }
    mLeafAnm.init(&mLyt, &mRes, leafAnmNames, LEAF_ANM_NUM);
}

// .text:0x1028
void dMenuMelody_c::updateNotes() {
    for (int i = 0; i < NOTE_NUM; i++) {
        u8 row = getRow(mTune[i]);
        mToneAnm[i].setFrame(row);
        mNoteLyt[i].setRow(row);
    }
    onFlag(FLAG_CALC);
    onFlag(FLAG_NOTES_POS);
    onFlag(FLAG_NOTES_CALC);
}

// .text:0x10F4
void dMenuMelody_c::calcLeafAnm() {
    if (mLeafAnm.play()) {
        onFlag(FLAG_CALC);
    }
}

// .text:0x1138
void dMenuMelody::Note_c::create(m2d::ResAccIf_c *res) {
    mLyt.build(res, "muramero_note_00.brlyt");
    mLyt.setAnm(&mColorAnm, "muramero_note_00_color_00.brlan", nullptr);
    mLyt.setAnm(&mPushAnm, "muramero_note_00_note_Psh.brlan", nullptr);
    mPushAnm.setToEnd();
    mLyt.setAnm(&mPlayAnm, "muramero_note_00_silent_Play.brlan", nullptr);
    mLyt.setVisible("note_Psh", false);
    mLyt.setVisible("mo_Psh", false);
    mLyt.setVisible("slur_Psh", false);
    mLyt.setMessage("T_mo_00", MSG_RANDOM, nullptr);
    mLyt.setMessage("T_slur_00", MSG_SLUR, nullptr);
    mLyt.setAlpha("note_Fc", 0xFF);
    mLyt.setAlpha("mo_Fc", 0xFF);
    mLyt.setAlpha("slur_Fc", 0xFF);
    setSelected(false);
    mLyt.calc();
}

// .text:0x1264
void dMenuMelody::Note_c::draw() {
    if (!mPushAnm.mFrameCtrl.isStop()) {
        mPushAnm.play();
        mFlags |= 1;
    }
    if (mFlags & 1) {
        mFlags &= ~1;
        mLyt.calc();
    }
    mLyt.entry();
}

// .text:0x12E0
void dMenuMelody::Note_c::requestCalc() {
    mFlags |= 1;
}

// .text:0x12F0
void dMenuMelody::Note_c::setPos(m2d::SimpleEx_c *base, const char *name) {
    base->copyPos(&mLyt, name, "N_all");
}

// .text:0x1308
void dMenuMelody::Note_c::setRow(int row) {
    static u16 noteMsgs[ROW_NUM] = {
        MSG_SLUR, MSG_SLUR, 0xE2, 0xE3, 0xE4, 0xDE, 0xDF, 0xE0,
        0xE1,     0xE2,     0xE3, 0xE4, 0xDE, 0xDF, 0xE0, MSG_RANDOM};
    mRow = row;
    mLyt.setVisible("N_silent_00", row == ROW_REST);
    mLyt.setVisible("N_slur_00", row == ROW_SLUR);
    mLyt.setVisible("N_normal_00",
                    row >= ROW_NOTE_FIRST && row <= ROW_NOTE_LAST);
    mLyt.setVisible("N_mo_00", row == ROW_RANDOM);
    setIdle(true);
    mColorAnm.setFrame(row);
    if (row >= ROW_NOTE_FIRST && row <= ROW_NOTE_LAST) {
        mLyt.setMessage("T_note_00", noteMsgs[row], nullptr);
    }
}

// .text:0x1414
void dMenuMelody::Note_c::setSelected(BOOL select) {
    mLyt.setVisible("note_Fc", select && isNormal());
    mLyt.setVisible("mo_Fc", select && mRow == ROW_RANDOM);
    mLyt.setVisible("slur_Fc", select && mRow == ROW_SLUR);
}

// .text:0x14E8
bool dMenuMelody::Note_c::isPushEnd() {
    return mPushAnm.mFrameCtrl.isStop();
}

// .text:0x14F0
void dMenuMelody::Note_c::setIdle(bool idle) {
    switch (mRow) {
    case ROW_REST:
        mPlayAnm.setFrame(idle ? 0.0f : 1.0f);
        mFlags |= 1;
        break;
    case ROW_SLUR: {
        bool play = !idle;
        mLyt.setVisible("N_slurMouth", !play);
        mLyt.setVisible("N_slurPlay_00", play);
        break;
    }
    case ROW_RANDOM: {
        bool play = !idle;
        mLyt.setVisible("N_moMouth", !play);
        mLyt.setVisible("N_moPlay", play);
        break;
    }
    default: {
        bool play = !idle;
        mLyt.setVisible("N_normalMouth", !play);
        mLyt.setVisible("N_normalPlay", play);
        break;
    }
    }
}

// .text:0x1610
u8 dMenuMelody_c::getRow(u8 note) const {
    static const u8 rows[NOTE_NUM] = {2,  3,  4,  5,  6,  7,  8, 9,
                                      10, 11, 12, 13, 14, 15, 1, 0};
    return rows[note];
}

// .text:0x1620
u8 dMenuMelody_c::getNote(u8 row) const {
    static const u8 notes[NOTE_NUM] = {15, 14, 0, 1, 2,  3,  4,  5,
                                       6,  7,  8, 9, 10, 11, 12, 13};
    return notes[row];
}

// .text:0x1630
void dMenuMelody_c::showPlayTutorial() {
    if (dObjTutorial_c::request(0, dObjTutorial_c::MSG_PLAY, TRUE)) {
        nw4r::lyt::Pane *pane = mLyt.findPane("playIcon_00");
        mVec3_c pos;
        m2d::getScreenPos(&pos, pane);
        mVec2_c pos2;
        pos2.x = pos.x;
        pos2.y = pos.y;
        dObjTutorial_c::setMsg(&pos2, dObjTutorial_c::MSG_PLAY, TRUE);
    }
}
