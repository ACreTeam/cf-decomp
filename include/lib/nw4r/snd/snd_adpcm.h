#ifndef NW4R_SND_ADPCM_H
#define NW4R_SND_ADPCM_H
#include <nw4r/types_nw4r.h>

#include <revolution/ax.h>

namespace nw4r {
namespace snd {
namespace detail {

struct AdpcmParam {
    u16 coef[16];   // at 0x0
    u16 gain;       // at 0x20
    u16 pred_scale; // at 0x22
    u16 yn1;        // at 0x24
    u16 yn2;        // at 0x26
}; // size 0x28

struct AdpcmLoopParam {
    u16 loop_pred_scale; // at 0x0
    u16 loop_yn1;        // at 0x2
    u16 loop_yn2;        // at 0x4
}; // size 0x6

struct AdpcmInfo {
    AdpcmParam param;         // at 0x0
    AdpcmLoopParam loopParam; // at 0x28
    u16 PADDING_0x2E;         // at 0x2E
}; // size 0x30

s16 DecodeDspAdpcm(AXPBADPCM* pAdpcm, u8 bits);

} // namespace detail
} // namespace snd
} // namespace nw4r

#endif
