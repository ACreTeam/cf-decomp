#include <game/cLib/c_lib.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_kbd.hpp>
#include <game/mLib/m_pad.hpp>
#include <revolution/kpad.h>
#include <revolution/wpad.h>

namespace dPad {
static mVec2_c sRawPointer; // Normalized DPD coordinates.
static f32 sStickX;
static f32 sStickY;
static s32 sInputMode;
static u16 sHeld;
static u16 sTriggered;
static u16 sPrevious;
static u16 sReleased;
static mVec2_c sPointerDirection(1.0f, 0.0f);
static mVec2_c sPointerPosition; // Screen coordinates, centered on (0, 0).

// 800FA08C: inferred name; disconnect newly connected remotes other than the
// current one.
void disconnectOtherRemotes(const EGG::CoreControllerConnectArg &arg) {
    int current = mPad::getCurrentCoreID();
    if (arg.result == 0 && current != arg.chan) {
        WPADControlMotor(arg.chan, WPAD_MOTOR_STOP);
        WPADDisconnect(arg.chan);
    }
}

// 800FA0E0
void init() {
    EGG::CoreControllerMgr::setConnectCallback(disconnectOtherRemotes);
}

// 800FA0F0
void updateConnection() {
    BOOL connected = mPad::getCore()->isConnected();
    BOOL enabled = WPADGetAcceptConnection();
    if (connected) {
        if (enabled) {
            WPADSetAcceptConnection(FALSE);
        }
    } else {
        if (!enabled) {
            WPADSetAcceptConnection(TRUE);
        }
    }
}

// 800FA150
void begin() {
    KPADEnableStickCrossClamp();
    mPad::beginPad();
    if ((int)mPad::getCore()->mCoreStatus[0].dev_type != WPAD_DEV_FREESTYLE &&
        mPad::getCurrentCoreID() < 4) {
        EGG::CoreController *core =
            mPad::getMgr()->getNthController(mPad::getCurrentCoreID());
        if (core) {
            cLib::memSet(&core->mCoreStatus[0].ex_status, 0,
                         sizeof(KPADEXStatus));
        }
    }
    if (mPad::getCore()->getDpdValidFlag() > 0) {
        sRawPointer = readPointer();
        sPointerDirection = readHorizon();
        sPointerPosition.x = -304.0f + 304.0f * (1.0f + sRawPointer.x);
        sPointerPosition.y = 228.0f + -228.0f * (1.0f + sRawPointer.y);
    }
    if (isRemoteUnavailable()) {
        sStickX = 0.0f;
        sStickY = 0.0f;
    } else {
        sStickX = mPad::getCore()->getFreeStickX();
        sStickY = mPad::getCore()->getFreeStickY();
    }
    if ((sStickX > 0.9f || sStickX < -0.9f) && sStickY < 0.1f &&
        sStickY > -0.1f) {
        sStickY = 0.0f;
    }
    if ((sStickY > 0.9f || sStickY < -0.9f) && sStickX < 0.1f &&
        sStickX > -0.1f) {
        sStickX = 0.0f;
    }
    sPrevious = sHeld;
    sHeld = 0;
    s32 previousMode = sInputMode;
    sInputMode = INPUT_NONE;
    if (sStickX != 0.0f || sStickY != 0.0f) {
        sInputMode = INPUT_STICK;
    }
    if (sStickX >= 0.5f) {
        sHeld |= BUTTON_RIGHT;
    } else if (sStickX <= -0.5f) {
        sHeld |= BUTTON_LEFT;
    }
    if (sStickY >= 0.5f) {
        sHeld |= BUTTON_UP;
    } else if (sStickY <= -0.5f) {
        sHeld |= BUTTON_DOWN;
    }
    if (sInputMode != INPUT_REMOTE) {
        if (mPad::getCore()->down(WPAD_BUTTON_C)) {
            sHeld |= BUTTON_CONFIRM;
        }
        if (mPad::getCore()->down(WPAD_BUTTON_Z)) {
            sHeld |= BUTTON_CANCEL;
        }
        if (sHeld) {
            sInputMode = INPUT_STICK;
        }
    }
    if (sInputMode != INPUT_STICK && sHeld) {
        sInputMode = INPUT_REMOTE;
    }
    if (sInputMode == INPUT_NONE) {
        sInputMode = previousMode;
    }
    dKbd::calcHKB();
    if (sPrevious & BUTTON_SHAKE) {
        if (mPad::getCore()->mCoreStatus[0].acc_value > 2.0f ||
            mPad::getCore()->mCoreStatus[0].acc_speed > 1.0f) {
            sHeld |= BUTTON_SHAKE;
        }
    } else {
        if (mPad::getCore()->mCoreStatus[0].acc_value > 3.0f ||
            mPad::getCore()->mCoreStatus[0].acc_speed > 2.0f) {
            sHeld |= BUTTON_SHAKE;
        }
    }
}

// 800FA4C4
BOOL updateButtons(BOOL suppressA) {
    if (!suppressA && mPad::getCore()->down(WPAD_BUTTON_A)) {
        if (!mPad::getCore()->downTrigger(WPAD_BUTTON_A)) {
            sPrevious |= (BUTTON_A | BUTTON_CONFIRM);
        }
        sHeld |= (BUTTON_A | BUTTON_CONFIRM);
    }
    if (mPad::getCore()->down(WPAD_BUTTON_B)) {
        sHeld |= (BUTTON_B | BUTTON_CANCEL);
    }
    if (mPad::getCore()->down(WPAD_BUTTON_C)) {
        sHeld |= BUTTON_C;
    }
    int held = sHeld;
    int previous = sPrevious;
    sTriggered = held & ~previous;
    sReleased = previous & ~held;
    return isShakeTriggered();
}

// 800FA5B4
void end() {
    updateConnection();
    mPad::endPad();
}

// 800FA5D8
BOOL isRemoteUnavailable() {
    u32 device;
    s32 result = WPADProbe(mPad::getCurrentCoreID(), &device);
    if (result == WPAD_ERR_NO_CONTROLLER) {
        return TRUE;
    }
    if (result != WPAD_ERR_OK) {
        return FALSE;
    }
    if (device == WPAD_DEV_NOT_FOUND) {
        return TRUE;
    }
    return device == WPAD_DEV_NOT_SUPPORTED;
}

// 800FA640
BOOL hasUnsupportedExtension() {
    u32 device;
    if (WPADProbe(mPad::getCurrentCoreID(), &device) != WPAD_ERR_OK) {
        return FALSE;
    }
    if (device == WPAD_DEV_NOT_FOUND) {
        return FALSE;
    }
    if (device == WPAD_DEV_CORE) {
        return FALSE;
    }
    return device != WPAD_DEV_FREESTYLE;
}

// 800FA6AC
mVec2_c getRawPointer() {
    return sRawPointer;
}
// 800FA6C4
mVec2_c getPointerPosition() {
    return sPointerPosition;
}
// 800FA6DC
mVec2_c getPointerDirection() {
    return sPointerDirection;
}
// 800FA6F4
int getInputMode() {
    return sInputMode;
}
// 800FA6FC
f32 getStickX() {
    return sStickX;
}
// 800FA704
f32 getStickY() {
    return sStickY;
}
// 800FA70C
u16 getTriggered() {
    return sTriggered;
}
// 800FA714
u16 getHeld() {
    return sHeld;
}
// 800FA71C
u16 getReleased() {
    return sReleased;
}
// 800FA724
BOOL isATriggered() {
    return (sTriggered >> 6) & 1;
}
// 800FA730
BOOL isAHeld() {
    return (sHeld >> 6) & 1;
}
// 800FA73C
BOOL isCTriggered() {
    return (sTriggered >> 8) & 1;
}
// 800FA748
BOOL isConfirmTriggered() {
    return (sTriggered >> 4) & 1;
}
// 800FA754
BOOL isConfirmHeld() {
    return (sHeld >> 4) & 1;
}
// 800FA760
BOOL isCancelTriggered() {
    return (sTriggered >> 5) & 1;
}
// 800FA76C
BOOL isCancelHeld() {
    return (sHeld >> 5) & 1;
}
// 800FA778
BOOL isShakeTriggered() {
    return (sTriggered >> 9) & 1;
}
// 800FA784
BOOL isShakeHeld() {
    return (sHeld >> 9) & 1;
}
// 800FA790
BOOL isUpTriggered() {
    if (mPad::getCore()->mCoreStatus[0].hold & BUTTON_DIRECTION) {
        return mPad::getCore()->downTrigger(WPAD_BUTTON_UP);
    }
    return (getTriggered() >> 3) & 1;
}
// 800FA7DC
BOOL isDownTriggered() {
    if (mPad::getCore()->mCoreStatus[0].hold & BUTTON_DIRECTION) {
        return mPad::getCore()->downTrigger(WPAD_BUTTON_DOWN);
    }
    return (getTriggered() >> 2) & 1;
}
// 800FA828
BOOL isLeftTriggered() {
    if (mPad::getCore()->mCoreStatus[0].hold & BUTTON_DIRECTION) {
        return mPad::getCore()->downTrigger(WPAD_BUTTON_LEFT);
    }
    return getTriggered() & 1;
}
// 800FA874
BOOL isRightTriggered() {
    if (mPad::getCore()->mCoreStatus[0].hold & BUTTON_DIRECTION) {
        return mPad::getCore()->downTrigger(WPAD_BUTTON_RIGHT);
    }
    return (getTriggered() >> 1) & 1;
}
// 800FA8C0
BOOL isUpHeld() {
    if (mPad::getCore()->mCoreStatus[0].hold & BUTTON_DIRECTION) {
        return mPad::getCore()->down(WPAD_BUTTON_UP);
    }
    return (getHeld() >> 3) & 1;
}
// 800FA90C
BOOL isDownHeld() {
    if (mPad::getCore()->mCoreStatus[0].hold & BUTTON_DIRECTION) {
        return mPad::getCore()->down(WPAD_BUTTON_DOWN);
    }
    return (getHeld() >> 2) & 1;
}
// 800FA958
BOOL isLeftHeld() {
    if (mPad::getCore()->mCoreStatus[0].hold & BUTTON_DIRECTION) {
        return mPad::getCore()->down(WPAD_BUTTON_LEFT);
    }
    return getHeld() & 1;
}
// 800FA9A4
BOOL isRightHeld() {
    if (mPad::getCore()->mCoreStatus[0].hold & BUTTON_DIRECTION) {
        return mPad::getCore()->down(WPAD_BUTTON_RIGHT);
    }
    return (getHeld() >> 1) & 1;
}
} // namespace dPad
