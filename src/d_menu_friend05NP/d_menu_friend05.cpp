#include <game/game/d_menu_friend05.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_land.hpp>
#include <game/game/d_menu_friend.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_net_friend.hpp>
#include <game/game/d_net_mgr.hpp>
#include <game/game/d_obj_chat.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_obj_voice.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_string.hpp>
#include <game/game/d_wifi.hpp>
#include <game/mLib/m_pad.hpp>
#include <game/snd/snd_id.hpp>
#include <stdio.h>
#include <string.h>

namespace dMenuFriend05 {
// .text:0x5C
BASE_PROFILE(MENU_FRIEND05, dMenuFriend05_c)
} // namespace dMenuFriend05

static const char *l_allPane = "N_all";

// .text:0x180
int dMenuFriend05_c::create() {
    BOOL resLoaded = mRes.load("Layout/friend_visitor.arc");
    BOOL loaded = resLoaded & mSlct.load();
    if (!loaded) {
        return NOT_READY;
    }
    mLyt.build(&mRes, "visitor_win_00.brlyt");
    initAnms();
    mSlct.create();
    setTitle();
    startAnm(&mLyt, l_allPane, ANM_WIN_IN, STATE_MAIN, TRUE);
    initMembers();
    initRowWords();
    refreshAll();
    updateVoices();
    mLyt.calc();
    onFlag(FLAG_DRAW);
    dMenuFriend_c::setTabVisible(TRUE);
    return SUCCEEDED;
}

// .text:0x278
int dMenuFriend05_c::doDelete() {
    if (mRes.remove() == FALSE) {
        return NOT_READY;
    }
    return mSlct.remove() != 0;
}

// .text:0x2CC
int dMenuFriend05_c::stateMain() {
    if (checkCloseRequest()) {
        return SUCCEEDED;
    }
    executeSubState();
    return SUCCEEDED;
}

// .text:0x314
int dMenuFriend05_c::stateAnm() {
    return dMenuBase_c::stateAnm();
}

// .text:0x318
int dMenuFriend05_c::execute() {
    int prevCursor = mCursor;
    int result = dMenuBase_c::execute();
    int cursor = mCursor;
    if (cursor != CURSOR_NONE && prevCursor != cursor && !isActionTab(cursor)) {
        if (isTab(cursor)) {
            playSeSubMenuPopUp();
        } else {
            playSeCursor();
        }
    }
    updateVoice();
    mSlct.execute();
    calcAnms();
    mLyt.calc();
    return result;
}

// .text:0x3EC
int dMenuFriend05_c::stateClose() {
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x410
int dMenuFriend05_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mLyt.entry();
    }
    if (isFlag(FLAG_DIALOG)) {
        mSlct.draw();
    }
    return SUCCEEDED;
}

// .text:0x470
void dMenuFriend05_c::subStateMain() {
    dMenuFriend_c::clearTabFocus();
    setCursor(mCursor, FALSE);
    dMenuFriend_c::hideTutorial();
    int tab = dAcUitag::getTabInput(dAcUitag_c::TAB_FRIEND, FALSE);
    if (tab != CURSOR_NONE && selectActionTab(tab, TRUE)) {
        return;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    mCursor = hitCheck();
    setCursor(mCursor, TRUE);
    if (mCursor != CURSOR_NONE) {
        if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
            push();
            dMenuFriend_c::hideTutorial();
        }
    }
}

// .text:0x53C
void dMenuFriend05_c::subStatePush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0x578
void dMenuFriend05_c::subStateDialogIn() {
    if (mSlct.isWinAnmEnd() && mSlct.checkInput()) {
        setSubState(SUB_DIALOG);
    }
}

// .text:0x5D8
void dMenuFriend05_c::subStateDialog() {
    if (mSlct.isPushEnd()) {
        setSubState(SUB_DIALOG_OUT);
        mSlct.open(1, FALSE, FALSE);
    }
}

// .text:0x63C
void dMenuFriend05_c::subStateDialogOut() {
    if (mSlct.isWinAnmEnd()) {
        offFlag(FLAG_DIALOG);
        switch (mSlct.mPushedBtn) {
        case DIALOG_BTN_YES:
            mFriendIdx = dWifi::findFreeFriend();
            if (mFriendIdx == -1) {
                openNotice(MSG_ROSTER_FULL);
                playSeCanNot();
            } else if (getFriendState(mSel)) {
                openNotice(MSG_ALREADY_REGISTERED);
                playSeCanNot();
            } else {
                startRegister();
            }
            break;
        case DIALOG_BTN_NO:
        case DIALOG_BTN_OK:
        default:
            setSubState(SUB_MAIN);
            break;
        }
    }
}

// .text:0x708
void dMenuFriend05_c::subStateRequestData() {
    dPrivateData_c *player = getPlayer(mSel);
    if (dNetMgr_c::reqMyData(&player->mFriends.mUserData)) {
        setSubState(SUB_WAIT_DATA);
    }
}

// .text:0x760
void dMenuFriend05_c::subStateWaitData() {
    if (dNetMgr_c::sInstance->mReq.takeResultData(
            dNetFriend::NET_FRIEND_REQ_MY_DATA, &mFriendData)) {
        setSubState(SUB_REGISTER);
    }
}

// .text:0x7C0
void dMenuFriend05_c::subStateRegister() {
    if (dWifi::registerFriendReq(mFriendIdx, &mFriendData, &mFriend)) {
        setSubState(SUB_MAIN);
        updateSituations();
        dObjChat::postPreset(5, mMember[mSel]);
    }
}

// .text:0x838
BOOL dMenuFriend05_c::closeMenu() {
    dAcUitag::closeTab();
    startClose();
    return FALSE;
}

// .text:0x870
BOOL dMenuFriend05_c::closeDialog() {
    if (!mSlct.isWinAnmEnd()) {
        return FALSE;
    }
    mSlct.open(1, FALSE, FALSE);
    return closeMenu();
}

// .text:0x8CC
BOOL dMenuFriend05_c::checkCloseRequest() {
    if (!dMenuMng_c::isCloseRequested()) {
        return FALSE;
    }
    typedef BOOL (dMenuFriend05_c::*CloseProc)();
    static CloseProc closeProcs[SUB_NUM] = {&dMenuFriend05_c::closeMenu,
                                            NULL,
                                            &dMenuFriend05_c::closeDialog,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL};
    if (!closeProcs[mSubState]) {
        return FALSE;
    }
    BOOL result = (this->*closeProcs[mSubState])();
    if (result) {
        dMenuMng_c::clearCloseRequest();
    }
    return result;
}

// .text:0x9E8
void dMenuFriend05_c::executeSubState() {
    typedef void (dMenuFriend05_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {
        &dMenuFriend05_c::subStateMain,
        &dMenuFriend05_c::subStatePush,
        &dMenuFriend05_c::subStateDialogIn,
        &dMenuFriend05_c::subStateDialog,
        &dMenuFriend05_c::subStateDialogOut,
        &dMenuFriend05_c::subStateRequestData,
        &dMenuFriend05_c::subStateWaitData,
        &dMenuFriend05_c::subStateRegister,
    };
    (this->*subStateProcs[mSubState])();
}

// .text:0xA20
int dMenuFriend05_c::hitActionTab(const mVec2_c *pos) {
    int tab;
    if (dObjCursor_c::isVisible()) {
        tab = dAcUitag::getTabAt(pos);
    } else {
        tab = CURSOR_NONE;
    }
    dAcUitag::setTabFocus(tab);
    if (tab != CURSOR_NONE) {
        return tab;
    }
    return CURSOR_NONE;
}

// .text:0xA84
BOOL dMenuFriend05_c::selectActionTab(int tab, BOOL wait) {
    if (dAcUitag::selectTab(tab, wait)) {
        startClose();
        return TRUE;
    }
    return FALSE;
}

// .text:0xAD4
void dMenuFriend05_c::startClose() {
    startAnm(&mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, FALSE);
    dMenuFriend_c::requestClose();
}

// .text:0xB10
void dMenuFriend05_c::startReopen() {
    startAnm(&mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, FALSE);
    dMenuFriend_c::requestReopen();
    playSeSubMenuPush();
}

// .text:0xB50
int dMenuFriend05_c::hitCheck() {
    mVec2_c pos = dObjCursor_c::getPos();
    int cursor = hitActionTab(&pos);
    if (cursor != CURSOR_NONE) {
        return cursor;
    }
    if (!dObjCursor_c::isVisible()) {
        return CURSOR_NONE;
    }
    int tab = dMenuFriend_c::getTabAt(&pos);
    if (tab != CURSOR_NONE) {
        return CURSOR_TAB + tab;
    }
    mVec2_c rowPos = pos;
    cursor = m2d::hitButton(&rowPos, &mLyt, CURSOR_ROW, CURSOR_ROW_LAST, -1, -1,
                            1.0f);
    if (cursor != CURSOR_NONE && mMember[cursor - CURSOR_ROW] == MEMBER_NONE) {
        cursor = CURSOR_NONE;
    }
    if (cursor != CURSOR_NONE) {
        return cursor;
    }
    return CURSOR_NONE;
}

// .text:0xC2C
void dMenuFriend05_c::setCursor(int cursor, BOOL select) {
    if (isTab(cursor)) {
        dMenuFriend_c::setTabFocus(cursor - CURSOR_TAB, select);
    }
    if (isRow(cursor)) {
        mRowAnm[cursor - CURSOR_ROW].startEnd(select == FALSE);
    }
}

// .text:0xCD4
void dMenuFriend05_c::push() {
    int cursor = mCursor;
    if (isActionTab(cursor)) {
        selectActionTab(cursor, FALSE);
        return;
    }
    if (isTab(cursor)) {
        if (!dMenuFriend_c::startTabPush(cursor - CURSOR_TAB)) {
            return;
        }
    } else if (isRow(cursor)) {
        mRowAnm[cursor - CURSOR_ROW].start(ROW_ANM_PUSH);
        playSeListChoice();
    } else {
        return;
    }
    setSubState(SUB_PUSH);
}

// .text:0xDC0
BOOL dMenuFriend05_c::isPushEnd() {
    int cursor = mCursor;
    if (isTab(cursor)) {
        return dMenuFriend_c::isTabPushEnd(cursor - CURSOR_TAB);
    }
    if (isRow(cursor)) {
        return mRowAnm[cursor - CURSOR_ROW].isEnd();
    }
    return TRUE;
}

// .text:0xE28
void dMenuFriend05_c::finishPush() {
    int cursor = mCursor;
    if (isTab(cursor)) {
        dMenuFriend_c::setTab(cursor - CURSOR_TAB);
        startReopen();
    } else if (isRow(cursor)) {
        openAddDialog(cursor - CURSOR_ROW);
    } else {
        setSubState(SUB_MAIN);
    }
}

// .text:0xED0
void dMenuFriend05_c::setTitle() {
    mLyt.setMessage("T_title_00", MSG_TITLE, nullptr);
}

// .text:0xEE8
void dMenuFriend05_c::refreshAll() {
    updateSituations();
    updateRowWords();
}

// .text:0xF1C
void dMenuFriend05_c::initRowWords() {
    char nameName[40];
    char townName[40];
    for (int i = 0; i < ROW_NUM; i++) {
        static char townFormat[] = "T_msg_%02d";
        sprintf(townName, townFormat, i);
        mTownWord[i].setTextBox(mLyt.findTextBox(townName));
        static char nameFormat[13] = "T_name_%02d";
        sprintf(nameName, nameFormat, i);
        mNameWord[i].setTextBox(mLyt.findTextBox(nameName));
    }
}

// .text:0xFCC
void dMenuFriend05_c::updateRowWords() {
    for (int i = 0; i < ROW_NUM; i++) {
        char name[40];
        sprintf(name, "B_msg_%02d", i);
        dPrivateData_c *player = getPlayer(i);
        if (player) {
            mTownWord[i].mpTextBox->SetVisible(true);
            mLyt.setVisible(name, true);
            mTownWord[i].clear();
            dLandNameWord_c land;
            land.set(player->mPID.land.mName, 0);
            dString::setWord(0, &land);
            dString::loadString(&mTownWord[i], MSG_WORD0, "sys_2D/SYS_2D_menu");
            mTownWord[i].update();
            mNameWord[i].clear();
            mNameWord[i].set(player->mPID.player.mName, 0);
            mNameWord[i].update();
        } else {
            mTownWord[i].mpTextBox->SetVisible(false);
            mLyt.setVisible(name, false);
        }
    }
}

// .text:0x1110
void dMenuFriend05_c::updateSituations() {
    dPlayerMgr::Mgr_c::getCurrentPlayer();
    for (int i = 0; i < ROW_NUM; i++) {
        char name[40];
        sprintf(name, "situation_%02d", i);
        int visible = FALSE;
        if (getPlayer(i)) {
            int state = getFriendState(i);
            if (state != SITUATION_NONE) {
                visible = TRUE;
                switch (state) {
                case SITUATION_KNOWN:
                    mSituationAnm[i].setFrame(0.0f);
                    break;
                case SITUATION_NO_KEY:
                    mSituationAnm[i].setFrame(1.0f);
                    break;
                case SITUATION_KEY_ONLY:
                    mSituationAnm[i].setFrame(2.0f);
                    break;
                }
            }
        }
        mLyt.setVisible(name, visible != FALSE);
    }
}

// .text:0x1214
void dMenuFriend05_c::updateVoices() {
    for (int i = 0; i < ROW_NUM; i++) {
        char name[40];
        sprintf(name, "voice_%02d", i);
        int member = mMember[i];
        int visible = FALSE;
        if (isFlag(FLAG_VOICE_QUERY) && member != MEMBER_NONE) {
            switch (dObjVoice_c::getStatus(member)) {
            case VOICE_MSG_READY:
                visible = TRUE;
                mVoiceAnm[i].setFrame(1.0f);
                break;
            case VOICE_MSG_NOT_READY:
                break;
            case VOICE_MSG_NONE:
                visible = TRUE;
                mVoiceAnm[i].setFrame(0.0f);
                break;
            }
        }
        mLyt.setVisible(name, visible != FALSE);
    }
}

// .text:0x1314
int dMenuFriend05_c::getMember(int idx) {
    int myNo = dNet::getMyNo();
    for (int i = 0; i < NET_MEMBER_MAX; i++) {
        if (i != myNo && dNet::isMember(i)) {
            if (idx > 0) {
                idx--;
            } else {
                return i;
            }
        }
    }
    return MEMBER_NONE;
}

// .text:0x1398
void dMenuFriend05_c::initMembers() {
    for (int i = 0; i < ROW_NUM; i++) {
        mMember[i] = getMember(i);
    }
}

// .text:0x13F8
dPrivateData_c *dMenuFriend05_c::getPlayer(int row) {
    if (mMember[row] == MEMBER_NONE) {
        return nullptr;
    }
    return dPlayerMgr::Mgr_c::getNetPlayer(mMember[row]);
}

// .text:0x141C
int dMenuFriend05_c::getFriendState(int row) {
    dPrivateData_c *net = getPlayer(row);
    if (net == nullptr) {
        return 0;
    }
    dPrivateData_c *current = dPlayerMgr::Mgr_c::getCurrentPlayer();
    int idx = dWifi::findFriend(&net->mPID);
    if (idx < 0) {
        return 0;
    }
    return current->mFriends.getState(idx);
}

// .text:0x148C
void dMenuFriend05_c::updateVoice() {
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        if (isFlag(FLAG_VOICE_QUERY)) {
            if (dObjVoice_c::isAllReplied()) {
                updateVoices();
                offFlag(FLAG_VOICE_QUERY);
            }
        } else if (mVoiceTimer > 0) {
            mVoiceTimer--;
        } else {
            startVoiceQuery();
        }
    }
}

// .text:0x1524
void dMenuFriend05_c::startVoiceQuery() {
    mVoiceTimer = VOICE_QUERY_INTERVAL;
    dObjVoice_c::queryStatus();
    onFlag(FLAG_VOICE_QUERY);
}

// .text:0x1564
void dMenuFriend05_c::initAnms() {
    static const char *rowAnmNames[ROW_ANM_NUM] = {
        "visitor_win_00_T_msg_In.brlan",
        "visitor_win_00_T_msg_Out.brlan",
        "visitor_win_00_T_msg_Psh.brlan",
    };
    mLyt.setAnm(&mScrlIconAnm, "visitor_win_00_scrlIcon_00.brlan", nullptr);
    for (int i = 0; i < ROW_NUM; i++) {
        mLyt.changeAnm(&mSituationAnm[i],
                       "visitor_win_00_situationIcon_Ptrn.brlan", nullptr);
        m2d::setAnmNumber(&mSituationAnm[i], i);
        mLyt.bind(&mSituationAnm[i]);
        mSituationAnm[i].setFrame(0.0f);
        mLyt.changeAnm(&mVoiceAnm[i], "visitor_win_00_voice_Ptrn.brlan",
                       nullptr);
        m2d::setAnmNumber(&mVoiceAnm[i], i);
        mLyt.bind(&mVoiceAnm[i]);
        mVoiceAnm[i].setFrame(0.0f);
        mRowAnm[i].init(&mLyt, &mRes, rowAnmNames, ROW_ANM_NUM);
        mRowAnm[i].setNumber(i);
        mRowAnm[i].startEnd(ROW_ANM_OUT);
    }
}

// .text:0x167C
void dMenuFriend05_c::calcAnms() {
    for (int i = 0; i < ROW_NUM; i++) {
        mRowAnm[i].play();
    }
    mScrlIconAnm.play();
}

// .text:0x16DC
void dMenuFriend05_c::openAddDialog(int row) {
    mSlct.setBtn(DIALOG_BTN_NO, MSG_NO, nullptr, SE_cancel_little);
    mSlct.setBtn(DIALOG_BTN_YES, MSG_YES, nullptr, 0);
    mSlct.setBtnVisible(DIALOG_BTN_OK, false);
    mSlct.open(0, FALSE, FALSE);
    setSubState(SUB_DIALOG_IN);
    onFlag(FLAG_DIALOG);
    dPrivateData_c *player = getPlayer(row);
    dLandNameWord_c land;
    land.set(player->mPID.land.mName, 0);
    dString::setWord(0, &land);
    dHmnName::Word_c name;
    player->mPID.setWord(&name);
    dString::setWord(1, &name);
    mSlct.setMsg(MSG_ADD_FRIEND, nullptr);
    mSel = row;
}

// .text:0x1810
void dMenuFriend05_c::openNotice(u16 msg) {
    mSlct.setBtnVisible(DIALOG_BTN_NO, false);
    mSlct.setBtnVisible(DIALOG_BTN_YES, false);
    mSlct.setBtn(DIALOG_BTN_OK, MSG_OK, nullptr, 0);
    mSlct.open(0, TRUE, FALSE);
    setSubState(SUB_DIALOG_IN);
    onFlag(FLAG_DIALOG);
    mSlct.setMsg(msg, nullptr);
}

// .text:0x18C4
void dMenuFriend05_c::startRegister() {
    dPrivateData_c *player = getPlayer(mSel);
    mFriend.clear();
    mFriend.mKey = dNet::getWiiNumber(mMember[mSel]);
    mFriend.mPID = player->mPID;
    memcpy(mFriend.mMessage, player->mFriends.mName, sizeof(mFriend.mMessage));
    setSubState(SUB_REQUEST_DATA);
}
