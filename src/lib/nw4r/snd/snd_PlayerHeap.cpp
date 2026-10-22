#include <nw4r/snd.h>

namespace nw4r {
namespace snd {
namespace detail {

PlayerHeap::PlayerHeap()
    : mSound(NULL),
      mPlayer(NULL),
      mStartAddress(NULL),
      mEndAddress(NULL),
      mAllocAddress(NULL) {}

PlayerHeap::~PlayerHeap() {
    Destroy();
}

bool PlayerHeap::Create(void* pBase, u32 size) {
    void* pEnd = ut::AddOffsetToPtr(pBase, size);
    pBase = ut::RoundUp(pBase, 32);

    if (pBase > pEnd) {
        return false;
    }

    mStartAddress = pBase;
    mEndAddress = pEnd;
    mAllocAddress = mStartAddress;

    return true;
}

void* PlayerHeap::Alloc(u32 size) {
    void* pEnd = ut::AddOffsetToPtr(mAllocAddress, size);

    if (pEnd > mEndAddress) {
        return NULL;
    }

    void* pAlloc = mAllocAddress;
    mAllocAddress = ut::RoundUp(pEnd, 32);
    return pAlloc;
}

void PlayerHeap::Clear() {
    SoundThread::AutoLock lock;

    DisposeCallbackManager::GetInstance().Dispose(
        mStartAddress, ut::GetOffsetFromPtr(mStartAddress, mAllocAddress),
        NULL);

    DisposeCallbackManager::GetInstance().DisposeWave(
        mStartAddress, ut::GetOffsetFromPtr(mStartAddress, mAllocAddress),
        NULL);

    mAllocAddress = mStartAddress;
}

u32 PlayerHeap::GetFreeSize() const {
    return ut::GetOffsetFromPtr(mAllocAddress, mEndAddress);
}

void PlayerHeap::AttachSound(BasicSound* pSound) {
    mSound = pSound;
}

void PlayerHeap::DetachSound(BasicSound* pSound) {
#pragma unused(pSound)
    mSound = NULL;
}

} // namespace detail
} // namespace snd
} // namespace nw4r
