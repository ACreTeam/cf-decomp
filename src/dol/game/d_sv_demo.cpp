#include <cstring>
#include <game/game/d_sv_demo.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_sv_mgr.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_nand.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_ftr.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_root.hpp>

namespace dSvDemo {
static inline bool isMessageHidden(dMsg::Comp_c *controller) {
    return controller->isState(&dMsg::Comp_c::executeHidden);
}
}

// 801B97B4
void *dSvDemo_c_classInit() { return new dSvDemo_c; }
fProf::fBaseProfile_c g_profile_SV_DEMO = { &dSvDemo_c_classInit, 0xA3, 0xE0 };

// 801B9848
int dSvDemo_c::create() {
    static const dState::mode_c<dSvDemo_c>::Entry modes[] = {
        {"待機", &dSvDemo_c::initializeWait, &dSvDemo_c::executeWait},
        {"メッセージ", &dSvDemo_c::initializeMessage, &dSvDemo_c::executeMessage},
        {"セーブシーンへ", &dSvDemo_c::initializeSaveScene, &dSvDemo_c::executeSaveScene},
        {"処理無し", &dSvDemo_c::initializeIdle, &dSvDemo_c::executeIdle},
        {"継続セーブ", &dSvDemo_c::initializeNetSaveContinue, &dSvDemo_c::executeNetSaveContinue},
        {"中断セーブ", &dSvDemo_c::initializeNetSaveInterrupt, &dSvDemo_c::executeNetSaveInterrupt},
    };
    mBedSide = BED_SIDE_NONE;
    dState::mode_c<dSvDemo_c>::initialize("dSvDemo_c", modes, ARRAY_SIZE(modes));
    return SUCCEEDED;
}

// 801B98F4
int dSvDemo_c::doDelete() {
    if (dDemo::isActive7()) dDemo::finish7();
    if (dNand::isIdle() == FALSE) return NOT_READY;
    return SUCCEEDED;
}

// 801B9938
int dSvDemo_c::execute() {
    dState::base_c<dSvDemo_c>::execute();
    return SUCCEEDED;
}

// 801B998C
void dSvDemo_c::onMessageEnd(int kind) {
    struct Entry { const char *label; int code; Method method; };
    static Entry messages[] = { {dSvMgr_c::mMessageLabel, MESSAGE_NETWORK_SAVE_WAIT, &dSvDemo_c::startNetworkSave} };
    u16 code = getMessageCode();
    const char *label = getMessageLabel();
    Entry *entry = messages;
    for (int i = 0; i < 1; i++, entry++) {
        if (std::strcmp(label, entry->label) == 0 && code == entry->code) {
            callMessageMethod(entry->method);
        }
    }
}

// 801B9A28
void dSvDemo_c::startNetworkSave() {
    dMsg::Comp_c *controller = mpController;
    controller->mLock = 1;
    controller->mWaitIconSe = 1;
    controller->mWaitIcon = 1;
    switch (mSaveKind) {
    case SAVE_CONTINUE: changeMode(&dSvDemo_c::executeNetSaveContinue); break;
    case SAVE_INTERRUPT: changeMode(&dSvDemo_c::executeNetSaveInterrupt); break;
    }
}

// 801B9CDC
void dSvDemo_c::onAnswer(int answer) {
    struct Entry { const char *label; int code; int answer; Method method; };
    static Entry messages[] = {
        {dSvMgr_c::mMessageLabel, MESSAGE_NORMAL_SAVE, 0, &dSvDemo_c::startNormalSave},
        {dSvMgr_c::mMessageLabel, MESSAGE_NETWORK_SAVE_CHOICE, 0, &dSvDemo_c::chooseInterruptSave},
        {dSvMgr_c::mMessageLabel, MESSAGE_NETWORK_SAVE_CHOICE, 1, &dSvDemo_c::chooseContinueSave},
    };
    u16 code = getMessageCode();
    const char *label;
    int selected = mpController->mSelect.mCursor;
    Entry *entry = messages;
    label = getMessageLabel();
    for (int i = 0; i < 3; i++, entry++) {
        if (std::strcmp(label, entry->label) == 0 &&
            code == entry->code && selected == entry->answer) {
            callMessageMethod(entry->method);
        }
    }
}

// 801B9DA8
void dSvDemo_c::startNormalSave() { changeMode(&dSvDemo_c::executeSaveScene); }

// 801B9F04
void dSvDemo_c::chooseContinueSave() {
    mSaveKind = SAVE_CONTINUE;
    mpController->setNextMsgCode(MESSAGE_NETWORK_SAVE_WAIT);
}

// 801B9F18
void dSvDemo_c::chooseInterruptSave() {
    mSaveKind = SAVE_INTERRUPT;
    mpController->setNextMsgCode(MESSAGE_NETWORK_SAVE_WAIT);
}

// 801B9F2C
void dSvDemo_c::requestMessage(u16 code) {
    dMsg::Comp_c *controller = dMsg::Comp_c::mInstance;
    setMessageLabel(dSvMgr_c::mMessageLabel);
    setMessageCode(code);
    controller->attachActor(this);
    controller->requestStart(0);
}

// 801B9FA8
BOOL dSvDemo_c::finishMessage() {
    dMsg::Comp_c *controller = mpController;
    if (dSvDemo::isMessageHidden(controller)) {
        mpController->detachActor();
        changeMode(&dSvDemo_c::executeIdle);
        return TRUE;
    }
    return FALSE;
}

// 801BA150
BOOL dSvDemo_c::finishMessageAndDemo() {
    dMsg::Comp_c *controller = mpController;
    if (dSvDemo::isMessageHidden(controller)) {
        mpController->detachActor();
        dDemo::finish7();
        changeMode(&dSvDemo_c::executeWait);
        return TRUE;
    }
    return FALSE;
}

// 801BA2FC
void dSvDemo_c::initializeWait() {}

// 801BA300
void dSvDemo_c::executeWait() {
    if (dDemo::isActive7()) {
        if (!dFTR::canReachExit()) {
            requestMessage(MESSAGE_EXIT_BLOCKED);
            changeMode(&dSvDemo_c::executeMessage);
        } else if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
            if (dNet::isHost()) {
                requestMessage(MESSAGE_NETWORK_SAVE_CHOICE);
                changeMode(&dSvDemo_c::executeMessage);
            } else {
                requestMessage(MESSAGE_SAVE_UNAVAILABLE);
                changeMode(&dSvDemo_c::executeMessage);
            }
        } else if (!dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID.isFromTown()) {
            requestMessage(MESSAGE_SAVE_UNAVAILABLE);
            changeMode(&dSvDemo_c::executeMessage);
        } else {
            dHomeList_c *homes = &dSaveData_c::getTown()->mHomes;
            if (homes->getHome(homes->findCurrentPlayer())) {
                requestMessage(MESSAGE_NORMAL_SAVE);
                changeMode(&dSvDemo_c::executeMessage);
            } else {
                requestMessage(MESSAGE_SAVE_UNAVAILABLE);
                changeMode(&dSvDemo_c::executeMessage);
            }
        }
    } else if ((!dNet::isSessionActive() || dNet::getMemberNum() <= 1) && mParam == 1) {
        dHomeList_c *homes = &dSaveData_c::getTown()->mHomes;
        if (homes->getHome(homes->findCurrentPlayer()) && dScene::isCurrentSceneMyHouse()) {
            mVec3_c bed;
            if (dPlayerMgr::getBedPosition(&bed, dPlayerMgr::PLAYER_INDEX_CURRENT)) {
                if (mPreviousPosition.x - bed.x > 0.0f) mBedSide = BED_SIDE_POSITIVE_X;
                else mBedSide = BED_SIDE_OTHER;
                dDemo::request7();
            }
        }
        mPreviousPosition = *dPlayerMgr::getPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    }
}

// 801BABAC
void dSvDemo_c::initializeMessage() {}

// 801BABB0
void dSvDemo_c::executeMessage() {
    if (finishMessageAndDemo()) {
        if ((u32)(mBedSide - 1) <= 1) {
            dPlayerMgr::getUpFromBed(mBedSide, 0);
            mBedSide = BED_SIDE_NONE;
        }
    }
}

// 801BAC04
void dSvDemo_c::initializeSaveScene() { dMsg::Comp_c::mInstance->mBgmKeep = 1; }

// 801BAC14
void dSvDemo_c::executeSaveScene() {
    if (finishMessage()) {
        dScene::requestSceneChange(0x39, 2, 5);
        dRoot_c::getSceneChange()->setReturnExitHere(0.0f);
        dSvMgr_c::requestSaveNormal();
    }
}

// 801BAC5C
void dSvDemo_c::initializeIdle() {}
// 801BAC60
void dSvDemo_c::executeIdle() {}

// 801BAC64
void dSvDemo_c::initializeNetSaveContinue() {
    static const dState::step_c<dSvDemo_c, stepNetSaveContinue_c>::Entry steps[] = {
        {"デモのロック", (stepNetSaveContinue_c::Method)&stepNetSaveContinue_c::requestSave},
        {"要求の結果", (stepNetSaveContinue_c::Method)&stepNetSaveContinue_c::waitRequest},
        {"メッセージ終了後セーブシーンへ", (stepNetSaveContinue_c::Method)&stepNetSaveContinue_c::waitMessageEnd},
        {"メッセージ終了待ち", (stepNetSaveContinue_c::Method)&stepNetSaveContinue_c::waitRefusedMessageEnd},
    };
    mContinueStep.initialize(this, "dSvDemo_c::stepNetSaveContinue_c", steps, ARRAY_SIZE(steps));
}

// 801BAD5C
void dSvDemo_c::executeNetSaveContinue() { mContinueStep.execute(); }
// 801BADA4
void dSvDemo_c::stepNetSaveContinue_c::requestSave() {
    if (dNet::requestContinue()) this->mCurrentMethod = &stepNetSaveContinue_c::waitRequest;
}
// 801BADF4
void dSvDemo_c::stepNetSaveContinue_c::waitRequest() {
    int result = dNet::getRequestResult();
    if (result > 0) {
        dMsg::Comp_c *controller = mpOwner->mpController;
        controller->mLock = 0;
        controller->mWaitIconSe = 0;
        controller->mWaitIcon = 0;
        mpOwner->mpController->mBgmKeep = 1;
        this->mCurrentMethod = &stepNetSaveContinue_c::waitMessageEnd;
    } else if (result < 0) {
        dMsg::Comp_c *controller = mpOwner->mpController;
        controller->mLock = 0;
        controller->mWaitIconSe = 0;
        controller->mWaitIcon = 0;
        mpOwner->mpController->setNextMsgCode(MESSAGE_NETWORK_SAVE_REFUSED);
        this->mCurrentMethod = &stepNetSaveContinue_c::waitRefusedMessageEnd;
    }
}
// 801BAEB8
void dSvDemo_c::stepNetSaveContinue_c::waitMessageEnd() {
    if (mpOwner->finishMessage()) dNet::startSaveEnd(FALSE);
}
// 801BAEEC
void dSvDemo_c::stepNetSaveContinue_c::waitRefusedMessageEnd() { if (mpOwner->finishMessageAndDemo()) return; }

// 801BAF10
void dSvDemo_c::initializeNetSaveInterrupt() {
    static const dState::step_c<dSvDemo_c, stepNetSaveInterrupt_c>::Entry steps[] = {
        {"デモのロック", (stepNetSaveInterrupt_c::Method)&stepNetSaveInterrupt_c::requestSave},
        {"要求の結果", (stepNetSaveInterrupt_c::Method)&stepNetSaveInterrupt_c::waitRequest},
        {"メッセージ終了後セーブシーンへ", (stepNetSaveInterrupt_c::Method)&stepNetSaveInterrupt_c::waitMessageEnd},
        {"メッセージ終了待ち", (stepNetSaveInterrupt_c::Method)&stepNetSaveInterrupt_c::waitRefusedMessageEnd},
    };
    mInterruptStep.initialize(this, "dSvDemo_c::stepNetSaveInterrupt_c", steps, ARRAY_SIZE(steps));
}
// 801BB008
void dSvDemo_c::executeNetSaveInterrupt() { mInterruptStep.execute(); }
// 801BB050
void dSvDemo_c::stepNetSaveInterrupt_c::requestSave() {
    if (dNet::requestInterrupt()) this->mCurrentMethod = &stepNetSaveInterrupt_c::waitRequest;
}
// 801BB0A0
void dSvDemo_c::stepNetSaveInterrupt_c::waitRequest() {
    int result = dNet::getRequestResult();
    if (result > 0) {
        dMsg::Comp_c *controller = mpOwner->mpController;
        controller->mLock = 0;
        controller->mWaitIconSe = 0;
        controller->mWaitIcon = 0;
        mpOwner->mpController->mBgmKeep = 1;
        this->mCurrentMethod = &stepNetSaveInterrupt_c::waitMessageEnd;
    } else if (result < 0) {
        dMsg::Comp_c *controller = mpOwner->mpController;
        controller->mLock = 0;
        controller->mWaitIconSe = 0;
        controller->mWaitIcon = 0;
        mpOwner->mpController->setNextMsgCode(MESSAGE_NETWORK_SAVE_REFUSED);
        this->mCurrentMethod = &stepNetSaveInterrupt_c::waitRefusedMessageEnd;
    }
}
// 801BB164
void dSvDemo_c::stepNetSaveInterrupt_c::waitMessageEnd() {
    if (mpOwner->finishMessage()) dNet::startSaveEnd(TRUE);
}
// 801BB198
void dSvDemo_c::stepNetSaveInterrupt_c::waitRefusedMessageEnd() { if (mpOwner->finishMessageAndDemo()) return; }

