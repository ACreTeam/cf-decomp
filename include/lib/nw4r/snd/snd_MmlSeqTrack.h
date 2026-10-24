#ifndef NW4R_SND_MML_SEQ_TRACK_H
#define NW4R_SND_MML_SEQ_TRACK_H

#include <nw4r/snd/snd_SeqTrack.h>

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
class MmlSeqTrack : public SeqTrack {
  public:
    MmlSeqTrack();

    virtual ParseResult Parse(bool doNoteOn);

    void SetMmlParser(MmlParser const *parser) {
        mParser = parser;
    }

  private:
    /* base SeqTrack */       // size 0xc8, offset 0x00
    MmlParser const *mParser; // size 0x04, offset 0xc8
}; // size 0xcc
} // namespace detail
} // namespace snd
} // namespace nw4r

#endif
