#ifndef NW4R_SND_PLAYER_HEAP_H
#define NW4R_SND_PLAYER_HEAP_H

#include <types.h>

#include <nw4r/snd/snd_SoundMemoryAllocatable.h>

#include <nw4r/ut/ut_LinkList.h>

namespace nw4r {
namespace snd {
namespace detail {
class BasicSound;
}
} // namespace snd
} // namespace nw4r
namespace nw4r {
namespace snd {
class SoundPlayer;
}
} // namespace nw4r

namespace nw4r {
namespace snd {
namespace detail {
class PlayerHeap : public SoundMemoryAllocatable {
  public:
    typedef ut::LinkList<PlayerHeap, 0x18> LinkList;

  public:
    PlayerHeap();
    virtual ~PlayerHeap();

    virtual void *Alloc(u32 size);

    bool Create(void *startAddress, u32 size);
    void Clear();
    void Destroy();

    u32 GetFreeSize() const;

    void AttachSound(BasicSound *sound);
    void DetachSound(BasicSound *sound);

    void AttachSoundPlayer(SoundPlayer *player) {
        mPlayer = player;
    }

  private:
    /* base SoundMemoryAllocatable */ // size 0x04, offset 0x00
    BasicSound *mSound;               // size 0x04, offset 0x04
    SoundPlayer *mPlayer;             // size 0x04, offset 0x08
    void *mStartAddress;              // size 0x04, offset 0x0c
    void *mEndAddress;                // size 0x04, offset 0x10
    void *mAllocAddress;              // size 0x04, offset 0x14
  public:
    ut::LinkListNode mLink; // size 0x08, offset 0x18
}; // size 0x20
} // namespace detail
} // namespace snd
} // namespace nw4r

#endif
