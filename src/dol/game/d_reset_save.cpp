// The network shutdown before a reset (dResetSave). .text 80107A28..80107D6C, .rodata 80475CC0..80475CE0,
// .sbss 8074E640..8074E648. See include/game/game/d_reset_save.hpp.
#include <game/game/d_reset_save.hpp>
#include <game/game/d_mii_icon.hpp>
#include <game/game/d_nand.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_reset.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_wifi_err.hpp>

namespace dResetSave {

int sStep;
bool sRequested;

static BOOL (*const sStepFuncs[STEP_NUM])() = {
    stepWait, stepCloseNet, stepWaitMiiIcon, stepWaitNand, stepSaveFriends, stepWaitSave, stepEndSession, stepDone,
};

// 80107A28
BOOL stepWait() {
    if (sRequested) {
        sStep = STEP_CLOSE_NET;
        return FALSE;
    }
    if (g_wifiErr.mActive) {
        sStep = STEP_CLOSE_NET;
        return FALSE;
    }
    if (dReset::Manage_c::GetInstance()->isResetting()) {
        sStep = STEP_CLOSE_NET;
        return FALSE;
    }
    return TRUE;
}

// 80107AC0
BOOL stepCloseNet() {
    if (dNet::closeNetConnection()) {
        sStep = STEP_WAIT_MII_ICON;
        return FALSE;
    }
    return TRUE;
}

// 80107AFC
BOOL stepWaitMiiIcon() {
    if (fn_8008B8DC()) {
        sStep = STEP_WAIT_NAND;
        return FALSE;
    }
    return TRUE;
}

// 80107B38
BOOL stepWaitNand() {
    if (dNand::isIdle()) {
        sStep = STEP_SAVE_FRIENDS;
        return FALSE;
    }
    return TRUE;
}

// 80107B74
BOOL stepSaveFriends() {
    if (dNet::getLocalPlayerNo() < 7 && dNand::getType(lbl_8074B010) == 1) {
        if (dNand::saveFriendList()) {
            sStep = STEP_WAIT_SAVE;
            return FALSE;
        }
        return TRUE;
    }
    sStep = STEP_END_SESSION;
    return FALSE;
}

// 80107BDC
BOOL stepWaitSave() {
    if (dNand::isIdle()) {
        sStep = STEP_END_SESSION;
        return FALSE;
    }
    return TRUE;
}

// 80107C18
BOOL stepEndSession() {
    dNet::endSession(0);
    sStep = STEP_DONE;
    return FALSE;
}

// 80107C48
BOOL stepDone() {
    return TRUE;
}

// 80107C50
void calc(BOOL sceneDelete) {
    for (;;) {
        if (sStep >= STEP_NUM) {
            break;
        }
        if ((sStep == STEP_END_SESSION && sceneDelete) || (sStep != STEP_END_SESSION && !sceneDelete)) {
            if (sStepFuncs[sStep] == NULL || sStepFuncs[sStep]() != FALSE) {
                break;
            }
        } else {
            break;
        }
    }
}

// 80107CD4
void clear() {
    sStep = STEP_WAIT;
    sRequested = false;
}

// 80107CE4
void execute() {
    calc(FALSE);
}

// 80107CEC
void executeSceneDelete() {
    calc(TRUE);
}

// 80107CF4
BOOL request() {
    if (!sRequested) {
        sRequested = true;
    }
    if (sStep == STEP_WAIT) {
        return TRUE;
    }
    return FALSE;
}

// 80107D24
BOOL isNetClosed() {
    return sStep == STEP_DONE || sStep == STEP_END_SESSION;
}

// 80107D44
BOOL isIdle() {
    return sStep == STEP_WAIT || sStep == STEP_END_SESSION || sStep == STEP_DONE;
}

} // namespace dResetSave
