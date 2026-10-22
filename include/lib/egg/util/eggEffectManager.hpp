#pragma once
#include <nw4r/math.h>
#include <nw4r/ef/ef_drawinfo.h>

namespace nw4r {
namespace ef {
class Effect;
}
} // namespace nw4r

namespace EGG {
class EffectCreator;
class Heap;

// City Folk's EGG::EffectManager: owns the effect memory pool (nw4r::ef::MemoryManager, see
// eggEffectManager.cpp) and the table of EffectCreators, one per creator slot.
class EffectManager {
public:
    struct Configuration {
        Heap *heap;
        int effects;
        int emitters;
        int managers; // particle managers
        int particles;
    };

    static EffectManager *createInstance(int creators, int groups, Heap *heap); // 8044BD68
    static EffectManager *sEffectManager;

    EffectManager(int creators, int groups, Heap *heap); // 8044C4F0
    int initialize(Configuration &config);                // 8044BDF4
    void setCreator(EffectCreator *creator, int index);   // 8044C2FC
    void resetCreator(int index);                         // 8044C30C
    void shutdown();                                      // 8044C320
    void setCamera(const nw4r::math::VEC3 *pos, const nw4r::math::MTX34 *mtx, f32 near, f32 far,
                   int group, bool onlyBillboard);         // 8044C390
    void draw(nw4r::ef::DrawInfo *info, int creator, int group); // 8044C448
    nw4r::ef::Effect *createEffect(const char *name, int creator); // 8044C474
    int getGroupNum() const;                              // 8044C490
    void setXFFlushSafe(bool safe);                       // 8044C498
    int getGroupBits() const;                             // 8044C4CC

    /* 0x00 */ int mCreatorNum;
    /* 0x04 */ int mGroupNum;      // effect groups per creator
    /* 0x08 */ int mTotalGroupNum; // mCreatorNum << getGroupBits()
    /* 0x0C */ char mDummyTexName[0x20];
    /* 0x2C */ EffectCreator **mCreators;
    /* 0x30 */ void *mMemory; // memory block of the nw4r::ef::MemoryManager
}; // size 0x34

} // namespace EGG
