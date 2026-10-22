#include <lib/egg/core/eggSystem.h>

#include <lib/egg/core/eggController.h>
#include <lib/egg/core/eggExpHeap.h>
#include <revolution/OS.h>

namespace EGG {

ConfigurationData *BaseSystem::mConfigData;

void ConfigurationData::initMemory() {
    void *mem1Lo = OSGetMEM1ArenaLo();
    void *mem1Hi = OSGetMEM1ArenaHi();
    void *mem2Lo = OSGetMEM2ArenaLo();
    void *mem2Hi = OSGetMEM2ArenaHi();

    void *mem1Arena = OSInitAlloc(mem1Lo, mem1Hi, 2);
    void *mem2Arena = OSInitAlloc(mem2Lo, mem2Hi, 2);

    mem1Lo = (void *)(((u32)mem1Arena + 31) & ~31);
    mem2Lo = (void *)(((u32)mem2Arena + 31) & ~31);

    mem1Arena = (void *)((u32)mem1Hi & ~31);
    mem2Arena = (void *)((u32)mem2Hi & ~31);

    OSBootInfo *bootInfo = (OSBootInfo *)OSPhysicalToCached(0);

    mCodeStart = bootInfo;
    mCodeEnd = mem1Lo;

    mMem1Start = mem1Lo;
    mMem1End = mem1Arena;

    mMem2Start = mem2Lo;
    mMem2End = mem2Arena;

    mMemorySize = bootInfo->physMemSize;

    OSSetMEM1ArenaLo(mem1Arena);
    OSSetMEM1ArenaHi(mem1Arena);

    OSSetMEM2ArenaLo(mem2Arena);
    OSSetMEM2ArenaHi(mem2Arena);

    Heap::initialize();

    u32 heapSizeMem1 = (u32)mMem1End - (u32)mMem1Start;
    u32 heapSizeMem2 = (u32)mMem2End - (u32)mMem2Start;
    u32 heapSizeDebug;

    if (heapSizeMem2 >= 0x4000000) {
        heapSizeDebug = 0x4000000;
        heapSizeMem2 = (heapSizeMem2 - heapSizeDebug) & ~31;
    } else {
        heapSizeDebug = 0;
    }

    mem1Lo = mMem1Start;
    mem2Lo = mMem2Start;
    void *memDebugStart = (u8 *)mem2Lo + heapSizeMem2;

    mRootHeapMem1 = ExpHeap::create(mem1Lo, heapSizeMem1, 0);
    mRootHeapMem1->setName("EGGRootMEM1");

    mRootHeapMem2 = ExpHeap::create(mem2Lo, heapSizeMem2, 0);
    mRootHeapMem2->setName("EGGRootMEM2");

    if (heapSizeDebug > 0) {
        mRootHeapDebug = ExpHeap::create(memDebugStart, heapSizeDebug, 0);
        mRootHeapDebug->setName("EGGRootDebug");
    } else {
        mRootHeapDebug = NULL;
    }

    mSystemHeap = ExpHeap::create(mSysHeapSize, mRootHeapMem1, 0);
    mSystemHeap->setName("EGGSystem");
    mSystemHeap->becomeCurrentHeap();
}

void ConfigurationData::run() {
    CoreControllerMgr *controllerMgr = CoreControllerMgr::instance();
    GCControllerMgr *gcMgr = GCControllerMgr::instance();

    while (true) {
        getDisplay()->beginFrame();
        onBeginFrame();
        getPerfView()->measureBeginFrame();

        controllerMgr->beginFrame();
        gcMgr->calc();

        getDisplay()->beginRender();
        getPerfView()->measureBeginRender();

        getSceneMgr()->draw();
        getPerfView()->draw();

        getPerfView()->measureEndRender();
        getDisplay()->endRender();

        getSceneMgr()->calc();

        controllerMgr->endFrame();

        if (getAudioMgr() != NULL) {
            getAudioMgr()->calc();
        }

        getDisplay()->endFrame();
        onEndFrame();
        getPerfView()->measureEndFrame();
    }
}

} // namespace EGG
