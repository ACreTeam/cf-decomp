#ifndef NW4R_SND_NOTE_ON_CALLBACK_H
#define NW4R_SND_NOTE_ON_CALLBACK_H

#include <types.h>

#include <nw4r/snd/snd_Channel.h>

namespace nw4r {
namespace snd {
namespace detail {
class SeqPlayer;
}
} // namespace snd
} // namespace nw4r

namespace nw4r {
namespace snd {
namespace detail {
struct NoteOnInfo {
    int prgNo;                          // size 0x04, offset 0x00
    int key;                            // size 0x04, offset 0x04
    int velocity;                       // size 0x04, offset 0x08
    int length;                         // size 0x04, offset 0x0c
    int initPan;                        // size 0x04, offset 0x10
    int priority;                       // size 0x04, offset 0x14
    int voiceOutCount;                  // size 0x04, offset 0x18
    Channel::Callback *channelCallback; // size 0x04, offset 0x1c
    u32 channelCallbackData;            // size 0x04, offset 0x20
}; // size 0x24
} // namespace detail
} // namespace snd
} // namespace nw4r

namespace nw4r {
namespace snd {
namespace detail {
class NoteOnCallback {
  public:
    virtual ~NoteOnCallback() {
    }

    virtual Channel *NoteOn(SeqPlayer *seqPlayer, int bankNo,
                            NoteOnInfo const &noteOnInfo) = 0;

  private:
    /* vtable */ // size 0x04, offset 0x00
}; // size 0x04
} // namespace detail
} // namespace snd
} // namespace nw4r

#endif
