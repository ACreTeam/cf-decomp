#include "revolution/hbm/HBMCommon.h"
#include "revolution/hbm/homebutton/HBMBase.hpp"

#include "revolution/hbm/homebutton/HBMAxSound.hpp"
#include "revolution/mem.h"
#include "revolution/version.h"

RVL_LIB_VERSION(HBM, "Jun 16 2008", "18:35:58", "0x4199_60831");

static MEMAllocator sAllocator;
MEMAllocator* spAllocator = &sAllocator;

void* HBMAllocMem(u32 size)
{
    return MEMAllocFromAllocator(spAllocator, size);
}

void HBMFreeMem(void* mem)
{
    MEMFreeToAllocator(spAllocator, mem);
}

enum HBMAllocatorType
{
    HBM_ALLOCATOR_APPLI,
    HBM_ALLOCATOR_LOCAL,
    HBM_ALLOCATOR_NW4HBM,
};

static HBMAllocatorType getAllocatorType(const HBMDataInfo* pHBInfo)
{
    if (pHBInfo->pAllocator)
        return HBM_ALLOCATOR_APPLI;
    else if (pHBInfo->mem)
        return HBM_ALLOCATOR_LOCAL;
    else
        return HBM_ALLOCATOR_NW4HBM;
}

void HBMCreate(const HBMDataInfo* pHBInfo)
{
    MEMHeapHandle hExpHeap;

    if (getAllocatorType(pHBInfo) == HBM_ALLOCATOR_LOCAL)
    {
        hExpHeap = MEMCreateExpHeapEx(pHBInfo->mem, pHBInfo->memSize, 0);
        MEMInitAllocatorForExpHeap(&sAllocator, hExpHeap, 32);
        spAllocator = &sAllocator;
    }

    switch (getAllocatorType(pHBInfo))
    {
    case HBM_ALLOCATOR_APPLI:
        nw4hbm::lyt::Layout::SetAllocator(pHBInfo->pAllocator);
        spAllocator = pHBInfo->pAllocator;
        break;

    case HBM_ALLOCATOR_LOCAL:
        nw4hbm::lyt::Layout::SetAllocator(spAllocator);
        break;

    case HBM_ALLOCATOR_NW4HBM:
        NW4HBMAssert(nw4hbm::lyt::Layout::GetAllocator());

        spAllocator = nw4hbm::lyt::Layout::GetAllocator();
        break;
    }

    homebutton::HomeButton::createInstance(pHBInfo);
    homebutton::HomeButton::getInstance()->create();
}

void HBMDelete()
{
    const HBMDataInfo* pHBInfo = homebutton::HomeButton::getInstance()->getHBMDataInfo();

    homebutton::HomeButton::deleteInstance();
    if (getAllocatorType(pHBInfo) == HBM_ALLOCATOR_LOCAL)
        MEMDestroyExpHeap((MEMHeapHandle)spAllocator->heap);
}

void HBMInit()
{
    NW4HBMAssert(homebutton::HomeButton::getInstance());

    OSRegisterVersion(__HBMVersion);
    homebutton::HomeButton::getInstance()->init();
}

HBMSelectBtnNum HBMCalc(const HBMControllerData* pController)
{
    NW4HBMAssert(homebutton::HomeButton::getInstance());

    homebutton::HomeButton::getInstance()->calc(pController);
    return homebutton::HomeButton::getInstance()->getSelectBtnNum();
}

void HBMDraw()
{
    NW4HBMAssert(homebutton::HomeButton::getInstance());

    homebutton::HomeButton::getInstance()->draw();
}

HBMSelectBtnNum HBMGetSelectBtnNum()
{
    return homebutton::HomeButton::getInstance()->getSelectBtnNum();
}

void HBMSetAdjustFlag(bool flag)
{
    homebutton::HomeButton::getInstance()->setAdjustFlag(flag);
}

void HBMStartBlackOut()
{
    homebutton::HomeButton::getInstance()->startBlackOut();
}

bool HBMIsReassignedControllers()
{
    NW4HBMAssert(homebutton::HomeButton::getInstance());
    return homebutton::HomeButton::getInstance()->getReassignedFlag();
}

void HBMSetBlackOutColor(u8 red, u8 green, u8 blue)
{
    homebutton::HomeButton::getInstance()->setBlackOutColor(red, green, blue);
}

void HBMSetVIBlack(BOOL flag)
{
    homebutton::HomeButton::getInstance()->setVIBlack(flag);
}

void HBMCreateSound(const char* soundData, void* work, u32 workSize)
{
    homebutton::HomeButton::getInstance()->createSound(soundData, work, workSize);
}

void HBMDeleteSound()
{
    homebutton::ShutdownAxSound();
}

void HBMUpdateSound()
{
    homebutton::HomeButton::getInstance()->update_sound();
}
