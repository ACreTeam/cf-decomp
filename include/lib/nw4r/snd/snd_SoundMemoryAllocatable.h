#ifndef NW4R_SND_SOUND_MEMORY_ALLOCATABLE_H
#define NW4R_SND_SOUND_MEMORY_ALLOCATABLE_H

#include <types.h>

namespace nw4r {
namespace snd {
class SoundMemoryAllocatable {
  public:
    virtual ~SoundMemoryAllocatable() {
    }

    virtual void *Alloc(u32 size) = 0;

  private:
    /* vtable */ // size 0x04, offset 0x00
}; // size 0x04
} // namespace snd
} // namespace nw4r

#endif
