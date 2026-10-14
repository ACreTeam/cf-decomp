// d_a_c_uitag.cpp: the UI tag process (menu tabs, save button, camera message, emotion buttons, photos).
// .text 8017E7B0..801811D0, .rodata 8047B508..8047B528, .data 804F8680..804F8D00, .sdata 8074BE18..8074BE58,
// .sbss 8074E978..8074E980, .sdata2 807513F8..80751428.
// See include/game/game/d_a_c_uitag.hpp. Names other than the class name are inferred.
#include <game/game/d_a_c_uitag.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_nand.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_obj_tutorial.hpp>
#include <game/game/d_obj_voice.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_photo.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_side_wipe_fader.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/mLib/m_fader.hpp>
#include <game/mLib/m_pad.hpp>
#include <lib/revolution/WPAD/WPAD.h>
#include <cstdio>

#define NO_TAB -1

using namespace dAcUitag;

namespace dAcUitag {

// 8017E7B0
CUSTOM_BASE_PROFILE(AC_UITAG, dAcUitag_c, 3, 0x123);

static int l_curTab = NO_TAB; // the selected tab
static int l_menuIdx = -1;    // l_menusA / l_menusB
static dAcUitag_c *l_this;

// The menu of each tab.
static const u8 l_tabMenus[dAcUitag_c::TAB_NUM] = {0x00, 0x0D, 0x03, 0x06, 0x05, 0x38, 0x02, 0x79};

// 8017E8FC
int getTabMenu(int tab) {
    if (dMenuMng_c::isFlag(1)) {
        return 4;
    }
    return l_tabMenus[tab];
}

} // namespace dAcUitag

static const char *l_saveBtnAnms[] = {
    "saveBtn_00_tbBtn_Strt.brlan",   "saveBtn_00_tbBtn_Fnsh.brlan", "saveBtn_00_tbBtn_00_In.brlan",
    "saveBtn_00_tbBtn_00_Out.brlan", "saveBtn_00_tbBtn_00_Psh.brlan",
};

static const char *l_cameraAnms[] = {
    "cameraMsg_00_AlpIn.brlan",
    "cameraMsg_00_AlpOut.brlan",
};

// 8017E944
int dAcUitag_c::create() {
    bool uitagLoaded = mResAcc.load("Layout/uitag.arc");
    bool actionLoaded = mActionResAcc.load("Layout/outdoor_actionBtn.arc");
    if (!(uitagLoaded & actionLoaded)) {
        return NOT_READY;
    }
    mTabLyt.build(&mResAcc, "tab_00.brlyt");
    mSaveBtnLyt.build(&mResAcc, "saveBtn_00.brlyt");
    mSaveBtnAnm.init(&mSaveBtnLyt, &mResAcc, l_saveBtnAnms, 5);
    mSaveBtnAnm.startEnd(1);
    mSaveBtnLyt.calc();
    mCameraLyt.build(&mResAcc, "cameraMsg_00.brlyt");
    mCameraAnm.init(&mCameraLyt, &mResAcc, l_cameraAnms, 2);
    mCameraAnm.startEnd(1);
    mCameraLyt.calc();
    mCameraMsgState = CAMERA_MSG_INIT;
    initAction();
    mFlags = 0;
    for (int i = 0; i < TAB_NUM; i++) {
        mTabBtnKind[i] = TAB_BTN_OUT;
        mTabBtnReq[i] = TAB_BTN_OUT;
    }
    l_curTab = NO_TAB;
    l_this = this;
    mTabLyt.setVisible("Icn_a_07", false);
    mTabLyt.setVisible("Icn_b_07", false);
    setTabBtnAnm(TAB_BTN_IN, 0);
    mTabLyt.calc();
    stopTabBtnAnm(&mTabBtnAnm[0], 0);
    setTabAnm(TAB_ANM_OUT, TRUE);
    mTabAnm.setToEnd();
    mState = STATE_HIDDEN;
    setPhotoState(PHOTO_STATE_IDLE);
    if (dNet::isSessionActive() && !dNet::isHost()) {
        mFlags |= FLAG_NO_SAVE;
    }
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player != NULL && player->isFlag0(0x28)) {
        mFlags |= FLAG_NO_SAVE;
    }
    return SUCCEEDED;
}

// 8017EB80
int dAcUitag_c::doDelete() {
    if (!mResAcc.remove()) {
        return NOT_READY;
    }
    if (!mActionResAcc.remove()) {
        return NOT_READY;
    }
    if (mFlags & FLAG_NAND_WAIT) {
        if (dNand::isIdle()) {
            mFlags &= ~FLAG_NAND_WAIT;
        } else {
            return NOT_READY;
        }
    }
    l_this = NULL;
    return SUCCEEDED;
}

static void (dAcUitag_c::*l_states[])() = {
    &dAcUitag_c::stateHidden,
    &dAcUitag_c::stateIdle,
    &dAcUitag_c::stateTabWait,
    &dAcUitag_c::stateMenuRequest,
    &dAcUitag_c::stateMenu,
    &dAcUitag_c::stateMenuClose,
    &dAcUitag_c::stateOpenMenuA,
    &dAcUitag_c::stateOpenMenuB,
    &dAcUitag_c::stateSave,
    &dAcUitag_c::stateActionRequest,
    &dAcUitag_c::stateEmotion,
    &dAcUitag_c::stateEmotionWait,
};

static void (dAcUitag_c::*l_photoStates[])() = {
    &dAcUitag_c::photoStateIdle,
    &dAcUitag_c::photoStateShutter,
    &dAcUitag_c::photoStateWait,
};

// 8017EC14
int dAcUitag_c::execute() {
    if ((mFlags & FLAG_NAND_WAIT) && dNand::isIdle()) {
        mFlags &= ~FLAG_NAND_WAIT;
    }
    if (mPointerWait > 0) {
        mPointerWait--;
    }
    mFlags &= ~FLAG_NO_A_BUTTON;
    (this->*l_photoStates[mPhotoState])();
    updateCameraMsg();
    if (playTabAnm()) {
        dPad::updateButtons(FALSE);
        calcAnm();
        return SUCCEEDED;
    }
    (this->*l_states[mState])();
    dPad::updateButtons((mFlags & FLAG_NO_A_BUTTON) != 0);
    calcAnm();
    return SUCCEEDED;
}

// 8017ED08
int dAcUitag_c::draw() {
    if (mFlags & FLAG_TAB_CALC) {
        mTabLyt.calc();
        mFlags &= ~FLAG_TAB_CALC;
    }
    if (mFlags & FLAG_SAVE_CALC) {
        mSaveBtnLyt.calc();
        mFlags &= ~FLAG_SAVE_CALC;
    }
    if (mFlags & FLAG_CAMERA_CALC) {
        mCameraLyt.calc();
        mFlags &= ~FLAG_CAMERA_CALC;
    }
    if (mFlags & FLAG_ACTION_CALC) {
        mActionLyt.calc();
        mFlags &= ~FLAG_ACTION_CALC;
    }
    if (mFlags & FLAG_SHOW) {
        mTabLyt.entry();
        if (!(mFlags & FLAG_NO_SAVE)) {
            mSaveBtnLyt.entry();
        }
    }
    mActionLyt.entry();
    mCameraLyt.entry();
    return SUCCEEDED;
}

// 8017EE1C
void dAcUitag_c::calcAnm() {
    for (int i = 0; i < TAB_NUM; i++) {
        m2d::Anm_c *btnAnm = &mTabBtnAnm[i];
        m2d::Anm_c *clrAnm = &mTabClrAnm[i];
        u32 bit = FLAG_TAB_BTN_ANM << i;
        if (mFlags & bit) {
            if (btnAnm->mFrameCtrl.isStop()) {
                if (mTabBtnKind[i] == TAB_BTN_PSH && mTabBtnReq[i] == TAB_BTN_PSH) {
                    mTabBtnKind[i] = TAB_BTN_IN;
                    mTabBtnReq[i] = TAB_BTN_IN;
                }
                stopTabBtnAnm(btnAnm, i);
                mFlags &= ~bit;
            } else {
                btnAnm->play();
            }
            mFlags |= FLAG_TAB_CALC;
        }
        if (!(mFlags & bit) && mTabBtnReq[i] != mTabBtnKind[i]) {
            setTabBtnAnm(mTabBtnReq[i], i);
            mFlags |= FLAG_TAB_CALC;
        }
        bit = FLAG_TAB_CLR_ANM << i;
        if (mFlags & bit) {
            if (clrAnm->mFrameCtrl.isStop()) {
                stopTabClrAnm(clrAnm, i);
                mFlags &= ~bit;
            } else {
                clrAnm->play();
            }
            mFlags |= FLAG_TAB_CALC;
        }
    }
    if (mSaveBtnAnm.play()) {
        mFlags |= FLAG_SAVE_CALC;
    }
    if (mCameraAnm.play()) {
        mFlags |= FLAG_CAMERA_CALC;
    }
    if ((mFlags & FLAG_ACTION_LOCK) && !dDemo::isType(dDemo::TYPE_KEYBOARD)) {
        updateActionIcons(mActionSel, FALSE);
        mFlags &= ~FLAG_ACTION_LOCK;
    }
    bool changed = mActionAnm.play();
    if (!mActionPshAnm.mFrameCtrl.isStop()) {
        mActionPshAnm.play();
        changed = true;
    }
    if (changed) {
        mFlags |= FLAG_ACTION_CALC;
    }
}

// 8017F028
void dAcUitag_c::stateHidden() {
    mFlags &= ~FLAG_SHOW;
    if (checkTabButton()) {
        mFlags |= FLAG_SHOW;
        playTabAnm();
        setTabAnm(TAB_ANM_IN, FALSE);
        hideCameraMsg();
    } else if (isPointerActive()) {
        setTabAnm(TAB_ANM_IN, FALSE);
        hideCameraMsg();
        setSaveBtnAnm(0, TRUE);
        mFlags |= FLAG_SHOW;
        playTabAnm();
        mState = STATE_IDLE;
        setCloseIcon(false);
    }
}

// 8017F0F4
void dAcUitag_c::stateIdle() {
    mSaveBtnAnm.setNext(3);
    int sel = mActionSel;
    if (sel != -1) {
        setActionHighlight(sel, FALSE);
        mActionSel = -1;
    }
    if (checkTabButton()) {
        setSaveBtnAnm(1, FALSE);
        return;
    }
    if (getButtonTab() != NO_TAB) {
        return;
    }
    if (isPointerActive()) {
        int tab = getCursorTab();
        if (tab == NO_TAB || tab == TAB_CLOSE) {
            setTabFocus(NO_TAB);
        } else {
            mFlags |= FLAG_NO_A_BUTTON;
            setTabFocus(tab);
            if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A) && !dDemo::isNoDayChange() && selectTab(tab, FALSE)) {
                setSaveBtnAnm(1, FALSE);
                mState = STATE_MENU_REQUEST;
                mMenuRetry = 10;
            }
            return;
        }
        if (hitSaveBtn() != -1) {
            mFlags |= FLAG_NO_A_BUTTON;
            if (mSaveBtnAnm.setNext(2)) {
                playSePopUp();
            }
            showBtnTutorial(&mSaveBtnLyt, 8);
            if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A) && !dDemo::isNoDayChange()) {
                mSaveBtnAnm.setNext(4);
                mState = STATE_SAVE;
                playSePush();
            }
        } else if (hitAction()) {
            mFlags |= FLAG_NO_A_BUTTON;
            if (mActionSel != sel) {
                playSePopUp();
            }
            showActionTutorial();
            if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
                dDemo::requestUitag();
                mState = STATE_ACTION_REQUEST;
                dObjTutorial_c::hide();
            }
        }
    } else {
        setTabFocus(NO_TAB);
        setTabAnm(TAB_ANM_OUT, FALSE);
        setSaveBtnAnm(1, FALSE);
        mState = STATE_HIDDEN;
        if (l_curTab != NO_TAB) {
            setTabClrAnm(l_curTab, 0);
            l_curTab = NO_TAB;
        }
    }
}

// 8017F378
void dAcUitag_c::stateTabWait() {
    if (mTabWait > 0) {
        mTabWait--;
        int tab = getNextTab(l_curTab);
        if (tab != NO_TAB) {
            if (tab != l_curTab || !(mFlags & FLAG_MENU_WAIT)) {
                selectTab(tab, TRUE);
            }
        }
    } else if (dMenuMng_c::open(getTabMenu(l_curTab), (mFlags & FLAG_MENU_WAIT) == 0)) {
        mFlags &= ~FLAG_MENU_WAIT;
        setCloseIcon(true);
        mState = STATE_MENU;
    }
}

// 8017F42C
void dAcUitag_c::stateMenuRequest() {
    if (dDemo::isType(dDemo::TYPE_ANY) && !dDemo::isType(dDemo::TYPE_MENU)) {
        finishTab();
        return;
    }
    getTabMenu(l_curTab);
    if (dDemo::isType(dDemo::TYPE_MENU)) {
        mState = STATE_TAB_WAIT;
        mFlags |= FLAG_MENU_WAIT;
    } else if (mMenuRetry > 0) {
        mMenuRetry--;
        dDemo::requestMenu(0x79);
    } else {
        finishTab();
    }
}

// 8017F4D8
void dAcUitag_c::stateMenu() {}

// 8017F4DC
void dAcUitag_c::stateMenuClose() {
    if (dMenuMng_c::isFree()) {
        dDemo::finishMenu();
        finishTab();
        setSaveBtnAnm(0, FALSE);
    }
}

static const u8 l_menusA[] = {0x0D, 0x0E, 0x0F, 0x1C, 0x03, 0x04, 0x53, 0x54, 0x00, 0x01, 0x41, 0x42, 0x0A};
static const u8 l_menusB[] = {0x43, 0x2B, 0x5D, 0x08, 0x09, 0x62, 0x64, 0x10, 0x1D, 0x1F, 0x20, 0x3F};

// 8017F52C
void dAcUitag_c::stateOpenMenuA() {
    if (dMenuMng_c::open(l_menusA[l_menuIdx], TRUE)) {
        mState = STATE_MENU;
    }
}

// 8017F57C
void dAcUitag_c::stateOpenMenuB() {
    if (dMenuMng_c::open(l_menusB[l_menuIdx], TRUE)) {
        mState = STATE_HIDDEN;
    }
}

// 8017F5CC
void dAcUitag_c::stateSave() {
    mFlags |= FLAG_NO_A_BUTTON;
    if (mSaveBtnAnm.isEnd()) {
        dDemo::request7();
        mState = STATE_IDLE;
        mSaveBtnAnm.setNext(3);
    }
}

// 8017F628
void dAcUitag_c::stateActionRequest() {
    if (dDemo::isActiveUitag()) {
        setActionPsh(mActionSel);
        mState = STATE_EMOTION;
        stateEmotion();
    } else {
        mState = STATE_IDLE;
        stateIdle();
    }
}

// 8017F68C
void dAcUitag_c::stateEmotion() {
    if (dPlayerMgr::playEmotion(getEmotion(mActionSel))) {
        mState = STATE_EMOTION_WAIT;
    }
}

// 8017F6D4
void dAcUitag_c::stateEmotionWait() {
    if (mActionPshAnm.mFrameCtrl.isStop() && dPlayerMgr::isStartingDemo(4)) {
        dDemo::finishUitag();
        mState = STATE_IDLE;
    }
}

static const char *l_tabAnms[] = {
    "tab_00_tbIn.brlan",
    "tab_00_tbOut.brlan",
};

// 8017F728
void dAcUitag_c::setTabAnm(u8 anm, BOOL silent) {
    if (mTabAnm.isBound()) {
        unbindTabAnm();
    }
    mTabAnm.create(l_tabAnms[anm], &mResAcc);
    mTabAnm.mFrameCtrl.mFlags = m2d::FrameCtrl_c::NO_LOOP;
    mTabLyt.bind(&mTabAnm);
    mTabAnmIdx = anm;
    mFlags |= FLAG_TAB_ANM;
    if (!silent) {
        switch (anm) {
            case TAB_ANM_IN:
                playSe(0x1748);
                break;
            case TAB_ANM_OUT:
                playSe(0x1749);
                break;
        }
    }
}

// 8017F7E8
void dAcUitag_c::unbindTabAnm() {
    mFlags &= ~FLAG_TAB_ANM;
    mTabLyt.unbind(&mTabAnm);
}

// 8017F800
void dAcUitag_c::setTabBtnAnm(u8 kind, int tab) {
    static const char *l_tabBtnAnms[] = {
        "tab_00_dpdIn.brlan",
        "tab_00_dpdOut.brlan",
        "tab_00_dpdPsh.brlan",
    };

    m2d::Anm_c *anm = &mTabBtnAnm[tab];
    mFlags |= FLAG_TAB_BTN_ANM << tab;
    mTabBtnKind[tab] = kind;
    anm->create(l_tabBtnAnms[kind], &mResAcc);
    anm->mFrameCtrl.mFlags = m2d::FrameCtrl_c::NO_LOOP;
    m2d::setAnmNumber(anm, tab);
    mTabLyt.bind(anm);
    if (kind == TAB_BTN_PSH && tab != TAB_CLOSE) {
        setTabClrAnm(tab, 1);
    }
    switch (kind) {
        case TAB_BTN_OUT:
            return;
        case TAB_BTN_IN:
            playSePopUp();
            break;
    }
}

// 8017F8D8
void dAcUitag_c::changeTabBtnAnm(u8 kind, int tab) {
    u32 bit = FLAG_TAB_BTN_ANM << tab;
    if (l_this->mFlags & bit) {
        l_this->stopTabBtnAnm(&l_this->mTabBtnAnm[tab], tab);
        l_this->mFlags &= ~bit;
    }
    l_this->mTabBtnReq[tab] = kind;
    l_this->mTabBtnKind[tab] = kind;
    l_this->setTabBtnAnm(kind, tab);
}

// 8017F974
void dAcUitag_c::stopTabBtnAnm(m2d::Anm_c *anm, int tab) {
    m2d::setAnmNumber(anm, tab);
    mTabLyt.unbind(anm);
}

static const char *l_tabClrAnms[] = {
    "tab_00_tbClrOff_00.brlan", "tab_00_tbClrOff_01.brlan", "tab_00_tbClrOff_02.brlan", "tab_00_tbClrOff_03.brlan",
    "tab_00_tbClrOff_04.brlan", "tab_00_tbClrOff_05.brlan", "tab_00_tbClrOff_06.brlan", "tab_00_tbClrOff_07.brlan",
    "tab_00_tbClrOn_00.brlan",  "tab_00_tbClrOn_01.brlan",  "tab_00_tbClrOn_02.brlan",  "tab_00_tbClrOn_03.brlan",
    "tab_00_tbClrOn_04.brlan",  "tab_00_tbClrOn_05.brlan",  "tab_00_tbClrOn_06.brlan",  "tab_00_tbClrOn_07.brlan",
};

// 8017F9C0
void dAcUitag_c::setTabClrAnm(int tab, int on) {
    m2d::Anm_c *anm = &mTabClrAnm[tab];
    if (anm->isBound()) {
        stopTabClrAnm(anm, tab);
    }
    anm->create(l_tabClrAnms[tab + on * TAB_NUM], &mResAcc);
    anm->mFrameCtrl.mFlags = m2d::FrameCtrl_c::NO_LOOP;
    mTabLyt.bind(anm);
    mFlags |= FLAG_TAB_CLR_ANM << tab;
}

// 8017FA78
void dAcUitag_c::stopTabClrAnm(m2d::Anm_c *anm, int tab) {
    mTabLyt.unbind(anm);
}

// 8017FA80
BOOL dAcUitag_c::playTabAnm() {
    if (!(mFlags & FLAG_TAB_ANM)) {
        return FALSE;
    }
    mTabAnm.mFrameCtrl.isStop();
    if (mTabAnm.mFrameCtrl.isStop()) {
        unbindTabAnm();
    }
    mTabAnm.play();
    mFlags |= FLAG_TAB_CALC;
    return TRUE;
}

// 8017FAF4
BOOL dAcUitag::canOpenTab() {
    if (dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_DEMO)) {
        return FALSE;
    }
    if (dDemo::isType(dDemo::TYPE_KEYBOARD)) {
        return TRUE;
    }
    if (dDemo::isType(dDemo::TYPE_ANY)) {
        return FALSE;
    }
    if (dPlayerMgr::fn_800FFCA8() || dPlayerMgr::fn_800FFAC0() || dPlayerMgr::fn_800FFBE4() ||
        dPlayerMgr::fn_800FFC38() || dPlayerMgr::fn_800FFB18()) {
        return FALSE;
    }
    return dPlayerMgr::canStartMovementDemo() != FALSE;
}

// 8017FBB4
BOOL dAcUitag_c::checkTabButton() {
    if (!canOpenTab()) {
        return FALSE;
    }
    if (dDemo::isNoDayChange()) {
        return FALSE;
    }
    int tab = getButtonTab();
    if (tab == NO_TAB) {
        return FALSE;
    }
    setTabFocus(tab);
    selectTab(tab, TRUE);
    mState = STATE_MENU_REQUEST;
    mMenuRetry = 10;
    return TRUE;
}

// 8017FC4C
int dAcUitag_c::getButtonTab() {
    if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_MINUS)) {
        return 0;
    }
    if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_PLUS)) {
        return 3;
    }
    if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_2)) {
        return 5;
    }
    return NO_TAB;
}

static int l_minusTabs[dAcUitag_c::TAB_NUM] = {0, 0, 1, 4, 6, 2, 5, 7};
static int l_plusTabs[dAcUitag_c::TAB_NUM] = {1, 2, 5, 3, 3, 6, 4, 7};

// 8017FCE0
int dAcUitag::getNextTab(int tab) {
    if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_MINUS)) {
        return l_minusTabs[tab];
    }
    if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_PLUS)) {
        return l_plusTabs[tab];
    }
    return NO_TAB;
}

// 8017FD74
int dAcUitag::getTabInput(int tab, BOOL noClose) {
    int next = getNextTab(tab);
    if (next != NO_TAB) {
        return next;
    }
    if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_2)) {
        return 5;
    }
    if ((mPad::g_currentCore->downTrigger(WPAD_BUTTON_B) || mPad::g_currentCore->downTrigger(WPAD_BUTTON_FS_Z)) &&
        !noClose) {
        return dAcUitag_c::TAB_CLOSE;
    }
    return NO_TAB;
}

// 8017FE28
BOOL dAcUitag::isPointerInScreen() {
    if (!dObjCursor_c::isVisible()) {
        return FALSE;
    }
    f32 maxX = 1.05f;
    f32 maxY = 1.075f;
    mVec2_c pos = dPad::getRawPointer();
    if (pos.x < -maxX || pos.x > maxX || pos.y < -maxY || pos.y > maxY) {
        return FALSE;
    }
    return TRUE;
}

// 8017FEC4
BOOL dAcUitag::isPointerActive() {
    if (!isPointerInScreen()) {
        l_this->mPointerWait = 0;
    }
    if (!canOpenTab()) {
        l_this->mPointerWait = 45;
        return FALSE;
    }
    if (mPad::g_currentCore->down(WPAD_BUTTON_B)) {
        if (l_this->mPointerWait > 0) {
            l_this->mPointerWait--;
        }
        return FALSE;
    }
    if (l_this->mPointerWait > 0) {
        return FALSE;
    }
    return isPointerInScreen();
}

// 8017FF74
bool dAcUitag::isBottomPanelShown() {
    if (l_this != NULL) {
        return l_this->mTabAnmIdx == dAcUitag_c::TAB_ANM_IN;
    }
    return false;
}

// 8017FF98
int dAcUitag::getTabAt(const mVec2_c *pos) {
    if (pos->y < -258.0f) {
        return NO_TAB;
    }
    struct {
        mVec2_c pos;
        f32 scale;
        nw4r::lyt::Pane *pane;
    } hit;
    hit.pane = NULL;
    hit.scale = 1.0f;
    hit.pos = *pos;
    l_this->mTabLyt.patrolPane(m2d::isInPane, NULL, &hit);
    if (hit.pane != NULL) {
        return m2d::getNameNumber(hit.pane->GetName(), 2);
    }
    return NO_TAB;
}

// 80180024
void dAcUitag::setTabFocus(int tab) {
    for (int i = 0; i < dAcUitag_c::TAB_NUM; i++) {
        l_this->mTabBtnReq[i] = dAcUitag_c::TAB_BTN_OUT;
    }
    if (tab != NO_TAB) {
        l_this->mTabBtnReq[tab] = dAcUitag_c::TAB_BTN_IN;
        showTabTutorial(tab);
    } else {
        dObjTutorial_c::hide();
    }
}

// The menu closes: back to the tabs.
static inline void closeMenu() {
    l_this->mState = dAcUitag_c::STATE_MENU_CLOSE;
    l_this->mFlags &= ~dAcUitag_c::FLAG_HIDE_AFTER_MENU;
    lbl_8074E9B0->_1A8 = lbl_8074E9B0->_1A4;
    dMenuBase_c::playSeSubMenuClose();
}

// 80180088
BOOL dAcUitag::selectTab(int tab, BOOL delay) {
    if (l_this->mTabBtnKind[tab] != dAcUitag_c::TAB_BTN_IN && !delay) {
        return FALSE;
    }
    m2d::Anm_c *anm = &l_this->mTabClrAnm[tab];
    if (anm->isBound()) {
        l_this->stopTabClrAnm(anm, tab);
        l_this->mFlags &= ~(dAcUitag_c::FLAG_TAB_CLR_ANM << tab);
    }
    l_this->changeTabBtnAnm(dAcUitag_c::TAB_BTN_PSH, tab);
    if (l_curTab != NO_TAB && tab != l_curTab) {
        l_this->setTabClrAnm(l_curTab, 0);
        l_this->mTabBtnReq[l_curTab] = dAcUitag_c::TAB_BTN_OUT;
    }
    if (tab == dAcUitag_c::TAB_CLOSE || tab == l_curTab) {
        closeMenu();
    } else {
        l_this->mState = dAcUitag_c::STATE_TAB_WAIT;
        playSePush();
        if (delay) {
            l_this->mTabWait = 30;
        } else {
            l_this->mTabWait = 0;
        }
    }
    l_curTab = tab;
    dObjTutorial_c::hide();
    return TRUE;
}

// 801801E4
void dAcUitag::openMenuA(int idx) {
    l_menuIdx = idx;
    l_this->mState = dAcUitag_c::STATE_OPEN_MENU_A;
}

// 801801F8
BOOL dAcUitag::openMenuB(int idx) {
    if (l_this->mState != dAcUitag_c::STATE_HIDDEN) {
        return FALSE;
    }
    l_menuIdx = idx;
    l_this->mState = dAcUitag_c::STATE_OPEN_MENU_B;
    fn_8019A7A4();
    return TRUE;
}

// 80180240
BOOL dAcUitag::closeTab() {
    if (l_curTab != NO_TAB) {
        l_this->setTabClrAnm(l_curTab, 0);
        l_curTab = NO_TAB;
    }
    closeMenu();
    return TRUE;
}

// 801802AC
void dAcUitag::setHideAfterMenu() {
    l_this->mFlags |= dAcUitag_c::FLAG_HIDE_AFTER_MENU;
}

// 801802C0
void dAcUitag::hideTab() {
    l_this->setTabAnm(dAcUitag_c::TAB_ANM_OUT, FALSE);
}

// 801802D0
void dAcUitag::showTab() {
    l_this->setTabAnm(dAcUitag_c::TAB_ANM_IN, FALSE);
}

// 801802E0
m2d::ResAccLoader_c *dAcUitag::getActionResAcc() {
    return &l_this->mActionResAcc;
}

// 801802EC
void dAcUitag::setNandWait() {
    l_this->mFlags |= dAcUitag_c::FLAG_NAND_WAIT;
}

// 80180300
int dAcUitag_c::getCursorTab() {
    mVec2_c pos = dObjCursor_c::getPos();
    return getTabAt(&pos);
}

// 8018032C
void dAcUitag_c::setCloseIcon(bool on) {
    if (on == isFlag(FLAG_CLOSE_ICON)) {
        return;
    }
    if (on) {
        mFlags |= FLAG_CLOSE_ICON;
    } else {
        mFlags &= ~FLAG_CLOSE_ICON;
    }
    setTabClrAnm(TAB_CLOSE, on);
    mTabLyt.setVisible("Icn_a_07", on);
    mTabLyt.setVisible("Icn_b_07", on);
}

// 801803C4
void dAcUitag_c::finishTab() {
    if (l_curTab != NO_TAB) {
        setTabClrAnm(l_curTab, 0);
        l_curTab = NO_TAB;
    }
    if (mFlags & FLAG_HIDE_AFTER_MENU) {
        mFlags &= ~FLAG_HIDE_AFTER_MENU;
        mState = STATE_HIDDEN;
    } else {
        mState = STATE_IDLE;
    }
    setCloseIcon(false);
}

// 8018043C
void dAcUitag_c::setSaveBtnAnm(int anm, BOOL force) {
    mSaveBtnAnm.startEnd(3);
    mSaveBtnLyt.calc();
    if (force) {
        mSaveBtnAnm.setNext(anm);
        if (anm <= 1) {
            mActionAnm.setNext(anm);
        }
    } else {
        mSaveBtnAnm.setNext(anm);
        if (anm <= 1) {
            mActionAnm.setNext(anm);
        }
    }
    if (!(mFlags & FLAG_NO_SAVE)) {
        if (anm == 1) {
            dObjVoice_c::setAnmFinish();
        } else if (anm == 0) {
            dObjVoice_c::setAnmStart();
        }
    }
    if (anm == 0 && !(mFlags & FLAG_ACTION_LOCK)) {
        updateActionIcons(-1, FALSE);
    }
}

// 80180534
int dAcUitag_c::hitSaveBtn() {
    if (mFlags & FLAG_NO_SAVE) {
        return -1;
    }
    mVec2_c cursor = dObjCursor_c::getPos();
    mVec2_c pos = cursor;
    return m2d::hitButton(&pos, &mSaveBtnLyt, 0, 0, -1, -1, 1.0f);
}

// 801805A8
void dAcUitag_c::setPhotoState(int state) {
    mPhotoState = state;
}

// 801805B0
void dAcUitag_c::photoStateIdle() {
    if (!isPhotoBlocked() && mPad::g_currentCore->downTrigger(WPAD_BUTTON_1) && dPhoto::getMode() == dPhoto::MODE_NONE) {
        dPhoto::requestCapture();
        setPhotoState(PHOTO_STATE_SHUTTER);
    }
}

// 80180620
void dAcUitag_c::photoStateShutter() {
    if (dPhoto::getMode() == dPhoto::MODE_NONE && startShutterFadeOut()) {
        playSe(0x174E);
        setPhotoState(PHOTO_STATE_WAIT);
    }
}

// 80180674
void dAcUitag_c::photoStateWait() {
    if (isShutterFaderHidden()) {
        setPhotoState(PHOTO_STATE_IDLE);
    }
}

// 801806B4
BOOL dAcUitag::isPhotoBlocked() {
    BOOL fading = mFader_c::isStatus(mFaderBase_c::FADE_IN) || mFader_c::isStatus(mFaderBase_c::FADE_OUT);
    if (fading) {
        return TRUE;
    }
    if (dDemo::isType(dDemo::TYPE_ANY) && !dDemo::isTalking() && !dDemo::isType(dDemo::TYPE_EVENT) &&
        !dDemo::isType(dDemo::TYPE_MENU) && !dDemo::isType(dDemo::TYPE_UITAG) && !dDemo::isType(dDemo::TYPE_KEYBOARD) &&
        !dDemo::isType(dDemo::TYPE_9)) {
        return TRUE;
    }
    if (dMenuMng_c::isFlag(2)) {
        return TRUE;
    }
    return dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_DEMO) != FALSE;
}

// 801807D8
BOOL dAcUitag::showTabTutorial(int tab) {
    if (dObjTutorial_c::request(0, tab + 1, TRUE)) {
        u16 msg = tab + 1;
        if (tab == dAcUitag_c::TAB_CLOSE) {
            msg = 11;
        }
        char name[0x28];
        sprintf(name, "Icn_a_%02d", tab);
        nw4r::lyt::Pane *pane = l_this->mTabLyt.findPane(name);
        mVec3_c screen;
        m2d::getScreenPos(&screen, pane);
        mVec2_c pos(screen.x, 10.0f + screen.y);
        dObjTutorial_c::setMsg(&pos, msg, 1);
        return TRUE;
    }
    return FALSE;
}

// 8018089C
void dAcUitag::showBtnTutorial(m2d::SimpleEx_c *lyt, u16 msg) {
    if (dObjTutorial_c::request(0, msg, TRUE)) {
        nw4r::lyt::Pane *pane = lyt->findPane("tbBtnIcn_00");
        mVec3_c screen;
        m2d::getScreenPos(&screen, pane);
        mVec2_c pos;
        pos.x = screen.x;
        pos.y = screen.y;
        dObjTutorial_c::setMsg(&pos, msg, 0);
    }
}

// 80180920
void dAcUitag_c::showActionTutorial() {
    int sel = mActionSel;
    u8 emotion = getEmotion(sel);
    if (emotion != 0xFF) {
        u16 msg = emotion + 50;
        if (dObjTutorial_c::request(0, msg, TRUE)) {
            char name[0x30];
            sprintf(name, "actnIcn_%02d", sel);
            nw4r::lyt::Pane *pane = mActionLyt.findPane(name);
            mVec3_c screen;
            m2d::getScreenPos(&screen, pane);
            mVec2_c pos;
            pos.x = screen.x;
            pos.y = screen.y;
            dObjTutorial_c::setMsg(&pos, msg, 0);
        }
    }
}

// 801809E0
void dAcUitag_c::updateCameraMsg() {
    switch (mCameraMsgState) {
        case CAMERA_MSG_INIT:
            if (!dDemo::isType(dDemo::TYPE_ANY)) {
                if (dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_FTR_ROOM)) {
                    mCameraMsgState = CAMERA_MSG_SHOWN;
                    mCameraMsgTimer = 300;
                    mCameraAnm.setNext(0);
                } else {
                    mCameraMsgState = CAMERA_MSG_DONE;
                }
            }
            break;
        case CAMERA_MSG_SHOWN:
            if (mCameraMsgTimer > 0) {
                mCameraMsgTimer--;
            } else {
                mCameraAnm.setNext(1);
                mCameraMsgState = CAMERA_MSG_DONE;
            }
            break;
    }
}

// 80180AA4
void dAcUitag_c::hideCameraMsg() {
    if (mCameraMsgState == CAMERA_MSG_INIT) {
        mCameraMsgState = CAMERA_MSG_DONE;
    } else if (mCameraMsgState == CAMERA_MSG_SHOWN) {
        mCameraMsgTimer = 0;
    }
}

// 80180AD0
void dAcUitag::hideCameraMsg() {
    if (l_this != NULL) {
        l_this->hideCameraMsg();
    }
}

static const char *l_actionAnms[] = {
    "outdoor_actionBtn_00_actnBtn_Strt.brlan",
    "outdoor_actionBtn_00_actnBtn_Fnsh.brlan",
};

static const char *l_actionIconAnms[] = {
    "outdoor_actionBtn_00_actnIcnPattern.brlan",
    NULL,
};

static const char *l_actionColorAnms[] = {
    "outdoor_actionBtn_00_color_dpd.brlan",
    "outdoor_actionBtn_00_color_keyboard.brlan",
};

// 80180AE4
void dAcUitag_c::initAction() {
    mActionLyt.build(&mActionResAcc, "outdoor_actionBtn_00.brlyt");
    mActionAnm.init(&mActionLyt, &mActionResAcc, l_actionAnms, 2);
    mActionAnm.startEnd(1);
    mActionLyt.setAnm(&mActionPshAnm, "outdoor_actionBtn_00_actnBtn_Psh.brlan", NULL);
    mActionPshAnm.setToEnd();
    mActionLyt.calc();
    mActionSel = -1;
    for (int i = 0; i < ACTION_NUM; i++) {
        mActionIconAnm[i].init(&mActionLyt, &mActionResAcc, l_actionIconAnms, 1);
        mActionIconAnm[i].setNumber(i);
        mActionIconAnm[i].start(0);
        mActionColorAnm[i].init(&mActionLyt, &mActionResAcc, l_actionColorAnms, 2);
        mActionColorAnm[i].setNumber(i);
    }
    updateActionIcons(-1, FALSE);
}

// 80180C1C
void dAcUitag_c::updateActionIcons(int sel, BOOL anim) {
    for (int i = 0; i < ACTION_NUM; i++) {
        u8 emotion = getEmotion(i);
        char offName[0x28];
        char onName[0x28];
        sprintf(offName, "N_actnBtnOff_%02d", i);
        sprintf(onName, "N_actnBtn_%02d", i);
        if (emotion == 0xFF) {
            mActionLyt.setVisible(offName, false);
            mActionLyt.setVisible(onName, false);
        } else {
            if (sel == i) {
                mActionLyt.setVisible(onName, true);
                mActionLyt.setVisible(offName, false);
                if (anim) {
                    mActionColorAnm[i].startEnd(1);
                }
            } else {
                mActionLyt.setVisible(onName, false);
                mActionLyt.setVisible(offName, true);
            }
            mActionIconAnm[i].setFrame(emotion - 1);
        }
        if (sel == -1 || !anim) {
            mActionColorAnm[i].startEnd(0);
        }
    }
    mFlags |= FLAG_ACTION_CALC;
}

// 80180DA8
void dAcUitag_c::setActionHighlight(int idx, BOOL on) {
    if (!(mFlags & FLAG_ACTION_LOCK)) {
        char name[0x28];
        sprintf(name, "N_actnBtnOff_%02d", idx);
        mActionLyt.setVisible(name, !on);
        sprintf(name, "N_actnBtn_%02d", idx);
        mActionLyt.setVisible(name, on);
    }
}

// 80180E50
BOOL dAcUitag_c::hitAction() {
    mVec2_c cursor = dObjCursor_c::getPos();
    mVec2_c pos = cursor;
    mActionSel = m2d::hitButton(&pos, &mActionLyt, 0, 3, -1, -1, 1.0f);
    if (mActionSel == -1) {
        return FALSE;
    }
    setActionHighlight(mActionSel, TRUE);
    return TRUE;
}

// 80180ED8
u8 dAcUitag_c::getEmotion(int idx) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player == NULL) {
        return 0xFF;
    }
    if (player->mEmotions.isValid(idx) == FALSE) {
        return 0xFF;
    }
    return player->mEmotions.get(idx);
}

// 80180F50
void dAcUitag_c::setActionPsh(int idx) {
    mActionPshAnm.setFrame(0.0f);
    char name[0x30];
    sprintf(name, "N_actnBtn_%02d", idx);
    mActionLyt.setPosFrom(name, "actnBtn_Psh", 0.0f, 0.0f);
}

// 80180FC4
void dAcUitag::startEmotion(int slot) {
    if (l_this != NULL) {
        l_this->mFlags |= dAcUitag_c::FLAG_ACTION_LOCK;
        l_this->updateActionIcons(slot, TRUE);
        l_this->setActionPsh(slot);
    }
}

// 80181020
void dAcUitag::playSePush() {
    dMenuBase_c::playSeSubMenuPush();
}

// 80181024
void dAcUitag::playSePopUp() {
    if (!dDemo::isType(dDemo::TYPE_ANY) || dDemo::isType(dDemo::TYPE_MENU)) {
        dMenuBase_c::playSeSubMenuPopUp();
    }
}

// 80181064
dAcUitag_c::~dAcUitag_c() {}
