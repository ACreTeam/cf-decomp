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
#include <revolution/GX.h>
#include <revolution/SC.h>
#include <revolution/WPAD.h>

extern "C" void fn_803BCE8C(s8, s8); // Default controller pointer parameters.

namespace EGG {

class PerformanceView { u8 _0000[4]; };
class ProcessMeter : public Thread, public PerformanceView {};
class SceneManager;
class SimpleAudioMgr;

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

    u8 mPad[0x14];
    Heap *mRootHeapMem1;
    Heap *mRootHeapMem2;
    Heap *mRootHeapDebug;
    Heap *mSystemHeap;
    Thread *mThread;
    u8 mPad2[0x8];
    u32 mSysHeapSize;
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
