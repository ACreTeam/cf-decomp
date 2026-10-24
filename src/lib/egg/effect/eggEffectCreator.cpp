#include <lib/egg/util/eggEffectCreator.hpp>
#include <lib/egg/util/eggEffectManager.hpp>
#include <nw4r/ef.h>

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
        return (flags & 1) ? 3 : 2;
    }
    return (flags & 1) ? 1 : 0;
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

} // namespace EGG
