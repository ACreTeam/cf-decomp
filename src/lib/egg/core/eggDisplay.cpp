#include <lib/egg/core/eggDisplay.h>

#include <lib/egg/core/eggSystem.h>
#include <lib/egg/core/eggVideo.h>
#include <lib/egg/core/eggXfbManager.h>
#include <revolution/gx.h>
#include <revolution/os.h>
#include <revolution/vi.h>

namespace EGG {

u32 Display::sTickThreshold = OSMicrosecondsToTicks(300);

static void PreRetraceCallback(u32 retraceCount) {
    BaseSystem::mConfigData->getDisplay()->postVRetrace();
}

Display::Display(u8 frameRate)
    : mFrameRate(frameRate),
      mScreenStateFlag(0),
      mRetraceCount(0),
      mFrameCount(0),
      mClearColor(0x808080FF),
      mClearZ(0xFFFFFF),
      mPrevFrameTick(0) {
    mFlag.setBit(0);
    mFlag.setBit(1);
    VISetPreRetraceCallback(PreRetraceCallback);
}

u32 Display::getTickPerFrame() {
    return Video::getTickPerVRetrace();
}

void Display::beginFrame() {
    GXDrawDone();

    if (mFlag.onBit(1) || (BaseSystem::mConfigData->getXfbMgr()->m_0c == 1 &&
                     (u32)(OSGetTick() - mRetraceTick) > sTickThreshold)) {
        VIWaitForRetrace();
    }

    while (BaseSystem::mConfigData->getXfbMgr()->m_04 == 0) {
        VIWaitForRetrace();
    }

    copyEFBtoXFB();
    calcFrequency();
    mFrameCount++;
}

void Display::beginRender() {}

void Display::endRender() {}

void Display::endFrame() {}

void Display::postVRetrace() {
    mRetraceTick = OSGetTick();

    if (VIGetRetraceCount() - mRetraceCount >= mFrameRate) {
        if (mScreenStateFlag & 1) {
            BaseSystem::mConfigData->getVideo()->changeBlack();
            mScreenStateFlag &= ~1;
        }

        BaseSystem::mConfigData->getXfbMgr()->setNextFrameBuffer();
        mRetraceCount = VIGetRetraceCount();
    }
}

void Display::copyEFBtoXFB() {
    if (mFlag.onBit(0)) {
        GXSetCopyClear(mClearColor, mClearZ);
    }

    const GXRenderModeObj *rm = BaseSystem::mConfigData->getVideo()->pRenderMode;
    GXSetCopyFilter(rm->aa, rm->sample_pattern, !rm->aa, rm->vfilter);

    bool clearEfb = mFlag.onBit(0);
    XfbManager *mgr = BaseSystem::mConfigData->getXfbMgr();
    mgr->copyEFB(clearEfb);
}

void Display::calcFrequency() {
    s32 now = OSGetTick();
    mFrameCostTick = now - mPrevFrameTick;

    mFrequency =
        1000000.0f / static_cast<u32>(OSTicksToMicroseconds(mFrameCostTick));

    mPrevFrameTick = now;
}

} // namespace EGG
