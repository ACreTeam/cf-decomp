#pragma once
#include <lib/egg/core/eggDisposer.h>

namespace EGG {

class EffectResource : public Disposer {
public:
    EffectResource(void *effectProject, void *textureProject); // 8044D0D0
    virtual ~EffectResource();
    virtual void set(void *effectProject, void *textureProject); // 8044D220
    static void initialize(); // 8044D33C

private:
    u8 _10[0x10];
}; // 0x20

} // namespace EGG
