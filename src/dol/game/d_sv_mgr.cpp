// -*- coding: cp932 -*-
// vim: set fileencoding=cp932 :
// MWCC needs an extra backslash after a CP932 trail byte of 0x5C in string
// literals.
#include <cstring>
#include <game/game/d_demo.hpp>
#include <game/game/d_sv_mgr.hpp>
#include <game/game/d_sv_runtime.hpp>
#include <game/mLib/m_fader.hpp>

// TODO: Not linkable yet; every function matches, but the section layout does not.
// - The target is likely two TUs. .text/.ctors/.bss put the boundary after
//   clearRequestedMode (really __sinit for mRequestedMode = __ptmf_null), with
//   the remaining stepX_c helpers, checkSaveResult and ~dSvMgr_c in the second.
// - But the vtable block sits mid-.data, after stepLoad_c's data (+0x16A8),
//   instead of at the end of the first TU's .data as MWCC emits it.
// - Weak dtors sit after the __sinit thunks and before ~dSvMgr_c, not after
//   createMgr. MWCC places weak dtors after a ctor, so the original likely had
//   constructor code there that the linker stripped.
// - Declaring the stepX_c/stepBase_c dtors `inline virtual ~X();` (no body)
//   fixes the getStateName order, and declaring ~dSvMgr_c after execute()
//   with stepLoad_c defined last fixes the vtable order.

using namespace dSvRuntime;

dSvMgr_c::Method dSvMgr_c::mRequestedMode;
int dSvMgr_c::mVisitors[3];
int dSvMgr_c::mCurrentVisitor = 4;
int dSvMgr_c::mPlayerIndex = 3;
const char *dSvMgr_c::mMessageLabel = "SYS_MESS/SYS_Save_and_Load";
// Unreferenced in the DOL and RELs; its original name is unknown.
const char *lbl_8074C3CC = "SYS_MESS/SYS_NetworkError";

static u16 sLoadErrorMessage;

dSvMgr_c *dSvMgr_c::createMgr() {
    return new dSvMgr_c;
}

void dSvMgr_c::requestSaveNormal() {
    mRequestedMode = &dSvMgr_c::executeSaveNormal;
}

void dSvMgr_c::requestSaveRetireNetVst() {
    fn_800DCF58();
    mPlayerIndex = fn_801017B8();
    mRequestedMode = &dSvMgr_c::executeSaveRetireNetVst;
}

void dSvMgr_c::requestSaveConnectNetHst() {
    mRequestedMode = &dSvMgr_c::executeSaveConnectNetHst;
}

void dSvMgr_c::requestSaveConnectNetVst() {
    mRequestedMode = &dSvMgr_c::executeSaveConnectNetVst;
}

void dSvMgr_c::requestSaveContinueNetHst() {
    mRequestedMode = &dSvMgr_c::executeSaveContinueNetHst;
}

void dSvMgr_c::requestSaveContinueNetVst() {
    mRequestedMode = &dSvMgr_c::executeSaveContinueNetVst;
}

void dSvMgr_c::requestSaveInterruptNetHst() {
    mRequestedMode = &dSvMgr_c::executeSaveInterruptNetHst;
}

void dSvMgr_c::requestSaveInterruptNetVst() {
    fn_800DCF58();
    mPlayerIndex = fn_801017B8();
    mRequestedMode = &dSvMgr_c::executeSaveInterruptNetVst;
}

void dSvMgr_c::cancelRequest() {
    mRequestedMode = NULL;
    mPlayerIndex = 3;
    mCurrentVisitor = 4;
    clearVisitors();
}

int dSvMgr_c::isTransferComplete(int offset, int size) {
    if (fn_800D2404() &&
        static_cast<unsigned int>(offset) ==
            static_cast<unsigned int>(fn_800D240C()) &&
        static_cast<unsigned int>(size) ==
            static_cast<unsigned int>(fn_800D241C())) {
        return 1;
    }
    return 0;
}

int dSvMgr_c::isFullTransferComplete() {
    return isTransferComplete(0, lbl_80750AD0);
}

int dSvMgr_c::isTownTransferComplete() {
    fn_8010DC3C();
    return isTransferComplete(0, fn_8010DC04());
}

int dSvMgr_c::isHostTransferComplete() {
    return isTransferComplete(getHostDataOffset(), 0x200004);
}

int dSvMgr_c::isPlayerTransferComplete(int player) {
    return isTransferComplete(getPlayerDataOffset(player), 0x86C0);
}

int dSvMgr_c::getHostDataOffset() {
    void *base = fn_8010DC3C();
    char *host = static_cast<char *>(fn_8010DC3C()) + 0x20F320;
    return host - static_cast<char *>(base);
}

int dSvMgr_c::getVisitorDataOffset() {
    void *base = fn_8010DC3C();
    return reinterpret_cast<int>(fn_8010DC3C()) + 0x21B20 -
           reinterpret_cast<int>(base);
}

int dSvMgr_c::getPlayerDataOffset(int player) {
    void *base = fn_8010DC3C();
    return reinterpret_cast<int>(fn_801016DC(player)) -
           reinterpret_cast<int>(base);
}

// State names retain the original CP932 bytes.
int dSvMgr_c::create() {
    static const dState::mode_c<dSvMgr_c>::Entry modes[] = {
        {"待機", &dSvMgr_c::initializeWait, &dSvMgr_c::executeWait},
        {"処理なし", &dSvMgr_c::initializeIdle, &dSvMgr_c::executeIdle},
        {"通常セーブ", &dSvMgr_c::initializeSaveNormal,
         &dSvMgr_c::executeSaveNormal},
        {"退場者セーブ", &dSvMgr_c::initializeSaveRetireNetVst,
         &dSvMgr_c::executeSaveRetireNetVst},
        {"ネット接続セーブ（親）", &dSvMgr_c::initializeSaveConnectNetHst,
         &dSvMgr_c::executeSaveConnectNetHst},
        {"ネット接続セーブ（子）", &dSvMgr_c::initializeSaveConnectNetVst,
         &dSvMgr_c::executeSaveConnectNetVst},
        {"ネット継続セーブ（親）", &dSvMgr_c::initializeSaveContinueNetHst,
         &dSvMgr_c::executeSaveContinueNetHst},
        {"ネット継続セーブ（子）", &dSvMgr_c::initializeSaveContinueNetVst,
         &dSvMgr_c::executeSaveContinueNetVst},
        {"ネット中断セーブ（親）", &dSvMgr_c::initializeSaveInterruptNetHst,
         &dSvMgr_c::executeSaveInterruptNetHst},
        {"ネット中断セーブ（子）", &dSvMgr_c::initializeSaveInterruptNetVst,
         &dSvMgr_c::executeSaveInterruptNetVst},
        {"ソ\フトリセット用ロード処理", &dSvMgr_c::initializeLoad,
         &dSvMgr_c::executeLoad},
    };
    dState::mode_c<dSvMgr_c>::initialize("dSvMgr_c", modes, 11);
    mLoadFailed = false;
    if (fn_80162548() == 0x3A) {
        fn_80154358(lbl_8074E7F8);
        changeMode(&dSvMgr_c::executeLoad);
    }
    field<u32>(fn_8017D8E8(), 0x1AC) |= 0x200;
    return SUCCEEDED;
}

int dSvMgr_c::doDelete() {
    if (!fn_800D2A4C()) {
        return NOT_READY;
    }
    if (fn_80162548() == 0x3A) {
        int result;
        if (mLoadFailed) {
            result = fn_8015436C(lbl_8074E7F8, 5);
        } else {
            result = fn_8015436C(lbl_8074E7F8, 6);
        }
        if (!result) {
            return NOT_READY;
        }
    }
    field<u32>(fn_8017D8E8(), 0x1AC) &= ~0x200;
    return SUCCEEDED;
}

int dSvMgr_c::execute() {
    dState::base_c<dSvMgr_c>::execute();
    return SUCCEEDED;
}

void dSvMgr_c::requestMessage(u16 code) {
    dDemo_c *demo = dDemo_c::mInstance;
    setMessageLabel(mMessageLabel);
    setMessageCode(code);
    // The existing demo binding API names this receiver parameter as an actor.
    demo->attachActor(
        reinterpret_cast<dDemoActor_c *>(static_cast<dMsg::Rcpt_c *>(this)));
    fn_801A316C(demo, 0);
}

void dSvMgr_c::rcptHook14() {
    switch (mMessageCode) {
    case 0x0D:
        mMessageCallback = &dSvMgr_c::lockMessage;
        break;
    case 0x1B:
        mMessageCallback = &dSvMgr_c::lockMessage;
        break;
    }
    if (mMessageCallback) {
        (this->*mMessageCallback)();
        mMessageCallback = NULL;
    }
}

void dSvMgr_c::lockMessage() {
    void *controller = mpController;
    field<u8>(controller, 0x6C7F) = 1;
    field<u8>(controller, 0x6C7D) = 0;
    field<u8>(controller, 0x6C7C) = 1;
}

void dSvMgr_c::initializeWait() {
}

void dSvMgr_c::executeWait() {
    if (mFader_c::isStatus(mFaderBase_c::HIDDEN) && mRequestedMode) {
        Method requested = mRequestedMode;
        changeMode(requested);
        mRequestedMode = NULL;
    }
}

void dSvMgr_c::initializeIdle() {
}

void dSvMgr_c::executeIdle() {
}

void dSvMgr_c::initializeSaveNormal() {
    static const stepSaveNormal_c::Entry steps[] = {
        {"セーブメッセージ要求", (stepSaveNormal_c::Method)&stepSaveNormal_c::requestSaveMessage},
        {"メッセージロックまで待つ", (stepSaveNormal_c::Method)&stepSaveNormal_c::waitMessageLock},
        {"HBMとReset禁止ON", (stepSaveNormal_c::Method)&stepSaveNormal_c::disableHomeAndReset},
        {"開いているなら門を閉じる", (stepSaveNormal_c::Method)&stepSaveNormal_c::closeGate},
        {"WiFiコネクション切断", (stepSaveNormal_c::Method)&stepSaveNormal_c::disconnectWiFi},
        {"WC24送信開始", (stepSaveNormal_c::Method)&stepSaveNormal_c::startWC24Send},
        {"WC24送信終了", (stepSaveNormal_c::Method)&stepSaveNormal_c::waitWC24Send},
        {"他の村への手紙処理開始", (stepSaveNormal_c::Method)&stepSaveNormal_c::startVillageMail},
        {"他の村への手紙処理終了", (stepSaveNormal_c::Method)&stepSaveNormal_c::waitVillageMail},
        {"DL BOX生成初期化", (stepSaveNormal_c::Method)&stepSaveNormal_c::initializeDownloadBox},
        {"DL BOX生成", (stepSaveNormal_c::Method)&stepSaveNormal_c::createDownloadBox},
        {"全体セーブ初期化", (stepSaveNormal_c::Method)&stepSaveNormal_c::initializeFullSave},
        {"全体セーブ開始", (stepSaveNormal_c::Method)&stepSaveNormal_c::startFullSave},
        {"全体セーブ終了", (stepSaveNormal_c::Method)&stepSaveNormal_c::waitFullSave},
        {"終了", (stepSaveNormal_c::Method)&stepSaveNormal_c::finish},
        {"ロック解除", (stepSaveNormal_c::Method)&stepSaveNormal_c::releaseMessageLock},
        {"HBMとReset禁止OFF", (stepSaveNormal_c::Method)&stepSaveNormal_c::enableHomeAndReset},
        {"メッセージ終了待ち", (stepSaveNormal_c::Method)&stepSaveNormal_c::waitMessageEnd},
        {"タイトルへ", (stepSaveNormal_c::Method)&stepSaveNormal_c::gotoTitle},
        {"終了", (stepSaveNormal_c::Method)&stepSaveNormal_c::finishFinal},
        {"セーブ失敗", (stepSaveNormal_c::Method)&stepSaveNormal_c::saveFailed},
    };
    mSaveNormal.initialize(this, "dSvMgr_c::stepSaveNormal_c", steps, 21);
}

void dSvMgr_c::executeSaveNormal() {
    mSaveNormal.execute();
}

// Save/load step callbacks. Nonvirtual helper ownership is provisional.
// 801BC58C
void dSvMgr_c::stepSaveNormal_c::closeGate() {
    if (fn_800DCEDC() == 1) {
        dSvMgr_c::closeGateForSave();
    }
    nextStep();
}

// 801BC634
void dSvMgr_c::stepSaveNormal_c::disconnectWiFi() {
    if (fn_800DCDFC(0)) {
        nextStep();
    }
}

// 801BC6DC
void dSvMgr_c::stepSaveNormal_c::startWC24Send() {
    if (fn_801782EC()) {
        nextStep();
    }
}

// 801BC780
void dSvMgr_c::stepSaveNormal_c::waitWC24Send() {
    if (fn_80178374() != -1) {
        nextStep();
    }
}

// 801BC824
void dSvMgr_c::stepSaveNormal_c::startVillageMail() {
    fn_80103094();
    nextStep();
}

// 801BC8C0
void dSvMgr_c::stepSaveNormal_c::waitVillageMail() {
    if (fn_801030D0()) {
        fn_8010DC18();
        fn_8010DDE0();
        nextStep();
    }
}

// 801BC96C
void dSvMgr_c::stepSaveNormal_c::finish() {
    fn_8010DC18();
    fn_8010DED0();
    fn_801A4E44(mpOwner->getMessageController(), 0x0E);
    nextStep();
}

void dSvMgr_c::initializeSaveRetireNetVst() {
    static const stepSaveRetireNetVst_c::Entry steps[] = {
        {"通信セーブメッセージ要求", (stepSaveRetireNetVst_c::Method)&stepSaveRetireNetVst_c::requestNetworkSaveMessage},
        {"メッセージロックまで待つ", (stepSaveRetireNetVst_c::Method)&stepSaveRetireNetVst_c::waitMessageLock},
        {"HBMとReset禁止ON", (stepSaveRetireNetVst_c::Method)&stepSaveRetireNetVst_c::disableHomeAndReset},
        {"ロード初期化", (stepSaveRetireNetVst_c::Method)&stepSaveRetireNetVst_c::initializeLoad},
        {"ロード開始", (stepSaveRetireNetVst_c::Method)&stepSaveRetireNetVst_c::startLoad},
        {"ロード完了？", (stepSaveRetireNetVst_c::Method)&stepSaveRetireNetVst_c::waitLoad},
        {"ロード処理完了", (stepSaveRetireNetVst_c::Method)&stepSaveRetireNetVst_c::processLoadedSave},
        {"成長処理前処理", (stepSaveRetireNetVst_c::Method)&stepSaveRetireNetVst_c::initializeGrowth},
        {"成長処理", (stepSaveRetireNetVst_c::Method)&stepSaveRetireNetVst_c::processGrowth},
        {"DL BOX生成初期化", (stepSaveRetireNetVst_c::Method)&stepSaveRetireNetVst_c::initializeDownloadBox},
        {"DL BOX生成", (stepSaveRetireNetVst_c::Method)&stepSaveRetireNetVst_c::createDownloadBox},
        {"全体セーブ初期化", (stepSaveRetireNetVst_c::Method)&stepSaveRetireNetVst_c::initializeFullSave},
        {"全体セーブ開始", (stepSaveRetireNetVst_c::Method)&stepSaveRetireNetVst_c::startFullSave},
        {"全体セーブ終了", (stepSaveRetireNetVst_c::Method)&stepSaveRetireNetVst_c::waitFullSave},
        {"ロック解除", (stepSaveRetireNetVst_c::Method)&stepSaveRetireNetVst_c::releaseMessageLock},
        {"HBMとReset禁止OFF", (stepSaveRetireNetVst_c::Method)&stepSaveRetireNetVst_c::enableHomeAndReset},
        {"メッセージ終了待ち", (stepSaveRetireNetVst_c::Method)&stepSaveRetireNetVst_c::waitMessageEnd},
        {"門の外へ", (stepSaveRetireNetVst_c::Method)&stepSaveRetireNetVst_c::gotoOutsideGate},
        {"終了", (stepSaveRetireNetVst_c::Method)&stepSaveRetireNetVst_c::finish},
        {"セーブ失敗", (stepSaveRetireNetVst_c::Method)&stepSaveRetireNetVst_c::saveFailed},
    };
    mSaveRetireNetVst.initialize(this, "dSvMgr_c::stepSaveRetireNetVst_c",
                                 steps, 20);
}

void dSvMgr_c::executeSaveRetireNetVst() {
    mSaveRetireNetVst.execute();
}

void dSvMgr_c::initializeSaveConnectNetHst() {
    static const stepSaveConnectNetHst_c::Entry steps[] = {
        {"初期化処理", (stepSaveConnectNetHst_c::Method)&stepSaveConnectNetHst_c::initializeSave},
        {"通信セーブメッセージ要求", (stepSaveConnectNetHst_c::Method)&stepSaveConnectNetHst_c::requestNetworkSaveMessage},
        {"メッセージロックまで待つ", (stepSaveConnectNetHst_c::Method)&stepSaveConnectNetHst_c::waitMessageLock},
        {"セーブの許可待ち", (stepSaveConnectNetHst_c::Method)&stepSaveConnectNetHst_c::waitSavePermission},
        {"村＋その他データセーブ初期化", (stepSaveConnectNetHst_c::Method)&stepSaveConnectNetHst_c::initializeTownSave},
        {"村＋その他データセーブ開始", (stepSaveConnectNetHst_c::Method)&stepSaveConnectNetHst_c::startTownSave},
        {"村＋その他データセーブ終了", (stepSaveConnectNetHst_c::Method)&stepSaveConnectNetHst_c::waitTownSave},
        {"新規参入プレイヤ受信完了待ち", (stepSaveConnectNetHst_c::Method)&stepSaveConnectNetHst_c::waitNewPlayer},
        {"ともだち情報で検索", (stepSaveConnectNetHst_c::Method)&stepSaveConnectNetHst_c::findFriend},
        {"ネットホストセーブ初期化", (stepSaveConnectNetHst_c::Method)&stepSaveConnectNetHst_c::initializeHostSave},
        {"ネットホストセーブ開始", (stepSaveConnectNetHst_c::Method)&stepSaveConnectNetHst_c::startHostSave},
        {"ネットホストセーブ終了", (stepSaveConnectNetHst_c::Method)&stepSaveConnectNetHst_c::waitHostSave},
        {"ロック解除", (stepSaveConnectNetHst_c::Method)&stepSaveConnectNetHst_c::releaseMessageLock},
        {"メッセージ終了待ち", (stepSaveConnectNetHst_c::Method)&stepSaveConnectNetHst_c::waitMessageEnd},
        {"迎えデモへ", (stepSaveConnectNetHst_c::Method)&stepSaveConnectNetHst_c::gotoWelcomeDemo},
        {"終了", (stepSaveConnectNetHst_c::Method)&stepSaveConnectNetHst_c::finish},
        {"セーブ失敗", (stepSaveConnectNetHst_c::Method)&stepSaveConnectNetHst_c::saveFailed},
    };
    mSaveConnectNetHst.initialize(this, "dSvMgr_c::stepSaveConnectNetHst_c",
                                  steps, 17);
}

void dSvMgr_c::executeSaveConnectNetHst() {
    mSaveConnectNetHst.execute();
}

// 801BD098
void dSvMgr_c::stepSaveConnectNetHst_c::initializeSave() {
    if (fn_800DCF30() == 1) {
        fn_80102AA4();
        fn_80102AA4();
        dSvMgr_c::clearVisitors();
    }
    nextStep();
}

void dSvMgr_c::initializeSaveConnectNetVst() {
    static const stepSaveConnectNetVst_c::Entry steps[] = {
        {"通信セーブメッセージ要求", (stepSaveConnectNetVst_c::Method)&stepSaveConnectNetVst_c::requestNetworkSaveMessage},
        {"メッセージロックまで待つ", (stepSaveConnectNetVst_c::Method)&stepSaveConnectNetVst_c::waitMessageLock},
        {"セーブの許可待ち", (stepSaveConnectNetVst_c::Method)&stepSaveConnectNetVst_c::waitSavePermission},
        {"新規参入プレイヤ受信完了待ち", (stepSaveConnectNetVst_c::Method)&stepSaveConnectNetVst_c::waitNewPlayer},
        {"ともだち情報で検索", (stepSaveConnectNetVst_c::Method)&stepSaveConnectNetVst_c::findFriend},
        {"ネットビジターセーブ初期化", (stepSaveConnectNetVst_c::Method)&stepSaveConnectNetVst_c::initializeVisitorSave},
        {"ネットビジターセーブ開始", (stepSaveConnectNetVst_c::Method)&stepSaveConnectNetVst_c::startVisitorSave},
        {"ネットビジターセーブ終了", (stepSaveConnectNetVst_c::Method)&stepSaveConnectNetVst_c::waitVisitorSave},
        {"ロック解除", (stepSaveConnectNetVst_c::Method)&stepSaveConnectNetVst_c::releaseMessageLock},
        {"メッセージ終了待ち", (stepSaveConnectNetVst_c::Method)&stepSaveConnectNetVst_c::waitMessageEnd},
        {"迎えデモへ", (stepSaveConnectNetVst_c::Method)&stepSaveConnectNetVst_c::gotoWelcomeDemo},
        {"終了", (stepSaveConnectNetVst_c::Method)&stepSaveConnectNetVst_c::finish},
        {"セーブ失敗", (stepSaveConnectNetVst_c::Method)&stepSaveConnectNetVst_c::saveFailed},
    };
    mSaveConnectNetVst.initialize(this, "dSvMgr_c::stepSaveConnectNetVst_c",
                                  steps, 13);
}

void dSvMgr_c::executeSaveConnectNetVst() {
    mSaveConnectNetVst.execute();
}

void dSvMgr_c::initializeSaveContinueNetHst() {
    static const stepSaveContinueNetHst_c::Entry steps[] = {
        {"通信セーブメッセージ要求", (stepSaveContinueNetHst_c::Method)&stepSaveContinueNetHst_c::requestNetworkSaveMessage},
        {"メッセージロックまで待つ", (stepSaveContinueNetHst_c::Method)&stepSaveContinueNetHst_c::waitMessageLock},
        {"一斉セーブ時にセーブできる状態か", (stepSaveContinueNetHst_c::Method)&stepSaveContinueNetHst_c::waitCanSave},
        {"DL BOX生成初期化", (stepSaveContinueNetHst_c::Method)&stepSaveContinueNetHst_c::initializeDownloadBox},
        {"DL BOX生成", (stepSaveContinueNetHst_c::Method)&stepSaveContinueNetHst_c::createDownloadBox},
        {"全体セーブ初期化", (stepSaveContinueNetHst_c::Method)&stepSaveContinueNetHst_c::initializeFullSave},
        {"全体セーブ開始", (stepSaveContinueNetHst_c::Method)&stepSaveContinueNetHst_c::startFullSave},
        {"全体セーブ終了", (stepSaveContinueNetHst_c::Method)&stepSaveContinueNetHst_c::waitFullSave},
        {"全マシンのセーブ終了待ち", (stepSaveContinueNetHst_c::Method)&stepSaveContinueNetHst_c::waitAllMachinesSaved},
        {"ロック解除", (stepSaveContinueNetHst_c::Method)&stepSaveContinueNetHst_c::releaseMessageLock},
        {"メッセージ終了待ち", (stepSaveContinueNetHst_c::Method)&stepSaveContinueNetHst_c::waitMessageEnd},
        {"元のシーンへ", (stepSaveContinueNetHst_c::Method)&stepSaveContinueNetHst_c::gotoPreviousScene},
        {"終了", (stepSaveContinueNetHst_c::Method)&stepSaveContinueNetHst_c::finish},
        {"セーブ失敗", (stepSaveContinueNetHst_c::Method)&stepSaveContinueNetHst_c::saveFailed},
    };
    mSaveContinueNetHst.initialize(this, "dSvMgr_c::stepSaveContinueNetHst_c",
                                   steps, 14);
}

void dSvMgr_c::executeSaveContinueNetHst() {
    mSaveContinueNetHst.execute();
}

// 801BD6C8
void dSvMgr_c::stepSaveContinueNetHst_c::waitAllMachinesSaved() {
    if (fn_800DDA7C()) {
        fn_800DD4C8();
        fn_800DD588(0x1D, 4);
        fn_800DDAE4();
        fn_800DD4A0();
        fn_800DDE4C();
        nextStep();
    }
}

void dSvMgr_c::initializeSaveContinueNetVst() {
    static const stepSaveContinueNetVst_c::Entry steps[] = {
        {"通信セーブメッセージ要求", (stepSaveContinueNetVst_c::Method)&stepSaveContinueNetVst_c::requestNetworkSaveMessage},
        {"メッセージロックまで待つ", (stepSaveContinueNetVst_c::Method)&stepSaveContinueNetVst_c::waitMessageLock},
        {"一斉セーブ時にセーブできる状態か", (stepSaveContinueNetVst_c::Method)&stepSaveContinueNetVst_c::waitCanSave},
        {"ネットビジターセーブ初期化", (stepSaveContinueNetVst_c::Method)&stepSaveContinueNetVst_c::initializeVisitorSave},
        {"ネットビジターセーブ開始", (stepSaveContinueNetVst_c::Method)&stepSaveContinueNetVst_c::startVisitorSave},
        {"ネットビジターセーブ終了", (stepSaveContinueNetVst_c::Method)&stepSaveContinueNetVst_c::waitVisitorSave},
        {"「セーブ終了」をネットホストに送信", (stepSaveContinueNetVst_c::Method)&stepSaveContinueNetVst_c::notifyHostSaveComplete},
        {"セーブデモの終了命令がきたら", (stepSaveContinueNetVst_c::Method)&stepSaveContinueNetVst_c::waitSaveDemoEnd},
        {"ロック解除", (stepSaveContinueNetVst_c::Method)&stepSaveContinueNetVst_c::releaseMessageLock},
        {"メッセージ終了待ち", (stepSaveContinueNetVst_c::Method)&stepSaveContinueNetVst_c::waitMessageEnd},
        {"元のシーンへ", (stepSaveContinueNetVst_c::Method)&stepSaveContinueNetVst_c::gotoPreviousScene},
        {"終了", (stepSaveContinueNetVst_c::Method)&stepSaveContinueNetVst_c::finish},
        {"セーブ失敗", (stepSaveContinueNetVst_c::Method)&stepSaveContinueNetVst_c::saveFailed},
    };
    mSaveContinueNetVst.initialize(this, "dSvMgr_c::stepSaveContinueNetVst_c",
                                   steps, 13);
}

void dSvMgr_c::executeSaveContinueNetVst() {
    mSaveContinueNetVst.execute();
}

// 801BDA48
void dSvMgr_c::stepSaveContinueNetVst_c::notifyHostSaveComplete() {
    fn_800DD4C8();
    fn_800DD588(0x1C, 0);
    nextStep();
}

// 801BDAF0
void dSvMgr_c::stepSaveContinueNetVst_c::waitSaveDemoEnd() {
    if (fn_800DDAA4()) {
        fn_800DDAE4();
        fn_800DD4A0();
        fn_800DDE4C();
        nextStep();
    }
}

void dSvMgr_c::initializeSaveInterruptNetHst() {
    static const stepSaveInterruptNetHst_c::Entry steps[] = {
        {"初期化処理", (stepSaveInterruptNetHst_c::Method)&stepSaveInterruptNetHst_c::initializeSave},
        {"通信セーブメッセージ要求", (stepSaveInterruptNetHst_c::Method)&stepSaveInterruptNetHst_c::requestNetworkSaveMessage},
        {"メッセージロックまで待つ", (stepSaveInterruptNetHst_c::Method)&stepSaveInterruptNetHst_c::waitMessageLock},
        {"一斉セーブ時にセーブできる？", (stepSaveInterruptNetHst_c::Method)&stepSaveInterruptNetHst_c::waitCanSave},
        {"HBMとReset禁止ON", (stepSaveInterruptNetHst_c::Method)&stepSaveInterruptNetHst_c::disableHomeAndReset},
        {"成長処理前処理", (stepSaveInterruptNetHst_c::Method)&stepSaveInterruptNetHst_c::initializeGrowth},
        {"成長処理", (stepSaveInterruptNetHst_c::Method)&stepSaveInterruptNetHst_c::processGrowth},
        {"DL BOX生成初期化", (stepSaveInterruptNetHst_c::Method)&stepSaveInterruptNetHst_c::initializeDownloadBox},
        {"DL BOX生成", (stepSaveInterruptNetHst_c::Method)&stepSaveInterruptNetHst_c::createDownloadBox},
        {"全体セーブ初期化", (stepSaveInterruptNetHst_c::Method)&stepSaveInterruptNetHst_c::initializeFullSave},
        {"全体セーブ開始", (stepSaveInterruptNetHst_c::Method)&stepSaveInterruptNetHst_c::startFullSave},
        {"全体セーブ終了", (stepSaveInterruptNetHst_c::Method)&stepSaveInterruptNetHst_c::waitFullSave},
        {"ロック解除", (stepSaveInterruptNetHst_c::Method)&stepSaveInterruptNetHst_c::releaseMessageLock},
        {"HBMとReset禁止OFF", (stepSaveInterruptNetHst_c::Method)&stepSaveInterruptNetHst_c::enableHomeAndReset},
        {"メッセージ終了待ち", (stepSaveInterruptNetHst_c::Method)&stepSaveInterruptNetHst_c::waitMessageEnd},
        {"元のシーンへ", (stepSaveInterruptNetHst_c::Method)&stepSaveInterruptNetHst_c::gotoPreviousScene},
        {"終了", (stepSaveInterruptNetHst_c::Method)&stepSaveInterruptNetHst_c::finish},
        {"セーブ失敗", (stepSaveInterruptNetHst_c::Method)&stepSaveInterruptNetHst_c::saveFailed},
    };
    mSaveInterruptNetHst.initialize(this, "dSvMgr_c::stepSaveInterruptNetHst_c",
                                    steps, 18);
}

void dSvMgr_c::executeSaveInterruptNetHst() {
    mSaveInterruptNetHst.execute();
}

// 801BDED0
void dSvMgr_c::stepSaveInterruptNetHst_c::initializeSave() {
    u16 item[2];
    item[0] = 0xFFF1;
    fn_80168DC0(item, 1);
    fn_80168DC0(item, 2);
    fn_80168DC0(item, 3);
    fn_800A949C(1, 0);
    fn_800A949C(2, 0);
    fn_800A949C(3, 0);
    dSvMgr_c::closeGateForSave();
    nextStep();
}

void dSvMgr_c::initializeSaveInterruptNetVst() {
    static const stepSaveInterruptNetVst_c::Entry steps[] = {
        {"初期化処理", (stepSaveInterruptNetVst_c::Method)&stepSaveInterruptNetVst_c::initializeSave},
        {"セーブメッセージ要求", (stepSaveInterruptNetVst_c::Method)&stepSaveInterruptNetVst_c::requestSaveMessage},
        {"メッセージロックまで待つ", (stepSaveInterruptNetVst_c::Method)&stepSaveInterruptNetVst_c::waitMessageLock},
        {"一斉セーブ時にセーブできる？", (stepSaveInterruptNetVst_c::Method)&stepSaveInterruptNetVst_c::waitCanSave},
        {"HBMとReset禁止ON", (stepSaveInterruptNetVst_c::Method)&stepSaveInterruptNetVst_c::disableHomeAndReset},
        {"ロード初期化", (stepSaveInterruptNetVst_c::Method)&stepSaveInterruptNetVst_c::initializeLoad},
        {"ロード開始", (stepSaveInterruptNetVst_c::Method)&stepSaveInterruptNetVst_c::startLoad},
        {"ロード完了？", (stepSaveInterruptNetVst_c::Method)&stepSaveInterruptNetVst_c::waitLoad},
        {"ロード処理完了", (stepSaveInterruptNetVst_c::Method)&stepSaveInterruptNetVst_c::processLoadedSave},
        {"成長処理前処理", (stepSaveInterruptNetVst_c::Method)&stepSaveInterruptNetVst_c::initializeGrowth},
        {"成長処理", (stepSaveInterruptNetVst_c::Method)&stepSaveInterruptNetVst_c::processGrowth},
        {"DL BOX生成初期化", (stepSaveInterruptNetVst_c::Method)&stepSaveInterruptNetVst_c::initializeDownloadBox},
        {"DL BOX生成", (stepSaveInterruptNetVst_c::Method)&stepSaveInterruptNetVst_c::createDownloadBox},
        {"全体セーブ初期化", (stepSaveInterruptNetVst_c::Method)&stepSaveInterruptNetVst_c::initializeFullSave},
        {"全体セーブ開始", (stepSaveInterruptNetVst_c::Method)&stepSaveInterruptNetVst_c::startFullSave},
        {"全体セーブ終了", (stepSaveInterruptNetVst_c::Method)&stepSaveInterruptNetVst_c::waitFullSave},
        {"ロック解除", (stepSaveInterruptNetVst_c::Method)&stepSaveInterruptNetVst_c::releaseMessageLock},
        {"HBMとReset禁止OFF", (stepSaveInterruptNetVst_c::Method)&stepSaveInterruptNetVst_c::enableHomeAndReset},
        {"メッセージ終了待ち", (stepSaveInterruptNetVst_c::Method)&stepSaveInterruptNetVst_c::waitMessageEnd},
        {"門の外へ", (stepSaveInterruptNetVst_c::Method)&stepSaveInterruptNetVst_c::gotoOutsideGate},
        {"終了", (stepSaveInterruptNetVst_c::Method)&stepSaveInterruptNetVst_c::finish},
        {"セーブ失敗", (stepSaveInterruptNetVst_c::Method)&stepSaveInterruptNetVst_c::saveFailed},
    };
    mSaveInterruptNetVst.initialize(this, "dSvMgr_c::stepSaveInterruptNetVst_c",
                                    steps, 22);
}

void dSvMgr_c::executeSaveInterruptNetVst() {
    mSaveInterruptNetVst.execute();
}

// 801BE388
void dSvMgr_c::stepSaveInterruptNetVst_c::initializeSave() {
    nextStep();
}

void dSvMgr_c::addVisitor(int player) {
    for (int i = 0; i < 3; i++) {
        if (mVisitors[i] == 4) {
            mVisitors[i] = player;
            return;
        }
    }
}

void dSvMgr_c::removeVisitor(int player) {
    unsigned int index = 3;
    for (int i = 0; i < 3; i++) {
        if (mVisitors[i] == player) {
            index = i;
            break;
        }
    }
    for (int i = index; i < 2; i++) {
        mVisitors[i] = mVisitors[i + 1];
    }
    mVisitors[2] = 4;
}

void dSvMgr_c::clearVisitors() {
    mVisitors[0] = 4;
    mVisitors[1] = 4;
    mVisitors[2] = 4;
}

int dSvMgr_c::getLastVisitor() {
    for (int i = 2; i >= 0; i--) {
        if (mVisitors[i] != 4) {
            return mVisitors[i];
        }
    }
    return 4;
}

void dSvMgr_c::finishLoad() {
    if (mLoadFailed) {
        fn_8010EA88(fn_8010DC18());
        fn_8010DCB0(fn_8010DC18());
    }
    changeMode(&dSvMgr_c::executeIdle);
}

void dSvMgr_c::initializeLoad() {
    static const stepLoad_c::Entry steps[] = {
        {"ロード初期化", (stepLoad_c::Method)&stepLoad_c::initializeLoad},
        {"ロード開始", (stepLoad_c::Method)&stepLoad_c::startLoad},
        {"ロード完了？", (stepLoad_c::Method)&stepLoad_c::waitLoad},
        {"ロード終了", (stepLoad_c::Method)&stepLoad_c::processLoadedSave},
        {"タイトルへ", (stepLoad_c::Method)&stepLoad_c::gotoTitle},
        {"終了", (stepLoad_c::Method)&stepLoad_c::finish},
    };
    mLoad.initialize(this, "dSvMgr_c::stepLoad_c", steps, 6);
}

void dSvMgr_c::executeLoad() {
    mLoad.execute();
}

// 801BE938
void dSvMgr_c::stepLoad_c::processLoadedSave() {
    switch (fn_800D2A44()) {
    case 1:
        if (!fn_8010E0F8(fn_8010DC3C(), 1)) {
            mpOwner->setLoadFailed();
            setLoadErrorMessage(2);
        } else if (!fn_8010DC44(fn_8010DC18())) {
            mpOwner->setLoadFailed();
            setLoadErrorMessage(2);
        }
        break;
    case 4:
        mpOwner->setLoadFailed();
        break;
    case 3:
    case 5:
        mpOwner->setLoadFailed();
        setLoadErrorMessage(2);
        break;
    case 2:
        mpOwner->setLoadFailed();
        setLoadErrorMessage(0x0C);
        break;
    default:
        mpOwner->setLoadFailed();
        setLoadErrorMessage(0x12);
        break;
    }
    nextStep();
}

static inline int requiredSaveSize() {
    int auxiliary = fn_8017B190();
    int resource = fn_8017B18C();
    return (resource + lbl_80750AD0) + (auxiliary + lbl_80750330);
}
int dSvMgr_c::getRequiredBlocks() {
    static int blocks = blocksForSize(requiredSaveSize());
    return blocks;
}

int dSvMgr_c::getRequiredInodes() {
    static int save = inodesForSize(lbl_80750AD0);
    static int extra = inodesForSize(lbl_80750330);
    static int resource = inodesForSize(fn_8017B18C());
    static int auxiliary = inodesForSize(fn_8017B190());
    static int total = auxiliary + resource + save + extra;
    return total;
}

int dSvMgr_c::getRequiredFiles() {
    return 4;
}

unsigned int dSvMgr_c::blocksForSize(unsigned int size) {
    return (size + 0x1FFFF) >> 17;
}

unsigned int dSvMgr_c::inodesForSize(unsigned int size) {
    return (size + 0x3FFF) >> 14;
}

void dSvMgr_c::closeGateForSave() {
    fn_800F47F4();
    mCurrentVisitor = 4;
}

// 801BEC2C
void dSvMgr_c::stepLoad_c::gotoTitle() {
    fn_80162B1C(0x38, 5, 2);
    nextStep();
}

// 801BECD4
void dSvMgr_c::stepLoad_c::finish() {
    mpOwner->finishLoad();
}

// 801BECDC
void dSvMgr_c::stepSaveInterruptNetVst_c::requestSaveMessage() {
    mpOwner->requestMessage(0x1B);
    nextStep();
}

// 801BED80
void dSvMgr_c::stepSaveInterruptNetVst_c::waitMessageLock() {
    void *controller = mpOwner->getMessageController();
    DemoMethod expected = &dDemo_c::fn_801A4518;
    if (isDemoMode(controller, expected) &&
        field<u8>(mpOwner->getMessageController(), 0x6C7F)) {
        nextStep();
    }
}

// 801BEE68
void dSvMgr_c::stepSaveInterruptNetVst_c::releaseMessageLock() {
    releaseMessage(mpOwner->getMessageController());
    nextStep();
}

// 801BEF18
void dSvMgr_c::stepSaveInterruptNetVst_c::waitMessageEnd() {
    void *demo = dDemo_c::mInstance;
    DemoMethod expected = &dDemo_c::fn_801A334C;
    if (isDemoMode(demo, expected)) {
        static_cast<dDemo_c *>(demo)->detachActor();
        nextStep();
    }
}

// 801BEFF0
void dSvMgr_c::stepSaveInterruptNetVst_c::gotoOutsideGate() {
    fn_801634AC(fn_801BB7B8(), 1, 5, 0);
    nextStep();
}

// 801BF09C
void dSvMgr_c::stepSaveInterruptNetVst_c::finish() {
    mpOwner->finishLoad();
}

// 801BF0A4
void dSvMgr_c::stepSaveInterruptNetVst_c::waitCanSave() {
    if (fn_800DDBA8()) {
        nextStep();
    }
}

// 801BF148
void dSvMgr_c::stepSaveInterruptNetVst_c::processLoadedSave() {
    void *demo = dDemo_c::mInstance;
    switch (fn_800D2A44()) {
    case 1:
        if (!fn_8010E0F8(fn_8010DC3C(), 1)) {
            releaseMessage(demo);
            fn_801A4E44(demo, 0x13);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveInterruptNetVst_c::saveFailed;
        } else if (!fn_8010DC44(fn_8010DC18())) {
            releaseMessage(demo);
            fn_801A4E44(demo, 0x13);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveInterruptNetVst_c::saveFailed;
        } else {
            u8 savedIdentity[40];
            fn_800DCF58();
            int index = mPlayerIndex;
            fn_801017B8();
            void *player = fn_80101624(index);
            fn_8010DAFC(&field<u8>(player, 0x83E8));
            fn_80136C7C(fn_80101770(), player);
            fn_8010DC18();
            fn_8010DFC0();
            void *save = fn_8010DC3C();
            fn_8014C2C4(savedIdentity, &field<u8>(save, 0x73522));
            nextStep();
        }
        break;
    case 2:
        releaseMessage(demo);
        fn_801A4E44(demo, 0x0C);
        fn_801A4E34(demo, mMessageLabel);
        mCurrentMethod = &dSvMgr_c::stepSaveInterruptNetVst_c::saveFailed;
        break;
    default:
        releaseMessage(demo);
        fn_801A4E44(demo, 0x13);
        fn_801A4E34(demo, mMessageLabel);
        mCurrentMethod = &dSvMgr_c::stepSaveInterruptNetVst_c::saveFailed;
    }
}

// 801BF39C
void dSvMgr_c::stepSaveInterruptNetVst_c::initializeGrowth() {
    fn_80154358(lbl_8074E7F8);
    nextStep();
}

// 801BF43C
void dSvMgr_c::stepSaveInterruptNetVst_c::processGrowth() {
    if (fn_8015436C(lbl_8074E7F8, 4)) {
        nextStep();
    }
}

// 801BF4E8
void dSvMgr_c::stepSaveInterruptNetHst_c::requestNetworkSaveMessage() {
    mpOwner->requestMessage(0x1B);
    nextStep();
}

// 801BF58C
void dSvMgr_c::stepSaveInterruptNetHst_c::waitMessageLock() {
    void *controller = mpOwner->getMessageController();
    DemoMethod expected = &dDemo_c::fn_801A4518;
    if (isDemoMode(controller, expected) &&
        field<u8>(mpOwner->getMessageController(), 0x6C7F)) {
        nextStep();
    }
}

// 801BF674
void dSvMgr_c::stepSaveInterruptNetHst_c::releaseMessageLock() {
    releaseMessage(mpOwner->getMessageController());
    nextStep();
}

// 801BF724
void dSvMgr_c::stepSaveInterruptNetHst_c::waitMessageEnd() {
    void *demo = dDemo_c::mInstance;
    DemoMethod expected = &dDemo_c::fn_801A334C;
    if (isDemoMode(demo, expected)) {
        static_cast<dDemo_c *>(demo)->detachActor();
        nextStep();
    }
}

// 801BF7FC
void dSvMgr_c::stepSaveInterruptNetHst_c::gotoPreviousScene() {
    fn_801634AC(fn_801BB7B8(), 0, 5, 0);
    nextStep();
}

// 801BF8A8
void dSvMgr_c::stepSaveInterruptNetHst_c::finish() {
    mpOwner->finishLoad();
}

// 801BF8B0
void dSvMgr_c::stepSaveInterruptNetHst_c::waitCanSave() {
    if (fn_800DDBA8()) {
        nextStep();
    }
}

// 801BF954
void dSvMgr_c::stepSaveInterruptNetHst_c::initializeGrowth() {
    fn_80154358(lbl_8074E7F8);
    nextStep();
}

// 801BF9F4
void dSvMgr_c::stepSaveInterruptNetHst_c::processGrowth() {
    if (fn_8015436C(lbl_8074E7F8, 4)) {
        nextStep();
    }
}

// 801BFAA0
void dSvMgr_c::stepSaveContinueNetVst_c::requestNetworkSaveMessage() {
    mpOwner->requestMessage(0x1B);
    nextStep();
}

// 801BFB44
void dSvMgr_c::stepSaveContinueNetVst_c::waitMessageLock() {
    void *controller = mpOwner->getMessageController();
    DemoMethod expected = &dDemo_c::fn_801A4518;
    if (isDemoMode(controller, expected) &&
        field<u8>(mpOwner->getMessageController(), 0x6C7F)) {
        nextStep();
    }
}

// 801BFC2C
void dSvMgr_c::stepSaveContinueNetVst_c::releaseMessageLock() {
    releaseMessage(mpOwner->getMessageController());
    nextStep();
}

// 801BFCDC
void dSvMgr_c::stepSaveContinueNetVst_c::waitMessageEnd() {
    void *demo = dDemo_c::mInstance;
    DemoMethod expected = &dDemo_c::fn_801A334C;
    if (isDemoMode(demo, expected)) {
        static_cast<dDemo_c *>(demo)->detachActor();
        nextStep();
    }
}

// 801BFDB4
void dSvMgr_c::stepSaveContinueNetVst_c::gotoPreviousScene() {
    fn_801634AC(fn_801BB7B8(), 0, 5, 0);
    nextStep();
}

// 801BFE60
void dSvMgr_c::stepSaveContinueNetVst_c::finish() {
    mpOwner->finishLoad();
}

// 801BFE68
void dSvMgr_c::stepSaveContinueNetVst_c::waitCanSave() {
    if (fn_800DDB58()) {
        nextStep();
    }
}

// 801BFF0C
void dSvMgr_c::stepSaveContinueNetHst_c::requestNetworkSaveMessage() {
    mpOwner->requestMessage(0x1B);
    nextStep();
}

// 801BFFB0
void dSvMgr_c::stepSaveContinueNetHst_c::waitMessageLock() {
    void *controller = mpOwner->getMessageController();
    DemoMethod expected = &dDemo_c::fn_801A4518;
    if (isDemoMode(controller, expected) &&
        field<u8>(mpOwner->getMessageController(), 0x6C7F)) {
        nextStep();
    }
}

// 801C0098
void dSvMgr_c::stepSaveContinueNetHst_c::releaseMessageLock() {
    releaseMessage(mpOwner->getMessageController());
    nextStep();
}

// 801C0148
void dSvMgr_c::stepSaveContinueNetHst_c::waitMessageEnd() {
    void *demo = dDemo_c::mInstance;
    DemoMethod expected = &dDemo_c::fn_801A334C;
    if (isDemoMode(demo, expected)) {
        static_cast<dDemo_c *>(demo)->detachActor();
        nextStep();
    }
}

// 801C0220
void dSvMgr_c::stepSaveContinueNetHst_c::gotoPreviousScene() {
    fn_801634AC(fn_801BB7B8(), 0, 5, 0);
    nextStep();
}

// 801C02CC
void dSvMgr_c::stepSaveContinueNetHst_c::finish() {
    mpOwner->finishLoad();
}

// 801C02D4
void dSvMgr_c::stepSaveContinueNetHst_c::waitCanSave() {
    if (fn_800DDB58()) {
        nextStep();
    }
}

// 801C0378
void dSvMgr_c::stepSaveConnectNetVst_c::requestNetworkSaveMessage() {
    mpOwner->requestMessage(0x1B);
    nextStep();
}

// 801C041C
void dSvMgr_c::stepSaveConnectNetVst_c::waitMessageLock() {
    void *controller = mpOwner->getMessageController();
    DemoMethod expected = &dDemo_c::fn_801A4518;
    if (isDemoMode(controller, expected) &&
        field<u8>(mpOwner->getMessageController(), 0x6C7F)) {
        nextStep();
    }
}

// 801C0504
void dSvMgr_c::stepSaveConnectNetVst_c::releaseMessageLock() {
    releaseMessage(mpOwner->getMessageController());
    nextStep();
}

// 801C05B4
void dSvMgr_c::stepSaveConnectNetVst_c::waitMessageEnd() {
    void *demo = dDemo_c::mInstance;
    DemoMethod expected = &dDemo_c::fn_801A334C;
    if (isDemoMode(demo, expected)) {
        static_cast<dDemo_c *>(demo)->detachActor();
        nextStep();
    }
}

// 801C068C
void dSvMgr_c::stepSaveConnectNetVst_c::gotoWelcomeDemo() {
    if (fn_800DCEDC() && mCurrentVisitor < 4) {
        bool returning = false;
        void *player = fn_80101694(mCurrentVisitor);
        if ((field<u8>(player, 0x7FD4) >> 2) & 1) {
            if (field<TownIdentity>(player, 0x7FA8).isSame(
                    field<TownIdentity>(fn_8010E1E4(), 0x683FE))) {
                returning = true;
                player = &field<u8>(fn_8010E1E4(), 0x734F2);
                fn_8013E6FC(player);
                field<IdentityFlags>(player, 0x2C).state = 0;
                if (fn_800DCF90()) {
                    player = &field<u8>(fn_80101770(), 0x7FA8);
                    fn_8013E6FC(player);
                    field<IdentityFlags>(player, 0x2C).state = 0;
                }
            }
        }
        if (returning) {
            fn_80162B1C(0x41, 5, 0);
        } else {
            fn_80162B1C(0x3F, 5, 0);
        }
        nextStep();
    }
}

// 801C0824
void dSvMgr_c::stepSaveConnectNetVst_c::finish() {
    mpOwner->finishLoad();
}

// 801C082C
void dSvMgr_c::stepSaveConnectNetVst_c::waitNewPlayer() {
    if (fn_800DD2A4()) {
        fn_800D3E14();
        nextStep();
    }
}

// 801C08D4
void dSvMgr_c::stepSaveConnectNetVst_c::findFriend() {
    if (fn_800DCEDC()) {
        int visitor = mCurrentVisitor;
        if (visitor >= 4) return;
        void *player = fn_80101694(visitor);
        u64 friendCode = fn_800DDDAC(visitor);
        int index = fn_8017BBB8(&field<u8>(player, 4));
        if (index >= 0 || index == -1) {
            if (index >= 0) {
                fn_8017C3EC(index, &field<u8>(player, 0x7EC2),
                            &field<u8>(player, 0x10C8), &friendCode);
            }
            nextStep();
        }
    }
}

// 801C09D0
void dSvMgr_c::stepSaveConnectNetVst_c::waitSavePermission() {
    if (fn_800DDB08()) {
        nextStep();
    }
}

// 801C0A74
void dSvMgr_c::stepSaveConnectNetHst_c::requestNetworkSaveMessage() {
    mpOwner->requestMessage(0x1B);
    nextStep();
}

// 801C0B18
void dSvMgr_c::stepSaveConnectNetHst_c::waitMessageLock() {
    void *controller = mpOwner->getMessageController();
    DemoMethod expected = &dDemo_c::fn_801A4518;
    if (isDemoMode(controller, expected) &&
        field<u8>(mpOwner->getMessageController(), 0x6C7F)) {
        nextStep();
    }
}

// 801C0C00
void dSvMgr_c::stepSaveConnectNetHst_c::releaseMessageLock() {
    releaseMessage(mpOwner->getMessageController());
    nextStep();
}

// 801C0CB0
void dSvMgr_c::stepSaveConnectNetHst_c::waitMessageEnd() {
    void *demo = dDemo_c::mInstance;
    DemoMethod expected = &dDemo_c::fn_801A334C;
    if (isDemoMode(demo, expected)) {
        static_cast<dDemo_c *>(demo)->detachActor();
        nextStep();
    }
}

// 801C0D88
void dSvMgr_c::stepSaveConnectNetHst_c::gotoWelcomeDemo() {
    if (fn_800DCEDC() && mCurrentVisitor < 4) {
        bool returning = false;
        void *player = fn_80101694(mCurrentVisitor);
        if ((field<u8>(player, 0x7FD4) >> 2) & 1) {
            if (field<TownIdentity>(player, 0x7FA8).isSame(
                    field<TownIdentity>(fn_8010E1E4(), 0x683FE))) {
                returning = true;
                player = &field<u8>(fn_8010E1E4(), 0x734F2);
                fn_8013E6FC(player);
                field<IdentityFlags>(player, 0x2C).state = 0;
                if (fn_800DCF90()) {
                    player = &field<u8>(fn_80101770(), 0x7FA8);
                    fn_8013E6FC(player);
                    field<IdentityFlags>(player, 0x2C).state = 0;
                }
            }
        }
        if (returning) {
            fn_80162B1C(0x41, 5, 0);
        } else {
            fn_80162B1C(0x3F, 5, 0);
        }
        nextStep();
    }
}

// 801C0F20
void dSvMgr_c::stepSaveConnectNetHst_c::finish() {
    mpOwner->finishLoad();
}

// 801C0F28
void dSvMgr_c::stepSaveConnectNetHst_c::waitNewPlayer() {
    if (fn_800DD2A4()) {
        fn_800D3E14();
        nextStep();
    }
}

// 801C0FD0
void dSvMgr_c::stepSaveConnectNetHst_c::findFriend() {
    if (fn_800DCEDC()) {
        int visitor = mCurrentVisitor;
        if (visitor >= 4) return;
        void *player = fn_80101694(visitor);
        u64 friendCode = fn_800DDDAC(visitor);
        int index = fn_8017BBB8(&field<u8>(player, 4));
        if (index >= 0 || index == -1) {
            if (index >= 0) {
                fn_8017C3EC(index, &field<u8>(player, 0x7EC2),
                            &field<u8>(player, 0x10C8), &friendCode);
            }
            nextStep();
        }
    }
}

// 801C10CC
void dSvMgr_c::stepSaveConnectNetHst_c::waitSavePermission() {
    if (fn_800DDB08()) {
        nextStep();
    }
}

// 801C1170
void dSvMgr_c::stepSaveRetireNetVst_c::requestNetworkSaveMessage() {
    mpOwner->requestMessage(0x1B);
    nextStep();
}

// 801C1214
void dSvMgr_c::stepSaveRetireNetVst_c::waitMessageLock() {
    void *controller = mpOwner->getMessageController();
    DemoMethod expected = &dDemo_c::fn_801A4518;
    if (isDemoMode(controller, expected) &&
        field<u8>(mpOwner->getMessageController(), 0x6C7F)) {
        nextStep();
    }
}

// 801C12FC
void dSvMgr_c::stepSaveRetireNetVst_c::releaseMessageLock() {
    releaseMessage(mpOwner->getMessageController());
    nextStep();
}

// 801C13AC
void dSvMgr_c::stepSaveRetireNetVst_c::waitMessageEnd() {
    void *demo = dDemo_c::mInstance;
    DemoMethod expected = &dDemo_c::fn_801A334C;
    if (isDemoMode(demo, expected)) {
        static_cast<dDemo_c *>(demo)->detachActor();
        nextStep();
    }
}

// 801C1484
void dSvMgr_c::stepSaveRetireNetVst_c::gotoOutsideGate() {
    fn_801634AC(fn_801BB7B8(), 1, 5, 0);
    nextStep();
}

// 801C1530
void dSvMgr_c::stepSaveRetireNetVst_c::finish() {
    mpOwner->finishLoad();
}

// 801C1538
void dSvMgr_c::stepSaveRetireNetVst_c::processLoadedSave() {
    void *demo = dDemo_c::mInstance;
    switch (fn_800D2A44()) {
    case 1:
        if (!fn_8010E0F8(fn_8010DC3C(), 1)) {
            releaseMessage(demo);
            fn_801A4E44(demo, 0x13);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveRetireNetVst_c::saveFailed;
        } else if (!fn_8010DC44(fn_8010DC18())) {
            releaseMessage(demo);
            fn_801A4E44(demo, 0x13);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveRetireNetVst_c::saveFailed;
        } else {
            u8 savedIdentity[40];
            fn_800DCF58();
            int index = mPlayerIndex;
            fn_801017B8();
            void *player = fn_80101624(index);
            fn_8010DAFC(&field<u8>(player, 0x83E8));
            fn_80136C7C(fn_80101770(), player);
            fn_8010DC18();
            fn_8010DFC0();
            void *save = fn_8010DC3C();
            fn_8014C2C4(savedIdentity, &field<u8>(save, 0x73522));
            nextStep();
        }
        break;
    case 2:
        releaseMessage(demo);
        fn_801A4E44(demo, 0x0C);
        fn_801A4E34(demo, mMessageLabel);
        mCurrentMethod = &dSvMgr_c::stepSaveRetireNetVst_c::saveFailed;
        break;
    default:
        releaseMessage(demo);
        fn_801A4E44(demo, 0x13);
        fn_801A4E34(demo, mMessageLabel);
        mCurrentMethod = &dSvMgr_c::stepSaveRetireNetVst_c::saveFailed;
    }
}

// 801C178C
void dSvMgr_c::stepSaveRetireNetVst_c::initializeGrowth() {
    fn_80154358(lbl_8074E7F8);
    nextStep();
}

// 801C182C
void dSvMgr_c::stepSaveRetireNetVst_c::processGrowth() {
    if (fn_8015436C(lbl_8074E7F8, 4)) {
        nextStep();
    }
}

// 801C18D8
void dSvMgr_c::stepSaveNormal_c::requestSaveMessage() {
    mpOwner->requestMessage(0x0D);
    nextStep();
}

// 801C197C
void dSvMgr_c::stepSaveNormal_c::waitMessageLock() {
    void *controller = mpOwner->getMessageController();
    DemoMethod expected = &dDemo_c::fn_801A4518;
    if (isDemoMode(controller, expected) &&
        field<u8>(mpOwner->getMessageController(), 0x6C7F)) {
        nextStep();
    }
}

// 801C1A64
void dSvMgr_c::stepSaveNormal_c::releaseMessageLock() {
    releaseMessage(mpOwner->getMessageController());
    nextStep();
}

// 801C1B14
void dSvMgr_c::stepSaveNormal_c::waitMessageEnd() {
    void *demo = dDemo_c::mInstance;
    DemoMethod expected = &dDemo_c::fn_801A334C;
    if (isDemoMode(demo, expected)) {
        static_cast<dDemo_c *>(demo)->detachActor();
        nextStep();
    }
}

// 801C1BEC
void dSvMgr_c::stepSaveNormal_c::gotoTitle() {
    fn_80162B1C(0x38, 5, 2);
    nextStep();
}

// 801C1C94
void dSvMgr_c::stepSaveNormal_c::finishFinal() {
    mpOwner->finishLoad();
}

// 801C1C9C
void dSvMgr_c::clearRequestedMode() {
    mRequestedMode = NULL;
}

// 801C30C0
void dSvMgr_c::stepLoad_c::initializeLoad() {
    nextStep();
}

// 801C3158
void dSvMgr_c::stepLoad_c::startLoad() {
    if (fn_800D27E0()) {
        nextStep();
    }
}

// 801C31FC
void dSvMgr_c::stepLoad_c::waitLoad() {
    if (fn_800D2A4C()) {
        nextStep();
    }
}

// 801C32A0
void dSvMgr_c::stepSaveInterruptNetVst_c::initializeLoad() {
    nextStep();
}

// 801C3338
void dSvMgr_c::stepSaveInterruptNetVst_c::startLoad() {
    if (fn_800D27E0()) {
        nextStep();
    }
}

// 801C33DC
void dSvMgr_c::stepSaveInterruptNetVst_c::waitLoad() {
    if (fn_800D2A4C()) {
        nextStep();
    }
}

// 801C3480
void dSvMgr_c::stepSaveInterruptNetVst_c::initializeFullSave() {
    fn_8010DCF0(fn_8010DC18());
    fn_8016D53C();
    fn_800D22F8();
    nextStep();
}

// 801C3528
void dSvMgr_c::stepSaveInterruptNetVst_c::startFullSave() {
    if (fn_800D2780()) {
        nextStep();
    }
}

// 801C35CC
void dSvMgr_c::stepSaveInterruptNetVst_c::waitFullSave() {
    checkSaveResult();
}

// 801C35D0
void dSvMgr_c::stepSaveInterruptNetVst_c::initializeDownloadBox() {
    if (fn_800D34C4(lbl_8074B010) == 1) {
        nextStep();
    } else if (fn_801781D8()) {
        nextStep();
    }
}

// 801C36E8
void dSvMgr_c::stepSaveInterruptNetVst_c::createDownloadBox() {
    if (fn_800D34C4(lbl_8074B010) == 1) {
        nextStep();
    } else {
        void *demo = dDemo_c::mInstance;
        switch (fn_80178374()) {
        case -1:
            break;
        case 0:
            if (fn_80178448() == 0x0C) {
                releaseMessage(demo);
                fn_801A4E44(demo, 0x0C);
                fn_801A4E34(demo, mMessageLabel);
                mCurrentMethod =
                    &dSvMgr_c::stepSaveInterruptNetVst_c::saveFailed;
            } else {
                releaseMessage(demo);
                fn_801A4E44(demo, 0x13);
                fn_801A4E34(demo, mMessageLabel);
                mCurrentMethod =
                    &dSvMgr_c::stepSaveInterruptNetVst_c::saveFailed;
            }
            break;
        case 1:
            nextStep();
            break;
        }
    }
}

// 801C38B4
void dSvMgr_c::stepSaveInterruptNetVst_c::disableHomeAndReset() {
    field<u32>(fn_8017D8E8(), 0x1AC) |= 0x200;
    fn_80106988();
    fn_80106F18();
    nextStep();
}

// 801C3964
void dSvMgr_c::stepSaveInterruptNetVst_c::enableHomeAndReset() {
    enableReset();
    nextStep();
}

// 801C3A14
void dSvMgr_c::stepSaveInterruptNetVst_c::saveFailed() {
}

// 801C3A18
void dSvMgr_c::stepSaveInterruptNetHst_c::initializeFullSave() {
    fn_8010DCF0(fn_8010DC18());
    fn_8016D53C();
    fn_800D22F8();
    nextStep();
}

// 801C3AC0
void dSvMgr_c::stepSaveInterruptNetHst_c::startFullSave() {
    if (fn_800D2780()) {
        nextStep();
    }
}

// 801C3B64
void dSvMgr_c::stepSaveInterruptNetHst_c::waitFullSave() {
    checkSaveResult();
}

// 801C3B68
void dSvMgr_c::stepSaveInterruptNetHst_c::initializeDownloadBox() {
    if (fn_800D34C4(lbl_8074B010) == 1) {
        nextStep();
    } else if (fn_801781D8()) {
        nextStep();
    }
}

// 801C3C80
void dSvMgr_c::stepSaveInterruptNetHst_c::createDownloadBox() {
    if (fn_800D34C4(lbl_8074B010) == 1) {
        nextStep();
    } else {
        void *demo = dDemo_c::mInstance;
        switch (fn_80178374()) {
        case -1:
            break;
        case 0:
            if (fn_80178448() == 0x0C) {
                releaseMessage(demo);
                fn_801A4E44(demo, 0x0C);
                fn_801A4E34(demo, mMessageLabel);
                mCurrentMethod =
                    &dSvMgr_c::stepSaveInterruptNetHst_c::saveFailed;
            } else {
                releaseMessage(demo);
                fn_801A4E44(demo, 0x13);
                fn_801A4E34(demo, mMessageLabel);
                mCurrentMethod =
                    &dSvMgr_c::stepSaveInterruptNetHst_c::saveFailed;
            }
            break;
        case 1:
            nextStep();
            break;
        }
    }
}

// 801C3E4C
void dSvMgr_c::stepSaveInterruptNetHst_c::disableHomeAndReset() {
    field<u32>(fn_8017D8E8(), 0x1AC) |= 0x200;
    fn_80106988();
    fn_80106F18();
    nextStep();
}

// 801C3EFC
void dSvMgr_c::stepSaveInterruptNetHst_c::enableHomeAndReset() {
    enableReset();
    nextStep();
}

// 801C3FAC
void dSvMgr_c::stepSaveInterruptNetHst_c::saveFailed() {
}

// 801C3FB0
void dSvMgr_c::stepSaveContinueNetVst_c::initializeVisitorSave() {
    fn_8010DAC8();
    nextStep();
}

// 801C404C
void dSvMgr_c::stepSaveContinueNetVst_c::startVisitorSave() {
    if (fn_800D25D8()) {
        nextStep();
    }
}

// 801C40F0
void dSvMgr_c::stepSaveContinueNetVst_c::waitVisitorSave() {
    checkSaveResult();
}

// 801C40F4
void dSvMgr_c::stepSaveContinueNetVst_c::saveFailed() {
}

// 801C40F8
void dSvMgr_c::stepSaveContinueNetHst_c::initializeFullSave() {
    fn_8010DCF0(fn_8010DC18());
    fn_8016D53C();
    fn_800D22F8();
    nextStep();
}

// 801C41A0
void dSvMgr_c::stepSaveContinueNetHst_c::startFullSave() {
    if (fn_800D2780()) {
        nextStep();
    }
}

// 801C4244
void dSvMgr_c::stepSaveContinueNetHst_c::waitFullSave() {
    checkSaveResult();
}

// 801C4248
void dSvMgr_c::stepSaveContinueNetHst_c::initializeDownloadBox() {
    if (fn_800D34C4(lbl_8074B010) == 1) {
        nextStep();
    } else if (fn_801781D8()) {
        nextStep();
    }
}

// 801C4360
void dSvMgr_c::stepSaveContinueNetHst_c::createDownloadBox() {
    if (fn_800D34C4(lbl_8074B010) == 1) {
        nextStep();
    } else {
        void *demo = dDemo_c::mInstance;
        switch (fn_80178374()) {
        case -1:
            break;
        case 0:
            if (fn_80178448() == 0x0C) {
                releaseMessage(demo);
                fn_801A4E44(demo, 0x0C);
                fn_801A4E34(demo, mMessageLabel);
                mCurrentMethod =
                    &dSvMgr_c::stepSaveContinueNetHst_c::saveFailed;
            } else {
                releaseMessage(demo);
                fn_801A4E44(demo, 0x13);
                fn_801A4E34(demo, mMessageLabel);
                mCurrentMethod =
                    &dSvMgr_c::stepSaveContinueNetHst_c::saveFailed;
            }
            break;
        case 1:
            nextStep();
            break;
        }
    }
}

// 801C452C
void dSvMgr_c::stepSaveContinueNetHst_c::saveFailed() {
}

// 801C4530
void dSvMgr_c::stepSaveConnectNetVst_c::initializeVisitorSave() {
    fn_8010DAC8();
    nextStep();
}

// 801C45CC
void dSvMgr_c::stepSaveConnectNetVst_c::startVisitorSave() {
    if (fn_800D25D8()) {
        nextStep();
    }
}

// 801C4670
void dSvMgr_c::stepSaveConnectNetVst_c::waitVisitorSave() {
    checkSaveResult();
}

// 801C4674
void dSvMgr_c::stepSaveConnectNetVst_c::saveFailed() {
}

// 801C4678
void dSvMgr_c::stepSaveConnectNetHst_c::initializeHostSave() {
    fn_8010DAC8();
    nextStep();
}

// 801C4714
void dSvMgr_c::stepSaveConnectNetHst_c::startHostSave() {
    if (fn_800D25D8()) {
        nextStep();
    }
}

// 801C47B8
void dSvMgr_c::stepSaveConnectNetHst_c::waitHostSave() {
    checkSaveResult();
}

// 801C47BC
void dSvMgr_c::stepSaveConnectNetHst_c::initializeTownSave() {
    fn_8010DCF0(fn_8010DC18());
    nextStep();
}

// 801C485C
void dSvMgr_c::stepSaveConnectNetHst_c::startTownSave() {
    if (fn_800D2720()) {
        nextStep();
    }
}

// 801C4900
void dSvMgr_c::stepSaveConnectNetHst_c::waitTownSave() {
    checkSaveResult();
}

// 801C4904
void dSvMgr_c::stepSaveConnectNetHst_c::saveFailed() {
}

// 801C4908
void dSvMgr_c::stepSaveRetireNetVst_c::initializeLoad() {
    nextStep();
}

// 801C49A0
void dSvMgr_c::stepSaveRetireNetVst_c::startLoad() {
    if (fn_800D27E0()) {
        nextStep();
    }
}

// 801C4A44
void dSvMgr_c::stepSaveRetireNetVst_c::waitLoad() {
    if (fn_800D2A4C()) {
        nextStep();
    }
}

// 801C4AE8
void dSvMgr_c::stepSaveRetireNetVst_c::initializeFullSave() {
    fn_8010DCF0(fn_8010DC18());
    fn_8016D53C();
    fn_800D22F8();
    nextStep();
}

// 801C4B90
void dSvMgr_c::stepSaveRetireNetVst_c::startFullSave() {
    if (fn_800D2780()) {
        nextStep();
    }
}

// 801C4C34
void dSvMgr_c::stepSaveRetireNetVst_c::waitFullSave() {
    checkSaveResult();
}

// 801C4C38
void dSvMgr_c::stepSaveRetireNetVst_c::initializeDownloadBox() {
    if (fn_800D34C4(lbl_8074B010) == 1) {
        nextStep();
    } else if (fn_801781D8()) {
        nextStep();
    }
}

// 801C4D50
void dSvMgr_c::stepSaveRetireNetVst_c::createDownloadBox() {
    if (fn_800D34C4(lbl_8074B010) == 1) {
        nextStep();
    } else {
        void *demo = dDemo_c::mInstance;
        switch (fn_80178374()) {
        case -1:
            break;
        case 0:
            if (fn_80178448() == 0x0C) {
                releaseMessage(demo);
                fn_801A4E44(demo, 0x0C);
                fn_801A4E34(demo, mMessageLabel);
                mCurrentMethod = &dSvMgr_c::stepSaveRetireNetVst_c::saveFailed;
            } else {
                releaseMessage(demo);
                fn_801A4E44(demo, 0x13);
                fn_801A4E34(demo, mMessageLabel);
                mCurrentMethod = &dSvMgr_c::stepSaveRetireNetVst_c::saveFailed;
            }
            break;
        case 1:
            nextStep();
            break;
        }
    }
}

// 801C4F1C
void dSvMgr_c::stepSaveRetireNetVst_c::disableHomeAndReset() {
    field<u32>(fn_8017D8E8(), 0x1AC) |= 0x200;
    fn_80106988();
    fn_80106F18();
    nextStep();
}

// 801C4FCC
void dSvMgr_c::stepSaveRetireNetVst_c::enableHomeAndReset() {
    enableReset();
    nextStep();
}

// 801C507C
void dSvMgr_c::stepSaveRetireNetVst_c::saveFailed() {
}

// 801C5080
void dSvMgr_c::stepSaveNormal_c::initializeFullSave() {
    fn_8010DCF0(fn_8010DC18());
    fn_8016D53C();
    fn_800D22F8();
    nextStep();
}

// 801C5128
void dSvMgr_c::stepSaveNormal_c::startFullSave() {
    if (fn_800D2780()) {
        nextStep();
    }
}

// 801C51CC
void dSvMgr_c::stepSaveNormal_c::waitFullSave() {
    checkSaveResult();
}

// 801C51D0
void dSvMgr_c::stepSaveNormal_c::initializeDownloadBox() {
    if (fn_800D34C4(lbl_8074B010) == 1) {
        nextStep();
    } else if (fn_801781D8()) {
        nextStep();
    }
}

// 801C52E8
void dSvMgr_c::stepSaveNormal_c::createDownloadBox() {
    if (fn_800D34C4(lbl_8074B010) == 1) {
        nextStep();
    } else {
        void *demo = dDemo_c::mInstance;
        switch (fn_80178374()) {
        case -1:
            break;
        case 0:
            if (fn_80178448() == 0x0C) {
                releaseMessage(demo);
                fn_801A4E44(demo, 0x0C);
                fn_801A4E34(demo, mMessageLabel);
                mCurrentMethod = &dSvMgr_c::stepSaveNormal_c::saveFailed;
            } else {
                releaseMessage(demo);
                fn_801A4E44(demo, 0x13);
                fn_801A4E34(demo, mMessageLabel);
                mCurrentMethod = &dSvMgr_c::stepSaveNormal_c::saveFailed;
            }
            break;
        case 1:
            nextStep();
            break;
        }
    }
}

// 801C54B4
void dSvMgr_c::stepSaveNormal_c::disableHomeAndReset() {
    field<u32>(fn_8017D8E8(), 0x1AC) |= 0x200;
    fn_80106988();
    fn_80106F18();
    nextStep();
}

// 801C5564
void dSvMgr_c::stepSaveNormal_c::enableHomeAndReset() {
    enableReset();
    nextStep();
}

// 801C5614
void dSvMgr_c::stepSaveNormal_c::saveFailed() {
}

inline void dSvMgr_c::setLoadErrorMessage(u16 code) {
    sLoadErrorMessage = code;
}

// 801C5618
void dSvMgr_c::stepSaveInterruptNetVst_c::checkSaveResult() {
    void *demo = dDemo_c::mInstance;
    if (fn_800D2A4C()) {
        switch (fn_800D2A44()) {
        case 1:
            nextStep();
            break;
        case 2:
            releaseMessage(demo);
            fn_801A4E44(demo, 0x0C);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveInterruptNetVst_c::saveFailed;
            break;
        case 7:
            enableReset();
            releaseMessage(demo);
            fn_801A4E44(demo, 8);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveInterruptNetVst_c::saveFailed;
            break;
        case 8:
            enableReset();
            releaseMessage(demo);
            fn_801A4E44(demo, 9);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveInterruptNetVst_c::saveFailed;
            break;
        default:
            enableReset();
            releaseMessage(demo);
            fn_801A4E44(demo, 0x13);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveInterruptNetVst_c::saveFailed;
            break;
        }
    }
}

// 801C5864
void dSvMgr_c::stepSaveInterruptNetHst_c::checkSaveResult() {
    void *demo = dDemo_c::mInstance;
    if (fn_800D2A4C()) {
        switch (fn_800D2A44()) {
        case 1:
            nextStep();
            break;
        case 2:
            releaseMessage(demo);
            fn_801A4E44(demo, 0x0C);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveInterruptNetHst_c::saveFailed;
            break;
        case 7:
            enableReset();
            releaseMessage(demo);
            fn_801A4E44(demo, 8);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveInterruptNetHst_c::saveFailed;
            break;
        case 8:
            enableReset();
            releaseMessage(demo);
            fn_801A4E44(demo, 9);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveInterruptNetHst_c::saveFailed;
            break;
        default:
            enableReset();
            releaseMessage(demo);
            fn_801A4E44(demo, 0x13);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveInterruptNetHst_c::saveFailed;
            break;
        }
    }
}

// 801C5AB0
void dSvMgr_c::stepSaveContinueNetVst_c::checkSaveResult() {
    void *demo = dDemo_c::mInstance;
    if (fn_800D2A4C()) {
        switch (fn_800D2A44()) {
        case 1:
            nextStep();
            break;
        case 2:
            releaseMessage(demo);
            fn_801A4E44(demo, 0x0C);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveContinueNetVst_c::saveFailed;
            break;
        case 7:
            enableReset();
            releaseMessage(demo);
            fn_801A4E44(demo, 8);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveContinueNetVst_c::saveFailed;
            break;
        case 8:
            enableReset();
            releaseMessage(demo);
            fn_801A4E44(demo, 9);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveContinueNetVst_c::saveFailed;
            break;
        default:
            enableReset();
            releaseMessage(demo);
            fn_801A4E44(demo, 0x13);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveContinueNetVst_c::saveFailed;
            break;
        }
    }
}

// 801C5CFC
void dSvMgr_c::stepSaveContinueNetHst_c::checkSaveResult() {
    void *demo = dDemo_c::mInstance;
    if (fn_800D2A4C()) {
        switch (fn_800D2A44()) {
        case 1:
            nextStep();
            break;
        case 2:
            releaseMessage(demo);
            fn_801A4E44(demo, 0x0C);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveContinueNetHst_c::saveFailed;
            break;
        case 7:
            enableReset();
            releaseMessage(demo);
            fn_801A4E44(demo, 8);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveContinueNetHst_c::saveFailed;
            break;
        case 8:
            enableReset();
            releaseMessage(demo);
            fn_801A4E44(demo, 9);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveContinueNetHst_c::saveFailed;
            break;
        default:
            enableReset();
            releaseMessage(demo);
            fn_801A4E44(demo, 0x13);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveContinueNetHst_c::saveFailed;
            break;
        }
    }
}

// 801C5F48
void dSvMgr_c::stepSaveConnectNetVst_c::checkSaveResult() {
    void *demo = dDemo_c::mInstance;
    if (fn_800D2A4C()) {
        switch (fn_800D2A44()) {
        case 1:
            nextStep();
            break;
        case 2:
            releaseMessage(demo);
            fn_801A4E44(demo, 0x0C);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveConnectNetVst_c::saveFailed;
            break;
        case 7:
            enableReset();
            releaseMessage(demo);
            fn_801A4E44(demo, 8);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveConnectNetVst_c::saveFailed;
            break;
        case 8:
            enableReset();
            releaseMessage(demo);
            fn_801A4E44(demo, 9);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveConnectNetVst_c::saveFailed;
            break;
        default:
            enableReset();
            releaseMessage(demo);
            fn_801A4E44(demo, 0x13);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveConnectNetVst_c::saveFailed;
            break;
        }
    }
}

// 801C6194
void dSvMgr_c::stepSaveConnectNetHst_c::checkSaveResult() {
    void *demo = dDemo_c::mInstance;
    if (fn_800D2A4C()) {
        switch (fn_800D2A44()) {
        case 1:
            nextStep();
            break;
        case 2:
            releaseMessage(demo);
            fn_801A4E44(demo, 0x0C);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveConnectNetHst_c::saveFailed;
            break;
        case 7:
            enableReset();
            releaseMessage(demo);
            fn_801A4E44(demo, 8);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveConnectNetHst_c::saveFailed;
            break;
        case 8:
            enableReset();
            releaseMessage(demo);
            fn_801A4E44(demo, 9);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveConnectNetHst_c::saveFailed;
            break;
        default:
            enableReset();
            releaseMessage(demo);
            fn_801A4E44(demo, 0x13);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveConnectNetHst_c::saveFailed;
            break;
        }
    }
}

// 801C63E0
void dSvMgr_c::stepSaveRetireNetVst_c::checkSaveResult() {
    void *demo = dDemo_c::mInstance;
    if (fn_800D2A4C()) {
        switch (fn_800D2A44()) {
        case 1:
            nextStep();
            break;
        case 2:
            releaseMessage(demo);
            fn_801A4E44(demo, 0x0C);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveRetireNetVst_c::saveFailed;
            break;
        case 7:
            enableReset();
            releaseMessage(demo);
            fn_801A4E44(demo, 8);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveRetireNetVst_c::saveFailed;
            break;
        case 8:
            enableReset();
            releaseMessage(demo);
            fn_801A4E44(demo, 9);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveRetireNetVst_c::saveFailed;
            break;
        default:
            enableReset();
            releaseMessage(demo);
            fn_801A4E44(demo, 0x13);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveRetireNetVst_c::saveFailed;
            break;
        }
    }
}

// 801C662C
void dSvMgr_c::stepSaveNormal_c::checkSaveResult() {
    void *demo = dDemo_c::mInstance;
    if (fn_800D2A4C()) {
        switch (fn_800D2A44()) {
        case 1:
            nextStep();
            break;
        case 2:
            releaseMessage(demo);
            fn_801A4E44(demo, 0x0C);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveNormal_c::saveFailed;
            break;
        case 7:
            enableReset();
            releaseMessage(demo);
            fn_801A4E44(demo, 8);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveNormal_c::saveFailed;
            break;
        case 8:
            enableReset();
            releaseMessage(demo);
            fn_801A4E44(demo, 9);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveNormal_c::saveFailed;
            break;
        default:
            enableReset();
            releaseMessage(demo);
            fn_801A4E44(demo, 0x13);
            fn_801A4E34(demo, mMessageLabel);
            mCurrentMethod = &dSvMgr_c::stepSaveNormal_c::saveFailed;
            break;
        }
    }
}

dSvMgr_c::~dSvMgr_c() {
}

// Original demo callbacks; only their addresses are used here.
extern "C" {
}
