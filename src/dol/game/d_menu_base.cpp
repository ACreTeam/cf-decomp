#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/snd/snd_id.hpp>
#include <string.h>

static const char *l_anmName[dMenuBase_c::ANM_NUM] = {
    "sb_win_N_all_sbWinIn.brlan",
    "sb_win_N_all_sbWinOut.brlan",
    "sb_win_N_all_sbWinUp.brlan",
    "sb_win_N_all_sbWinDwn.brlan",
    "sb_win_N_all_upperWin_In.brlan",
    "sb_win_N_all_upperWin_Out.brlan",
};

static const char *l_allPaneName = "N_all";

static int (dMenuBase_c::*l_stateFunc[dMenuBase_c::STATE_NUM])() = {
    &dMenuBase_c::stateAnm,
    &dMenuBase_c::stateMain,
    &dMenuBase_c::state2,
    &dMenuBase_c::state3,
    &dMenuBase_c::stateClose,
};

static const int l_pageTurnSe[16] = {
    SE_gamen_okuri01, SE_gamen_okuri02, SE_gamen_okuri03, SE_gamen_okuri04,
    SE_gamen_okuri05, SE_gamen_okuri06, SE_gamen_okuri07, SE_gamen_okuri08,
    SE_gamen_okuri09, SE_gamen_okuri10, SE_gamen_okuri11, SE_gamen_okuri12,
    SE_gamen_okuri13, SE_gamen_okuri14, SE_gamen_okuri15, SE_gamen_okuri16,
};

u32 dMenuBase_c::s_postFlags;

dMenuBase_c::dMenuBase_c() {}

dMenuBase_c::~dMenuBase_c() {}

const char *dMenuBase_c::getAnmName(int anm) {
    return l_anmName[anm];
}

void dMenuBase_c::setRepeat(int delay, int interval, int count) {
    mRepeatDelay = delay;
    mRepeatCnt[0] = 0;
    mRepeatCnt[1] = 0;
    mRepeatCnt[2] = 0;
    mRepeatCnt[3] = 0;
    mRepeatInterval = interval;
    mRepeatCount = count;
}

int dMenuBase_c::preCreate() {
    mNode.mpOwner = this;
    mMenuNo = mParam;
    mpAnmLayout = nullptr;
    setRepeat(20, 15, 5);
    return SUCCEEDED;
}

void dMenuBase_c::postCreate(MAIN_STATE_e state) {
    if (state == SUCCESS) {
        dMenuMng_c::addMenu(&mNode);
        fBase_c *parent = getConnectParent();
        if (parent != nullptr) {
            setExecuteOrder(parent->getExecuteOrder() + 1);
            setDrawOrder(parent->getDrawOrder() + 1);
        }
    }
    fBase_c::postCreate(state);
}

int dMenuBase_c::preDelete() {
    return fBase_c::preDelete() != NOT_READY;
}

void dMenuBase_c::postDelete(MAIN_STATE_e state) {
    if (state == SUCCESS) {
        dMenuMng_c::removeMenu(&mNode);
    }
    fBase_c::postDelete(state);
}

int dMenuBase_c::preExecute() {
    return fBase_c::preExecute() != NOT_READY;
}

void dMenuBase_c::postExecute(MAIN_STATE_e state) {}

int dMenuBase_c::execute() {
    (this->*l_stateFunc[mState])();
    return SUCCEEDED;
}

void dMenuBase_c::setState(u8 state) {
    mState = state;
}

bool dMenuBase_c::isState(u8 state) {
    return state == mState;
}

void dMenuBase_c::changeNextState() {
    setState(mNextState);
}

void dMenuBase_c::setNextState(u8 state) {
    mNextState = state;
}

void dMenuBase_c::startAnm(m2d::Simple_c *layout, const char *paneName, int anm, u8 nextState, BOOL playSe) {
    mpAnmLayout = layout;
    m2d::ResAccIf_c *resAcc = dMenuMng_c::getWinResAcc();
    mAnm.create(getAnmName(anm), resAcc);
    mAnm.mFrameCtrl.mFlags = m2d::FrameCtrl_c::NO_LOOP;
    mAnm.changeTargetName(l_allPaneName, paneName);
    mpAnmLayout->bind(&mAnm);
    mAnm.changeTargetName(paneName, l_allPaneName);
    strcpy(mAnmPaneName, paneName);
    setState(STATE_ANM);
    mNextState = nextState;
    switch (anm) {
    case ANM_WIN_IN:
        if (playSe) {
            playSeSubMenuOpen();
        }
        break;
    case ANM_WIN_OUT:
        if (playSe) {
            playSeSubMenuClose();
        }
        break;
    }
}

int dMenuBase_c::stateAnm() {
    if (!calcAnm()) {
        return SUCCEEDED;
    }
    changeNextState();
    return SUCCEEDED;
}

bool dMenuBase_c::calcAnm() {
    if (mpAnmLayout != nullptr) {
        mAnm.play();
        mpAnmLayout->calc();
        if (!mAnm.mFrameCtrl.isStop()) {
            return false;
        }
        mAnm.changeTargetName(l_allPaneName, mAnmPaneName);
        mpAnmLayout->unbind(&mAnm);
        mAnm.changeTargetName(mAnmPaneName, l_allPaneName);
        mpAnmLayout = nullptr;
    }
    return true;
}

int dMenuBase_c::stateClose() {
    deleteRequest();
    return SUCCEEDED;
}

void dMenuBase_c::onFlag(int bit) {
    mFlags |= 1 << bit;
}

void dMenuBase_c::offFlag(int bit) {
    mFlags &= ~(1 << bit);
}

bool dMenuBase_c::isFlag(int bit) {
    return mFlags & (1 << bit);
}

void dMenuBase_c::setSubState(u8 v) {
    mSubState = v;
}

void dMenuBase_c::setNextSubState(u8 v) {
    mNextSubState = v;
}

void dMenuBase_c::changeNextSubState() {
    mSubState = mNextSubState;
}

void dMenuBase_c::setPostFlags(u32 flags) {
    s_postFlags = flags;
}

u32 dMenuBase_c::getPostFlags() {
    return s_postFlags;
}

void dMenuBase_c::playSeCursor() {
    playSe(SE_cursor);
}

void dMenuBase_c::playSeDecide() {
    playSe(SE_decide_little);
}

void dMenuBase_c::playSeDecide2() {
    playSe(SE_decide_little2);
}

void dMenuBase_c::playSeCancel() {
    playSe(SE_cancel_little);
}

void dMenuBase_c::playSeCancel2() {
    playSe(SE_cancel_little2);
}

void dMenuBase_c::playSeSlideClick() {
    playSe(SE_SlideClick);
}

void dMenuBase_c::playSeAPush() {
    playSe(SE_A_Push);
}

void dMenuBase_c::playSeSubMenuOpen() {
    playSe(SE_DpdSubMenuOpen);
}

void dMenuBase_c::playSeSubMenuClose() {
    playSe(SE_DpdSubMenuClose);
}

void dMenuBase_c::playSeSubMenuPopUp() {
    playSe(SE_DpdSubMenuPopUp);
}

void dMenuBase_c::playSeSubMenuPush() {
    playSe(SE_DpdSubMenuPush);
}

void dMenuBase_c::playSePageTurn() {
    playSe(SE_gamen_okuri);
}

void dMenuBase_c::playSePageTurnNo(int page) {
    playSe(l_pageTurnSe[page]);
}

void dMenuBase_c::playSeItemLift() {
    playSe(SE_item_lift);
}

void dMenuBase_c::playSeItemDrop() {
    playSe(SE_item_drop);
}

void dMenuBase_c::playSeSelectWindowOpen() {
    playSe(SE_select_window_open);
}

void dMenuBase_c::playSeListChoice() {
    playSe(SE_ListChoice);
}

void dMenuBase_c::playSeCanNotLeave() {
    playSe(SE_CanNotLeave);
}

void dMenuBase_c::playSeCanNot() {
    playSe(SE_CanNot);
}

void dMenuMove_c::calcFollow() {
    if (mFollowCnt > 0) {
        mFollowCnt--;
        f32 rate = (f32)mFollowCnt / (f32)(mFollowCnt + 1);
        mOfs *= rate;
    }
    mVec2_c cursor;
    fn_801B3D58(&cursor);
    mPos = mOfs + cursor;
}

void dMenuMove_c::startFollow() {
    mFollowCnt = 5;
}

mVec2_c *dMenuMove_c::getPos() {
    return &mPos;
}

void dMenuMove_c::setOfs(const mVec2_c &ofs) {
    mOfs = ofs;
}

bool dMenuMove_c::isMoveEnd() {
    return mMoveCnt == 0;
}

bool dMenuMove_c::calcMove() {
    if (mMoveCnt > 0) {
        mMoveCnt--;
        mPos = (mPos - mTarget) * ((f32)mMoveCnt / (f32)(mMoveCnt + 1)) + mTarget;
    }
    return mMoveCnt == 0;
}

void dMenuMove_c::startMove(const mVec2_c &target) {
    mTarget = target;
    mMoveCnt = 6;
}

void dMenuMove_c::stop() {
    mFollowCnt = 0;
    mMoveCnt = 0;
}

void dMenuMove_c::setPos(const mVec2_c &pos) {
    mPos = pos;
}

int fn_800CEC1C() {
    return 15;
}
