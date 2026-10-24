#ifndef NW4R_SND_MML_SEQ_TRACK_ALLOCATOR_H
#define NW4R_SND_MML_SEQ_TRACK_ALLOCATOR_H

#include <types.h>

#include <nw4r/snd/snd_InstancePool.h>
#include <nw4r/snd/snd_SeqTrackAllocator.h>

namespace nw4r {
namespace snd {
namespace detail {
class MmlParser;
}
} // namespace snd
} // namespace nw4r
namespace nw4r {
namespace snd {
namespace detail {
class MmlSeqTrack;
}
} // namespace snd
} // namespace nw4r

namespace nw4r {
namespace snd {
namespace detail {
class MmlSeqTrackAllocator : public SeqTrackAllocator {
  public:
    MmlSeqTrackAllocator(MmlParser *parser) : mParser(parser) {
    }

    virtual SeqTrack *AllocTrack(SeqPlayer *player);
    virtual void FreeTrack(SeqTrack *track);
    virtual int GetAllocatableTrackCount() const {
        return mTrackPool.Count();
    }

    u32 Create(void *buffer, u32 size);
    void Destroy(void *buffer, u32 size);

  private:
    /* base SeqTrackAllocator */          // size 0x04, offset 0x00
    MmlParser *mParser;                   // size 0x04, offset 0x04
    InstancePool<MmlSeqTrack> mTrackPool; // size 0x04, offset 0x08
}; // size 0x0c
} // namespace detail
} // namespace snd
} // namespace nw4r

#endif
