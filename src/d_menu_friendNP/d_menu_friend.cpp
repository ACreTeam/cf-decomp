#include <game/game/d_menu_friend.hpp>
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_nand.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_net_mgr.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_tutorial.hpp>
#include <game/game/d_obj_voice.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_wifi.hpp>
#include <stdio.h>
#include <string.h>

namespace dMenuFriend {
fProf::fBaseProfile_c g_profile_MENU_FRIEND = {classInit, fProf::MENU_FRIEND,
                                               fProf::DRAW_ORDER::MENU_FRIEND};

// .text:0x5C
void *classInit() {
    return new dMenuFriend_c;
}
} // namespace dMenuFriend

static const char *l_pushPaneName = "topTbBase_Psh";
static const char *l_focusPaneName = "topTbFrm_Fc";

dMenuFriend_c *dMenuFriend_c::sInstance;

// .text:0x138
int dMenuFriend_c::create() {
    bool tabLoaded = mTabRes.load("Layout/friend_tab.arc");
    bool frameLoaded = mFrameRes.load("Layout/friend_frame.arc");
    if (!(tabLoaded & frameLoaded)) {
        return NOT_READY;
    }
    mLytTab.build(&mTabRes, "friend_WiFi_tab_00.brlyt");
    mLytTab.mPriority = LYT_PRIORITY;
    mLytFrame.build(&mFrameRes, "my_friendCord_frame_00.brlyt");
    mLytFrame.mPriority = LYT_PRIORITY;
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        if (dObjVoice_c::isActive()) {
            mTab = TAB_MIC;
        } else {
            mTab = TAB_VISIT;
        }
    } else {
        mTab = TAB_ROSTER;
    }
    setTabAnm(TAB_ANM_IN);
    sInstance = this;
    initTabAnm();
    initFrameAnm();
    mLytTab.calc();
    mLytFrame.calc();
    setState(STATE_MAIN);
    setSubState(SUB_OPEN_TAB);
    mFocus = NO_FOCUS;
    mLytTab.setAlpha(l_focusPaneName, 0xFF);
    mLytTab.setVisible(l_focusPaneName, false);
    return SUCCEEDED;
}

// .text:0x2B0
int dMenuFriend_c::doDelete() {
    if (!mTabRes.remove()) {
        return NOT_READY;
    }
    if (!mFrameRes.remove()) {
        return NOT_READY;
    }
    sInstance = nullptr;
    return SUCCEEDED;
}

// .text:0x318
int dMenuFriend_c::stateMain() {
    preSubState();
    executeSubState();
    postSubState();
    return SUCCEEDED;
}

// .text:0x358
int dMenuFriend_c::stateAnm() {
    return dMenuBase_c::stateAnm();
}

// .text:0x35C
int dMenuFriend_c::execute() {
    return dMenuBase_c::execute();
}

// .text:0x360
int dMenuFriend_c::stateClose() {
    if (dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag0(
            PRIVATE_FLAG0_TRAVELLING)) {
        offFlag(FLAG_SAVE);
    }
    if (isFlag(FLAG_SAVE)) {
        if (dNand::getType(lbl_8074B010) != 1) {
            offFlag(FLAG_SAVE);
        } else if (dNand::saveFriendList()) {
            offFlag(FLAG_SAVE);
            dAcUitag::setNandWait();
        } else {
            return SUCCEEDED;
        }
    }
    if (mNode.getNext() == nullptr) {
        setState(STATE_CLOSE);
    }
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x42C
int dMenuFriend_c::draw() {
    if (isFlag(FLAG_TAB)) {
        calcTab();
        if (isFlag(FLAG_TAB_CALC)) {
            offFlag(FLAG_TAB_CALC);
            mLytTab.calc();
        }
        mLytTab.entry();
    }
    if (isFlag(FLAG_FRAME)) {
        calcFrame();
        if (isFlag(FLAG_FRAME_CALC)) {
            offFlag(FLAG_FRAME_CALC);
            mLytFrame.calc();
        }
        mLytFrame.entry();
    }
    return SUCCEEDED;
}

// .text:0x504
void dMenuFriend_c::subStateIdle() {
}

// .text:0x508
void dMenuFriend_c::subStateOpenTab() {
    static const u8 tabMenu[TAB_NUM] = {
        dMenuMng_c::MENU_FRIEND04, dMenuMng_c::MENU_FRIEND06,
        dMenuMng_c::MENU_FRIEND05, dMenuMng_c::MENU_FRIEND07};
    if (dMenuMng_c::openSub(tabMenu[mTab])) {
        setSubState(SUB_IDLE);
    }
}

// .text:0x564
void dMenuFriend_c::subStateWaitReopen() {
    if (mNode.getNext() != nullptr) {
        return;
    }
    setSubState(SUB_OPEN_TAB);
}

// .text:0x588
void dMenuFriend_c::subStateOpenMenu() {
    static const u8 openMenu[OPEN_MENU_NUM] = {
        dMenuMng_c::MENU_INPUT_NEW_FRIEND_NAME,
        dMenuMng_c::MENU_INPUT_NEW_FRIEND_TOWN,
        dMenuMng_c::MENU_INPUT_FRIEND_NAME,
        dMenuMng_c::MENU_INPUT_FRIEND_TOWN,
        dMenuMng_c::MENU_INPUT_NEW_FRIEND_COMMENT,
        dMenuMng_c::MENU_INPUT_FRIEND_COMMENT,
        dMenuMng_c::MENU_INPUT_MY_FRIEND_NAME,
        dMenuMng_c::MENU_FRIEND_CODE_ADD,
        dMenuMng_c::MENU_FRIEND_CODE_EDIT};
    if (dMenuMng_c::openSub(openMenu[mNextMenu])) {
        setSubState(SUB_IDLE);
    }
}

// .text:0x5E4
void dMenuFriend_c::subStateWaitMenu() {
    if (mNode.getNext() == nullptr) {
        offFlag(FLAG_TAB);
        setSubState(SUB_OPEN_MENU);
    }
}

// .text:0x638
void dMenuFriend_c::subStateWaitClose() {
    if (mNode.getNext() != nullptr) {
        return;
    }
    setState(STATE_CLOSE);
}

// .text:0x650
void dMenuFriend_c::executeSubState() {
    static SubStateProc procs[SUB_NUM] = {
        &dMenuFriend_c::subStateIdle,       &dMenuFriend_c::subStateOpenTab,
        &dMenuFriend_c::subStateWaitReopen, &dMenuFriend_c::subStateWaitClose,
        &dMenuFriend_c::subStateOpenMenu,   &dMenuFriend_c::subStateWaitMenu};
    (this->*procs[mSubState])();
}

// Both are empty in the retail build.
// .text:0x688
void dMenuFriend_c::preSubState() {
}

// .text:0x68C
void dMenuFriend_c::postSubState() {
}

// .text:0x690
void dMenuFriend_c::setTabAnm(int anm) {
    static const char *names[] = {
        "friend_WiFi_tab_00_topTbIn.brlan",
        "friend_WiFi_tab_00_topTbOut.brlan",
    };
    mLytTab.setAnm(&mTabAnm, names[anm], nullptr);
}

// .text:0x6B0
void dMenuFriend_c::initTabAnm() {
    static const char *names[] = {
        "friend_WiFi_tab_00_topTbOn.brlan",
        "friend_WiFi_tab_00_topTbOff.brlan",
    };
    for (int i = 0; i < TAB_NUM; i++) {
        mTabBtnAnm[i].init(&mLytTab, &mTabRes, names, 2);
        mTabBtnAnm[i].setNumber(i);
    }
    mTabBtnAnm[mTab].start(TAB_BTN_ANM_ON);
    mTabBtnAnm[mTab].setToEnd();
    mLytTab.setAnm(&mTabPushAnm, "friend_WiFi_tab_00_topTbPsh.brlan", nullptr);
    mTabPushAnm.setToEnd();
}

// .text:0x77C
void dMenuFriend_c::calcTab() {
    int changed = 0;
    for (int i = 0; i < TAB_NUM; i++) {
        changed |= mTabBtnAnm[i].play();
    }
    if (changed) {
        char name[0x28];
        sprintf(name, "N_topTb_%02d", mTab);
        mLytTab.setPosFrom(name, l_focusPaneName, 0.0f, 9.0f);
    }
    if (!mTabAnm.mFrameCtrl.isStop()) {
        mTabAnm.play();
        changed = 1;
    }
    if (!mTabPushAnm.mFrameCtrl.isStop()) {
        mTabPushAnm.play();
        changed = 1;
    }
    if (changed) {
        onFlag(FLAG_TAB_CALC);
    }
}

// .text:0x874
void dMenuFriend_c::initFrameAnm() {
    static const char *names[] = {
        "my_friendCord_frame_00_textBox_In.brlan",
        "my_friendCord_frame_00_textBox_Out.brlan",
    };
    mFrameAnm.init(&mLytFrame, &mFrameRes, names, 2);
    mLytFrame.setAnm(&mFramePushAnm, "my_friendCord_frame_00_textBox_Psh.brlan",
                     nullptr);
    mFramePushAnm.setToEnd();
    initFrameText();
}

// .text:0x8DC
void dMenuFriend_c::calcFrame() {
    bool changed = mFrameAnm.play();
    if (!mFramePushAnm.mFrameCtrl.isStop()) {
        mFramePushAnm.play();
        changed = true;
    }
    if (isFlag(FLAG_FRAME_ANM)) {
        if (mFrameMoveAnm.mFrameCtrl.isStop()) {
            if (isFlag(FLAG_FRAME_ANM_END)) {
                offFlag(FLAG_FRAME_ANM_END);
                offFlag(FLAG_FRAME_ANM);
                onFlag(FLAG_FRAME_ANM_DONE);
                mLytFrame.unbind(&mFrameMoveAnm);
                mFrameMoveAnm.remove();
            }
        } else {
            mFrameMoveAnm.play();
            changed = true;
        }
    }
    if (changed) {
        onFlag(FLAG_FRAME_CALC);
    }
}

// .text:0x9C8
BOOL dMenuFriend_c::requestClose() {
    if (sInstance->mSubState != SUB_IDLE) {
        return FALSE;
    }
    sInstance->setSubState(SUB_WAIT_CLOSE);
    sInstance->setTabAnm(TAB_ANM_OUT);
    return TRUE;
}

// .text:0xA2C
BOOL dMenuFriend_c::requestReopen() {
    if (sInstance->mSubState != SUB_IDLE) {
        return FALSE;
    }
    sInstance->setSubState(SUB_WAIT_REOPEN);
    return TRUE;
}

// .text:0xA7C
BOOL dMenuFriend_c::requestOpenMenu(int idx) {
    if (sInstance->mSubState != SUB_IDLE) {
        return FALSE;
    }
    sInstance->setSubState(SUB_WAIT_MENU);
    if (sInstance->isFlag(FLAG_TAB)) {
        dAcUitag::hideTab();
        sInstance->setTabAnm(TAB_ANM_OUT);
    }
    sInstance->mNextMenu = idx;
    return TRUE;
}

// .text:0xB10
BOOL dMenuFriend_c::requestShowTabs() {
    if (sInstance->mSubState != SUB_IDLE) {
        return FALSE;
    }
    dAcUitag::showTab();
    sInstance->setSubState(SUB_WAIT_REOPEN);
    sInstance->setTabAnm(TAB_ANM_IN);
    return TRUE;
}

// .text:0xB7C
void dMenuFriend_c::setTabVisible(BOOL visible) {
    if (visible) {
        sInstance->onFlag(FLAG_TAB);
    } else {
        sInstance->offFlag(FLAG_TAB);
    }
}

// .text:0xBA4
void dMenuFriend_c::setTabFocus(int tab, BOOL focus) {
    static const u16 tabMsg[TAB_NUM] = {
        dObjTutorial_c::MSG_FRIEND_ROSTER, dObjTutorial_c::MSG_FRIEND_CODE,
        dObjTutorial_c::MSG_FRIEND_VISIT, dObjTutorial_c::MSG_MIC_CHAT};
    if (focus) {
        if (sInstance->mFocus == NO_FOCUS) {
            sInstance->mLytTab.setVisible(l_focusPaneName, true);
        }
        sInstance->mFocus = tab;
        char name[0x28];
        // The target keeps a second copy of this format string in .data.
        static char paneFmt[] = "N_topTb_%02d";
        sprintf(name, paneFmt, tab);
        sInstance->mLytTab.setPosFrom(name, l_focusPaneName, 0.0f, 9.0f);
        sprintf(name, "topTbIcn_%02d", tab);
        showTutorial(tabMsg[tab], sInstance->mLytTab.findPane(name), 0);
    } else if (sInstance->mFocus == tab) {
        sInstance->mLytTab.setVisible(l_focusPaneName, false);
        sInstance->mFocus = NO_FOCUS;
    }
    sInstance->onFlag(FLAG_TAB_CALC);
}

// .text:0xCDC
void dMenuFriend_c::clearTabFocus() {
    for (int i = 0; i < TAB_NUM; i++) {
        setTabFocus(i, FALSE);
    }
}

// The unused default-scale locals in these two helpers are inferred. MWCC
// retains their literal-address register after removing the unused loads.
// See notes/TUs/d_menu_friend.txt for the compiler trace.
// .text:0xD1C
int dMenuFriend_c::getTabAt(const mVec2_c *pos) {
    f32 scale = 1.0f;
    mVec2_c p = *pos;
    return m2d::hitButton(&p, &sInstance->mLytTab, 0, TAB_NUM - 1, -1, -1,
                          1.0f);
}

// .text:0xD74
BOOL dMenuFriend_c::isFrameHit(const mVec2_c *pos) {
    f32 scale = 1.0f;
    mVec2_c p = *pos;
    return m2d::hitButton(&p, &sInstance->mLytFrame, 0, 0, -1, -1, 1.0f) != -1;
}

// .text:0xDDC
BOOL dMenuFriend_c::startTabPush(int tab) {
    if (tab == sInstance->mTab) {
        return FALSE;
    }
    sInstance->mTabPushAnm.setFrame(0.0f);
    char name[0x28];
    sprintf(name, "N_topTb_%02d", tab);
    sInstance->mLytTab.setPosFrom(name, l_pushPaneName, 0.0f, 9.0f);
    return TRUE;
}

// .text:0xE80
BOOL dMenuFriend_c::isTabPushEnd(int tab) {
    return sInstance->mTabPushAnm.mFrameCtrl.isStop();
}

// .text:0xE90
void dMenuFriend_c::setTab(int tab) {
    sInstance->mTabBtnAnm[sInstance->mTab].setNext(TAB_BTN_ANM_OFF);
    sInstance->mTabBtnAnm[tab].setNext(TAB_BTN_ANM_ON);
    sInstance->mTab = tab;
}

// .text:0xF00
void dMenuFriend_c::placeFrameTextBox(m2d::SimpleEx_c *lyt,
                                      const char *paneName) {
    if (sInstance->isFlag(FLAG_FRAME_ANM) ||
        sInstance->isFlag(FLAG_FRAME_ANM_DONE)) {
        return;
    }
    lyt->copyPos(&sInstance->mLytFrame, paneName, "N_textBox");
    sInstance->mLytFrame.addPos("N_textBox", 0.0f, -79.0f);
    sInstance->onFlag(FLAG_FRAME_CALC);
}

// .text:0xFBC
void dMenuFriend_c::setFrameVisible(BOOL visible) {
    if (sInstance->isFlag(FLAG_FRAME_ANM) ||
        sInstance->isFlag(FLAG_FRAME_ANM_DONE)) {
        return;
    }
    if (visible) {
        sInstance->onFlag(FLAG_FRAME);
    } else {
        sInstance->offFlag(FLAG_FRAME);
    }
}

// .text:0x1040
void dMenuFriend_c::clearEntry() {
    memset(&sInstance->mFriendData, 0, sizeof(DWCFriendData));
    sInstance->mFriend.clear();
}

// .text:0x1088
void dMenuFriend_c::setEntryPlayerName(const wchar_t *name) {
    sInstance->mFriend.mPID.setPlayerName(name);
}

// .text:0x109C
void dMenuFriend_c::setEntryTown(const wchar_t *townName) {
    dLandID_c *land = &sInstance->mFriend.mPID.land;
    land->setRandomId(townName, getLanguage());
}

// .text:0x10EC
BOOL dMenuFriend_c::setEntryKey(u64 key) {
    DWCFriendData data;
    dNetMgr_c::makeFriendData(key, &data);
    int idx = dWifi::findFriendData(&data);
    if (idx != -1 && sInstance->mFriendIdx != idx) {
        return FALSE;
    }
    memcpy(&sInstance->mFriendData, &data, sizeof(DWCFriendData));
    return TRUE;
}

// .text:0x1158
void dMenuFriend_c::setEntryFriendData(const DWCFriendData *data) {
    memcpy(&sInstance->mFriendData, data, sizeof(DWCFriendData));
}

// .text:0x1170
u64 dMenuFriend_c::getFriendKey() {
    return dWifi::getFriendKey(sInstance->mFriendIdx);
}

// .text:0x1180
void dMenuFriend_c::setEntryMessage(const wchar_t *message) {
    memcpy(sInstance->mFriend.mMessage, message,
           sizeof(sInstance->mFriend.mMessage));
    if (message[0] != 0) {
        sInstance->mFriend.mWc24Flag.mHasMessage = 1;
    }
}

// .text:0x11E4
void dMenuFriend_c::setEntryFromNetPlayer(int member) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getNetPlayer(member);
    sInstance->mFriend.clear();
    dMenuFriend_c *self = sInstance;
    self->mFriend.mKey = dNet::getWiiNumber(member);
    self->mFriend.mPID = player->mPID;
    memcpy(self->mFriend.mMessage, player->mFriends.mName,
           sizeof(self->mFriend.mMessage));
}

// .text:0x1308
void dMenuFriend_c::setFriendIdx(int idx) {
    sInstance->mFriendIdx = idx;
}

// .text:0x1318
int dMenuFriend_c::getFriendIdx() {
    return sInstance->mFriendIdx;
}

// .text:0x1328
BOOL dMenuFriend_c::registerEntry() {
    return dWifi::registerFriendReq(
        sInstance->mFriendIdx, &sInstance->mFriendData, &sInstance->mFriend);
}

// .text:0x1340
void dMenuFriend_c::loadFriend(int idx) {
    sInstance->mFriendIdx = idx;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    sInstance->mFriend.copy(player->mFriends.getFriend(idx));
    memcpy(&sInstance->mFriendData, player->mFriends.getKey(idx),
           sizeof(DWCFriendData));
}

// .text:0x13C0
void dMenuFriend_c::startTextBoxUp() {
    sInstance->mLytFrame.setAnm(&sInstance->mFrameMoveAnm,
                                "my_friendCord_frame_00_textBoxUp.brlan",
                                nullptr);
    sInstance->onFlag(FLAG_FRAME_ANM);
    sInstance->mFrameAnm.setNext(FRAME_ANM_OUT);
}

// .text:0x1420
void dMenuFriend_c::startTextBoxDown() {
    sInstance->mLytFrame.setAnm(&sInstance->mFrameMoveAnm,
                                "my_friendCord_frame_00_textBoxDown.brlan",
                                nullptr);
    sInstance->onFlag(FLAG_FRAME_ANM);
    sInstance->onFlag(FLAG_FRAME_ANM_END);
}

// .text:0x147C
void dMenuFriend_c::startUpperWinOut() {
    sInstance->mLytFrame.setAnm(&sInstance->mFrameMoveAnm,
                                "my_friendCord_frame_00_upperWin_Out.brlan",
                                nullptr);
    sInstance->onFlag(FLAG_FRAME_ANM);
    requestClose();
}

// .text:0x14D0
BOOL dMenuFriend_c::isFrameAnmDone() {
    if (sInstance->isFlag(FLAG_FRAME_ANM_DONE)) {
        sInstance->offFlag(FLAG_FRAME_ANM_DONE);
        return TRUE;
    }
    return FALSE;
}

// .text:0x1524
void dMenuFriend_c::showTextBox(BOOL show) {
    if (show) {
        sInstance->mFrameAnm.setNext(FRAME_ANM_IN);
        showTutorial(dObjTutorial_c::MSG_FRIEND_COMMENT,
                     sInstance->mLytFrame.findPane("cmntIcn_00"), 1);
    } else {
        sInstance->mFrameAnm.setNext(FRAME_ANM_OUT);
    }
}

// .text:0x15A0
void dMenuFriend_c::resetTextBoxPush() {
    sInstance->mFramePushAnm.setFrame(0.0f);
}

// .text:0x15B8
BOOL dMenuFriend_c::isTextBoxPushEnd() {
    return sInstance->mFramePushAnm.mFrameCtrl.isStop();
}

// .text:0x15C8
nw4r::lyt::TextBox *dMenuFriend_c::getTextBox() {
    return sInstance->mLytFrame.findTextBox("T_textBox_00");
}

// .text:0x15E0
nw4r::lyt::Pane *dMenuFriend_c::getTextBoxWindow() {
    return sInstance->mLytFrame.findPane("W_textBox_00");
}

// .text:0x15F8
void dMenuFriend_c::initFrameText() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    m2d::stringWord_c word;
    word.set(player->mFriends.mName, 0);
    sInstance->mLytFrame.setWord("T_textBox_00", &word);
}

// .text:0x1664
void dMenuFriend_c::setSave() {
    sInstance->onFlag(FLAG_SAVE);
}

// .text:0x1674
void dMenuFriend_c::showTutorial(u16 msg, nw4r::lyt::Pane *pane, int tail) {
    if (dObjTutorial_c::request(0, msg, TRUE)) {
        mVec3_c pos;
        m2d::getScreenPos(&pos, pane);
        mVec2_c pos2(static_cast<const float *>(pos));
        dObjTutorial_c::setMsg(&pos2, msg, tail);
    }
}

// .text:0x16F8
void dMenuFriend_c::hideTutorial() {
    dObjTutorial_c::hide();
}
