#pragma once

#include <lib/egg/core/eggBitFlag.h>
#include <lib/nw4r/ut.h>
#include <types.h>

namespace EGG {

class Display : public TBitFlag<u8> {
public:
    Display(u8 frameRate);

    virtual void beginFrame();     // 0x08
    virtual void beginRender();    // 0x0C
    virtual void endRender();      // 0x10
    virtual void endFrame();       // 0x14
    virtual u32 getTickPerFrame(); // 0x18
    virtual void postVRetrace();   // 0x1C

    void setClearColor(nw4r::ut::Color color) { mClearColor = color; }
    nw4r::ut::Color getClearColor() const { return mClearColor; }

    u8 mFrameRate;                 // 0x08
    volatile u8 mScreenStateFlag;  // 0x09
    u32 mRetraceCount;             // 0x0C
    u32 mFrameCount;               // 0x10
    nw4r::ut::Color mClearColor;   // 0x14
    u32 mClearZ;                   // 0x18
    s32 mPrevFrameTick;            // 0x1C
    s32 mRetraceTick;              // 0x20
    s32 mFrameCostTick;            // 0x24
    f32 mFrequency;                // 0x28

protected:
    static u32 sTickThreshold;

    void copyEFBtoXFB();
    void calcFrequency();
};

} // namespace EGG
