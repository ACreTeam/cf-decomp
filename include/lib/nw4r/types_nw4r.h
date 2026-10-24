#ifndef NW4R_TYPES_H
#define NW4R_TYPES_H
#include <types.h>

#define NW4R_BYTEORDER_BIG 0xFEFF
#define NW4R_BYTEORDER_LITTLE 0xFFFE

#ifdef NW4R_LITTLE_ENDIAN
#define NW4R_BYTEORDER_NATIVE NW4R_BYTEORDER_LITTLE
#else
#define NW4R_BYTEORDER_NATIVE NW4R_BYTEORDER_BIG
#endif

#define NW4R_VERSION(major, minor) ((major & 0xFF) << 8 | minor & 0xFF)

#define NW4R_LIB_VERSION(NAME, ORIGINAL_DATE, ORIGINAL_TIME, ORIGINAL_CWCC)    \
    const char* NW4R_##NAME##_Version_ =                                       \
        "<< NW4R    - " #NAME " \tfinal   build: " ORIGINAL_DATE               \
        " " ORIGINAL_TIME " (" ORIGINAL_CWCC ") >>"

#if defined(CHAR_BIT)
#define CHAR_BIT_	CHAR_BIT
#else
#define CHAR_BIT_	8 // most common; default
#endif

#define NW4R_BYTE_(byte_, offset_)	\
	(static_cast<unsigned char>(byte_) << CHAR_BIT_ * (offset_))

#define NW4R_FOUR_BYTE(A, B, C, D)	\
	(NW4R_BYTE_(A, 3) | NW4R_BYTE_(B, 2) | NW4R_BYTE_(C, 1) | NW4R_BYTE_(D, 0))

#define NW4R_FILE_VERSION(MAJOR, MINOR)	\
	(NW4R_BYTE_(MAJOR, 1) | NW4R_BYTE_(MINOR, 0))

#ifdef __cplusplus
namespace nw4r {
namespace snd {

class FxBase;
class SoundPlayer;
class SoundHandle;
class SeqSoundHandle;
class SoundArchive;
class SoundArchivePlayer;
class Sound3DActor;
class Sound3DListener;
class Sound3DManager;
class StrmSoundHandle;
class WaveSoundHandle;

namespace detail {

struct AxfxImpl;
class AxManager;
class AxVoice;
class AxVoiceManager;
class ExternalSoundPlayer;
class BasicSound;
class BasicPlayer;
class ChannelManager;
class SeqSound;
class SeqPlayer;
class MmlParser;
class MmlSeqTrack;
class SeqTrack;
class SeqTrackAllocator;
template <typename T> class SoundInstanceManager;
class StrmBufferPool;
class StrmSound;
class NoteOnCallback;
class PlayerHeap;
class SoundArchiveFileReader;
class SoundThread;
class WaveSound;
class StrmPlayer;
class WsdPlayer;

} // namespace detail
} // namespace snd
} // namespace nw4r
#endif

#endif
