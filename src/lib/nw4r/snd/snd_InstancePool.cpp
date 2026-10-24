#include <nw4r/snd/snd_InstancePool.h>

#include <types.h>

#include <nw4r/ut/ut_Lock.h>
#include <nw4r/ut/ut_algorithm.h>

#include <nw4r/NW4RAssert.hpp>

namespace nw4r {
namespace snd {
namespace detail {
u32 PoolImpl::CreateImpl(void *buffer, u32 size, u32 objSize) {
    NW4RAssertPointerNonnull_Line(38, buffer);

    ut::AutoInterruptLock lock;

    char *ptr = static_cast<char *>(ut::RoundUp(buffer, 4));
    objSize = ut::RoundUp(objSize, 4);

    u32 numObjects = (size - (ptr - static_cast<char *>(buffer))) / objSize;

    for (u32 i = 0; i < numObjects; i++, ptr += objSize) {
        PoolImpl *head = reinterpret_cast<PoolImpl *>(ptr);

        head->mNext = mNext;
        mNext = head;
    }

    return numObjects;
}

void PoolImpl::DestroyImpl(void *buffer, u32 size) {
    NW4RAssertPointerNonnull_Line(68, buffer);

    ut::AutoInterruptLock lock;

    void *begin = buffer;
    void *end = static_cast<char *>(begin) + size;

    for (PoolImpl *ptr = mNext, *prev = this; ptr; ptr = ptr->mNext) {
        if (begin <= ptr && ptr < end) {
            prev->mNext = ptr->mNext;
        } else {
            prev = ptr;
        }
    }
}

int PoolImpl::CountImpl() const {
    ut::AutoInterruptLock lock;

    int count = 0;

    for (PoolImpl *ptr = mNext; ptr; ptr = ptr->mNext) {
        count++;
    }

    return count;
}

void *PoolImpl::AllocImpl() {
    ut::AutoInterruptLock lock;

    if (!mNext) {
        return nullptr;
    }

    PoolImpl *head = mNext;

    mNext = head->mNext;

    return head;
}

void PoolImpl::FreeImpl(void *ptr) {
    ut::AutoInterruptLock lock;

    PoolImpl *head = static_cast<PoolImpl *>(ptr);

    head->mNext = mNext;
    mNext = head;
}
} // namespace detail
} // namespace snd
} // namespace nw4r
