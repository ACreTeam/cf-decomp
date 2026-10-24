#include <nw4r/snd/snd_MmlSeqTrack.h>

#include <nw4r/snd/snd_MmlParser.h>

namespace nw4r {
namespace snd {
namespace detail {
MmlSeqTrack::MmlSeqTrack() {
}

MmlSeqTrack::ParseResult MmlSeqTrack::Parse(bool doNoteOn) {
    return mParser->Parse(this, doNoteOn);
}
} // namespace detail
} // namespace snd
} // namespace nw4r
