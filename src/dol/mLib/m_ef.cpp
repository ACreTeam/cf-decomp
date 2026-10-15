#include <game/mLib/m_effect.hpp>
#include <lib/egg/core/eggExpHeap.h>
#include <lib/egg/util/eggEffectResource.hpp>
#include <lib/egg/util/eggExEffect.hpp>
#include <string.h>

namespace mEf {

// The end sentinel is an object pointer whose link fields overlay this list's root.
template <typename T, int Offset> class effectList_c {
public:
    class iterator {
    public:
        iterator(T *object) : ptr(object) {}
        T *operator->() const { return ptr; }
        operator T *() const { return ptr; }
        iterator &operator++() { ptr = ptr->mNext; return *this; }
        bool operator!=(const iterator &other) const { return ptr != other.ptr; }
        T *ptr;
    };

    effectList_c() { mLast = mFirst = end(); mCount = 0; }
    T *end() { return (T *)((u8 *)this - Offset); }
    iterator begin() const {
        T *root = head();
        if (root != mFirst) return iterator(mFirst);
        return iterator(root);
    }
    void pushBack(T *object) {
        T *sentinel = (T *)((u8 *)this - Offset);
        if (sentinel == mFirst) {
            object->mNext = sentinel;
            object->mPrev = sentinel;
            mFirst = object;
            mLast = object;
            mCount++;
        } else {
            object->mPrev = mLast;
            object->mNext = sentinel;
            mLast->mNext = object;
            mLast = object;
            mCount++;
        }
    }
    bool isEnd(T *object) const { return head() == object; }
    void remove(T *object) {
        T *next = object->mNext;
        T *prev = object->mPrev;
        if (isEnd(prev)) mFirst = next;
        else prev->mNext = next;
        next = object->mNext;
        if (end() == next) mLast = object->mPrev;
        else next->mPrev = object->mPrev;
        object->mPrev = NULL;
        object->mNext = NULL;
        mCount--;
    }
    iterator erase(iterator it) {
        iterator next = it.ptr->mNext ? iterator(it.ptr->mNext) : iterator(end());
        T *object = it.ptr;
        if (isEnd(object->mPrev)) mFirst = object->mNext;
        else object->mPrev->mNext = object->mNext;
        if (end() == object->mNext) mLast = object->mPrev;
        else object->mNext->mPrev = object->mPrev;
        object->mPrev = NULL;
        object->mNext = NULL;
        mCount--;
        return next;
    }
private:
    T *head() const { return (T *)((const u8 *)&mFirst - offsetof(T, mNext)); }

    T *mLast;
    T *mFirst;
    int mCount;
};

static effectList_c<levelEffect_c, 0x7C> l_levelEffects;
static effectList_c<effectCB_c, 0> l_callbacks;
static EGG::EffectResource *l_resource;
static EGG::Heap *l_callbackHeap;
static nw4r::ef::Handle<nw4r::ef::Effect> l_nullHandle;

// 802B7A6C
BOOL initialize(EGG::Heap *heap, int effects, int emitters, int managers, int particles,
                int creators, int groups) {
    EGG::EffectManager::createInstance(creators, groups, heap);
    EGG::EffectManager::Configuration config;
    config.heap = heap;
    config.effects = effects;
    config.emitters = emitters;
    config.managers = managers;
    config.particles = particles;
    EGG::EffectManager::sEffectManager->initialize(config);
    EGG::EffectResource::initialize();
    l_resource = new (heap, 4) EGG::EffectResource(NULL, NULL);
    return TRUE;
}

// 802B7B0C
void shutdown() {
    EGG::EffectManager::sEffectManager->shutdown();
    if (l_callbackHeap != NULL) {
        effectCB_c *cb = l_callbacks.begin();
        if (cb != l_callbacks.end()) {
            while (cb != l_callbacks.end()) cb = l_callbacks.erase(cb);
        }
        static_cast<EGG::ExpHeap *>(l_callbackHeap)->freeAll();
    }
}

// 802B7BFC
void setCallbackHeap(EGG::Heap *heap) {
    l_callbackHeap = heap;
}

// 802B7C04
void setResource(void *breff, void *breft) {
    l_resource->set(breff, breft);
}

// 802B7C24
static void executeCallbacks() {
    if (l_callbackHeap != NULL) {
        effectCB_c *it = l_callbacks.begin();
        effectList_c<effectCB_c, 0>::iterator next = it, last = l_callbacks.end();
        while (it != last) {
            next = it;
            ++next;
            if (!it->mHandle.IsAlive()) {
                l_callbacks.remove(it);
                delete it;
            } else it->execute();
            it = next;
        }
    }
}

// 802B7D50
static void cleanupLevelEffects() {
    effectList_c<levelEffect_c, 0x7C>::iterator it = l_levelEffects.begin();
    effectList_c<levelEffect_c, 0x7C>::iterator last = l_levelEffects.end();
    while (it != last) {
        levelEffect_c *effect = it.ptr;
        effectList_c<levelEffect_c, 0x7C>::iterator next = it;
        ++next;
        if (!effect->_85) effect->cleanup();
        effect->_85 = false;
        it = next;
    }
}

// 802B7DE8
void beginCalc() {
    executeCallbacks();
    cleanupLevelEffects();
}

// 802B7E0C
void endCalc() {}

// 802B7E10
void setCamera(const nw4r::math::VEC3 *pos, const nw4r::math::MTX34 *mtx, f32 nearZ, f32 farZ, int group) {
    EGG::EffectManager::sEffectManager->setCamera(pos, mtx, nearZ, farZ, group, false);
}

// 802B7E30
void draw(nw4r::ef::DrawInfo *info, int creator, int group) {
    EGG::EffectManager::sEffectManager->draw(info, creator, group);
    GXSetClipMode(GX_CLIP_ENABLE);
}

// 802B7E70
BOOL effect_c::createEffect(const char *name, int creator) {
    retire();
    strcpy(mName, name);
    _24 = creator;
    return create();
}

// 802B7EE4
static void makeMtx(const mVec3_c *pos, const mAng3_c *angle, const mVec3_c *scale, mMtx_c *out) {
    if (pos != NULL) PSMTXTrans(*out, pos->x, pos->y, pos->z);
    else PSMTXIdentity(*out);
    if (angle != NULL) {
        out->ZrotM(angle->z);
        out->XrotM(angle->x);
        out->YrotM(angle->y);
    }
    if (scale != NULL) {
        mMtx_c scaleMtx;
        PSMTXScale(scaleMtx, scale->x, scale->y, scale->z);
        PSMTXConcat(*out, scaleMtx, *out);
    }
}

// 802B7FC0
bool effect_c::createEffect(const char *name, ulong creator, const mVec3_c *pos,
                          const mAng3_c *angle, const mVec3_c *scale) {
    if (!createEffect(name, creator)) return false;
    mMtx_c mtx;
    makeMtx(pos, angle, scale, &mtx);
    setMtx(&mtx);
    update();
    return true;
}

// 802B8070
bool effect_c::createEffect(const char *name, ulong creator, const mMtx_c *mtx) {
    if (!createEffect(name, creator)) return false;
    setMtx(mtx);
    update();
    return true;
}

// 802B80F4
BOOL effect_c::createEffect(const char *name, ulong creator, const mMtx_c *mtx,
                          void (*callback)(nw4r::ef::Particle *, u32), u32 arg) {
    effectCB_c *entry = new (l_callbackHeap, 4) effectCB_c(callback, arg);
    if (entry != NULL) {
        if (createEffect(name, creator, mtx)) {
            static_cast<nw4r::ef::HandleBase &>(entry->mHandle) = mHandle;
            l_callbacks.pushBack(entry);
            return TRUE;
        }
        delete entry;
        return FALSE;
    }
    return FALSE;
}

// 802B8244
bool effect_c::createEffect(const char *name, ulong creator, const mVec3_c *pos,
                          const mAng3_c *angle, const mVec3_c *scale,
                          void (*callback)(nw4r::ef::Particle *, u32), u32 arg) {
    effectCB_c *entry = new (l_callbackHeap, 4) effectCB_c(callback, arg);
    if (entry != NULL) {
        if (createEffect(name, creator, pos, angle, scale)) {
            static_cast<nw4r::ef::HandleBase &>(entry->mHandle) = mHandle;
            l_callbacks.pushBack(entry);
            return true;
        }
        delete entry;
        return false;
    }
    return false;
}

// 802B83A4
bool effect_c::follow(const mVec3_c *pos, const mAng3_c *angle, const mVec3_c *scale) {
    if (!mHandle.IsValid()) return false;
    mMtx_c mtx;
    makeMtx(pos, angle, scale, &mtx);
    setMtx(&mtx);
    update();
    return true;
}

// 802B844C
bool effect_c::follow(const mMtx_c *mtx) {
    if (!mHandle.IsValid()) return false;
    setMtx(mtx);
    update();
    return true;
}

// 802B84C8
static void applyCallback(void *object, u32 param) {
    nw4r::ef::ParticleManager *manager = (nw4r::ef::ParticleManager *)object;
    nw4r::ut::List *list = &manager->mActivityList.mActiveList;
    effectCB_c *callback = (effectCB_c *)param;
    nw4r::ef::Particle *particle = (nw4r::ef::Particle *)nw4r::ut::List_GetNext(list, NULL);
    if (particle != NULL) {
        while (particle != NULL) {
            callback->mCallback(particle, callback->mArg);
            particle = (nw4r::ef::Particle *)nw4r::ut::List_GetNext(list, particle);
        }
    }
}

// 802B854C
void effectCB_c::execute() {
    if (mHandle.IsValid() && mCallback != NULL) {
        mHandle.GetPtr()->ForeachParticleManager(applyCallback, (u32)this, true);
    }
}

// 802B85AC
BOOL levelEffect_c::createEffect(const char *name, int creator) {
    if (mActive) {
        if (strcmp(mName, name) == 0) {
            _85 = true;
            return TRUE;
        }
        cleanup();
    }
    if (!effect_c::createEffect(name, creator)) return FALSE;
    l_levelEffects.pushBack(this);
    mActive = true;
    _85 = true;
    return TRUE;
}

// 802B86AC
bool levelEffect_c::createEffect(const char *name, ulong creator, const mVec3_c *pos,
                               const mAng3_c *angle, const mVec3_c *scale) {
    if (!createEffect(name, creator)) return false;
    mMtx_c mtx;
    makeMtx(pos, angle, scale, &mtx);
    setMtx(&mtx);
    update();
    return true;
}

// 802B875C
bool levelEffect_c::createEffect(const char *name, ulong creator, const mMtx_c *mtx) {
    if (!createEffect(name, creator)) return false;
    setMtx(mtx);
    update();
    return true;
}

// 802B87E0
static void applyLevelCallback(void *object, u32 param) {
    nw4r::ef::ParticleManager *manager = (nw4r::ef::ParticleManager *)object;
    nw4r::ut::List *list = &manager->mActivityList.mActiveList;
    levelEffect_c *effect = (levelEffect_c *)param;
    nw4r::ef::Particle *particle;
    for (particle = (nw4r::ef::Particle *)nw4r::ut::List_GetNext(list, NULL);
         particle != NULL;
         particle = (nw4r::ef::Particle *)nw4r::ut::List_GetNext(list, particle)) {
        effect->_88(particle, effect->_8C);
    }
}

// 802B885C
void levelEffect_c::update() {
    if (mHandle.IsValid() && _88 != NULL) {
        static_cast<nw4r::ef::Effect *>(mHandle.GetPtr())->ForeachParticleManager(applyLevelCallback, (u32)this, true);
    }
    EGG::Effect::update();
}

// 802B88C4
void levelEffect_c::cleanup() {
    if (mActive) {
        retire();
        l_levelEffects.remove(this);
        mActive = false;
    }
}

// 802B8980
void levelEffect_c::setParticleCallback(void (*callback)(nw4r::ef::Particle *, u32), u32 arg) {
    _88 = callback;
    _8C = arg;
}

// 802B898C
void levelEffect_c::retire() {
    _88 = NULL;
    _8C = 0;
    EGG::Effect::retire();
}

// 802B899C
void levelEffect_c::retireAll() {
    _88 = NULL;
    _8C = 0;
    EGG::Effect::retireAll();
}

// 802B89AC
bool levelEffect_c::follow(const mVec3_c *pos, const mAng3_c *angle, const mVec3_c *scale) {
    if (!effect_c::follow(pos, angle, scale)) return false;
    mActive = true;
    _85 = true;
    return true;
}

// 802B89F8
bool levelEffect_c::follow(const mMtx_c *mtx) {
    if (!effect_c::follow(mtx)) return false;
    mActive = true;
    _85 = true;
    return true;
}

// 802B8A44
BOOL oneEmitter_c::create(const char *name, const mVec3_c *pos, const mAng3_c *angle, const mVec3_c *scale) {
    _24 = (u32)name;
    mMtx_c mtx;
    makeMtx(pos, angle, scale, &mtx);
    EGG::ExOneEmitter emitter(this);
    emitter.create();
    emitter.setPos(pos);
    setMtx(&mtx);
    update();
    emitter.update();
    return TRUE;
}

// 802B8AF4
BOOL oneEmitter_c::create(const char *name, const mMtx_c *mtx) {
    _24 = (u32)name;
    mVec3_c pos;
    mtx->multVecZero(pos);
    EGG::ExOneEmitter emitter(this);
    emitter.create();
    emitter.setPos(&pos);
    setMtx(mtx);
    update();
    emitter.update();
    return TRUE;
}

// 802B8B9C
void oneEmitter_c::create(const char *name) {
    strcpy(mName, name);
}

} // namespace mEf
