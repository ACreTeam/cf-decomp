#ifndef NW4R_SND_MIDI_SEQ_TRACK_ALLOCATOR_H
#define NW4R_SND_MIDI_SEQ_TRACK_ALLOCATOR_H

#include <nw4r/snd/snd_SeqTrackAllocator.h>

namespace nw4r {
namespace snd {
namespace detail {
class MidiSeqTrackAllocator : public SeqTrackAllocator {
    MidiSeqTrackAllocator();
};
} // namespace detail
} // namespace snd
} // namespace nw4r

#endif
