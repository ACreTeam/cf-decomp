#pragma once

#include <types.h>
#include <lib/egg/core/eggBitFlag.h>
#include <lib/egg/core/eggDisposer.h>
#include <lib/egg/math/eggMatrix.h>
#include <lib/egg/math/eggVector.h>
#include <lib/egg/prim/eggBuffer.h>
#include <revolution/PAD.h>
#include <revolution/KPAD.h>
#include <revolution/WPAD.h>

namespace EGG {

class CoreController;
class ControllerRumbleMgr;

enum eCoreDevType {
    cDEV_CORE = WPAD_DEV_CORE,
    cDEV_FREESTYLE = WPAD_DEV_FREESTYLE,
    cDEV_CLASSIC = WPAD_DEV_CLASSIC,
    cDEV_FUTURE = WPAD_DEV_FUTURE,
    cDEV_NOT_SUPPORTED = WPAD_DEV_NOT_SUPPORTED,
    cDEV_NOT_FOUND = WPAD_DEV_NOT_FOUND,
    cDEV_NULL = WPAD_DEV_NULL,
    cDEV_UNKNOWN = WPAD_DEV_UNKNOWN
};

enum eCoreButton {
    cCORE_FSSTICK_UP = 1 << 16,
    cCORE_FSSTICK_DOWN = 1 << 17,
    cCORE_FSSTICK_LEFT = 1 << 18,
    cCORE_FSSTICK_RIGHT = 1 << 19,

    cCORE_FSSTICK_BUTTONS = cCORE_FSSTICK_UP | cCORE_FSSTICK_DOWN |
                            cCORE_FSSTICK_LEFT | cCORE_FSSTICK_RIGHT
};

typedef CoreController *(*CoreControllerFactory)();

struct CoreControllerConnectArg {
    s32 chan;   // 0x0
    s32 result; // 0x4
};

typedef void (*CoreControllerConnectCallback)(
    const CoreControllerConnectArg &arg);

struct CoreControllerExtensionArg {
    eCoreDevType newDevType; // 0x0
    eCoreDevType oldDevType; // 0x4
    s32 chan;                // 0x8
};

class CoreControllerExtensionCallback {
  public:
    virtual void onConnect(const CoreControllerExtensionArg &arg); // 0x08
};

// Layouts from the Skyward Sword decompilation (zeldaret/ss,
// egg/core/eggController.h), adjusted to CF's older EGG (KPADStatus is 0x84
// bytes here, and the status array starts at 0x14).
class CoreStatus : public KPADStatus {
  public:
    CoreStatus() {
    }

    void init();
    u32 getFSStickButton();

    Vector2f getPosition() const {
        return Vector2f(pos.x, pos.y);
    } // 8017E79C
    Vector2f getHorizon() const {
        return Vector2f(horizon.x, horizon.y);
    } // 800FAAF8

    Vector2f getAccelVertical() {
        return Vector2f(acc_vertical.x, acc_vertical.y);
    }

    Vector3f getAccel() const {
        return Vector3f(acc.x, acc.y, acc.z);
    }

    f32 getFSStickX() const {
        return ex_status.fs.stick.x;
    }
    f32 getFSStickY() const {
        return ex_status.fs.stick.y;
    }

    eCoreDevType getDevType() const {
        return static_cast<eCoreDevType>(dev_type);
    }
    bool isFreestyle() const {
        return getDevType() == cDEV_FREESTYLE;
    }
};

class CoreController {
  public:
    CoreController();

    virtual void setPosParam(float a, float b) {
        KPADSetPosParam(mChannelID, a, b);
    }
    virtual void setHoriParam(float a, float b) {
        KPADSetHoriParam(mChannelID, a, b);
    }
    virtual void setDistParam(float a, float b) {
        KPADSetDistParam(mChannelID, a, b);
    }
    virtual void setAccParam(float a, float b) {
        KPADSetAccParam(mChannelID, a, b);
    }
    virtual bool down(ulong mask) const {
        return (mask & mCoreStatus[0].hold) != 0;
    }
    virtual bool up(ulong mask) const {
        return (mask & mCoreStatus[0].hold) != mask;
    }
    virtual bool downTrigger(ulong mask) const {
        return (mask & mCoreStatus[0].trig) != 0;
    }
    virtual bool upTrigger(ulong mask) const {
        return (mask & mCoreStatus[0].release) != 0;
    }
    virtual bool downAll(ulong mask) const {
        return (mask & mCoreStatus[0].hold) == mask;
    }
    virtual bool upAll(ulong mask) const {
        return (mask & mCoreStatus[0].hold) == 0;
    }
    virtual void beginFrame(PADStatus *status = NULL);
    virtual void endFrame();

    void sceneReset();
    void startMotor();
    void stopMotor();
    void stopRumbleMgr(); // 80443574
    // > 0 while the pointer is valid (the first status's dpd_valid_fg).
    s32 getDpdValidFlag() const;
    void createRumbleMgr(u8 num); // 80443508
    void startPatternRumble(const char *pattern, int frames,
                            bool force); // 80443560

    Vector3f getAccel() const {
        return mCoreStatus[0].getAccel();
    }
    Vector2f getDpdRawPos() const;
    f32 getFreeStickX();
    f32 getFreeStickY();

    eCoreDevType getDevType() const {
        return mCoreStatus[0].getDevType();
    }

    bool isStable(u8 bits) const {
        return (mStableFlags & bits) == bits;
    }

    bool isConnected() const {
        return mFlag.onBit(0);
    }
    CoreStatus *getCoreStatus() {
        return mCoreStatus;
    }

    /* 0x004 */ int mChannelID;
    /* 0x008 */ u32 mFSStickHold;
    /* 0x00C */ u32 mFSStickTrig;
    /* 0x010 */ u32 mFSStickRelease;
    /* 0x014 */ CoreStatus mCoreStatus[KPAD_MAX_SAMPLES];
    /* 0x854 */ int mKPADReadLength;
    /* 0x858 */ TBitFlag<u8> mFlag;
    /* 0x85C */ Vector3f mAccelPrev;
    /* 0x868 */ Vector2f mDpdPosPrev;
    /* 0x870 */ u32 mIdleTime;
    /* 0x874 */ u32 mSimpleRumblePattern;
    /* 0x878 */ bool mEnableSimpleRumble;
    /* 0x87C */ int mSimpleRumbleFrame;
    /* 0x880 */ u8 mSimpleRumbleSize;
    /* 0x881 */ u8 mSimpleRumbleIndex;
    /* 0x884 */ ControllerRumbleMgr *mRumbleMgr;
    /* 0x888 */ Matrix34f mPostureMtx;
    /* 0x8B8 */ Matrix34f mPostureMtxPrev;
    /* 0x8E8 */ u8 mStableFlags;
    /* 0x8EC */ int mStableFrame;
    /* 0x8F0 */ int mStableTimer[3];
    /* 0x8FC */ f32 mStableMag;
    /* 0x900 */ Vector3f mStableAccel;

  protected:
    void calc_posture_matrix(Matrix34f &mtx, bool checkStable);
};

class CoreControllerMgr {
    class T__Disposer : public Disposer {
      public:
        virtual ~T__Disposer();

        static T__Disposer *sStaticDisposer;
    };

    /* 0x00 */ T__Disposer mDisposer;

  public:
    // 0x10 vtable
    /* vt 0x08 */ virtual void beginFrame();
    /* vt 0x0C */ virtual void endFrame();

    CoreController *getNthController(int index);

    static CoreControllerMgr *createInstance();
    static void deleteInstance();
    static CoreControllerMgr *instance() {
        return sInstance;
    }

    static void setCoreControllerFactory(CoreControllerFactory factory) {
        sCoreControllerFactory = factory;
    }
    static void setConnectCallback(CoreControllerConnectCallback callback) {
        sConnectCallback = callback;
    }

    static u32 sWPADWorkSize;

  protected:
    CoreControllerMgr();

    static void connectCallback(s32 chan, s32 result);

    /* 0x14 */ TBuffer<CoreController *> mControllers;
    /* 0x20 */ CoreControllerExtensionCallback *mExtensionCallback;
    /* 0x24 */ TBuffer<eCoreDevType> mDevTypes;
    /* 0x30 */ u8 mUnk30[0x30];

  private:
    static CoreControllerMgr *sInstance;
    static CoreControllerFactory sCoreControllerFactory;
    static CoreControllerConnectCallback sConnectCallback;
    static bool sUseExternalAllocator;
};

class NullController : public CoreController {
  public:
    NullController() {
        mNullStatus.dev_type = cDEV_NULL;
    }

    /* 0x90C */ CoreStatus mNullStatus;
};

class GCController {
  public:
    void calc();

    /* 0x0 */ PADStatus *mStatus;
    /* 0x4 */ u16 mHold;
};

class GCControllerMgr {
    class T__Disposer : public Disposer {
      public:
        virtual ~T__Disposer();

        static T__Disposer *sStaticDisposer;
    };

    /* 0x00 */ T__Disposer mDisposer;

  public:
    void calc();

    static GCControllerMgr *instance() {
        return sInstance;
    }

    /* 0x10 */ TBuffer<GCController *> mControllers;
    /* 0x1C */ PADStatus mStatus[4];

  private:
    static GCControllerMgr *sInstance;
};

class ControllerRumbleUnit {
  public:
    ControllerRumbleUnit() {
        init();
    }
    virtual ~ControllerRumbleUnit() {
    }

    void startPattern(const char *pattern, int frame);
    void startPowerFrame(float power, int frame);

    f32 calc();

  private:
    enum {
        BIT_USE_PATTERN,
        BIT_USE_POWER,
        BIT_POWER_LOOP,
        BIT_ENABLED,
        BIT_PATTERN_LOOP,
        BIT_PATTERN_TIMER,
    };

    void init();

    void setFlags(u8 bit) {
        mFlags.setBit(bit);
    }
    void clrFlags(u8 bit) {
        mFlags.resetBit(bit);
    }
    bool tstFlags(u8 bit) {
        return mFlags.onBit(bit);
    }

    /* 0x04 */ const char *mRumblePattern;
    /* 0x08 */ const char *mRumblePatternPtr;
    /* 0x0C */ int mRumbleFrame;
    /* 0x10 */ f32 mRumblePower;
    /* 0x14 */ f32 mRumbleValue;
    /* 0x18 */ TBitFlag<u8> mFlags;
    /* 0x1C */ nw4r::ut::Link mLink;
    /* 0x24 */ u32 mUnk24;

    friend class ControllerRumbleMgr;
};

class ControllerRumbleMgr {
  public:
    ControllerRumbleMgr();
    virtual ~ControllerRumbleMgr() {
    }

    void createUnit(u8 overlap_num, CoreController *controller);

    void stop();
    void calc();

    void startPattern(const char *pattern, int frame, bool force);
    void startPowerFrame(float power, int frame, bool force);

  private:
    ControllerRumbleUnit *getUnitFromList(bool force);

    /* 0x04 */ nw4r::ut::List mActiveUnitList;
    /* 0x10 */ nw4r::ut::List mStoppedUnitList;
    /* 0x1C */ CoreController *mController;
};

} // namespace EGG
