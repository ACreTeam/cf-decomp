#include <lib/egg/util/eggEffectMemoryManager.hpp>
#include <lib/egg/core/eggHeap.h>
#include <lib/egg/util/eggEffectCreator.hpp>
#include <lib/egg/util/eggEffectManager.hpp>
#include <lib/egg/util/eggEffectResource.hpp>
#include <string.h>

namespace EGG {

EffectCreator::EffectCreator(int creator) : mCreator(creator) {
    EffectManager *manager = EffectManager::sEffectManager;
    if (manager != NULL) {
        mGroupBase = creator << manager->getGroupBits();
        manager->setCreator(this, mCreator);
    }
}

EffectCreator::~EffectCreator() {
    retireAll();
    if (EffectManager::sEffectManager != NULL) {
        EffectManager::sEffectManager->resetCreator(mCreator);
    }
}

u32 EffectCreator::getGroupID(u32 flags) {
    if (flags & 0x8000) {
        return (flags & 1) + 2;
    }
    return flags & 1;
}

void EffectCreator::retireAll() {
    nw4r::ef::EffectSystem *system = nw4r::ef::EffectSystem::GetInstance();
    if (EffectManager::sEffectManager != NULL) {
        int groups = EffectManager::sEffectManager->getGroupNum();
        for (int i = 0; i < groups; i++) {
            u32 group = mGroupBase | i;
            system->RetireEmitterAll(group);
            system->RetireParticleAll(group);
        }
    }
}

void EffectCreator::calcAll(bool onlyBillboard) {
    nw4r::ef::EffectSystem *system = nw4r::ef::EffectSystem::GetInstance();
    if (EffectManager::sEffectManager != NULL) {
        int groups = EffectManager::sEffectManager->getGroupNum();
        for (int i = 0; i < groups; i++) {
            system->Calc(mGroupBase | i,
                         static_cast<nw4r::ef::CalcOption>(onlyBillboard));
        }
    }
}

void EffectCreator::calc(u32 group, bool onlyBillboard) {
    nw4r::ef::EffectSystem::GetInstance()->Calc(
        mGroupBase | group, static_cast<nw4r::ef::CalcOption>(onlyBillboard));
}

void EffectCreator::draw(const nw4r::ef::DrawInfo &info, u32 group) {
    nw4r::ef::EffectSystem::GetInstance()->Draw(info, mGroupBase | group);
}

nw4r::ef::Effect *EffectCreator::create(const char *name) {
    nw4r::ef::ResEmitter resource =
        nw4r::ef::Resource::GetInstance()->FindEmitter(name, NULL);
    if (resource.IsValid()) {
        nw4r::ef::EffectSystem *system = nw4r::ef::EffectSystem::GetInstance();
        nw4r::ef::EmitterResource *emitter = resource.ptr();
        u8 *userData = emitter->GetEmitterDesc()->userdata;
        u32 group = mGroupBase | getGroupID((userData[2] << 8) | userData[3]);
        return system->CreateEffect(emitter, group, 0);
    }
    return NULL;
}

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

    nw4r::ef::EffectSystem::GetInstance()->SetMemoryManager(memoryManager,
                                                            mTotalGroupNum);
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
