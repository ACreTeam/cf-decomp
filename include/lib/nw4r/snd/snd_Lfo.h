#ifndef NW4R_SND_LFO_H
#define NW4R_SND_LFO_H

#include <types.h>

namespace nw4r {
namespace snd {
namespace detail {
struct LfoParam {
  public:
    LfoParam() {
        Init();
    }

    void Init();

  public:
    f32 depth; // size 0x04, offset 0x00
    f32 speed; // size 0x04, offset 0x04
    u32 delay; // size 0x04, offset 0x08
    u8 range;  // size 0x01, offset 0x0c
    u8 padding[3];
}; // size 0x10

class Lfo {
  public:
    Lfo() : mDelayCounter(0), mCounter(0.0f) {
    }

    void Update(int msec);
    void Reset();

    LfoParam &GetParam() {
        return mParam;
    }

    void SetParam(LfoParam const &param) {
        mParam = param;
    }

    f32 GetValue() const;

  private:
    static s8 GetSinIdx(int i);

  private:
    static int const PERIOD;
    static int const TABLE_SIZE = 32;

  private:
    LfoParam mParam;   // size 0x10, offset 0x00
    u32 mDelayCounter; // size 0x04, offset 0x10
    f32 mCounter;      // size 0x04, offset 0x14
}; // size 0x18
} // namespace detail
} // namespace snd
} // namespace nw4r

#endif
