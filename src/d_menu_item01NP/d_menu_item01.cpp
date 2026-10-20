#include <game/game/d_menu_item01.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/mLib/m_pad.hpp>

namespace dMenuItem01 {
// .text:0x5C
BASE_PROFILE(MENU_ITEM01, dMenuItem01_c)
} // namespace dMenuItem01

// .text:0x250
int dMenuItem01_c::create() {
    if (!load()) {
        return NOT_READY;
    }
    if (createLayouts()) {
        return NOT_READY;
    }
    onFlag(FLAG_CALC_ITEM);
    onFlag(FLAG_DRAW_ITEM);
    onFlag(FLAG_CALC_LETTER);
    onFlag(FLAG_DRAW_LETTER_BUTTON);
    startAnm(mItemWin.getLayout(), "N_itmWin_all", ANM_WIN_IN, STATE_MAIN,
             TRUE);
    setSubState(SUB_MAIN);
    return SUCCEEDED;
}

// .text:0x318
int dMenuItem01_c::doDelete() {
    return remove();
}

// .text:0x31C
int dMenuItem01_c::stateMain() {
    updateReturn();
    executeSubState();
    updateExtra();
    return SUCCEEDED;
}

// .text:0x35C
void dMenuItem01_c::subStateMain() {
    if (!isReturning()) {
        hideTutorial();
        mVec2_c pos = dObjCursor_c::getPos();
        mCursor = hitPocket(pos, 1.0f);
        if (mCursor != SLOT_NONE) {
            if (!isPocketEmpty(mCursor)) {
                markSlot(mCursor);
            }
            showSlotTutorial(mCursor, 0, FALSE);
            if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A) &&
                setupSelect(mCursor)) {
                openSelect(mCursor, TRUE);
                setSubState(SUB_SELECT);
                mCursor = SLOT_NONE;
            }
        } else {
            mCursor = SLOT_NONE;
        }
    }
}

// .text:0x478
void dMenuItem01_c::subStateSelect() {
    if (mSelect.isMode(&dSelect::BComp_c::executeLock)) {
        u16 code = mSelect.mCode;
        mSubSelect.reset();
        mSubSelect.mNum = 0;
        switch (code) {
        case ANSWER_OPEN_SUB:
            onAnswerSub();
            break;
        default:
            mSelect.unlock(true);
            return;
        }
        mSubSelect.addItem(MSG_QUIT, ANSWER_QUIT, 0, 0);
        mSubSelect.setCancelLast();
        openSubSelect();
        setSubState(SUB_SUB_OPEN);
    }
    if (static_cast<dSelect::BaseComp_c &>(mSelect).isClosed()) {
        u16 code = mSelect.mCode;
        onFlag(FLAG_ANSWERED);
        if (code != ANSWER_QUIT) {
            hideTutorial();
        }
        switch (code) {
        case ANSWER_PICK:
            pick();
            break;
        case ANSWER_QUIT:
        default:
            cancel();
            break;
        }
    }
}

// .text:0x5CC
void dMenuItem01_c::subStateSubOpen() {
    if (mSubSelect.isMode(&dSelect::BComp_c::executeOut)) {
        switch (mSubSelect.mCode) {
        case ANSWER_CONFIRM:
            break;
        default:
            mSelect.unlock(true);
            break;
        }
        setSubState(SUB_SUB_SELECT);
    }
}

// .text:0x658
void dMenuItem01_c::subStateSubSelect() {
    if (static_cast<dSelect::BaseComp_c &>(mSubSelect).isClosed()) {
        switch (mSubSelect.mCode) {
        default:
            cancel();
            break;
        case ANSWER_CONFIRM:
            mSelect.unlock(false);
            setSubState(SUB_SELECT);
            break;
        case ANSWER_QUIT:
            cancel();
            break;
        }
    }
}

// .text:0x6F0
void dMenuItem01_c::pick() {
    dMenuMng_c::setResult(dMenuMng_c::RESULT_OK);
    dMenuMng_c::setSelSlot(mSelectSlot);
    closeWindowSe();
}

// .text:0x734
void dMenuItem01_c::cancel() {
    backToMain();
}

// .text:0x738
void dMenuItem01_c::onAnswerSub() {
}

// .text:0x73C
void dMenuItem01_c::executeSubState() {
    typedef void (dMenuItem01_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {
        &dMenuItem01_c::subStateMain,
        &dMenuItem01_c::subStateSelect,
        &dMenuItem01_c::subStateSubOpen,
        &dMenuItem01_c::subStateSubSelect,
    };
    (this->*subStateProcs[mSubState])();
}

// .text:0x774
void dMenuItem01_c::updateReturn() {
    if (calcReturn()) {
        return;
    }
}

// .text:0x794
void dMenuItem01_c::updateExtra() {
}

// .text:0x798
int dMenuItem01_c::draw() {
    return dMenuItemXX_c::draw();
}

// .text:0x79C
bool dMenuItem01_c::setupSelect(int slot) {
    mSelect.reset();
    mSelect.mNum = 0;
    if (!isPocket(slot) || isPocketEmpty(slot)) {
        return false;
    }
    mSelect.addItem(MSG_QUIT, ANSWER_QUIT, 0, 0);
    mSelect.setCancelLast();
    return true;
}

// .text:0x848
void dMenuItem01_c::backToMain() {
    if (dObjCursor_c::isVisible()) {
        mCursor = mSelectSlot;
    }
    setSubState(SUB_MAIN);
}
