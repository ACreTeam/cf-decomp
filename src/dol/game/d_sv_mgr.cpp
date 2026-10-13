#include <cstring>
#include <game/game/d_str.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_sv_mgr.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_sv_runtime.hpp>
#include <game/mLib/m_fader.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_post_office.hpp>
#include <game/game/d_wifi.hpp>

// The deferred message/load templates stay in this file. Save callbacks and
// destructors have separate definition files so -sym on places their weak
// code sections after the state framework, while retaining the PTMF pool order.

using namespace dSvRuntime;

// Seed deferred result specializations in declaration order. These unused
// address anchors are discarded by the linker.
static void (*const instantiate_stepSaveNormal_c)(dSvMgr_c::stepSaveNormal_c *) =
    &checkSaveResultImpl<dSvMgr_c::stepSaveNormal_c>;
static void (*const instantiate_stepSaveRetireNetVst_c)(dSvMgr_c::stepSaveRetireNetVst_c *) =
    &checkSaveResultImpl<dSvMgr_c::stepSaveRetireNetVst_c>;
static void (*const instantiate_stepSaveConnectNetHst_c)(dSvMgr_c::stepSaveConnectNetHst_c *) =
    &checkSaveResultImpl<dSvMgr_c::stepSaveConnectNetHst_c>;
static void (*const instantiate_stepSaveConnectNetVst_c)(dSvMgr_c::stepSaveConnectNetVst_c *) =
    &checkSaveResultImpl<dSvMgr_c::stepSaveConnectNetVst_c>;
static void (*const instantiate_stepSaveContinueNetHst_c)(dSvMgr_c::stepSaveContinueNetHst_c *) =
    &checkSaveResultImpl<dSvMgr_c::stepSaveContinueNetHst_c>;
static void (*const instantiate_stepSaveContinueNetVst_c)(dSvMgr_c::stepSaveContinueNetVst_c *) =
    &checkSaveResultImpl<dSvMgr_c::stepSaveContinueNetVst_c>;
static void (*const instantiate_stepSaveInterruptNetHst_c)(dSvMgr_c::stepSaveInterruptNetHst_c *) =
    &checkSaveResultImpl<dSvMgr_c::stepSaveInterruptNetHst_c>;
static void (*const instantiate_stepSaveInterruptNetVst_c)(dSvMgr_c::stepSaveInterruptNetVst_c *) =
    &checkSaveResultImpl<dSvMgr_c::stepSaveInterruptNetVst_c>;

dSvMgr_c::Method dSvMgr_c::mRequestedMode = NULL;
int dSvMgr_c::mVisitors[3];
int dSvMgr_c::mCurrentVisitor = 4;
int dSvMgr_c::mPlayerIndex = 3;
const char *dSvMgr_c::mMessageLabel = "SYS_MESS/SYS_Save_and_Load";
// Unreferenced in the DOL and RELs; its original name is unknown.
const char *lbl_8074C3CC = "SYS_MESS/SYS_NetworkError";

u16 sLoadErrorMessage;

dSvMgr_c *dSvMgr_c::createMgr() {
    return new dSvMgr_c;
}

void dSvMgr_c::requestSaveNormal() {
    mRequestedMode = &dSvMgr_c::executeSaveNormal;
}

void dSvMgr_c::requestSaveRetireNetVst() {
    dNet::getMyNo();
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
    dNet::getMyNo();
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
    if (dNand::isWriting() &&
        static_cast<unsigned int>(offset) ==
            static_cast<unsigned int>(dNand::getRequestOffset()) &&
        static_cast<unsigned int>(size) ==
            static_cast<unsigned int>(dNand::getRequestSize())) {
        return 1;
    }
    return 0;
}

int dSvMgr_c::isFullTransferComplete() {
    return isTransferComplete(0, lbl_80750AD0);
}

int dSvMgr_c::isTownTransferComplete() {
    dSaveData_c::getRaw();
    return isTransferComplete(0, dSaveData_c::getDLDataOffset());
}

int dSvMgr_c::isDLDataTransferComplete() {
    return isTransferComplete(getDLDataOffset(), offsetof(dSaveDLItemList_c, _200004)); // checksum + slots
}

int dSvMgr_c::isPlayerTransferComplete(int player) {
    return isTransferComplete(getPlayerDataOffset(player), 0x86C0);
}

int dSvMgr_c::getDLDataOffset() {
    dSaveData_c *base = dSaveData_c::getRaw();
    u8 *data = (u8 *)&dSaveData_c::getRaw()->mDLItems;
    return data - reinterpret_cast<u8 *>(base);
}

int dSvMgr_c::getVisitorDataOffset() {
    dSaveData_c *base = dSaveData_c::getRaw();
    return reinterpret_cast<int>(&dSaveData_c::getRaw()->mAnimals) -
           reinterpret_cast<int>(base);
}

int dSvMgr_c::getPlayerDataOffset(int player) {
    dSaveData_c *base = dSaveData_c::getRaw();
    return reinterpret_cast<int>(dPlayerMgr_c::getPlayerRaw(player)) -
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
    if (dScene::getCurrentScene() == dScene::SCENE_DM_LOAD) {
        fn_80154358(lbl_8074E7F8);
        changeMode(&dSvMgr_c::executeLoad);
    }
    field<u32>(fn_8017D8E8(), 0x1AC) |= 0x200;
    return SUCCEEDED;
}

int dSvMgr_c::doDelete() {
    if (!dNand::isIdle()) {
        return NOT_READY;
    }
    if (dScene::getCurrentScene() == dScene::SCENE_DM_LOAD) {
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
    dMsg::Comp_c *demo = dMsg::Comp_c::mInstance;
    setMessageLabel(mMessageLabel);
    setMessageCode(code);
    demo->attachActor(this);
    demo->requestStart(0);
}

void dSvMgr_c::onMessageEnd(int kind) {
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
    dMsg::Comp_c *controller = mpController;
    controller->mLock = 1;
    controller->mWaitIconSe = 0;
    controller->mWaitIcon = 1;
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
    if (dNet::isSessionActive() == 1) {
        dSvMgr_c::closeGateForSave();
    }
    nextStep();
}

// 801BC634
void dSvMgr_c::stepSaveNormal_c::disconnectWiFi() {
    if (dNet::disconnectNet(0)) {
        nextStep();
    }
}

// 801BC6DC
void dSvMgr_c::stepSaveNormal_c::startWC24Send() {
    if (dWifi::startDaily()) {
        nextStep();
    }
}

// 801BC780
void dSvMgr_c::stepSaveNormal_c::waitWC24Send() {
    if (dWifi::getThreadResult() != -1) {
        nextStep();
    }
}

// 801BC824
void dSvMgr_c::stepSaveNormal_c::startVillageMail() {
    dPostOffice::startSend();
    nextStep();
}

// 801BC8C0
void dSvMgr_c::stepSaveNormal_c::waitVillageMail() {
    if (dPostOffice::updateSend()) {
        dSaveData_c::get()->updateOnSave();
        nextStep();
    }
}

// 801BC96C
void dSvMgr_c::stepSaveNormal_c::finish() {
    dSaveData_c::get();
    fn_8010DED0();
    mpOwner->getMessageController()->setNextMsgCode(0x0E);
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
    if (dNet::getMemberNum() == 1) {
        dPostOffice::deliverAll();
        dPostOffice::deliverAll();
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
    if (dNet::isSaveEndAllSaved()) {
        dNet::beginPacket();
        dNet::sendPacket(NET_PACKET_SAVE_DONE_ALL, NET_MEMBER_NONE);
        dNet::resetSaveEnd();
        dNet::clearRequestMember();
        dNet::releaseRequest();
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
    dNet::beginPacket();
    dNet::sendPacket(NET_PACKET_SAVE_DONE, 0);
    nextStep();
}

// 801BDAF0
void dSvMgr_c::stepSaveContinueNetVst_c::waitSaveDemoEnd() {
    if (dNet::isSaveEndSaved()) {
        dNet::resetSaveEnd();
        dNet::clearRequestMember();
        dNet::releaseRequest();
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
    dItem::Item item((u16)0xFFF1);
    dSTR::unlock(&item, 1);
    dSTR::unlock(&item, 2);
    dSTR::unlock(&item, 3);
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
        dSaveData_c::get()->clear();
        dSaveData_c::get()->initialize();
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
    switch (dNand::getResult()) {
    case 1:
        if (!dSaveData_c::getRaw()->isExtraGood(1)) {
            mpOwner->setLoadFailed();
            setLoadErrorMessage(2);
        } else if (!dSaveData_c::get()->isGood()) {
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
    int auxiliary = dWifi::getNandAuxSize();
    int resource = dWifi::getNandResourceSize();
    return (resource + lbl_80750AD0) + (auxiliary + dBanner_c::sFileSize);
}
int dSvMgr_c::getRequiredBlocks() {
    static int blocks = blocksForSize(requiredSaveSize());
    return blocks;
}

int dSvMgr_c::getRequiredInodes() {
    static int save = inodesForSize(lbl_80750AD0);
    static int extra = inodesForSize(dBanner_c::sFileSize);
    static int resource = inodesForSize(dWifi::getNandResourceSize());
    static int auxiliary = inodesForSize(dWifi::getNandAuxSize());
    static int total = save + extra + resource + auxiliary;
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
    dScene::requestSceneChange(dScene::SCENE_DM_TITLE, 5, 2);
    nextStep();
}

// 801BECD4
void dSvMgr_c::stepLoad_c::finish() {
    mpOwner->finishLoad();
}

template <class A, class B> struct SvStepIs { enum { value = 0 }; };
template <class A> struct SvStepIs<A, A> { enum { value = 1 }; };

template <class Owner, class Step>
void dSvMgr_c::stepBase_c<Owner, Step>::requestSaveMessage() {
    this->mpOwner->requestMessage(SvStepIs<Step, stepSaveNormal_c>::value ? 0x0D : 0x1B);
    this->nextStep();
}

template <class Owner, class Step>
void dSvMgr_c::stepBase_c<Owner, Step>::requestNetworkSaveMessage() {
    this->mpOwner->requestMessage(0x1B);
    this->nextStep();
}

template <class Owner, class Step>
void dSvMgr_c::stepBase_c<Owner, Step>::waitMessageLock() {
    dMsg::Comp_c *controller = this->mpOwner->getMessageController();
    DemoMethod expected = &dMsg::Comp_c::executeComplete;
    if (isDemoMode(controller, expected) &&
        this->mpOwner->getMessageController()->mLock) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dSvMgr_c::stepBase_c<Owner, Step>::releaseMessageLock() {
    releaseMessage(this->mpOwner->getMessageController());
    this->nextStep();
}

template <class Owner, class Step>
void dSvMgr_c::stepBase_c<Owner, Step>::waitMessageEnd() {
    dMsg::Comp_c *demo = dMsg::Comp_c::mInstance;
    DemoMethod expected = &dMsg::Comp_c::executeHidden;
    if (isDemoMode(demo, expected)) {
        static_cast<dMsg::Comp_c *>(demo)->detachActor();
        this->nextStep();
    }
}

template <class Owner, class Step>
void dSvMgr_c::stepBase_c<Owner, Step>::gotoOutsideGate() {
    fn_801BB7B8()->requestExit(1, 5, 0);
    this->nextStep();
}

template <class Owner, class Step>
void dSvMgr_c::stepBase_c<Owner, Step>::gotoPreviousScene() {
    fn_801BB7B8()->requestExit(0, 5, 0);
    this->nextStep();
}

template <class Owner, class Step>
void dSvMgr_c::stepBase_c<Owner, Step>::gotoWelcomeDemo() {
    if (dNet::isSessionActive() && mCurrentVisitor < 4) {
        bool returning = false;
        void *player = dPlayerMgr_c::getNetPlayer(mCurrentVisitor);
        if ((field<u8>(player, 0x7FD4) >> 2) & 1) {
            if (field<TownIdentity>(player, 0x7FA8).isSame(
                    field<TownIdentity>(&dSaveData_c::getTown()->mLandID, 0))) {
                returning = true;
                player = &dSaveData_c::getTown()->mTownHost;
                ((dPersonalID_c *)player)->clear();
                field<IdentityFlags>(player, 0x2C).state = 0;
                if (dNet::isHost()) {
                    player = &field<u8>(dPlayerMgr_c::getCurrentPlayer(), 0x7FA8);
                    ((dPersonalID_c *)player)->clear();
                    field<IdentityFlags>(player, 0x2C).state = 0;
                }
            }
        }
        if (returning) {
            dScene::requestSceneChange(dScene::SCENE_DM_CHKP_MISS, 5, 0);
        } else {
            dScene::requestSceneChange(dScene::SCENE_DM_CHKP_CNNCT, 5, 0);
        }
        this->nextStep();
    }
}

template <class Owner, class Step>
void dSvMgr_c::stepBase_c<Owner, Step>::gotoTitle() {
    dScene::requestSceneChange(dScene::SCENE_DM_TITLE, 5, 2);
    this->nextStep();
}

template <class Owner, class Step>
void dSvMgr_c::stepBase_c<Owner, Step>::finish() {
    this->mpOwner->finishLoad();
}

template <class Owner, class Step>
void dSvMgr_c::stepBase_c<Owner, Step>::finishFinal() {
    this->mpOwner->finishLoad();
}

template <class Owner, class Step>
void dSvMgr_c::stepBase_c<Owner, Step>::waitCanSave() {
    if (SvStepIs<Step, stepSaveInterruptNetVst_c>::value ||
        SvStepIs<Step, stepSaveInterruptNetHst_c>::value) {
        if (dNet::canSaveInterrupt()) this->nextStep();
    } else if (dNet::canSaveContinue()) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dSvMgr_c::stepBase_c<Owner, Step>::waitNewPlayer() {
    if (dNet::isNewPlayerReady()) {
        dNet::setSaveVisitor();
        this->nextStep();
    }
}

template <class Owner, class Step>
void dSvMgr_c::stepBase_c<Owner, Step>::findFriend() {
    if (dNet::isSessionActive()) {
        int visitor = mCurrentVisitor;
        if (visitor >= 4) return;
        dPrivateData_c *player = dPlayerMgr_c::getNetPlayer(visitor);
        u64 friendCode = dNet::getMemberTime(visitor);
        int index = dWifi::findUser(&player->mFriends.mUserData);
        if (index >= 0 || index == -1) {
            if (index >= 0) {
                dWifi::updateFriend(index, &player->mPID, player->mFriends.mName, &friendCode);
            }
            this->nextStep();
        }
    }
}

template <class Owner, class Step>
void dSvMgr_c::stepBase_c<Owner, Step>::waitSavePermission() {
    if (dNet::isConnectSavePermitted()) {
        this->nextStep();
    }
}

template <class Owner, class Step>
void dSvMgr_c::stepBase_c<Owner, Step>::processLoadedSave() {
    dMsg::Comp_c *demo = dMsg::Comp_c::mInstance;
    switch (dNand::getResult()) {
    case 1:
        if (!dSaveData_c::getRaw()->isExtraGood(1)) {
            releaseMessage(demo);
            demo->setNextMsgCode(0x13);
            demo->setNextMsgLabel(mMessageLabel);
            this->mCurrentMethod = &Step::saveFailed;
        } else if (!dSaveData_c::get()->isGood()) {
            releaseMessage(demo);
            demo->setNextMsgCode(0x13);
            demo->setNextMsgLabel(mMessageLabel);
            this->mCurrentMethod = &Step::saveFailed;
        } else {
            dNet::getMyNo();
            int index = mPlayerIndex;
            fn_801017B8();
            dPrivateData_c *player = dPlayerMgr_c::getPlayer(index);
            dSaveOption::store(player->getOption());
            dPlayerMgr_c::getCurrentPlayer()->copy(player);
            dSaveData_c::get()->setupAfterLoad();
            dSaveData_c *save = dSaveData_c::getRaw();
            save->mSaveTime.get();
            this->nextStep();
        }
        break;
    case 2:
        releaseMessage(demo);
        demo->setNextMsgCode(0x0C);
        demo->setNextMsgLabel(mMessageLabel);
        this->mCurrentMethod = &Step::saveFailed;
        break;
    default:
        releaseMessage(demo);
        demo->setNextMsgCode(0x13);
        demo->setNextMsgLabel(mMessageLabel);
        this->mCurrentMethod = &Step::saveFailed;
    }
}

template <class Owner, class Step>
void dSvMgr_c::stepBase_c<Owner, Step>::initializeGrowth() {
    fn_80154358(lbl_8074E7F8);
    this->nextStep();
}

template <class Owner, class Step>
void dSvMgr_c::stepBase_c<Owner, Step>::processGrowth() {
    if (fn_8015436C(lbl_8074E7F8, 4)) {
        this->nextStep();
    }
}

#include "d_sv_mgr_tail.inc"
