#ifndef NW4R_EF_EFFECT_SYSTEM_H
#define NW4R_EF_EFFECT_SYSTEM_H
#include <nw4r/types_nw4r.h>

#include <nw4r/ef/ef_creationqueue.h>
#include <nw4r/ef/ef_random.h>
#include <nw4r/math.h>

namespace nw4r {
namespace ef {

// Forward declarations
class ActivityList;
class DrawInfo;
class DrawOrderBase;
class DrawStrategyBuilder;
class Effect;
class EmitFormBuilder;
struct EmitterResource;
class MemoryManagerBase;

class EffectSystem {
  private:
    MemoryManagerBase *mMemoryManager; // at 0x0

  public:
    DrawOrderBase *mDrawOrderFunc;             // at 0x4
    DrawStrategyBuilder *mDrawStrategyBuilder; // at 0x8
    EmitFormBuilder *mEmitFormBuilder;         // at 0xC
    CreationQueue mCreationQueue;              // at 0x10
    u32 mMaxGroupID;                           // at 0xC014
    ActivityList *mActivityList;               // at 0xC018
    Random mRandom;                            // at 0xC01C
    math::VEC3 mProcessCameraPos;              // at 0xC020
    math::MTX34 mProcessCameraMtx;             // at 0xC02C
    f32 mProcessCameraFar;                     // at 0xC05C
    f32 mProcessCameraNear;                    // at 0xC060
    bool mXFFlushSafe;                         // at 0xC064

    static int mDisplayVersion;
    static DrawInfo mSortEffectOperatorZDrawInfo;
    static EffectSystem instance;

  public:
    static EffectSystem *GetInstance();

    EffectSystem();
    ~EffectSystem();

    bool Initialize(u32 maxGroupID);
    bool Closing(Effect *pEffect);

    Effect *CreateEffect(const char *pName, u32 groupID, u16 calcRemain);
    // CF: the emitter resource is looked up by the caller
    // (EGG::EffectCreator::create)
    Effect *CreateEffect(EmitterResource *pResource, u32 groupID,
                         u16 calcRemain);
    u32 RetireEffect(Effect *pEffect);

    u32 GetNumEffect(u32 groupID) const; // inferred name
    u32 ForeachParticleManager(ForEachFunc pFunc, ForEachParam param,
                               u32 groupID,
                               bool ignoreLifeStatus); // inferred name

    u32 RetireEffectAll(u32 groupID);
    u32 RetireEmitterAll(u32 groupID);
    u32 RetireParticleAll(u32 groupID);

    void Calc(u32 groupID, CalcOption option);
    // CF: sets the camera of the processing pass (position, view matrix, near
    // and far plane)
    void SetProcessCamera(const math::VEC3 &position, const math::MTX34 &matrix,
                          f32 nnear, f32 ffar);
    void Draw(const DrawInfo &rInfo, u32 groupID);

    MemoryManagerBase *GetMemoryManager() const {
        return mMemoryManager;
    }
    void SetMemoryManager(MemoryManagerBase *pManager, u32 maxGroupID) {
        mMemoryManager = pManager;

        if (mMemoryManager != NULL) {
            Initialize(maxGroupID);
        } else {
            mMaxGroupID = 0;
            mActivityList = NULL;
        }
    }

    void SetXFFlushSafe(bool safe) {
        mXFFlushSafe = safe;
    }
};

} // namespace ef
} // namespace nw4r

#endif
