#pragma once

#include <types.h>
#include <lib/egg/core/eggBitFlag.h>
#include <lib/egg/core/eggDisposer.h>
#include <lib/egg/math/eggVector.h>
#include <revolution/PAD.h>
#include <revolution/KPAD.h>

namespace EGG {

// Layouts from the Skyward Sword decompilation (zeldaret/ss, egg/core/eggController.h), adjusted to
// CF's older EGG (KPADStatus is 0x84 bytes here, and the status array starts at 0x14).
class CoreStatus : public KPADStatus {
public:
    void init();

    Vector2f getAccelVertical() {
        return Vector2f(acc_vertical.x, acc_vertical.y);
    }
};

class CoreController {
public:
    virtual void setPosParam(float a, float b) { KPADSetPosParam(mNum, a, b); }
    virtual void setHoriParam(float, float);
    virtual void setDistParam(float, float);
    virtual void setAccParam(float, float);
    virtual bool down(ulong) const;
    virtual bool up(ulong) const;
    virtual bool downTrigger(ulong) const;
    virtual bool upTrigger(ulong) const;
    virtual bool downAll(ulong) const;
    virtual bool upAll(ulong) const;
    virtual void beginFrame(PADStatus *);
    virtual void endFrame();

    void sceneReset();
    void stopRumbleMgr(); // 80443574
    // > 0 while the pointer is valid (the first status's dpd_valid_fg).
    s32 getDpdValidFlag() const;
    void startPatternRumble(const char *, int, bool);

    bool isConnected() const { return mFlag.onBit(0); }
    CoreStatus *getCoreStatus() { return mCoreStatus; }

    /* 0x004 */ int mNum;
    /* 0x008 */ u32 mFSStickHold;
    /* 0x00C */ u32 mFSStickTrig;
    /* 0x010 */ u32 mFSStickRelease;
    /* 0x014 */ CoreStatus mCoreStatus[16];
    /* 0x854 */ int mKPADReadLength;
    /* 0x858 */ TBitFlag<u8> mFlag;
};

class CoreControllerMgr {
    class T__Disposer : public Disposer {
    public:
        virtual ~T__Disposer();
    };

    /* 0x00 */ T__Disposer mDisposer;

public:
    // 0x10 vtable
    /* vt 0x08 */ virtual void beginFrame();
    /* vt 0x0C */ virtual void endFrame();

    CoreController *getNthController(int index);

    static void createInstance();
    static CoreControllerMgr *instance() { return sInstance; }

    static u32 sWPADWorkSize;

private:
    static CoreControllerMgr *sInstance;
};

} // namespace EGG
