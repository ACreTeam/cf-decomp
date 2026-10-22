#pragma once
#include <game/mLib/m_heap.hpp>
#include <game/game/d_screenshot.hpp>
#include <lib/egg/core/eggAsyncDisplay.h>
#include <lib/egg/core/eggController.h>
#include <lib/egg/core/eggDvdFile.h>
#include <lib/egg/core/eggException.h>
#include <lib/egg/core/eggGraphicsFifo.h>
#include <lib/egg/core/eggHeap.h>
#include <lib/egg/core/eggThread.h>
#include <lib/egg/core/eggVideo.h>
#include <lib/egg/core/eggXfb.h>
#include <lib/egg/core/eggXfbManager.h>
#include <lib/egg/gfxe/eggGfxEngine.h>
#include <lib/egg/gfxe/eggScreen.h>
#include <lib/egg/gfxe/eggStateGX.h>
#include <lib/nw4r/db.h>
#include <revolution/gx.h>
#include <revolution/sc.h>
#include <revolution/kpad.h>
#include <revolution/wpad.h>

namespace EGG {

class PerformanceView {
public:
    virtual void measureBeginFrame() = 0;         // 0x08
    virtual void measureEndFrame() = 0;           // 0x0C
    virtual void measureBeginRender() = 0;        // 0x10
    virtual void measureEndRender() = 0;          // 0x14
    virtual void callbackDrawSync(u16 token) = 0; // 0x18
    virtual void VF_0x1C() {}                     // 0x1C
    virtual void draw() = 0;                      // 0x20
    virtual void setVisible(bool visible) = 0;    // 0x24
    virtual bool isVisible() = 0;                 // 0x28
};
class ProcessMeter : public Thread, public PerformanceView {};

class SceneManager {
public:
    virtual void calc(); // 0x08
    virtual void draw(); // 0x0C
};

class SimpleAudioMgr {
public:
    virtual void initialize(); // 0x08
    virtual void calc();       // 0x0C
};

class ConfigurationData {
public:
    ConfigurationData() : mSysHeapSize(0x500000) {}

    virtual Video *getVideo() = 0;
    virtual Heap *getSystemHeap() = 0;
    virtual Display *getDisplay() = 0;
    virtual XfbManager *getXfbMgr() = 0;
    virtual PerformanceView *getPerfView() = 0;
    virtual SceneManager *getSceneMgr() = 0;
    virtual SimpleAudioMgr *getAudioMgr() = 0;
    virtual void onBeginFrame() {}
    virtual void onEndFrame() {}
    virtual void initRenderMode() {}
    virtual void initMemory();
    virtual void run();

    void *mMem1Start;     // 0x04
    void *mMem1End;       // 0x08
    void *mMem2Start;     // 0x0C
    void *mMem2End;       // 0x10
    u32 mMemorySize;      // 0x14
    Heap *mRootHeapMem1;  // 0x18
    Heap *mRootHeapMem2;  // 0x1C
    Heap *mRootHeapDebug; // 0x20
    Heap *mSystemHeap;    // 0x24
    Thread *mThread;      // 0x28
    void *mCodeStart;     // 0x2C
    void *mCodeEnd;       // 0x30
    u32 mSysHeapSize;     // 0x34
    u32 mGraphicsFifoSize;
    GXRenderModeObj *mRenderMode;
};

class BaseSystem {
public:
    static ConfigurationData *mConfigData;
};

template <
    class TVideo = Video,
    class TDisplay = AsyncDisplay,
    class TXfbManager = XfbManager,
    class TAudioMgr = SimpleAudioMgr,
    class TSceneManager = SceneManager,
    class TPerformanceView = ProcessMeter
>
class TSystem : public BaseSystem {
public:
    class Configuration : public ConfigurationData {
    public:
        Configuration() {
            mGraphicsFifoSize = 0x80000;
            mRenderMode = nullptr;
        }

        virtual Video *getVideo() { return mpVideo; }
        virtual Heap *getSystemHeap() { return mSystemHeap; }
        virtual Display *getDisplay() { return mpDisplay; }
        virtual XfbManager *getXfbMgr() { return mpXfbMgr; }
        virtual PerformanceView *getPerfView() { return mpPerfView; }
        virtual SceneManager *getSceneMgr() { return mpSceneMgr; }
        virtual SimpleAudioMgr *getAudioMgr() { return mpAudioMgr; }

        virtual void initialize();

        TAudioMgr *mpAudioMgr;
        TVideo *mpVideo;
        TXfbManager *mpXfbMgr;
        TDisplay *mpDisplay;
        TPerformanceView *mpPerfView;
        TSceneManager *mpSceneMgr;
    };

    static Configuration *sConfiguration;
};

} // namespace EGG
