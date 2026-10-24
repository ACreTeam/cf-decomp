#include <nw4r/snd/snd_MmlSeqTrackAllocator.h>

#include <types.h>

#include <nw4r/snd/snd_InstancePool.h>
#include <nw4r/snd/snd_MmlSeqTrack.h>
#include <nw4r/snd/snd_SeqTrack.h>

#include <nw4r/NW4RAssert.hpp>

namespace nw4r {
namespace snd {
namespace detail {
SeqTrack *MmlSeqTrackAllocator::AllocTrack(SeqPlayer *player) {
    MmlSeqTrack *track = mTrackPool.Alloc();
    if (track) {
        track->SetSeqPlayer(player);
        track->SetMmlParser(mParser);
    }

    return track;
}

void MmlSeqTrackAllocator::FreeTrack(SeqTrack *track) {
    NW4RAssertPointerNonnull_Line(59, track);

    track->SetSeqPlayer(nullptr);
    mTrackPool.Free(static_cast<MmlSeqTrack *>(track));
}

u32 MmlSeqTrackAllocator::Create(void *buffer, u32 size) {
    return mTrackPool.Create(buffer, size);
}

void MmlSeqTrackAllocator::Destroy(void *buffer, u32 size) {
    mTrackPool.Destroy(buffer, size);
}
} // namespace detail
} // namespace snd
} // namespace nw4r
