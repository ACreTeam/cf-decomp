#pragma once

#include <lib/egg/core/eggDisplay.h>
#include <revolution/os.h>

namespace EGG {

class AsyncDisplay : public Display {
public:
    AsyncDisplay(u8 frameRate);

    virtual void beginFrame();     // 0x08
    virtual void endRender();      // 0x10
    virtual void endFrame();       // 0x14
    virtual u32 getTickPerFrame(); // 0x18
    virtual void postVRetrace();   // 0x1C

    void clearEFB();

private:
    void calcTimeScale();
    void clearEFB(u16 fbWidth, u16 fbHeight, u16 x, u16 y, u16 width, u16 height, nw4r::ut::Color color);

    u8 _2C[0x34];                  // 0x2C
    OSThreadQueue mSyncQueue;      // 0x60
    OSMessageQueue mMsgQueue;      // 0x68
    OSMessage mMsgBuf[4];          // 0x88
    int mSyncMode;                 // 0x98
    f32 mTimeScale;                // 0x9C
    f32 mTimeStep;                 // 0xA0
    u32 mRetraceCountA;            // 0xA4
    u32 mRetraceCountB;            // 0xA8
    int mPendingFrames;            // 0xAC
    u32 _B0;                       // 0xB0
    u32 _B4;                       // 0xB4
    u32 _B8;                       // 0xB8
    u32 _BC;                       // 0xBC
};

} // namespace EGG
