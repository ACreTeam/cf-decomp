#include <lib/egg/util/eggEffectMemoryManager.hpp>
#include <lib/egg/core/eggHeap.h>
#include <lib/egg/util/eggEffectCreator.hpp>
#include <lib/egg/util/eggEffectManager.hpp>
#include <lib/egg/util/eggEffectResource.hpp>
#include <string.h>

namespace EGG {

EffectManager *EffectManager::sEffectManager;

EffectManager *EffectManager::createInstance(int creators, int groups,
                                             Heap *heap) {
    if (sEffectManager == NULL) {
        EffectResource::initialize();
        if (heap == NULL) {
            heap = Heap::getCurrentHeap();
        }
        sEffectManager = new (heap, 4) EffectManager(creators, groups, heap);
    }
    return sEffectManager;
}

int EffectManager::initialize(Configuration &config) {
    if (config.heap == NULL) {
        config.heap = Heap::getCurrentHeap();
    }

    u32 size = MemoryManager::CalcMemorySize(config.effects, config.emitters,
                                             config.managers, config.particles,
                                             mTotalGroupNum);
    mMemory = Heap::alloc(size, 32, config.heap);

    MemoryManager *memoryManager = new (config.heap, 4)
        MemoryManager(mMemory, size, config.effects, config.emitters,
                      config.managers, config.particles);

    nw4r::ef::EffectSystem *system = nw4r::ef::EffectSystem::GetInstance();
    system->SetMemoryManager(memoryManager, mTotalGroupNum);
    return size;
}

void EffectManager::setCreator(EffectCreator *creator, int index) {
    mCreators[index] = creator;
}

void EffectManager::resetCreator(int index) {
    mCreators[index] = NULL;
}

void EffectManager::shutdown() {
    nw4r::ef::EffectSystem *system = nw4r::ef::EffectSystem::GetInstance();
    for (u32 i = 0; i < mTotalGroupNum; i++) {
        system->RetireEmitterAll(i);
        system->RetireParticleAll(i);
    }
}

void EffectManager::setCamera(const nw4r::math::VEC3 *pos,
                              const nw4r::math::MTX34 *mtx, f32 near, f32 far,
                              int group, bool onlyBillboard) {
    nw4r::ef::EffectSystem::GetInstance()->SetProcessCamera(*pos, *mtx, near,
                                                            far);
    for (int i = 0; i < mCreatorNum; i++) {
        if (mCreators[i] != NULL) {
            mCreators[i]->calc(group, onlyBillboard);
        }
    }
}

void EffectManager::draw(nw4r::ef::DrawInfo *info, int creator, int group) {
    if (mCreators[creator] != NULL) {
        mCreators[creator]->draw(*info, group);
    }
}

nw4r::ef::Effect *EffectManager::createEffect(const char *name, int creator) {
    return mCreators[creator]->create(name);
}

int EffectManager::getGroupNum() const {
    return mGroupNum;
}

void EffectManager::setXFFlushSafe(bool safe) {
    nw4r::ef::EffectSystem::GetInstance()->SetXFFlushSafe(safe);
}

int EffectManager::getGroupBits() const {
    int bits = 0;
    for (int n = mGroupNum - 1; n != 0; n >>= 1) {
        bits++;
    }
    return bits;
}

EffectManager::EffectManager(int creators, int groups, Heap *heap)
    : mCreatorNum(creators), mGroupNum(groups) {
    mTotalGroupNum = creators << getGroupBits();
    strcpy(mDummyTexName, "dummy_tex");
    mDummyTexName[0x1F] = '\0';
    mCreators = new (heap, 4) EffectCreator *[mCreatorNum];
    for (int i = 0; i < mCreatorNum; i++) {
        mCreators[i] = NULL;
    }
    mMemory = NULL;
    setXFFlushSafe(true);
}

} // namespace EGG
