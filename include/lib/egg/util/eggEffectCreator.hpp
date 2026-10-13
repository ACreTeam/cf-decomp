#pragma once
#include <lib/egg/core/eggDisposer.h>
#include <nw4r/ef/ef_drawinfo.h>
#include <nw4r/ef/ef_effect.h>

namespace EGG {
// City Folk layout; method names inferred from behaviour.
class EffectCreator : public Disposer {
public:
    EffectCreator(int); // 8044B9F0
    virtual ~EffectCreator(); // 8044BA70
    virtual u32 getGroupID(u32 flags); // 8044BAF8
    virtual void retireAll(); // 8044BB14
    virtual void calcAll(bool); // 8044BB98
    virtual void calc(u32, bool); // 8044BC14
    virtual void draw(const nw4r::ef::DrawInfo &, u32); // 8044BC68
    virtual nw4r::ef::Effect *create(const char *); // 8044BCBC
    /* 0x10 */ u32 _10;
    /* 0x14 */ u32 mGroupBase;
}; // size 0x18
}
