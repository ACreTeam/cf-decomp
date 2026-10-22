#pragma once
#include <lib/egg/core/eggDisposer.h>
#include <nw4r/ef/ef_drawinfo.h>
#include <nw4r/ef/ef_effect.h>

namespace EGG {

// Creates effects of one "creator slot": the effect group ids it hands out are
// (mGroupBase | group), where mGroupBase is the creator index shifted above the group bits.
// The game derives its own (dEffectGroupManager), overriding getGroupID.
class EffectCreator : public Disposer {
public:
    EffectCreator(int creator);                  // 8044B9F0
    virtual ~EffectCreator();                    // 8044BA70
    virtual u32 getGroupID(u32 flags);           // 8044BAF8
    virtual void retireAll();                    // 8044BB14
    virtual void calcAll(bool onlyBillboard);    // 8044BB98
    virtual void calc(u32 group, bool onlyBillboard); // 8044BC14
    virtual void draw(const nw4r::ef::DrawInfo &info, u32 group); // 8044BC68
    virtual nw4r::ef::Effect *create(const char *name);           // 8044BCBC

    /* 0x10 */ int mCreator;
    /* 0x14 */ u32 mGroupBase;
}; // size 0x18

} // namespace EGG
