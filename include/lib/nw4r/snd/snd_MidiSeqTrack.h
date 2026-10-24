#ifndef NW4R_SND_MIDI_SEQ_TRACK_H
#define NW4R_SND_MIDI_SEQ_TRACK_H

#include <nw4r/snd/snd_SeqTrack.h>

namespace nw4r {
namespace snd {
namespace detail {
class MidiSeqTrack : public SeqTrack {
    MidiSeqTrack();
};
} // namespace detail
} // namespace snd
} // namespace nw4r

#endif
