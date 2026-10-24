#ifndef NW4R_SND_GLOBAL_H
#define NW4R_SND_GLOBAL_H

#include <types.h>

#if defined(__cplusplus)
#define NW4R_RANGE_FOR(it_, list_)                                             \
    for (__decltype__((list_).GetBeginIter()) (it_) = (list_).GetBeginIter();  \
         (it_) != (list_).GetEndIter(); ++(it_))

#define NW4R_RANGE_FOR_NO_AUTO_INC(it_, list_)                                 \
    for (__decltype__((list_).GetBeginIter()) (it_) = (list_).GetBeginIter();  \
         (it_) != (list_).GetEndIter();)
#endif

namespace nw4r {
namespace snd {

static const int CHANNEL_MIN = 1;
static const int CHANNEL_MAX = 2;

static const int REMOTE_FILTER_MAX = 127;

static const f32 VOLUME_MIN_DB = -90.4f;
static const f32 VOLUME_MAX_DB = 6.0f;
static const f32 VOLUME_RANGE_DB = -(VOLUME_MIN_DB - VOLUME_MAX_DB);
static const int VOLUME_RANGE_MB = static_cast<int>(10 * VOLUME_RANGE_DB);

enum OutputLineFlag {
    OUTPUT_LINE_MAIN = (1 << 0),
    OUTPUT_LINE_REMOTE_N = (1 << 1),

    OUTPUT_LINE_REMOTE_1 = (1 << 1),
    OUTPUT_LINE_REMOTE_2 = (1 << 2),
    OUTPUT_LINE_REMOTE_3 = (1 << 3),
    OUTPUT_LINE_REMOTE_4 = (1 << 4),
};

enum AuxBus { AUX_A, AUX_B, AUX_C, AUX_BUS_NUM };

enum OutputMode {
    OUTPUT_MODE_STEREO,
    OUTPUT_MODE_SURROUND,
    OUTPUT_MODE_DPL2,
    OUTPUT_MODE_MONO
};

enum SampleFormat {
    SAMPLE_FORMAT_PCM_S32,
    SAMPLE_FORMAT_PCM_S16,
    SAMPLE_FORMAT_PCM_S8,
    SAMPLE_FORMAT_DSP_ADPCM,
};

struct VoiceOutParam {
    VoiceOutParam()
        : volume(1.0f),
          pitch(1.0f),
          pan(0.0f),
          surroundPan(0.0f),
          fxSend(0.0f),
          lpf(0.0f) {}

    f32 volume;      // at 0x0
    f32 pitch;       // at 0x4
    f32 pan;         // at 0x8
    f32 surroundPan; // at 0xC
    f32 fxSend;      // at 0x10
    f32 lpf;         // at 0x14
}; // size 0x18

namespace detail {

enum PanMode {
    PAN_MODE_DUAL,
    PAN_MODE_BALANCE,
};

enum PanCurve {
    PAN_CURVE_SQRT,
    PAN_CURVE_SQRT_0DB,
    PAN_CURVE_SQRT_0DB_CLAMP,

    PAN_CURVE_SINCOS,
    PAN_CURVE_SINCOS_0DB,
    PAN_CURVE_SINCOS_0DB_CLAMP,

    PAN_CURVE_LINEAR,
    PAN_CURVE_LINEAR_0DB,
    PAN_CURVE_LINEAR_0DB_CLAMP,
};

} // namespace detail
} // namespace snd
} // namespace nw4r

#endif
