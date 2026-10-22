#include <lib/egg/util/eggEffectResource.hpp>
#include <nw4r/ef.h>

namespace EGG {

nw4r::ut::List EffectResource::sList;

EffectResource::EffectResource(void *effectProject, void *textureProject)
    : mEffectProject(NULL), mTextureProject(NULL) {
    if (effectProject != NULL && textureProject != NULL) {
        setResource(effectProject, textureProject);
    }
    nw4r::ut::List_Append(&getList(), this);
}

EffectResource::~EffectResource() {
    if (isActive()) {
        resetResource();
    }
    nw4r::ut::List_Remove(&getList(), this);
}

void EffectResource::setResource(void *effectProject, void *textureProject) {
    if (mEffectProject == NULL && mTextureProject == NULL) {
        nw4r::ef::Resource *resource = nw4r::ef::Resource::GetInstance();
        mEffectProject = resource->Add(static_cast<u8 *>(effectProject));
        mTextureProject = resource->AddTexture(static_cast<u8 *>(textureProject));
        resource->RelocateCommand();
    }
}

void EffectResource::resetResource() {
    bool changed = false;
    nw4r::ef::Resource *resource = nw4r::ef::Resource::GetInstance();
    if (mEffectProject != NULL) {
        resource->RemoveEffectProject(mEffectProject);
        mEffectProject = NULL;
        changed = true;
    }
    if (mTextureProject != NULL) {
        resource->RemoveTextureProject(mTextureProject);
        mTextureProject = NULL;
        changed = true;
    }
    if (changed) {
        resource->RelocateCommand();
    }
}

void EffectResource::initialize() {
    nw4r::ut::List_Init(&getList(), offsetof(EffectResource, mLink));
}

} // namespace EGG
