#pragma once
#include <stddef.h>

// Inline placement new, as in the NW4R headers this is derived from (EGG's own is out of line).
inline void *operator new(size_t, void *ptr) {
    return ptr;
}
inline void *operator new[](size_t, void *ptr) {
    return ptr;
}

#include <lib/egg/util/eggExEffect.hpp>
#include <nw4r/ef.h>
#include <nw4r/ef/ef_memorymanager.h>
#include <nw4r/ut.h>

// nw4r::ef's MemoryManager / MemoryManagerTmp specialised for EGG's Ex* object types. City Folk's
// RTTI keeps the nw4r names but places both classes at global scope, so these are not nw4r's own
// (project) templates: this NW4R revision's MemoryManager has one object-count field per pool
// (no separate least / max counts), which makes it 0x3C bytes. Only eggEffectManager.cpp uses it.

template <typename T> class MemoryManagerTmp {
public:
    /* 0x00 */ T *mHead;
    /* 0x04 */ nw4r::ut::List mFreeList;
    /* 0x10 */ nw4r::ut::List mLeasedList;
    /* 0x1C */ int mStructSize;

    static const u32 OBJECT_ID_BASE = 0x10000;

    MemoryManagerTmp(int count, T *heap) {
        u16 offset = offsetof(T, mMemoryLink);
        int structSize = sizeof(T);

        nw4r::ut::List_Init(&mFreeList, offset);
        nw4r::ut::List_Init(&mLeasedList, offset);

        mStructSize = structSize;
        mHead = heap;

        for (u32 i = 0; i < count; i++) {
            heap[i].mObjectID = i;
            nw4r::ut::List_Append(&mFreeList, &heap[i]);
        }
    }

    virtual void AssignObjectID(void *obj) {
        static_cast<nw4r::ef::LinkedObject *>(obj)->mObjectID += OBJECT_ID_BASE;
    } // at 0x8

    void GarbageCollection();

    T *Alloc() {
        void *obj = nw4r::ut::List_GetFirst(&mFreeList);
        if (obj == NULL) {
            return NULL;
        }

        AssignObjectID(obj);
        nw4r::ut::List_Remove(&mFreeList, obj);
        nw4r::ut::List_Append(&mLeasedList, obj);

        return static_cast<T *>(obj);
    }

    void Free(T *obj) {
        nw4r::ut::List_Remove(&mLeasedList, obj);
        nw4r::ut::List_Append(&mFreeList, obj);
    }

    u32 GetNumAllocObject() const {
        return GetNumActiveObject() + GetNumFreeObject();
    }

    u32 GetNumActiveObject() const {
        return nw4r::ut::List_GetSize(&mLeasedList);
    }

    u32 GetNumFreeObject() const {
        return nw4r::ut::List_GetSize(&mFreeList);
    }
};

typedef MemoryManagerTmp<EGG::ExEffect> TEffectOM;
typedef MemoryManagerTmp<EGG::ExEmitter> TEmitterOM;
typedef MemoryManagerTmp<EGG::ExParticleManager> TParticleManagerOM;
typedef MemoryManagerTmp<EGG::ExParticle> TParticleOM;

template <> inline void TEffectOM::GarbageCollection() {
    void *obj = nw4r::ut::List_GetFirst(&mLeasedList);

    if (obj != NULL) {
        void *next = nw4r::ut::List_GetNext(&mLeasedList, obj);

        while (obj != NULL) {
            EGG::ExEffect *effect = static_cast<EGG::ExEffect *>(obj);

            if (effect->GetLifeStatus() == nw4r::ef::ReferencedObject::NW4R_EF_LS_CLOSING &&
                effect->GetRefCount() == 0) {
                effect->mManagerES->mActivityList[effect->mGroupID].ToFree(effect);
                nw4r::ut::List_Remove(&mLeasedList, effect);
                nw4r::ut::List_Append(&mFreeList, effect);
            }

            obj = next;
            next = nw4r::ut::List_GetNext(&mLeasedList, next);
        }
    }
}

template <> inline void TEmitterOM::GarbageCollection() {
    void *obj = nw4r::ut::List_GetFirst(&mLeasedList);

    if (obj != NULL) {
        void *next = nw4r::ut::List_GetNext(&mLeasedList, obj);

        while (obj != NULL) {
            EGG::ExEmitter *emitter = static_cast<EGG::ExEmitter *>(obj);

            if (emitter->GetLifeStatus() == nw4r::ef::ReferencedObject::NW4R_EF_LS_CLOSING &&
                emitter->GetRefCount() == 0) {
                emitter->mManagerEF->mActivityList.ToFree(emitter);
                nw4r::ut::List_Remove(&mLeasedList, emitter);
                nw4r::ut::List_Append(&mFreeList, emitter);
            }

            obj = next;
            next = nw4r::ut::List_GetNext(&mLeasedList, next);
        }
    }
}

template <> inline void TParticleManagerOM::GarbageCollection() {
    void *obj = nw4r::ut::List_GetFirst(&mLeasedList);

    if (obj != NULL) {
        void *next = nw4r::ut::List_GetNext(&mLeasedList, obj);

        while (obj != NULL) {
            EGG::ExParticleManager *manager = static_cast<EGG::ExParticleManager *>(obj);

            if (manager->GetLifeStatus() == nw4r::ef::ReferencedObject::NW4R_EF_LS_CLOSING &&
                manager->GetRefCount() == 0) {
                manager->mManagerEM->mActivityList.ToFree(manager);
                nw4r::ut::List_Remove(&mLeasedList, manager);
                nw4r::ut::List_Append(&mFreeList, manager);
            }

            obj = next;
            next = nw4r::ut::List_GetNext(&mLeasedList, next);
        }
    }
}

template <> inline void TParticleOM::GarbageCollection() {
}

class MemoryManager : public nw4r::ef::MemoryManagerBase {
public:
    struct MemInfo {
        MemInfo *prev;      // at 0x0
        MemInfo *next;      // at 0x4
        MemInfo *chainPrev; // at 0x8
        MemInfo *chainNext; // at 0xC
        u32 size;           // at 0x10
        bool active;        // at 0x14
        u8 PADDING_0x15[3]; // at 0x15
    };

    /* 0x04 */ int mNumEffect;
    /* 0x08 */ TEffectOM *mEffectOM;
    /* 0x0C */ int mNumEmitter;
    /* 0x10 */ TEmitterOM *mEmitterOM;
    /* 0x14 */ int mNumParticleManager;
    /* 0x18 */ TParticleManagerOM *mParticleManagerOM;
    /* 0x1C */ int mNumParticle;
    /* 0x20 */ TParticleOM *mParticleOM;
    /* 0x24 */ void *mHeapStartAddr;
    /* 0x28 */ void *mHeapEndAddr;
    /* 0x2C */ MemInfo *mActiveMem;
    /* 0x30 */ MemInfo *mFreeMem;
    /* 0x34 */ MemInfo *mFreeMemTail;
    /* 0x38 */ MemInfo *mAllChain;

    static u32 CalcMemorySize(u16 maxEffect, u16 maxEmitter, u16 maxParticleManager,
                              u16 maxParticle, u32 maxGroupID) {
        return ROUND_UP(sizeof(MemInfo), 32) + nw4r::ut::RoundUp<u32>(maxEffect * sizeof(EGG::ExEffect) + 32, 32) +
               ROUND_UP(sizeof(MemInfo), 32) + nw4r::ut::RoundUp<u32>(maxEmitter * sizeof(EGG::ExEmitter) + 32, 32) +
               ROUND_UP(sizeof(MemInfo), 32) + nw4r::ut::RoundUp<u32>(maxParticleManager * sizeof(EGG::ExParticleManager) + 32, 32) +
               ROUND_UP(sizeof(MemInfo), 32) + nw4r::ut::RoundUp<u32>(maxParticle * sizeof(EGG::ExParticle) + 32, 32) +
               ROUND_UP(sizeof(MemInfo), 32) + nw4r::ut::RoundUp<u32>(maxGroupID * sizeof(nw4r::ef::ActivityList) + 32, 32) +
               ROUND_UP(sizeof(MemInfo), 32) + ROUND_UP(sizeof(TEffectOM), 32) +
               ROUND_UP(sizeof(MemInfo), 32) + ROUND_UP(sizeof(TEmitterOM), 32) +
               ROUND_UP(sizeof(MemInfo), 32) + ROUND_UP(sizeof(TParticleManagerOM), 32) +
               ROUND_UP(sizeof(MemInfo), 32) + ROUND_UP(sizeof(TParticleOM), 32);
    }

    MemoryManager(void *pStartAddr, u32 size, int maxEffect, int maxEmitter, int maxParticleManager,
                  int maxParticle) {
        u32 headerSize = ROUND_UP(sizeof(MemInfo), 32);

        mNumEffect = maxEffect;
        mNumEmitter = maxEmitter;
        mNumParticleManager = maxParticleManager;
        mNumParticle = maxParticle;

        mEffectOM = NULL;
        mEmitterOM = NULL;
        mParticleManagerOM = NULL;
        mParticleOM = NULL;

        mActiveMem = NULL;
        mFreeMemTail = NULL;
        mAllChain = NULL;

        mHeapEndAddr = static_cast<u8 *>(pStartAddr) + size;
        mHeapStartAddr = reinterpret_cast<u8 *>(ROUND_UP(reinterpret_cast<u32>(pStartAddr), 32));

        mFreeMem = static_cast<MemInfo *>(mHeapStartAddr);
        mFreeMem->prev = NULL;
        mFreeMem->next = NULL;
        mFreeMem->chainPrev = NULL;
        mFreeMem->chainNext = NULL;
        mFreeMem->active = false;
        mFreeMem->size =
            static_cast<u8 *>(mHeapEndAddr) - static_cast<u8 *>(mHeapStartAddr) - headerSize;

        mAllChain = mFreeMem;
        mActiveMem = NULL;

        int numObjects = mNumEffect;
        EGG::ExEffect *pEffects = NULL;
        if (numObjects > 0) {
            pEffects = new (AllocHeap(sizeof(EGG::ExEffect) * numObjects + 32))
                EGG::ExEffect[numObjects];
        }
        mEffectOM = new (AllocHeap(sizeof(TEffectOM))) TEffectOM(mNumEffect, pEffects);

        numObjects = mNumEmitter;
        EGG::ExEmitter *pEmitters = NULL;
        if (numObjects > 0) {
            pEmitters = new (AllocHeap(sizeof(EGG::ExEmitter) * numObjects + 32))
                EGG::ExEmitter[numObjects];
        }
        mEmitterOM = new (AllocHeap(sizeof(TEmitterOM))) TEmitterOM(mNumEmitter, pEmitters);

        numObjects = mNumParticleManager;
        EGG::ExParticleManager *pParticleManagers = NULL;
        if (numObjects > 0) {
            pParticleManagers = new (AllocHeap(sizeof(EGG::ExParticleManager) * numObjects + 32))
                EGG::ExParticleManager[numObjects];
        }
        mParticleManagerOM =
            new (AllocHeap(sizeof(TParticleManagerOM))) TParticleManagerOM(mNumParticleManager, pParticleManagers);

        numObjects = mNumParticle;
        EGG::ExParticle *pParticles = NULL;
        if (numObjects > 0) {
            pParticles = new (AllocHeap(sizeof(EGG::ExParticle) * numObjects + 32))
                EGG::ExParticle[numObjects];
        }
        mParticleOM = new (AllocHeap(sizeof(TParticleOM))) TParticleOM(mNumParticle, pParticles);
    }

    virtual ~MemoryManager() {} // at 0x8

    virtual void GarbageCollection() {
        void *obj = nw4r::ut::List_GetFirst(&mParticleManagerOM->mLeasedList);

        while (obj != NULL) {
            void *next = nw4r::ut::List_GetNext(&mParticleManagerOM->mLeasedList, obj);

            EGG::ExParticleManager *manager = static_cast<EGG::ExParticleManager *>(obj);

            EGG::ExParticle *particle;
            EGG::ExParticle *nextParticle;

            for (particle = static_cast<EGG::ExParticle *>(manager->mActivityList.mClosingList.headObject);
                 particle != NULL; particle = nextParticle) {
                nextParticle = static_cast<EGG::ExParticle *>(
                    NW4R_UT_LIST_GET_LINK(manager->mActivityList.mClosingList, particle)->nextObject);

                particle->mParticleManager->ParticleToFree(particle);
                nw4r::ut::List_Remove(&mParticleOM->mLeasedList, particle);
                nw4r::ut::List_Append(&mParticleOM->mFreeList, particle);
            }

            obj = next;
        }

        if (mParticleManagerOM != NULL) {
            mParticleManagerOM->GarbageCollection();
        }

        if (mEmitterOM != NULL) {
            mEmitterOM->GarbageCollection();
        }

        if (mEffectOM != NULL) {
            mEffectOM->GarbageCollection();
        }
    } // at 0xC

    virtual EGG::ExEffect *AllocEffect() {
        if (mEffectOM == NULL) {
            return NULL;
        } else {
            return mEffectOM->Alloc();
        }
    } // at 0x10

    virtual void FreeEffect(void *obj) {
        if (mEffectOM != NULL) {
            mEffectOM->Free(static_cast<EGG::ExEffect *>(obj));
        }
    } // at 0x14

    virtual u32 GetNumAllocEffect() const {
        if (mEffectOM == NULL) {
            return 0;
        } else {
            return mEffectOM->GetNumAllocObject();
        }
    } // at 0x18

    virtual u32 GetNumActiveEffect() const {
        if (mEffectOM == NULL) {
            return 0;
        } else {
            return mEffectOM->GetNumActiveObject();
        }
    } // at 0x1C

    virtual u32 GetNumFreeEffect() const {
        if (mEffectOM == NULL) {
            return 0;
        } else {
            return mEffectOM->GetNumFreeObject();
        }
    } // at 0x20

    virtual EGG::ExEmitter *AllocEmitter() {
        if (mEmitterOM == NULL) {
            return NULL;
        } else {
            return mEmitterOM->Alloc();
        }
    } // at 0x24

    virtual void FreeEmitter(void *obj) {
        if (mEmitterOM != NULL) {
            mEmitterOM->Free(static_cast<EGG::ExEmitter *>(obj));
        }
    } // at 0x28

    virtual u32 GetNumAllocEmitter() const {
        if (mEmitterOM == NULL) {
            return 0;
        } else {
            return mEmitterOM->GetNumAllocObject();
        }
    } // at 0x2C

    virtual u32 GetNumActiveEmitter() const {
        if (mEmitterOM == NULL) {
            return 0;
        } else {
            return mEmitterOM->GetNumActiveObject();
        }
    } // at 0x30

    virtual u32 GetNumFreeEmitter() const {
        if (mEmitterOM == NULL) {
            return 0;
        } else {
            return mEmitterOM->GetNumFreeObject();
        }
    } // at 0x34

    virtual EGG::ExParticleManager *AllocParticleManager() {
        if (mParticleManagerOM == NULL) {
            return NULL;
        } else {
            return mParticleManagerOM->Alloc();
        }
    } // at 0x38

    virtual void FreeParticleManager(void *obj) {
        if (mParticleManagerOM != NULL) {
            mParticleManagerOM->Free(static_cast<EGG::ExParticleManager *>(obj));
        }
    } // at 0x3C

    virtual u32 GetNumAllocParticleManager() const {
        if (mParticleManagerOM == NULL) {
            return 0;
        } else {
            return mParticleManagerOM->GetNumAllocObject();
        }
    } // at 0x40

    virtual u32 GetNumActiveParticleManager() const {
        if (mParticleManagerOM == NULL) {
            return 0;
        } else {
            return mParticleManagerOM->GetNumActiveObject();
        }
    } // at 0x44

    virtual u32 GetNumFreeParticleManager() const {
        if (mParticleManagerOM == NULL) {
            return 0;
        } else {
            return mParticleManagerOM->GetNumFreeObject();
        }
    } // at 0x48

    virtual EGG::ExParticle *AllocParticle() {
        if (mParticleOM == NULL) {
            return NULL;
        } else {
            return mParticleOM->Alloc();
        }
    } // at 0x4C

    virtual void FreeParticle(void *obj) {
        if (mParticleOM != NULL) {
            mParticleOM->Free(static_cast<EGG::ExParticle *>(obj));
        }
    } // at 0x50

    virtual u32 GetNumAllocParticle() const {
        if (mParticleOM == NULL) {
            return 0;
        } else {
            return mParticleOM->GetNumAllocObject();
        }
    } // at 0x54

    virtual u32 GetNumActiveParticle() const {
        if (mParticleOM == NULL) {
            return 0;
        } else {
            return mParticleOM->GetNumActiveObject();
        }
    } // at 0x58

    virtual u32 GetNumFreeParticle() const {
        if (mParticleOM == NULL) {
            return 0;
        } else {
            return mParticleOM->GetNumFreeObject();
        }
    } // at 0x5C

    virtual void *AllocHeap(u32 size) {
        u32 headerSize = ROUND_UP(sizeof(MemInfo), 32);
        size = ROUND_UP(size, 32);

        for (MemInfo *it = mFreeMem; it != NULL; it = it->next) {
            if (it->size < size) {
                continue;
            }

            if (it->size < size + headerSize + 32) {
                if (it->prev != NULL) {
                    it->prev->next = it->next;
                } else {
                    mFreeMem = it->next;
                }

                if (it->next != NULL) {
                    it->next->prev = it->prev;
                }

                if (mActiveMem != NULL) {
                    mActiveMem->prev = it;
                }

                it->next = mActiveMem;
                it->prev = NULL;
                it->active = true;
                mActiveMem = it;
            } else {
                MemInfo *newFree = reinterpret_cast<MemInfo *>(reinterpret_cast<u8 *>(it) + headerSize + size);

                newFree->prev = it->prev;
                newFree->next = it->next;
                newFree->size = it->size - size - headerSize;
                newFree->active = false;
                newFree->chainNext = it->chainNext;

                if (it->chainNext != NULL) {
                    it->chainNext->chainPrev = newFree;
                }

                newFree->chainPrev = it;
                it->chainNext = newFree;

                if (it->prev != NULL) {
                    it->prev->next = newFree;
                } else {
                    mFreeMem = newFree;
                }

                if (it->next != NULL) {
                    newFree->next->prev = newFree;
                }

                if (mActiveMem != NULL) {
                    mActiveMem->prev = it;
                }

                it->next = mActiveMem;
                it->prev = NULL;
                it->size = size;
                it->active = true;
                mActiveMem = it;
            }

            return reinterpret_cast<u8 *>(it) + headerSize;
        }

        return NULL;
    } // at 0x60

    virtual void FreeHeap(void *ptr) {
        u32 headerSize = ROUND_UP(sizeof(MemInfo), 32);

        MemInfo *info = reinterpret_cast<MemInfo *>(static_cast<u8 *>(ptr) - headerSize);

        if (info->prev != NULL) {
            info->prev->next = info->next;
        } else {
            mActiveMem = info->next;
        }

        if (info->next != NULL) {
            info->next->prev = info->prev;
        }

        MemInfo *prev = NULL;
        if (info->chainPrev != NULL && !info->chainPrev->active) {
            prev = info->chainPrev;
        }

        MemInfo *next = NULL;
        if (info->chainNext != NULL && !info->chainNext->active) {
            next = info->chainNext;
        }

        if (prev != NULL) {
            if (next != NULL) {
                if (next->prev != NULL) {
                    next->prev->next = next->next;
                } else {
                    mFreeMem = next->next;
                }

                if (next->next != NULL) {
                    next->next->prev = next->prev;
                }

                prev->size += next->size + info->size + headerSize + headerSize;

                prev->chainNext = next->chainNext;

                if (next->chainNext != NULL) {
                    next->chainNext->chainPrev = prev;
                }
            } else {
                prev->size += info->size + headerSize;
                prev->chainNext = info->chainNext;

                if (info->chainNext != NULL) {
                    info->chainNext->chainPrev = prev;
                }
            }
        } else if (next != NULL) {
            if (next->prev != NULL) {
                next->prev->next = info;
            } else {
                mFreeMem = info;
            }

            if (next->next != NULL) {
                next->next->prev = info;
            }

            info->prev = next->prev;
            info->next = next->next;
            info->size += next->size + headerSize;
            info->active = false;
            info->chainNext = next->chainNext;

            if (next->chainNext != NULL) {
                next->chainNext->chainPrev = prev;
            }
        } else {
            if (mFreeMem != NULL) {
                mFreeMem->prev = info;
            }

            info->next = mFreeMem;
            info->prev = 0;
            info->active = false;
            mFreeMem = info;
        }
    } // at 0x64
};
