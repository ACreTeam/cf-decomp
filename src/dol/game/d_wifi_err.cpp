// The network error window (dWifiErr_c). .text 8017C4EC..8017CD3C.
// See include/game/game/d_wifi_err.hpp.
#include <game/game/d_wifi_err.hpp>
#include <game/game/d_caution.hpp>
#include <game/game/d_font.hpp>
#include <game/game/d_main.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_net_mgr.hpp>
#include <game/game/d_rumble.hpp>
#include <game/game/d_reset.hpp>
#include <game/game/d_reset_save.hpp>
#include <game/game/d_screenshot.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/mLib/m_pad.hpp>
#include <lib/revolution/WPAD/WPAD.h>

dWifiErr_c g_wifiErr; // 80621670

// 8017C4EC
void dWifiErr_c::create() {
    mResAcc.attach(g_cautionArc, "arc");
    mLyt.mpResAcc = &mResAcc;
    mLyt.build("caution_00.brlyt", NULL);
    mLyt.setFont(&dFont::g_mainFont.mFont);
    mAnm.create("caution_00.brlan", &mResAcc);
    mLyt.bind(&mAnm);
    nw4r::lyt::TextBox *text = mLyt.findTextBox("T_dialg_00");
    u16 len = text->GetStringBufferLength();
    text->AllocStringBuffer(len + 0x40);
    int msg;
    if (mFatal == 1) {
        mFatalMessage = TRUE;
        msg = 0x15E;
    } else {
        msg = 0x15E;
    }
    setMessage(msg);
    mAnm.play();
    mLyt.calc();
}

// 8017C600
void dWifiErr_c::setMessage(int msgId) {
    nw4r::lyt::TextBox *text = mLyt.findTextBox("T_dialg_00");
    dWifiErr::Word_c word;
    dWifiErr::setSysMessage(&word, msgId, "sys_2D/SYS_2D_menu");
    text->SetString(word.getText(), 0);
}

// 8017C69C
void dWifiErr_c::clear() {
    mActive = FALSE;
    mReported = FALSE;
    mFatal = FALSE;
    mFatalMessage = FALSE;
    mCloseRequest = FALSE;
    mClosing = FALSE;
    mResetRequest = FALSE;
    mForce = FALSE;
    mNetError = FALSE;
    mErrorCode = 0;
}

// 8017C6CC
u8 dWifiErr_c::check() {
    mReported = dNet::getNetReportedError() != 0;
    if ((dNet::isSessionActive() && dNetMgr_c::sInstance->mInfo.mError.mState != 0) || mForce) {
        mNetError = TRUE;
        int kind = dNetMgr_c::sInstance->mInfo.mError.getKind();
        mErrorCode = dNetMgr_c::sInstance->mInfo.mError.mDetail;
        if (kind == 2) {
        } else if (kind == 3) {
            mFatal = TRUE;
        }
    }
    if ((mReported || mNetError) && !mActive) {
        dResetSave::request();
        dMain::addStop(dMain::STOP_REASON_WIFI_ERR);
        create();
        mActive = TRUE;
        fn_8000E9C0();
        dRumble::stop();
    }
    return mActive;
}

// 8017C7A8
void dWifiErr_c::draw() {
    if (mActive != 1) {
        return;
    }
    if (!mResAcc.getResAccessor()->IsAttached()) {
        return;
    }
    mLyt.draw();
}

// 8017C7DC
void dWifiErr_c::execute() {
    if (mActive == 1 && mResAcc.getResAccessor()->IsAttached()) {
        if (!mFatalMessage && mFatal == 1) {
            mFatalMessage = TRUE;
            setMessage(0x15E);
        }
        mAnm.play();
        mLyt.calc();
        if (!mCloseRequest && !mClosing && !mFatal &&
            (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A) || mPad::g_currentCore->downTrigger(WPAD_BUTTON_FS_C) ||
             mResetRequest == 1) &&
            !mForce) {
            mCloseRequest = TRUE;
        }
        if (mCloseRequest) {
            BOOL close = TRUE;
            if (!dResetSave::isNetClosed()) {
                close = FALSE;
            }
            if (!dScreenshot::tryClose() && close) {
                close = FALSE;
            }
            if (close) {
                mCloseRequest = FALSE;
                mClosing = TRUE;
                dReset::Manage_c::GetInstance()->RequestErrorReset(0);
            }
        }
        if (mClosing && dReset::Manage_c::GetInstance()->IsFadeOutDone()) {
            dMain::isStopped(dMain::STOP_REASON_WIFI_ERR);
            dMain::removeStop(dMain::STOP_REASON_WIFI_ERR);
            mResAcc.detach();
            mLyt.unbind(&mAnm);
            mActive = FALSE;
            mReported = FALSE;
            mNetError = FALSE;
            mFatal = FALSE;
            mFatalMessage = FALSE;
            mClosing = FALSE;
            mResetRequest = FALSE;
        }
    }
}

namespace dWifiErr {

// The error code word for the number tags (a struct: its own implicit ctor / dtor).
struct CodeWord_s {
    Word_c mWord;
};

static CodeWord_s sCodeWord; // 80621808

// 8017C9AC
BOOL setSysMessage(dScript::Word_c *word, u16 msgId, const char *name) {
    const void *bmg = dScript::getBmgFile(dScript::getBank(), name);
    dScript::Res_c res(bmg);
    u16 count = res.getCount();
    if (msgId >= res.getCount()) {
        return FALSE;
    }
    if (bmg == NULL) {
        return FALSE;
    }
    word->set(res.getMessage(msgId), 0);
    return TRUE;
}

// 8017CA80
void setErrorCodeWord(int digits) {
    if (g_wifiErr.mErrorCode == 0) {
        sCodeWord.mWord.clear();
        return;
    }
    dScript::formatNumber(&sCodeWord.mWord, g_wifiErr.mErrorCode, digits, dScript::NUM_FORMAT_PLAIN, 0);
}

// 8017CAB8
dScript::Word_c *getErrorCodeWord() {
    return &sCodeWord.mWord;
}

// 8017CAC4
Word_c::Word_c() {
    clear();
}

// 8017CB08
Word_c::~Word_c() {}

// 8017CB60
u32 Word_c::getBufferSize() {
    return sizeof(mBuffer);
}

// 8017CB68
wchar_t *Word_c::getBuffer() {
    return mBuffer;
}

// 8017CB70: number tags 6.. show the error code with (id - 5) digits
int Word_c::procNumber(int pos) {
    setErrorCodeWord((u16)dScript::getTagId(getBuffer() + pos) - 5);
    return setTagWord(pos, getErrorCodeWord());
}

// 8017CBEC
int Word_c::procRubyTag(int pos) {
    return procRuby(pos);
}

} // namespace dWifiErr

