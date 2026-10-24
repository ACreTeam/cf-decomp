#ifndef NW4R_SND_INSTANCE_POOL_H
#define NW4R_SND_INSTANCE_POOL_H

#include <new>

#include <types.h>

namespace nw4r {
namespace snd {
namespace detail {
class PoolImpl {
  protected:
    PoolImpl() : mNext(nullptr) {
    }

    u32 CreateImpl(void *buffer, u32 size, u32 objSize);
    void DestroyImpl(void *buffer, u32 size);

    int CountImpl() const;

    void *AllocImpl();
    void FreeImpl(void *ptr);

  private:
    PoolImpl *mNext; // size 0x04, offset 0x00
}; // size 0x04

template <class T> class InstancePool : private PoolImpl {
  public:
    u32 Create(void *buffer, u32 size) {
        u32 objSize = sizeof(T);

        return CreateImpl(buffer, size, objSize);
    }
    void Destroy(void *buffer, u32 size) {
        DestroyImpl(buffer, size);
    }

    int Count() const {
        return CountImpl();
    }

    T *Alloc() {
        void *ptr = AllocImpl();
        if (!ptr) {
            return nullptr;
        }

        return new (ptr) T;
    }
    void Free(T *obj) {
        if (obj) {
            obj->~T();
            FreeImpl(obj);
        }
    }

  private:
    /* base PoolImpl */ // size 0x04, offset 0x00
}; // size 0x04

template <class T> class MemoryPool : private PoolImpl {
  public:
    u32 Create(void *buffer, u32 size) {
        u32 objSize = sizeof(T);

        return CreateImpl(buffer, size, objSize);
    }
    void Destroy(void *buffer, u32 size) {
        DestroyImpl(buffer, size);
    }

    int Count() const {
        return CountImpl();
    }

    T *Alloc() {
        return static_cast<T *>(AllocImpl());
    }
    void Free(T *obj) {
        FreeImpl(obj);
    }

  private:
    /* base PoolImpl */ // size 0x04, offset 0x00
}; // size 0x04
} // namespace detail
} // namespace snd
} // namespace nw4r

#endif
