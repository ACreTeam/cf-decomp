#include <game/game/d_menu_friend07.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_menu_friend.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_net_mgr.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_obj_voice.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_voice_chat.hpp>
#include <game/mLib/m_pad.hpp>
#include <game/snd/snd_id.hpp>

namespace dMenuFriend07 {
// .text:0x5C
BASE_PROFILE(MENU_FRIEND07, dMenuFriend07_c)
} // namespace dMenuFriend07

static const char *l_allPane = "N_all";

// .text:0x168
int dMenuFriend07_c::create() {
    BOOL loaded = mRes.load("Layout/chat_voice.arc");
    loaded &= mSlct.load();
    if (!loaded) {
        return NOT_READY;
    }
    mSlct.create();
    mLyt.build(&mRes, "chat_voice_win_00.brlyt");
    initMessages();
    initAnms();
    initState();
    startAnm(&mLyt, l_allPane, ANM_WIN_IN, STATE_MAIN, TRUE);
    mLyt.calc();
    onFlag(FLAG_DRAW);
    dMenuFriend_c::setTabVisible(TRUE);
    return SUCCEEDED;
}

// .text:0x248
int dMenuFriend07_c::doDelete() {
    if (mRes.remove() == FALSE) {
        return NOT_READY;
    }
    return mSlct.remove() != 0;
}

// .text:0x29C
int dMenuFriend07_c::stateMain() {
    if (checkCloseRequest()) {
        return SUCCEEDED;
    }
    executeSubState();
    return SUCCEEDED;
}

// .text:0x2E4
int dMenuFriend07_c::stateAnm() {
    return dMenuBase_c::stateAnm();
}

// .text:0x2E8
int dMenuFriend07_c::execute() {
    int prevCursor = mCursor;
    int result = dMenuBase_c::execute();
    if (mCursor != CURSOR_NONE && prevCursor != mCursor) {
        if (isTab(mCursor)) {
        } else if (isFriendTab(mCursor)) {
            playSeSubMenuPopUp();
        } else {
            playSeCursor();
        }
    }
    mSlct.execute();
    playAnms();
    mLyt.calc();
    return result;
}

// .text:0x3B4
int dMenuFriend07_c::stateClose() {
    deleteRequest();
    return SUCCEEDED;
}

// .text:0x3D8
int dMenuFriend07_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mLyt.entry();
    }
    if (isFlag(FLAG_DIALOG)) {
        mSlct.draw();
    }
    return SUCCEEDED;
}

// .text:0x438
void dMenuFriend07_c::subStateMain() {
    dMenuFriend_c::clearTabFocus();
    setSelect(mCursor, FALSE);
    dMenuFriend_c::hideTutorial();
    int tab = dAcUitag::getTabInput(dAcUitag_c::TAB_FRIEND, FALSE);
    if (tab == -1 || !selectTab(tab, TRUE)) {
        mVec2_c pos = dObjCursor_c::getPos();
        mCursor = hitCheck();
        setSelect(mCursor, TRUE);
        if (mCursor != CURSOR_NONE) {
            if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
                push();
                dMenuFriend_c::hideTutorial();
            }
        }
    }
}

// .text:0x504
void dMenuFriend07_c::subStatePush() {
    if (isPushEnd()) {
        finishPush();
    }
}

// .text:0x540
void dMenuFriend07_c::subStateDialog() {
    if (mSlct.isWinAnmEnd() && mSlct.checkInput()) {
        setSubState(SUB_DIALOG_PUSH);
    }
}

// .text:0x5A0
void dMenuFriend07_c::subStateDialogPush() {
    if (mSlct.isPushEnd()) {
        setSubState(SUB_DIALOG_CLOSE);
        mSlct.open(dMenuSlctB_c::WIN_ANM_OUT, FALSE, FALSE);
    }
}

// .text:0x604
void dMenuFriend07_c::subStateDialogClose() {
    if (mSlct.isWinAnmEnd()) {
        offFlag(FLAG_DIALOG);
        setSubState(SUB_MAIN);
    }
}

// .text:0x660
BOOL dMenuFriend07_c::closeMain() {
    dAcUitag::closeTab();
    startClose();
    return FALSE;
}

// .text:0x698
BOOL dMenuFriend07_c::closeDialog() {
    if (!mSlct.isWinAnmEnd()) {
        return FALSE;
    }
    mSlct.open(dMenuSlctB_c::WIN_ANM_OUT, FALSE, FALSE);
    return closeMain();
}

// .text:0x6F4
BOOL dMenuFriend07_c::checkCloseRequest() {
    if (!dMenuMng_c::isCloseRequested()) {
        return FALSE;
    }
    static CloseProc closeProcs[SUB_NUM] = {
        &dMenuFriend07_c::closeMain, nullptr, &dMenuFriend07_c::closeDialog,
        nullptr, nullptr};
    if (!closeProcs[mSubState]) {
        return FALSE;
    }
    BOOL result = (this->*closeProcs[mSubState])();
    if (result) {
        dMenuMng_c::clearCloseRequest();
    }
    return result;
}

// .text:0x7EC
void dMenuFriend07_c::executeSubState() {
    static SubStateProc subStateProcs[SUB_NUM] = {
        &dMenuFriend07_c::subStateMain,
        &dMenuFriend07_c::subStatePush,
        &dMenuFriend07_c::subStateDialog,
        &dMenuFriend07_c::subStateDialogPush,
        &dMenuFriend07_c::subStateDialogClose,
    };
    (this->*subStateProcs[mSubState])();
}

// .text:0x824
int dMenuFriend07_c::getTabAt(const mVec2_c *pos) {
    int tab;
    if (dObjCursor_c::isVisible()) {
        tab = dAcUitag::getTabAt(pos);
    } else {
        tab = -1;
    }
    dAcUitag::setTabFocus(tab);
    if (tab == -1) {
        return -1;
    }
    return tab;
}

// .text:0x888
BOOL dMenuFriend07_c::selectTab(int tab, BOOL delay) {
    if (dAcUitag::selectTab(tab, delay)) {
        startClose();
        return TRUE;
    }
    return FALSE;
}

// .text:0x8D8
void dMenuFriend07_c::startClose() {
    startAnm(&mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, FALSE);
    dMenuFriend_c::requestClose();
}

// .text:0x914
void dMenuFriend07_c::startReopen() {
    startAnm(&mLyt, l_allPane, ANM_WIN_OUT, STATE_CLOSE, FALSE);
    dMenuFriend_c::requestReopen();
    playSeSubMenuPush();
}

// .text:0x954
int dMenuFriend07_c::hitCheck() {
    mVec2_c pos = dObjCursor_c::getPos();
    int tab = getTabAt(&pos);
    if (tab != -1) {
        return tab;
    }
    if (!dObjCursor_c::isVisible()) {
        return -1;
    }
    tab = dMenuFriend_c::getTabAt(&pos);
    if (tab != -1) {
        return tab + CURSOR_FRIEND_TAB;
    }
    mVec2_c hitPos = pos;
    int hit = m2d::hitButton(&hitPos, &mLyt, CURSOR_BTN, CURSOR_BTN_LAST, -1,
                             -1, 1.0f);
    if (isVoiceChatOff() && (hit == CURSOR_VOL_DOWN || hit == CURSOR_VOL_UP ||
                             hit == CURSOR_USE || hit == CURSOR_NOT_USE)) {
        hit = -1;
    }
    if (hit != -1) {
        return hit;
    }
    return -1;
}

// .text:0xA3C
void dMenuFriend07_c::setSelect(int cursor, BOOL select) {
    if (isFriendTab(cursor)) {
        dMenuFriend_c::setTabFocus(cursor - CURSOR_FRIEND_TAB, select);
    }
    if (isBtn(cursor)) {
        mBtnAnm[cursor - CURSOR_BTN].setNext(select == FALSE);
    }
}

// .text:0xAE4
void dMenuFriend07_c::push() {
    if (isTab(mCursor)) {
        selectTab(mCursor, FALSE);
        return;
    }
    if (isFriendTab(mCursor)) {
        if (!dMenuFriend_c::startTabPush(mCursor - CURSOR_FRIEND_TAB)) {
            return;
        }
    } else if (isBtn(mCursor)) {
        switch (mCursor) {
        case CURSOR_ON:
            if (!isVoiceChatOff()) {
                return;
            }
            mSwitchAnm[0].setNext(SWITCH_ANM_PUSH);
            break;
        case CURSOR_OFF:
            if (isVoiceChatOff()) {
                return;
            }
            mSwitchAnm[1].setNext(SWITCH_ANM_PUSH);
            break;
        case CURSOR_VOL_DOWN:
            if (isVoiceChatOff() || getVolume() <= VOLUME_MIN) {
                return;
            }
            mVolAnm[0].start(VOL_ANM_PUSH);
            break;
        case CURSOR_VOL_UP:
            if (isVoiceChatOff() || getVolume() >= VOLUME_MAX) {
                return;
            }
            mVolAnm[1].start(VOL_ANM_PUSH);
            break;
        case CURSOR_USE:
            if (isHeadphoneUsed()) {
                return;
            }
            mUseAnm[0].setNext(USE_ANM_PUSH);
            break;
        case CURSOR_NOT_USE:
            if (!isHeadphoneUsed()) {
                return;
            }
            mUseAnm[1].setNext(USE_ANM_PUSH);
            break;
        default:
            return;
        }
    } else {
        return;
    }
    setSubState(SUB_PUSH);
}

// .text:0xCD8
BOOL dMenuFriend07_c::isPushEnd() {
    if (isFriendTab(mCursor)) {
        return dMenuFriend_c::isTabPushEnd(mCursor - CURSOR_FRIEND_TAB);
    }
    if (mCursor == CURSOR_ON) {
        return mSwitchAnm[0].mFrameCtrl.isStop();
    }
    if (mCursor == CURSOR_OFF) {
        return mSwitchAnm[1].mFrameCtrl.isStop();
    }
    if (mCursor == CURSOR_VOL_DOWN) {
        return mVolAnm[0].mFrameCtrl.isStop();
    }
    if (mCursor == CURSOR_VOL_UP) {
        return mVolAnm[1].mFrameCtrl.isStop();
    }
    if (mCursor == CURSOR_USE) {
        return mUseAnm[0].mFrameCtrl.isStop();
    }
    if (mCursor == CURSOR_NOT_USE) {
        return mUseAnm[1].mFrameCtrl.isStop();
    }
    return TRUE;
}

// .text:0xD6C
void dMenuFriend07_c::finishPush() {
    if (isFriendTab(mCursor)) {
        dMenuFriend_c::setTab(mCursor - CURSOR_FRIEND_TAB);
        startReopen();
    } else if (mCursor == CURSOR_ON) {
        setVoiceChatOff(FALSE);
        playSeSwitchOn();
        if (dNet::isSessionActive() && dNet::getMemberNum() > 1 &&
            dVoiceChat::isMicConnected()) {
            openDialog(MSG_MIC_CHAT_START);
        } else {
            setSubState(SUB_MAIN);
        }
    } else if (mCursor == CURSOR_OFF) {
        setVoiceChatOff(TRUE);
        setSubState(SUB_MAIN);
        playSeSwitchOff();
    } else if (mCursor == CURSOR_VOL_UP) {
        changeVolume(1);
        setSubState(SUB_MAIN);
    } else if (mCursor == CURSOR_VOL_DOWN) {
        changeVolume(-1);
        setSubState(SUB_MAIN);
    } else if (mCursor == CURSOR_USE) {
        setHeadphoneUsed(TRUE);
        playSeSwitchOn();
        openDialog(MSG_HEADPHONES_USE);
    } else if (mCursor == CURSOR_NOT_USE) {
        setHeadphoneUsed(FALSE);
        playSeSwitchOff();
        openDialog(MSG_HEADPHONES_DONT_USE);
    } else {
        setSubState(SUB_MAIN);
    }
}

// .text:0xF40
void dMenuFriend07_c::initMessages() {
    mLyt.setMessage("T_title_00", MSG_MIC_CHAT, nullptr);
    mLyt.setMessage("T_btnA_00", MSG_ON, nullptr);
    mLyt.setMessage("T_btnA_01", MSG_OFF, nullptr);
    mLyt.setMessage("T_title_01", MSG_VOLUME, nullptr);
    mLyt.setMessage("T_titleFrmAll", MSG_MIC_CHAT_SETTINGS, nullptr);
    mLyt.setMessage("T_title_02", MSG_HEADPHONES, nullptr);
    mLyt.setMessage("T_btnC_00", MSG_USE, nullptr);
    mLyt.setMessage("T_btnC_01", MSG_DONT_USE, nullptr);
}

// .text:0x1018
void dMenuFriend07_c::initState() {
    updateAnms(FALSE);
    updateVolumeGray();
}

// .text:0x1050
void dMenuFriend07_c::initAnms() {
    static const char *btnAnmNamesA[] = {
        "chat_voice_win_00_btnA_In.brlan",
        "chat_voice_win_00_btnA_Out.brlan",
    };
    static const char *btnAnmNamesB[] = {
        "chat_voice_win_00_btnB_In.brlan",
        "chat_voice_win_00_btnB_Out.brlan",
    };
    static const char *btnAnmNamesC[] = {
        "chat_voice_win_00_btnC_In.brlan",
        "chat_voice_win_00_btnC_Out.brlan",
    };
    static const char *switchAnmNames[] = {
        "chat_voice_win_00_btnA_Psh.brlan",
        "chat_voice_win_00_btnA_white_00.brlan",
        "chat_voice_win_00_btnA_normal_00.brlan",
    };
    static const char *volAnmNames[] = {
        "chat_voice_win_00_btnB_Psh.brlan",
        "chat_voice_win_00_btnB_gray_00.brlan",
        "chat_voice_win_00_btnB_gryYelw_00.brlan",
    };
    static const char *useAnmNames[] = {
        "chat_voice_win_00_btnC_Psh.brlan",
        "chat_voice_win_00_btnC_white_00.brlan",
        "chat_voice_win_00_btnC_normal_00.brlan",
        "chat_voice_win_00_btnC_gray_00.brlan",
        "chat_voice_win_00_btnC_gryYelw_00.brlan",
    };
    static const char *titleAnmNames[] = {
        "chat_voice_win_00_T_ttl01_gry.brlan",
        "chat_voice_win_00_T_ttl01_white.brlan",
    };
    mLyt.setAnm(&mIconAnm, "chat_voice_win_00_scrlIcon_00.brlan", nullptr);
    mBtnAnm[0].init(&mLyt, &mRes, btnAnmNamesA, 2);
    mBtnAnm[0].setNumber(0);
    mBtnAnm[1].init(&mLyt, &mRes, btnAnmNamesA, 2);
    mBtnAnm[1].setNumber(1);
    mBtnAnm[2].init(&mLyt, &mRes, btnAnmNamesB, 2);
    mBtnAnm[2].setNumber(0);
    mBtnAnm[3].init(&mLyt, &mRes, btnAnmNamesB, 2);
    mBtnAnm[3].setNumber(1);
    mBtnAnm[4].init(&mLyt, &mRes, btnAnmNamesC, 2);
    mBtnAnm[4].setNumber(0);
    mBtnAnm[5].init(&mLyt, &mRes, btnAnmNamesC, 2);
    mBtnAnm[5].setNumber(1);
    for (int i = 0; i < 6; i++) {
        mBtnAnm[i].setNext(BTN_ANM_OUT);
    }
    mSwitchAnm[0].init(&mLyt, &mRes, switchAnmNames, 3);
    mSwitchAnm[0].setNumber(0);
    mSwitchAnm[1].init(&mLyt, &mRes, switchAnmNames, 3);
    mSwitchAnm[1].setNumber(1);
    mVolAnm[0].init(&mLyt, &mRes, volAnmNames, 3);
    mVolAnm[0].setNumber(0);
    mVolAnm[1].init(&mLyt, &mRes, volAnmNames, 3);
    mVolAnm[1].setNumber(1);
    mUseAnm[0].init(&mLyt, &mRes, useAnmNames, 5);
    mUseAnm[0].setNumber(0);
    mUseAnm[1].init(&mLyt, &mRes, useAnmNames, 5);
    mUseAnm[1].setNumber(1);
    mLyt.setAnm(&mVolGrayAnm, "chat_voice_win_00_vol_gray_00.brlan", nullptr);
    mVolTitleAnm.init(&mLyt, &mRes, titleAnmNames, 2);
    mVolTitleAnm.setNumber(1);
    mUseTitleAnm.init(&mLyt, &mRes, titleAnmNames, 2);
    mUseTitleAnm.setNumber(2);
}

// .text:0x12DC
void dMenuFriend07_c::playAnms() {
    for (int i = 0; i < 6; i++) {
        mBtnAnm[i].play();
    }
    mSwitchAnm[0].play();
    mSwitchAnm[1].play();
    mVolAnm[0].play();
    mVolAnm[1].play();
    mUseAnm[0].play();
    mUseAnm[1].play();
    mIconAnm.play();
}

// .text:0x136C
BOOL dMenuFriend07_c::isVoiceChatOff() {
    return dSaveOption::getVoiceChat() == TRUE;
}

// .text:0x1398
void dMenuFriend07_c::setVoiceChatOff(BOOL off) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (off) {
        dSaveOption::setVoiceChat(TRUE);
        player->getOption()->mVoiceChat = TRUE;
        dNetMgr_c::setVoiceOn();
        if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
            dObjVoice_c::sendOff();
        }
    } else {
        dSaveOption::setVoiceChat(FALSE);
        player->getOption()->mVoiceChat = FALSE;
        dNetMgr_c::setVoiceOff();
        if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
            dObjVoice_c::sendOn();
        }
    }
    updateAnms(FALSE);
    updateVolumeGray();
}

// .text:0x1464
BOOL dMenuFriend07_c::isHeadphoneUsed() {
    return dSaveOption::getVoiceSound() == FALSE;
}

// .text:0x148C
void dMenuFriend07_c::setHeadphoneUsed(BOOL used) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (used) {
        dSaveOption::setVoiceSound(FALSE);
        player->getOption()->mVoiceSound = FALSE;
        dNetMgr_c::mute();
    } else {
        dSaveOption::setVoiceSound(TRUE);
        player->getOption()->mVoiceSound = TRUE;
        dNetMgr_c::unmute();
    }
    updateAnms(TRUE);
    updateVolumeGray();
}

// .text:0x1520
void dMenuFriend07_c::updateAnms(BOOL headphone) {
    mSwitchAnm[0].setNext(isVoiceChatOff() ? SWITCH_ANM_WHITE
                                           : SWITCH_ANM_NORMAL);
    mSwitchAnm[1].setNext(!isVoiceChatOff() ? SWITCH_ANM_WHITE
                                            : SWITCH_ANM_NORMAL);
    mVolAnm[0].start(isVoiceChatOff() ? VOL_ANM_GRAY : VOL_ANM_YELLOW);
    mVolAnm[1].start(isVoiceChatOff() ? VOL_ANM_GRAY : VOL_ANM_YELLOW);
    mVolTitleAnm.startEnd(!isVoiceChatOff());
    mUseTitleAnm.startEnd(!isVoiceChatOff());
    if (!headphone) {
        mUseAnm[0].startEnd(isVoiceChatOff() ? USE_ANM_GRAY : USE_ANM_YELLOW);
        mUseAnm[1].startEnd(isVoiceChatOff() ? USE_ANM_GRAY : USE_ANM_YELLOW);
        mLyt.calc();
        if (!isVoiceChatOff()) {
            mUseAnm[0].startEnd(!isHeadphoneUsed() ? USE_ANM_WHITE
                                                   : USE_ANM_NORMAL);
            mUseAnm[1].startEnd(isHeadphoneUsed() ? USE_ANM_WHITE
                                                  : USE_ANM_NORMAL);
        }
    } else {
        mUseAnm[0].setNext(!isHeadphoneUsed() ? USE_ANM_WHITE : USE_ANM_NORMAL);
        mUseAnm[1].setNext(isHeadphoneUsed() ? USE_ANM_WHITE : USE_ANM_NORMAL);
    }
}

// .text:0x16F0
u8 dMenuFriend07_c::getVolume() {
    return dSaveOption::getPlayerVoiceVolume();
}

// .text:0x16F4
void dMenuFriend07_c::setVolume(u8 volume) {
    dSaveOption::setPlayerVoiceVolume(volume);
    dNetMgr_c::setVolume(volume);
    updateVolumeGray();
}

// .text:0x1740
void dMenuFriend07_c::changeVolume(int delta) {
    int volume = getVolume() + delta;
    if (volume < VOLUME_MIN) {
        volume = VOLUME_MIN;
    }
    if (volume > VOLUME_MAX) {
        volume = VOLUME_MAX;
    }
    setVolume(volume);
    playSeVolume();
}

// .text:0x17AC
void dMenuFriend07_c::updateVolumeGray() {
    f32 frame = getVolume();
    if (isVoiceChatOff()) {
        frame = 0.0f;
    }
    mVolGrayAnm.setFrame(frame);
}

// .text:0x182C
void dMenuFriend07_c::playSeSwitchOn() {
    playSe(SE_SwitchOn);
}

// .text:0x1834
void dMenuFriend07_c::playSeSwitchOff() {
    playSe(SE_SwitchOff);
}

// .text:0x183C
void dMenuFriend07_c::playSeVolume() {
    static const int volumeSes[VOLUME_MAX] = {
        SE_VoiceChatVolume1, SE_VoiceChatVolume2, SE_VoiceChatVolume3,
        SE_VoiceChatVolume4, SE_VoiceChatVolume5, SE_VoiceChatVolume6,
        SE_VoiceChatVolume7, SE_VoiceChatVolume8, SE_VoiceChatVolume9,
        SE_VoiceChatVolume10};
    int idx = getVolume();
    if (idx > 0) {
        idx--;
    } else if (idx >= VOLUME_MAX) {
        idx = VOLUME_MAX - 1;
    }
    playSe(volumeSes[idx]);
}

// .text:0x188C
void dMenuFriend07_c::openDialog(u16 msg) {
    mSlct.setBtnVisible(0, false);
    mSlct.setBtnVisible(1, false);
    mSlct.setBtn(2, MSG_OK, nullptr, 0);
    mSlct.setMsg(msg, nullptr);
    mSlct.fitLines(0);
    mSlct.open(dMenuSlctB_c::WIN_ANM_IN, FALSE, FALSE);
    setSubState(SUB_DIALOG);
    onFlag(FLAG_DIALOG);
}
