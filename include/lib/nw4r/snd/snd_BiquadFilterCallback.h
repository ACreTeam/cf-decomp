#ifndef NW4R_SND_BIQUAD_FILTER_CALLBACK_H
#define NW4R_SND_BIQUAD_FILTER_CALLBACK_H

#include <types.h>

namespace nw4r {
namespace snd {
class BiquadFilterCallback {
  public:
    struct BiquadCoef {
        u16 b0; // size 0x02, offset 0x00
        u16 b1; // size 0x02, offset 0x02
        u16 b2; // size 0x02, offset 0x04
        u16 a1; // size 0x02, offset 0x06
        u16 a2; // size 0x02, offset 0x08
    }; // size 0x0a

  public:
    virtual ~BiquadFilterCallback() {
    }

    virtual void GetCoef(int type, f32 value, BiquadCoef *coef) const = 0;

  public:
    /* vtable */ // size 0x04, offset 0x00
}; // size 0x04
} // namespace snd
} // namespace nw4r

#endif
