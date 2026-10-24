#include <nw4r/snd/snd_DisposeCallbackManager.h>

#include <types.h>

#include <nw4r/snd/snd_SoundThread.h>

#include <nw4r/ut/ut_Lock.h>

namespace nw4r {
namespace snd {
namespace detail {
DisposeCallbackManager &DisposeCallbackManager::GetInstance() {
    ut::AutoInterruptLock lock;

    static DisposeCallbackManager instance;

    return instance;
}

DisposeCallbackManager::DisposeCallbackManager() {
}

void DisposeCallbackManager::RegisterDisposeCallback(
    DisposeCallback *callback) {
    mCallbackList.PushBack(callback);
}

void DisposeCallbackManager::UnregisterDisposeCallback(
    DisposeCallback *callback) {
    mCallbackList.Erase(callback);
}

void DisposeCallbackManager::Dispose(void *mem, u32 size,
                                     void *arg ATTR_UNUSED) {
    void *start = mem;
    void *end = static_cast<byte_t *>(mem) + size;

    SoundThread::AutoLock lock;

    NW4R_RANGE_FOR_NO_AUTO_INC(itr, GetInstance().mCallbackList) {
        __decltype__(itr) curItr = itr++;

        curItr->InvalidateData(start, end);
    }
}

void DisposeCallbackManager::DisposeWave(void *mem, u32 size,
                                         void *arg ATTR_UNUSED) {
    void *start = mem;
    void *end = static_cast<byte_t *>(mem) + size;

    SoundThread::AutoLock lock;

    NW4R_RANGE_FOR_NO_AUTO_INC(itr, GetInstance().mCallbackList) {
        __decltype__(itr) curItr = itr++;

        curItr->InvalidateWaveData(start, end);
    }
}
} // namespace detail
} // namespace snd
} // namespace nw4r
