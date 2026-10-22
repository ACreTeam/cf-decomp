#include <lib/egg/core/eggAllocator.h>
#include <lib/egg/core/eggController.h>
#include <lib/egg/core/eggExpHeap.h>
#include <lib/egg/core/eggSystem.h>
#include <revolution/vi.h>
#include <string.h>

namespace EGG {

NullController sNullGCCont;

void CoreStatus::init() {
    memset(this, 0, sizeof(CoreStatus));
}

u32 CoreStatus::getFSStickButton() {
    u32 buttons = 0;

    f32 sx = getFSStickX();
    if (!(-0.25f < sx && sx < 0.25f)) {
        if (sx <= -0.5f) {
            buttons = 0;
            buttons |= cCORE_FSSTICK_LEFT;
        } else if (sx >= 0.5f) {
            buttons = 0;
            buttons |= cCORE_FSSTICK_RIGHT;
        }
    }

    f32 sy = getFSStickY();
    if (!(-0.25f < sy && sy < 0.25f)) {
        if (sy <= -0.5f) {
            buttons &= ~cCORE_FSSTICK_UP;
            buttons |= cCORE_FSSTICK_DOWN;
        } else if (sy >= 0.5f) {
            buttons &= ~cCORE_FSSTICK_DOWN;
            buttons |= cCORE_FSSTICK_UP;
        }
    }

    return buttons;
}

CoreController::CoreController() : mRumbleMgr(NULL) {
    mFSStickHold = 0;
    mFSStickTrig = 0;
    mFSStickRelease = 0;

    sceneReset();
    mFlag.makeAllZero();
}

void CoreController::sceneReset() {
    mIdleTime = 0;
    mAccelPrev.set(0.0f, 0.0f, 0.0f);
    mDpdPosPrev.set(0.0f, 0.0f);

    mPostureMtxPrev.makeIdentity();
    mPostureMtx.makeIdentity();

    mStableFrame = 10;
    mStableMag = 0.13f;

    mStableAccel.set(0.0f, 0.0f, 0.0f);
    mStableFlags = 0;
    mStableTimer[0] = mStableTimer[1] = mStableTimer[2] = 0;

    mSimpleRumblePattern = 0;
    mSimpleRumbleFrame = 0;
    mEnableSimpleRumble = false;
    mSimpleRumbleSize = 32;
    mSimpleRumbleIndex = mSimpleRumbleSize - 1;

    stopRumbleMgr();
}

Vector2f CoreController::getDpdRawPos() const {
    return Vector2f(mCoreStatus[0].pos.x, mCoreStatus[0].pos.y);
}

s32 CoreController::getDpdValidFlag() const {
    return mCoreStatus[0].dpd_valid_fg;
}

void CoreController::startMotor() {
    WPADControlMotor(mChannelID, WPAD_MOTOR_ON);
}

void CoreController::stopMotor() {
    WPADControlMotor(mChannelID, WPAD_MOTOR_OFF);
}

void CoreController::createRumbleMgr(u8 overlap_num) {
    mRumbleMgr = new ControllerRumbleMgr();
    mRumbleMgr->createUnit(overlap_num, this);
}

void CoreController::startPatternRumble(const char *pattern, int frame,
                                        bool force) {
    if (mRumbleMgr != NULL) {
        mRumbleMgr->startPattern(pattern, frame, force);
    }
}

void CoreController::stopRumbleMgr() {
    if (mRumbleMgr != NULL) {
        mRumbleMgr->stop();
    }
}

void CoreController::calc_posture_matrix(Matrix34f &mtx, bool checkStable) {
    if (checkStable && !isStable(7)) {
        return;
    }

    Vector3f acc = -getAccel();

    Vector3f vy = acc;
    vy.normalise();

    Vector3f vz(0.0f, 0.0f, 1.0f);
    Vector3f vx(1.0f, 0.0f, 0.0f);

    Vector3f bx, by, bz;
    mPostureMtxPrev.getBase(0, bx);
    mPostureMtxPrev.getBase(1, by);
    mPostureMtxPrev.getBase(2, bz);

    Vector3f vz_n(0.0f, 0.0f, -1.0f);
    Vector3f vx_n(-1.0f, 0.0f, 0.0f);

    if (vz.dot(bz) < vz_n.dot(bz)) {
        vz = vz_n;
    }

    if (vx.dot(bx) < vx_n.dot(bx)) {
        vx = vx_n;
    }

    if (Math<f32>::abs(vy.dot(vz)) < Math<f32>::abs(vy.dot(vx))) {
        vx = vy.cross(vz);
        vx.normalise();
        vz = vx.cross(vy);
        vz.normalise();
    } else {
        vz = vx.cross(vy);
        vz.normalise();
        vx = vy.cross(vz);
        vx.normalise();
    }

    mtx.setBase(0, vx);
    mtx.setBase(1, vy);
    mtx.setBase(2, vz);
}

void CoreController::beginFrame(PADStatus *) {
    mKPADReadLength = KPADRead(mChannelID, mCoreStatus, KPAD_MAX_SAMPLES);

    u32 devType;
    switch (WPADProbe(mChannelID, &devType)) {
    case WPAD_ERR_OK:
        if (devType == WPAD_DEV_NOT_FOUND) {
            mFlag.resetBit(0);
        } else {
            mFlag.setBit(0);
        }
        break;
    case WPAD_ERR_NO_CONTROLLER:
        mFlag.resetBit(0);
        break;
    }

    if (mKPADReadLength > 0) {
        CoreStatus *coreStatus = &mCoreStatus[0];
        u32 stickHoldPrev = mFSStickHold;

        if (coreStatus->isFreestyle()) {
            mFSStickHold = coreStatus->getFSStickButton();
        } else {
            mFSStickHold = 0;
        }

        mFSStickTrig = mFSStickHold & ~stickHoldPrev;
        mFSStickRelease = stickHoldPrev & ~mFSStickHold;

        coreStatus->hold |= (mFSStickHold & cCORE_FSSTICK_BUTTONS);
        coreStatus->trig |= (mFSStickTrig & cCORE_FSSTICK_BUTTONS);
        coreStatus->release |= (mFSStickRelease & cCORE_FSSTICK_BUTTONS);
    }

    mPostureMtxPrev = mPostureMtx;
    mStableFlags = 0;

    Vector3f accel = getAccel();

    for (int i = 0; i < 3; i++) {
        if (accel(i) - mAccelPrev(i) < mStableMag) {
            if (mStableFrame <= ++mStableTimer[i]) {
                mStableTimer[i] = mStableFrame;
                mStableFlags |= 1 << i;
                mStableAccel(i) = accel(i);
            }
        } else {
            mStableFlags &= ~(1 << i);
            mStableTimer[i] = 0;
        }
    }

    calc_posture_matrix(mPostureMtx, true);

    if (mEnableSimpleRumble) {
        if (mSimpleRumblePattern & 1 << mSimpleRumbleIndex) {
            startMotor();
        } else {
            stopMotor();
        }

        if (mSimpleRumbleIndex <= 0) {
            mSimpleRumbleIndex = mSimpleRumbleSize - 1;
        } else {
            mSimpleRumbleIndex--;
        }

        if (--mSimpleRumbleFrame == 0) {
            stopMotor();
            mEnableSimpleRumble = false;
        }
    }

    if (mRumbleMgr != NULL) {
        mRumbleMgr->calc();
    }

    bool idle = true;
    if (mCoreStatus[0].hold != 0) {
        idle = false;
    }
    if (idle) {
        Vector3f d = mAccelPrev - getAccel();
        if (d.squaredLength() > 0.01f) {
            idle = false;
        }
    }
    if (idle) {
        Vector2f d = mDpdPosPrev - getDpdRawPos();
        if (d.squaredLength() > 0.05f) {
            idle = false;
        }
    }
    if (idle) {
        if (mIdleTime < 216000) {
            mIdleTime++;
        }
    } else {
        mIdleTime = 0;
    }
}

void CoreController::endFrame() {
    mAccelPrev = getAccel();
    mDpdPosPrev = getDpdRawPos();
}

f32 CoreController::getFreeStickX() {
    if (getDevType() == cDEV_CORE) {
        return 0.0f;
    }
    return mCoreStatus[0].getFSStickX();
}

f32 CoreController::getFreeStickY() {
    if (getDevType() == cDEV_CORE) {
        return 0.0f;
    }
    return mCoreStatus[0].getFSStickY();
}

CoreControllerMgr::T__Disposer::~T__Disposer() {
    if (this == sStaticDisposer) {
        deleteInstance();
    }
}

CoreControllerMgr *CoreControllerMgr::createInstance() {
    if (sInstance == NULL) {
        sInstance = new CoreControllerMgr();
        T__Disposer::sStaticDisposer = &sInstance->mDisposer;
    }
    return sInstance;
}

void CoreControllerMgr::deleteInstance() {
    sInstance = NULL;
    T__Disposer::sStaticDisposer = NULL;
}

CoreControllerMgr *CoreControllerMgr::sInstance = NULL;
CoreControllerMgr::T__Disposer *CoreControllerMgr::T__Disposer::sStaticDisposer = NULL;
CoreControllerFactory CoreControllerMgr::sCoreControllerFactory = NULL;
CoreControllerConnectCallback CoreControllerMgr::sConnectCallback = NULL;
bool CoreControllerMgr::sUseExternalAllocator = false;
u32 CoreControllerMgr::sWPADWorkSize = 0x32000;

CoreController *CoreControllerMgr::getNthController(int index) {
    return mControllers(index);
}

namespace {

Allocator *sAllocator = NULL;

void *alloc(u32 size) {
    return sAllocator->alloc(size);
}

BOOL free(void *block) {
    sAllocator->free(block);
    return TRUE;
}

} // namespace

void CoreControllerMgr::connectCallback(s32 chan, s32 result) {
    CoreControllerConnectArg arg;
    arg.chan = chan;
    arg.result = result;

    if (sConnectCallback != NULL) {
        sConnectCallback(arg);
    }
}

CoreControllerMgr::CoreControllerMgr() {
    s32 channels[WPAD_MAX_CONTROLLERS] = {
        WPAD_CHAN0,
        WPAD_CHAN1,
        WPAD_CHAN2,
        WPAD_CHAN3,
    };

    if (!sUseExternalAllocator) {
        ExpHeap *heap = ExpHeap::create(
            sWPADWorkSize, BaseSystem::mConfigData->mRootHeapMem2, 0);
        heap->setName("EGG::CoreControllerMgr");
        sAllocator = new Allocator(heap, 32);
        WPADRegisterAllocator(alloc, free);
    }

    KPADInit();

    beginFrame();
    endFrame();

    VIWaitForRetrace();

    for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
        WPADSetConnectCallback(channels[i], connectCallback);
    }

    mControllers.allocate(WPAD_MAX_CONTROLLERS);

    for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
        if (sCoreControllerFactory != NULL) {
            mControllers(i) = sCoreControllerFactory();
        } else {
            mControllers(i) = new CoreController();
        }
    }

    mDevTypes.allocate(WPAD_MAX_CONTROLLERS);

    for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
        mControllers(i)->mChannelID = channels[i];
        mDevTypes(i) = cDEV_NOT_FOUND;
    }

    mExtensionCallback = NULL;
}

void CoreControllerMgr::beginFrame() {
    for (int i = 0; i < mControllers.getSize(); i++) {
        mControllers(i)->beginFrame();
    }
}

void CoreControllerMgr::endFrame() {
    for (int i = 0; i < mControllers.getSize(); i++) {
        mControllers(i)->endFrame();

        u32 devType;
        s32 result = WPADProbe(i, &devType);

        eCoreDevType coreType;
        if (result == WPAD_ERR_OK) {
            coreType = static_cast<eCoreDevType>(devType);
        } else if (result == WPAD_ERR_NO_CONTROLLER) {
            coreType = cDEV_NOT_FOUND;
        } else {
            continue;
        }

        if (mExtensionCallback != NULL && coreType != mDevTypes(i)) {
            CoreControllerExtensionArg arg;
            arg.newDevType = coreType;
            arg.oldDevType = mDevTypes(i);
            arg.chan = i;

            mExtensionCallback->onConnect(arg);
        }

        mDevTypes(i) = coreType;
    }
}

GCControllerMgr *GCControllerMgr::sInstance = NULL;
GCControllerMgr::T__Disposer *GCControllerMgr::T__Disposer::sStaticDisposer = NULL;

void GCController::calc() {
    mHold = mStatus->button;
}

void GCControllerMgr::calc() {
    for (int i = 0; i < 4; i++) {
        mControllers(i)->calc();
    }

    PADRead(mStatus);
    PADClampCircle(mStatus);
}

void ControllerRumbleUnit::init() {
    mRumblePattern = NULL;
    mRumblePatternPtr = NULL;

    mRumbleFrame = 0;
    mRumblePower = mRumbleValue = 0.0f;

    mFlags = 0;
}

void ControllerRumbleUnit::startPattern(const char *pattern, int frame) {
    mRumblePattern = pattern;
    mRumblePatternPtr = pattern;

    clrFlags(BIT_PATTERN_LOOP);
    clrFlags(BIT_PATTERN_TIMER);

    if (frame < 0) {
        setFlags(BIT_PATTERN_LOOP);
    } else if (frame > 0) {
        setFlags(BIT_PATTERN_LOOP);
        setFlags(BIT_PATTERN_TIMER);
    }

    setFlags(BIT_USE_PATTERN);
    clrFlags(BIT_USE_POWER);
    setFlags(BIT_ENABLED);

    mRumbleFrame = frame;
}

f32 ControllerRumbleUnit::calc() {
    f32 result = 0.0f;

    if (tstFlags(BIT_ENABLED)) {
        if (tstFlags(BIT_USE_PATTERN)) {
            char ch = *++mRumblePatternPtr;

            if (ch == '\0') {
                if (tstFlags(BIT_PATTERN_LOOP)) {
                    mRumblePatternPtr = mRumblePattern;
                } else {
                    clrFlags(BIT_ENABLED);
                }
            } else {
                switch (ch) {
                case '*':
                    result = 1.0f;

                case '-':
                default:
                    break;
                }
            }

            if (tstFlags(BIT_PATTERN_TIMER) && --mRumbleFrame <= 0) {
                clrFlags(BIT_ENABLED);
            }

            return result;
        } else {
            mRumbleValue += mRumblePower;

            if (mRumbleValue >= 1.0f) {
                result = 1.0f;
                mRumbleValue = 0.0f;
            }

            if (!tstFlags(BIT_POWER_LOOP) && --mRumbleFrame <= 0) {
                clrFlags(BIT_ENABLED);
            }

            return result;
        }
    }

    return -1.0f;
}

ControllerRumbleMgr::ControllerRumbleMgr() : mController(NULL) {
    nw4r::ut::List_Init(&mActiveUnitList,
                        offsetof(ControllerRumbleUnit, mLink));
    nw4r::ut::List_Init(&mStoppedUnitList,
                        offsetof(ControllerRumbleUnit, mLink));
}

void ControllerRumbleMgr::createUnit(u8 overlap_num,
                                     CoreController *controller) {
    for (u8 i = 0; i < overlap_num; i++) {
        nw4r::ut::List_Append(&mStoppedUnitList, new ControllerRumbleUnit());
    }

    mController = controller;
}

void ControllerRumbleMgr::stop() {
    mController->stopMotor();

    while (nw4r::ut::List_GetSize(&mActiveUnitList) > 0) {
        ControllerRumbleUnit *unit = static_cast<ControllerRumbleUnit *>(
            nw4r::ut::List_GetFirst(&mActiveUnitList));

        nw4r::ut::List_Remove(&mActiveUnitList, unit);
        nw4r::ut::List_Append(&mStoppedUnitList, unit);
    }
}

void ControllerRumbleMgr::calc() {
    if (nw4r::ut::List_GetSize(&mActiveUnitList) == 0) {
        return;
    }

    ControllerRumbleUnit *unit = static_cast<ControllerRumbleUnit *>(
        nw4r::ut::List_GetFirst(&mActiveUnitList));

    f32 totalPower = 0.0f;

    while (unit != NULL) {
        f32 unitPower = unit->calc();

        ControllerRumbleUnit *next = static_cast<ControllerRumbleUnit *>(
            nw4r::ut::List_GetNext(&mActiveUnitList, unit));

        if (unitPower < 0.0f) {
            nw4r::ut::List_Remove(&mActiveUnitList, unit);
            nw4r::ut::List_Append(&mStoppedUnitList, unit);
        } else {
            totalPower += unitPower;
        }

        unit = next;
    }

    if (totalPower >= 1.0f) {
        mController->startMotor();
    } else {
        mController->stopMotor();
    }
}

void ControllerRumbleMgr::startPattern(const char *pattern, int frame,
                                       bool force) {
    ControllerRumbleUnit *unit = getUnitFromList(force);
    if (unit == NULL) {
        return;
    }

    unit->startPattern(pattern, frame);
}

ControllerRumbleUnit *ControllerRumbleMgr::getUnitFromList(bool force) {
    ControllerRumbleUnit *unit = static_cast<ControllerRumbleUnit *>(
        nw4r::ut::List_GetFirst(&mStoppedUnitList));

    if (unit != NULL) {
        nw4r::ut::List_Remove(&mStoppedUnitList, unit);
        nw4r::ut::List_Append(&mActiveUnitList, unit);
    } else if (force) {
        unit = static_cast<ControllerRumbleUnit *>(
            nw4r::ut::List_GetFirst(&mActiveUnitList));

        if (unit != NULL) {
            nw4r::ut::List_Remove(&mActiveUnitList, unit);
            nw4r::ut::List_Append(&mActiveUnitList, unit);
        }
    }

    return unit;
}

} // namespace EGG
