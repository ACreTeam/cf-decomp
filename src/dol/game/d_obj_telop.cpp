#include <game/game/d_obj_telop.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_obj_voice.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_reset.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_string.hpp>
#include <game/mLib/m_pad.hpp>
#include <game/snd/snd_id.hpp>
#include <lib/revolution/SC/scapi.h>
#include <lib/revolution/WPAD/WPAD.h>

extern "C" BOOL fn_8038E36C();

CUSTOM_BASE_PROFILE(OBJ_TELOP, dObjTelop_c, 0x127, 0x129);

static WPADInfo l_wpadInfo;
static dObjTelop_c::Req_c l_req;
static s32 l_wpadResult;

// SYS_2D_menu message of each dTelopMsg_e.
static const u16 l_msgIds[TELOP_MSG_NUM] = {
    0x000, // TELOP_NONE
    0x0CF, // TELOP_TIMER_START
    0x0D0, // TELOP_TIMER_STOP
    0x0D1, // TELOP_TIMER_END
    0x11E, // TELOP_HIDE_AND_SEEK_START
    0x11F, // TELOP_HIDE_AND_SEEK_END
    0x0F1, // TELOP_NET_ARRIVE
    0x0F2, // TELOP_NET_LEAVE
    0x0F3, // TELOP_NET_SAVE
    0x0F4, // TELOP_NET_CLOSE
    0x0F7, // TELOP_NET_CANCEL
    0x0F5, // TELOP_NET_STOP_TALK
    0x0F6, // TELOP_NET_CLOSE_WINDOW
    0x12A, // TELOP_NO_REMOTE
    0x0EF, // TELOP_LOW_BATTERY
    0x0AE, // TELOP_EXTENSION
    0x160, // TELOP_MIC_CHAT_START
    0x161, // TELOP_MIC_CHAT_UNAVAILABLE
};

inline bool isNetMsg(int msg) {
    return msg >= TELOP_NET_ARRIVE && msg <= TELOP_NET_CANCEL;
}

inline bool isNetWarnMsg(int msg) {
    return msg >= TELOP_NET_STOP_TALK && msg <= TELOP_NET_CLOSE_WINDOW;
}

int dObjTelop_c::create() {
    if (!mRes.load("Layout/telop.arc")) {
        return NOT_READY;
    }
    mLyt.build(&mRes, "telop_00.brlyt");
    initAnm();
    mLyt.calc();
    mLyt.mPriority = 0x8A;
    mState = 0;
    if (dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_DEMO)) {
        l_req.clear();
    }
    l_req.unlock();
    mWpadState = WPAD_STATE_WAIT;
    l_req.checkNet();
    return SUCCEEDED;
}

int dObjTelop_c::doDelete() {
    return mRes.remove() != FALSE;
}

int dObjTelop_c::execute() {
    static void (dObjTelop_c::*l_stateFuncs[])() = {
        &dObjTelop_c::stateMain,
    };
    if (dPad::hasUnsupportedExtension()) {
        if (mExtensionCnt < 5) {
            mExtensionCnt++;
        } else {
            request(TELOP_EXTENSION);
        }
    } else {
        mExtensionCnt = 0;
    }
    if ((!fn_8038E36C() || dPad::isRemoteUnavailable()) && !dReset::Manage_c::GetInstance()->isShutdown()) {
        if (mNoRemoteCnt < 20) {
            mNoRemoteCnt++;
        } else if (dScene::getCurrentScene() != dScene::SCENE_DM_TITLE) {
            request(TELOP_NO_REMOTE);
        }
    } else {
        mNoRemoteCnt = 0;
    }
    calcWpad();
    l_req.calc();
    (this->*l_stateFuncs[mState])();
    playAnm();
    if (mFlags & FLAG_CALC) {
        mFlags &= ~FLAG_CALC;
        mLyt.calc();
    }
    return SUCCEEDED;
}

int dObjTelop_c::draw() {
    if (mFlags & FLAG_SHOW) {
        mLyt.entry();
    }
    return SUCCEEDED;
}

void dObjTelop_c::stateMain() {
    if (!mAnm.isEnd()) {
        return;
    }
    switch (mAnm.mCurIdx) {
    case ANM_IN:
        if (mMsg != l_req.mMsg) {
            mAnm.setNext(ANM_OUT);
            break;
        }
        mScrollX -= 3.0f;
        if (mScrollX < -304.0f - mTextWidth) {
            mScrollX += 400.0f + mTextWidth;
        }
        setPos();
        break;
    case ANM_OUT:
        mFlags &= ~FLAG_SHOW;
        if (l_req.mMsg == TELOP_NONE) {
            break;
        }
        mMsg = l_req.mMsg;
        l_req.setTimer();
        setMsg();
        setPos();
        mAnm.setNext(ANM_IN);
        mFlags |= FLAG_SHOW;
        if (isNetMsg(mMsg) || isNetWarnMsg(mMsg)) {
            playSe(SE_Network_MessageAlert);
        }
        break;
    }
}

void dObjTelop_c::setMsg() {
    m2d::stringWordLL_c word;
    dString::loadString(&word, l_msgIds[mMsg], "sys_2D/SYS_2D_menu");
    mLyt.setWord("T_msgSdw_00", &word);
    mLyt.setWord("T_msg_00", &word);
    mLyt.setWord("T_msgSdw_01", &word);
    mLyt.setWord("T_msg_01", &word);
    mScrollX = 160.0f;
    nw4r::lyt::TextBox *textBox = mLyt.findTextBox("T_msg_00");
    mTextWidth = m2d::calcTextWidth(textBox, word.getText(), word.getCapacity());
    if (SCGetAspectRatio() == SC_ASPECT_WIDE) {
        mTextWidth *= 0.730769f;
    }
}

void dObjTelop_c::setPos() {
    nw4r::lyt::Pane *pane = mLyt.findPane("N_msg_00");
    mVec3_c trans = pane->GetTranslate();
    trans.x = mScrollX;
    pane->SetTranslate(trans);
    pane = mLyt.findPane("N_msg_01");
    trans = pane->GetTranslate();
    trans.x = 400.0f + (mScrollX + mTextWidth);
    pane->SetTranslate(trans);
    mFlags |= FLAG_CALC;
}

void dObjTelop_c::initAnm() {
    static const char *l_anmName[] = {
        "telop_00_AlpIn.brlan",
        "telop_00_AlpOut.brlan",
    };
    mAnm.init(&mLyt, &mRes, l_anmName, 2);
    mAnm.start(ANM_OUT);
    mAnm.setToEnd();
}

void dObjTelop_c::playAnm() {
    if (mAnm.play()) {
        mFlags |= FLAG_CALC;
    }
}

static void wpadInfoCallback(s32 chan, s32 result) {
    l_wpadResult = result;
}

void dObjTelop_c::wpadWait() {
    if (dPad::isRemoteUnavailable()) {
        mWpadState = WPAD_STATE_CHECK;
        mLowBatteryCnt = 0;
        mWpadTimer = 0;
    }
}

void dObjTelop_c::wpadCheck() {
    if (!dPad::isRemoteUnavailable()) {
        if (mWpadTimer > 0) {
            mWpadTimer--;
        } else {
            l_wpadResult = WPAD_ERR_BUSY;
            if (WPADGetInfoAsync(mPad::g_currentCoreId, &l_wpadInfo, wpadInfoCallback) == WPAD_ERR_OK) {
                mWpadState = WPAD_STATE_INFO;
            }
        }
    } else {
        mLowBatteryCnt = 0;
        mWpadTimer = 0;
    }
}

void dObjTelop_c::wpadInfo() {
    if (l_wpadResult == WPAD_ERR_BUSY) {
        return;
    }
    if (l_wpadResult == WPAD_ERR_OK && l_wpadInfo.battery <= 1) {
        if (++mLowBatteryCnt >= 2) {
            request(TELOP_LOW_BATTERY);
        } else {
            mWpadTimer = 1800;
            mWpadState = WPAD_STATE_CHECK;
            return;
        }
    }
    mWpadState = WPAD_STATE_WAIT;
}

void dObjTelop_c::calcWpad() {
    static void (dObjTelop_c::*l_wpadFuncs[])() = {
        &dObjTelop_c::wpadWait,
        &dObjTelop_c::wpadCheck,
        &dObjTelop_c::wpadInfo,
    };
    (this->*l_wpadFuncs[mWpadState])();
}

void dObjTelop_c::request(int msg) {
    l_req.request(msg);
}

void dObjTelop_c::lock() {
    l_req.lock();
}

void dObjTelop_c::unlock() {
    l_req.unlock();
}

BOOL dObjTelop_c::isLocked() {
    return l_req.isLocked();
}

void dObjTelop_c::setNetCheckHost() {
    l_req.setNetCheckHost();
}

void dObjTelop_c::setNetCheckAll() {
    l_req.setNetCheckAll();
}

void dObjTelop_c::Req_c::clear() {
    mTimer = 0;
    mMsg = TELOP_NONE;
}

void dObjTelop_c::Req_c::calc() {
    if (mLock) {
        clear();
    }
    if (mTimer > 0) {
        if (--mTimer == 0) {
            mMsg = TELOP_NONE;
        }
    }
}

void dObjTelop_c::Req_c::setTimer() {
    if (mMsg == TELOP_NO_REMOTE || mMsg == TELOP_EXTENSION) {
        mTimer = 10;
    } else {
        mTimer = 360;
    }
}

// Lower values are higher priority: request() drops a message less urgent than the current one.
int dObjTelop_c::Req_c::getPriority(int msg) {
    switch (msg) {
    case TELOP_NONE:
        return 8;
    case TELOP_TIMER_START:
    case TELOP_TIMER_STOP:
    case TELOP_TIMER_END:
        return 4;
    case TELOP_HIDE_AND_SEEK_START:
    case TELOP_HIDE_AND_SEEK_END:
        return 4;
    case TELOP_NET_ARRIVE:
    case TELOP_NET_LEAVE:
    case TELOP_NET_SAVE:
    case TELOP_NET_CLOSE:
    case TELOP_NET_CANCEL:
    case TELOP_NET_STOP_TALK:
    case TELOP_NET_CLOSE_WINDOW:
        return 1;
    case TELOP_NO_REMOTE:
        return 7;
    case TELOP_LOW_BATTERY:
        return 5;
    case TELOP_EXTENSION:
        return 6;
    case TELOP_MIC_CHAT_START:
        return 3;
    case TELOP_MIC_CHAT_UNAVAILABLE:
        return 2;
    }
    return 8;
}

void dObjTelop_c::Req_c::request(int msg) {
    if (mLock) {
        return;
    }
    int cur = getPriority(mMsg);
    if (cur < getPriority(msg)) {
        return;
    }
    if (isNetMsg(msg) && msg != TELOP_NET_CANCEL) {
        if (dDemo::isTalking()) {
            msg = TELOP_NET_STOP_TALK;
        }
        if (dDemo::isType(dDemo::TYPE_MENU)) {
            fn_8019A7AC();
        }
    }
    if (mMsg == msg && mTimer > 0) {
        setTimer();
    }
    if (mMsg != msg) {
        mMsg = msg;
        setTimer();
    }
}

void dObjTelop_c::Req_c::lock() {
    mLock = 1;
}

void dObjTelop_c::Req_c::unlock() {
    mLock = 0;
}

BOOL dObjTelop_c::Req_c::isLocked() {
    return mLock != 0;
}

void dObjTelop_c::Req_c::setNetCheckHost() {
    mNetCheck = NET_CHECK_HOST;
}

void dObjTelop_c::Req_c::setNetCheckAll() {
    mNetCheck = NET_CHECK_ALL;
}

void dObjTelop_c::Req_c::checkNet() {
    int check = mNetCheck;
    if (check == NET_CHECK_NONE) {
        return;
    }
    mNetCheck = NET_CHECK_NONE;
    if (!dNet::isSessionActive() || dNet::getMemberNum() <= 1) {
        return;
    }
    if (check == NET_CHECK_HOST) {
        if (dNet::isHost()) {
            if (dNet::getMemberNum() != 2) {
                return;
            }
        } else {
            return;
        }
    }
    if (dObjVoice_c::isActive()) {
        dObjTelop_c::request(TELOP_MIC_CHAT_START);
    }
}

dObjTelop_c::~dObjTelop_c() {}
