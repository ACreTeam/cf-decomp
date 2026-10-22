#pragma once
#include <lib/egg/core/eggDisposer.h>
#include <nw4r/ef/ef_resource.h>
#include <nw4r/ut.h>

namespace EGG {

// A loaded .breff / .breft pair (nw4r::ef::Resource keeps the data; this just owns the projects).
class EffectResource : public Disposer {
public:
    EffectResource(void *effectProject, void *textureProject); // 8044D0D0
    virtual ~EffectResource();                                 // 8044D16C
    virtual void setResource(void *effectProject, void *textureProject); // 8044D220
    virtual void resetResource();                                        // 8044D2AC
    static void initialize(); // 8044D33C

    static nw4r::ut::List &getList() {
        return sList;
    }
    bool isActive() const {
        return mEffectProject != NULL && mTextureProject != NULL;
    }

private:
    /* 0x10 */ nw4r::ef::EffectProject *mEffectProject;
    /* 0x14 */ nw4r::ef::TextureProject *mTextureProject;
    /* 0x18 */ nw4r::ut::Link mLink;

    static nw4r::ut::List sList; // 80746E70
}; // 0x20

} // namespace EGG
